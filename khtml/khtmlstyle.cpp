/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    Based on the Chimera CSS implementation of
                       John Kilburg <john@cs.unlv.edu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Cascading Style Sheets / 1
// $Id$

#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "khtmlstyle.h"
#include "khtmltokenizer.h"

#include "khtmlattrs.h"
#include "khtmltags.h"

#include <qstring.h>

//
// Classes for internal use only
//
/////////////////////////////////

//
// The following file includes all CSS_PROP_* defines
// CSS properties range from CSS_PROP_MIN to (and including) CSS_PROP_MAX
// Combined property-keywords are ranging from CSS_PROP_MAX+1 to
// (and including) CSS_PROP_TOTAL-1
#include "kcssprop.h"

//
// The following file defines the function
//     const struct props *findProp(const char *word, int len)
//
// with 'props->id' a CSS property in the range from CSS_PROP_MIN to
// (and including) CSS_PROP_TOTAL-1
#include "kcssprop.c"

class CSSProperty
{
public:
    int   propId;
#if 0
    enum contentEnum { contEnum, contAbsValue, contRelValue, contPercent,
    contEm, contEx, contPx };
    int   contentId;
    // contentId determines how contentValue should be interpreted
    int   contentValue;
    const QChar *contentString;
#else
    QString value;
#endif
    int   weight;
};

class CSSSelector
{
public:
    CSSSelector(void);
    ~CSSSelector(void);
    void print(void);
    // tag == -1 means apply to all elements (Selector = *)
    int          tag;

    enum Match
    {
	Exact = 0,
	Set,
	List,
	Hyphen
    };

    Match 	 match;
    int          attr;
    QString      value;

    enum Relation
    {
	Descendant = 0,
	Child,
	Sibling
    };

    Relation relation;
    CSSSelector *tagHistory;
    CSSPropList *propList;
};

CSSSelector::CSSSelector(void)
: tag(0), tagHistory(0), propList(0)
{
    attr = 0;
    match = Exact;
    relation = Descendant;
}

CSSSelector::~CSSSelector(void)
{
    if (tagHistory)
    {
        delete tagHistory;
    }
}

void CSSSelector::print(void)
{
    printf("[Selector: tag = %d, attr = \"%d\", value = \"%s\" relation = %d\n",
    	tag, attr, value.data(), (int)relation);
}

CSSStyleSheet::CSSStyleSheet(const HTMLSettings *_settings)
{
    settings = _settings;
}

CSSStyleSheet::~CSSStyleSheet()
{
}

CSSStyle *
CSSStyleSheet::newStyle(CSSStyle *parentStyle)
{
    CSSStyle *newStyle = new CSSStyle();

    if (parentStyle)
    {
        newStyle->font = parentStyle->font;
        newStyle->text = parentStyle->text;
        newStyle->list = parentStyle->list;
    }
    else
    {
        newStyle->font.decoration = CSSStyleFont::decNormal;
        newStyle->font.color = settings->fontBaseColor;
        newStyle->font.family = settings->fontBaseFace;
        newStyle->font.fixed = false;
        newStyle->font.style = CSSStyleFont::stNormal;
        newStyle->font.weight = CSSStyleFont::Normal;
        newStyle->font.size = settings->fontBaseSize;
        newStyle->font.fp = 0;

        newStyle->text.transform = CSSStyleText::transNormal;
        newStyle->text.halign = CSSStyleText::halLeft;
        newStyle->text.indent = 0;
        newStyle->text.height = CSSStyleText::heightNormal;
        newStyle->text.valign = CSSStyleText::valBaseline;
        newStyle->text.valignOffset = 0;

        newStyle->list.type = CSSStyleList::liDisc;	
    }

    newStyle->box.marginF = 0;
    newStyle->box.marginP = UNDEFINED;
    newStyle->box.paddingF = 0;
    newStyle->box.paddingP = UNDEFINED;
    newStyle->box.borderColor = newStyle->font.color;
    newStyle->box.border = 2;
    newStyle->box.borderStyle = CSSStyleBox::borderNone;
    newStyle->bgtransparent = true;
    newStyle->width = UNDEFINED;
    newStyle->width_percent = UNDEFINED;
    newStyle->height = UNDEFINED;
    newStyle->height_percent = UNDEFINED;
    newStyle->bgcolor = Qt::lightGray;

    return newStyle;
}

void
CSSStyleSheet::getStyle(int /*tagID*/, HTMLStackElem */*tagStack*/,
                        CSSStyle */*styleElem*/,
                        const QChar */*klass*/, const QChar */*id*/)
{
}

void
CSSStyleSheet::addStyle(CSSStyle */*currentStyle*/, const QChar */*CSSString*/)
{
}

const QChar *
CSSStyleSheet::parseSpace(const QChar *curP, const QChar *endP)
{
  bool sc = false;     // possible start comment?
  bool ec = false;     // possible end comment?
  bool ic = false;     // in comment?

  while (curP < endP)
  {
      if (ic)
      {
          if (ec && (*curP == '/'))
              ic = false;
          else if (*curP == '*')
              ec = true;
          else
              ec = false;
      }
      else if (sc && (*curP == '*'))
      {
          ic = true;
      }
      else if (*curP == '/')
      {
          sc = true;
      }
      else if (!isspace(*curP))
      {
          return(curP);
      }
      else
      {
          sc = false;
      }
      curP++;
  }

  return(0);
}

/*
 * ParseToChar
 *
 * Search for an expected character.  Deals with escaped characters,
 * quoted strings, and pairs of braces/parens/brackets.
 */
const QChar *
CSSStyleSheet::parseToChar(const QChar *curP, const QChar *endP, QChar c, bool chkws)
{
    bool sq = false; /* in single quote? */
    bool dq = false; /* in double quote? */
    bool esc = false; /* escape mode? */

    while (curP < endP)
    {
        if (esc)
            esc = false;
        else if (*curP == '\\')
            esc = true;
        else if (dq && (*curP != '"'))
            dq = false;
        else if (sq && (*curP != '\''))
            sq = false;
        else if (*curP == '"')
            dq = true;
        else if (*curP == '\'')
            sq = true;
        else if (*curP == c)
            return(curP);
        else if (chkws && isspace(*curP))
            return(curP);
        else if (*curP == '{')
        {
            curP = parseToChar(curP + 1, endP, '}', false);
            if (!curP)
                return(0);
        }
        else if (*curP == '(')
        {
            curP = parseToChar(curP + 1, endP, ')', false);
            if (!curP)
                return(0);
        }
        else if (*curP == '[')
        {
            curP = parseToChar(curP + 1, endP, ']', false);
            if (!curP)
                return(0);
        }
        curP++;
    }

    return(0);
}

const QChar *
CSSStyleSheet::parseAt(const QChar *curP, const QChar *endP)
{
//    const char *startP = curP;
    while (curP < endP)
    {
        if (*curP == '{')
        {
            curP = parseToChar(curP + 1, endP, '}', false);
            return curP;
        }
        else if (*curP == ';')
        {
            // check if [startP, curP] contains an include...
            // if so, emit a signal
            curP++;
            return curP;
        }
        curP++;
    }

    return(0);
}

CSSSelector *
CSSStyleSheet::parseSelector2(const QChar *curP, const QChar *endP)
{
    CSSSelector *cs = new CSSSelector();
    QString selecString( curP, endP - curP );

printf("selectString = \"%s\"\n", selecString.ascii());

    if (*curP == '#')
    {
	cs->tag = -1;
	cs->attr = ATTR_ID;
	cs->value = QString( curP + 1, endP - curP -1 );
    }
    else if (*curP == '.')
    {
	cs->tag = -1;
	cs->attr = ATTR_CLASS;
	cs->value = QString( curP + 1, endP - curP -1 );
    }
    else if (*curP == ':')
    {
	cs->tag = -1;
	cs->attr = ATTR_CLASS;
	cs->value = QString( curP, endP - curP );
    }
    else
    {
        const QChar *startP = curP;
	QString tag;
        while (curP < endP)
        {
            if (*curP =='#')
            {
                tag = QString( startP, curP-startP );
                QString tmp( curP + 1, endP - curP - 1);
                cs->attr = ATTR_ID;
		cs->value = tmp;
                break;
            }
            else if (*curP == '.')
            {
                tag = QString( startP, curP - startP );
                QString tmp( curP + 1, endP - curP - 1);
		cs->attr = ATTR_CLASS;
		cs->match = CSSSelector::List;
                cs->value = tmp;
                break;
            }
            else if (*curP == ':')
            {
                tag = QString( startP, curP - startP );
                QString tmp( curP, endP - curP );
                cs->value = tmp;
                break;
            }
            else if (*curP == '[')
            {
		// ### FIXME
                tag = QString( startP, curP - startP );
                QString tmp( curP, endP - curP );
                cs->value = tmp;
                break;
            }
            else
            {
                curP++;
            }
        }
        if (curP == endP)
        {
            tag = QString( startP, curP - startP );
        }
	if(tag == "*")
	    cs->tag = -1;
	else
	    cs->tag = getTagID(tag.lower().data(), tag.length());
	printf("found tag \"%s\"\n", tag.ascii());
   }
   if (cs->tag == 0)
   {
       delete cs;
       return(0);
   }
   return(cs);
}

CSSSelector *
CSSStyleSheet::parseSelector1(const QChar *curP, const QChar *endP)
{
    CSSSelector *selecStack=0;

    curP = parseSpace(curP, endP);
    if (!curP)
        return(0);

    const QChar *startP = curP;
    while (curP <= endP)
    {
        if ((curP == endP) || isspace(*curP) || *curP == '+' || *curP == '>')
        {
            CSSSelector *cs = parseSelector2(startP, curP);
            if (cs)
            {
                cs->tagHistory = selecStack;
                selecStack = cs;
            }
	    else
	    {
		// invalid selector, delete
		delete selecStack;
		return 0;
	    }
		    
            curP = parseSpace(curP, endP);
            if (!curP)
                return(selecStack);

	    if(*curP == '+')
	    {
		cs->relation = CSSSelector::Sibling;
		curP++;
		curP = parseSpace(curP, endP);
	    }
	    else if(*curP == '>')
	    {
		cs->relation = CSSSelector::Child;
		curP++;
		curP = parseSpace(curP, endP);
	    }
	    if(cs)
		cs->print();
            startP = curP;
        }
        else
        {
            curP++;
        }
    }
    return(selecStack);
}

CSSSelecList *
CSSStyleSheet::parseSelector(const QChar *curP, const QChar *endP)
{
    CSSSelecList *slist  = 0;
    const QChar *startP;

    while (curP < endP)
    {
        startP = curP;
        curP = parseToChar(curP, endP, ',', false);
        if (!curP)
            curP = endP;

        CSSSelector *selector = parseSelector1(startP, curP);
        if (selector)
        {
            if (!slist)
            {
                slist = new CSSSelecList();
            }
            slist->append(selector);
        }
	else
	{
	    // invalid selector, delete
	    delete slist;
	    return 0;
	}
        curP++;
    }
    return(slist);
}

CSSProperty *
CSSStyleSheet::parseProperty(const QChar *curP, const QChar *endP)
{
    const QChar *colon;
    // Get rid of space in front of the declaration

    curP = parseSpace(curP, endP);
    if (!curP)
        return(0);

    // Search for the required colon or white space
    colon = parseToChar(curP, endP, ':', true);
    if (!colon)
        return(0);

    QString propName( curP, colon - curP );

printf("Property-name = \"%s\"\n", propName.data());
    // May have only reached white space before
    if (*colon != ':')
    {
        // Search for the required colon
        colon = parseToChar(curP, endP, ':', false);
        if (!colon)
            return(0);
    }
    // remove space in front of the value
    curP = parseSpace(colon+1, endP);
    if (!curP)
        return(0);

    // remove space after the value;
    while (endP > curP)
    {

        if (!isspace(*(endP-1)))
            break;
        endP--;
    }

    QString propVal( curP , endP - curP );
printf("Property-value = \"%s\"\n", propVal.data());

    const struct props *propPtr = findProp(propName.lower().ascii(), propName.length());
    if (!propPtr)
    {
         printf("Unknown property\n");
         return (0);
    }
    CSSProperty *prop = new CSSProperty();
    prop->propId = propPtr->id;
    prop->value = propVal.data();

    return(prop);
}

CSSPropList *
CSSStyleSheet::parseProperties(const QChar *curP, const QChar *endP)
{
    CSSPropList *propList=0;

    while (curP < endP)
    {
        const QChar *startP = curP;
        curP = parseToChar(curP, endP, ';', false);
        if (!curP)
            curP = endP;

        CSSProperty *prop = parseProperty(startP, curP);
        if (prop)
        {
            if (!propList)
            {
                propList = new CSSPropList();
            }
            propList->append(prop);
        }
        curP++;
    }
    return(propList);
}

const QChar *
CSSStyleSheet::parseRule(const QChar *curP, const QChar *endP)
{
    const QChar *startP;
    CSSSelecList *slist;
    CSSPropList *plist;

    startP = curP;
    curP = parseToChar(startP, endP, '{', false);
    if (!curP)
        return(0);

    slist = parseSelector(startP, curP );

    curP++; // need to get past the '{' from above

    startP = curP;
    curP = parseToChar(startP, endP, '}', false);
    if (!curP)
    {
        delete slist;
        return(0);
    }

    plist = parseProperties(startP, curP );

    curP++; // need to get past the '}' from above

    if (!plist || !slist)
    {
        // Useless rule
        delete slist;
        delete plist;
        return(curP);
    }

    
    // Add rule to our data structures...
    // WABA: To be done
    return(curP);
}

void
CSSStyleSheet::parseSheet(const QChar *src, int len)
{
    const QChar *curP = src;
    const QChar *endP = src+len;

    curP = parseSpace(curP, endP);
    while (curP && (curP < endP))
    {
        if (*curP == '@')
        {
            curP = parseAt(curP, endP);
        }
        else
        {
            curP = parseRule(curP, endP);
        }

        if (curP)
            curP = parseSpace(curP, endP);
    }
}

void
CSSStyleSheet::test(void)
{
    char buf[40000];

    int fd = open("/home/kde/test.css", O_RDONLY);

    if (fd < 0)
    {
        perror("Couldn't open /home/kde/test.css:");
        return;
    }

    int len = read(fd, buf, 40000);

    QString str = buf;

    close(fd);

    parseSheet(str.unicode(), len);
}

// ------------------------------------------------------------------------
const HTMLFont *getFont(CSSStyle *currentStyle)
{

    int fontsize = currentStyle->font.size;
    if ( fontsize < 0 )
	fontsize = 0;
    else if ( fontsize >= MAXFONTSIZES )
	fontsize = MAXFONTSIZES - 1;

    currentStyle->font.size = fontsize;

    HTMLFont f( currentStyle->font.family,
                fontsize,
                currentStyle->font.fixed ? pSettings->fixedFontSizes : pSettings->fontSizes,
                currentStyle->font.weight / 10,
                (currentStyle->font.style != CSSStyleFont::stNormal),
                pSettings->charset );
    f.setTextColor( currentStyle->font.color );
    f.setUnderline( currentStyle->font.decoration == CSSStyleFont::decUnderline );
    f.setStrikeOut( currentStyle->font.decoration == CSSStyleFont::decLineThrough );
    if (currentStyle->text.valign == CSSStyleText::valOffset)
    {
       f.setVOffset( currentStyle->text.valignOffset );
    }

    const HTMLFont *fp = pFontManager->getFont( f );

    currentStyle->font.fp = fp;

    return fp;
}


void setNamedColor(QColor &color, const QString name)
{
    bool ok;
    // also recognize "color=ffffff"
    if (name[0] != QChar('#') && name.length() == 6 &&
        name.toInt(&ok, 16) )
    {
        QString col("#");
        col += name;
        color.setNamedColor(col);
    }
    else
    {
        color.setNamedColor(name);
    }
}
