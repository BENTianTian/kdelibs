/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *               1999 Waldo Bastian (bastian@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#include "css_stylesheetimpl.h"

#include "css_stylesheet.h"
#include "css_rule.h"
#include "css_ruleimpl.h"
#include "css_valueimpl.h"
#include "csshelper.h"

#include "dom_string.h"
#include "xml/dom_nodeimpl.h"
#include "html/html_documentimpl.h"
#include "dom_exception.h"
using namespace DOM;

#include <stdio.h>

#include "htmlhashes.h"
#include "misc/helper.h"

#include "cssproperties.h"
#include "cssvalues.h"


//
// The following file defines the function
//     const struct props *findProp(const char *word, int len)
//
// with 'props->id' a CSS property in the range from CSS_PROP_MIN to
// (and including) CSS_PROP_TOTAL-1
#include "cssproperties.c"
#include "cssvalues.c"

int DOM::getPropertyID(const char *tagStr, int len)
{
    const struct props *propsPtr = findProp(tagStr, len);
    if (!propsPtr)
        return 0;

    return propsPtr->id;
}

// ------------------------------------------------------------------------------------------------------

bool StyleBaseImpl::deleteMe()
{
    if(!m_parent && _ref <= 0) return true;
    return false;
}

void StyleBaseImpl::setParent(StyleBaseImpl *parent)
{
    m_parent = parent;
}

void StyleBaseImpl::checkLoaded()
{
    if(m_parent) m_parent->checkLoaded();
}

DOMString StyleBaseImpl::baseUrl()
{
    // try to find the style sheet. If found look for it's url.
    // If it has none, look for the parentsheet, or the parentNode and
    // try to find out about their url
    StyleBaseImpl *b = this;
    while(b && !b->isStyleSheet())
	b = b->parent();

    if(!b) return 0;

    StyleSheetImpl *sheet = static_cast<StyleSheetImpl *>(b);
    if(!sheet->href().isNull())
	return sheet->href();

    // find parent
    if(sheet->parent()) return sheet->parent()->baseUrl();

    if(!sheet->ownerNode()) return 0;

    DocumentImpl *doc = sheet->ownerNode()->ownerDocument();
    if(!doc->isHTMLDocument()) return 0;

    HTMLDocumentImpl *htmldoc = static_cast<HTMLDocumentImpl *>(doc);
    return htmldoc->URL();
}

/*
 * parsing functions for stylesheets
 */

bool isspace(const QChar &c)
{
     return (c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r');
}

const QChar *
StyleBaseImpl::parseSpace(const QChar *curP, const QChar *endP)
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
StyleBaseImpl::parseToChar(const QChar *curP, const QChar *endP, QChar c, bool chkws)
{
    //printf("parsetochar: \"%s\" searching %c ws=%d\n", QString(curP, endP-curP).latin1(), c.latin1(), chkws);

    bool sq = false; /* in single quote? */
    bool dq = false; /* in double quote? */
    bool esc = false; /* escape mode? */

    while (curP < endP)
    {
        if (esc)
            esc = false;
        else if (*curP == '\\')
            esc = true;
        else if (dq && (*curP == '"'))
            dq = false;
        else if (sq && (*curP == '\''))
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

CSSRuleImpl *
StyleBaseImpl::parseAtRule(const QChar *&curP, const QChar *endP)
{
    curP++;
    const QChar *startP = curP;
    while( *curP != ' ' && *curP != '{' && *curP != '\'')
	curP++;

    QString rule(startP, curP-startP);
    rule = rule.lower();

    //printf("rule = '%s'\n", rule.ascii());

    if(rule == "import")
    {
	// ### load stylesheet and pass it over
	curP = parseSpace(curP, endP);
	if(!curP) return 0;
	startP = curP++;
	curP = parseToChar(startP, endP, ';', true);
	DOMString url = khtml::parseURL(DOMString(startP, curP - startP));
	startP = curP;
	curP = parseToChar(startP, endP, ';', false);
	QString media(startP, curP - startP);
	// ### check if at the beginning of the stylesheet (no style rule
	//     before the import rule)
	//printf("url = %s\n", url.string().ascii());
	//printf("media = %s\n", media.ascii());
	// ### add medialist
	if(!this->isCSSStyleSheet()) return 0;
	return new CSSImportRuleImpl(this, url, 0);
    }
    else if(rule == "charset")
    {
	// ### invoke decoder
	startP = curP++;
	curP = parseToChar(startP, endP, ';', false);
	//printf("charset = %s\n", QString(startP, curP - startP).ascii());
    }
    else if(rule == "font-face")
    {
	startP = curP++;
	curP = parseToChar(startP, endP, '}', false);
	//printf("font rule = %s\n", QString(startP, curP - startP).ascii());	
    }
    else if(rule == "media")
    {
	startP = curP++;
	curP = parseToChar(startP, endP, '}', false);
	//printf("media rule = %s\n", QString(startP, curP - startP).ascii());	
    }
    else if(rule == "page")
    {
	startP = curP++;
	curP = parseToChar(startP, endP, '}', false);
	//printf("page rule = %s\n", QString(startP, curP - startP).ascii());	
    }

	
    return 0;
}

CSSSelector *
StyleBaseImpl::parseSelector2(const QChar *curP, const QChar *endP)
{
    CSSSelector *cs = new CSSSelector();
    QString selecString( curP, endP - curP );

//printf("selectString = \"%s\"\n", selecString.ascii());

    if (*curP == '#')
    {
	cs->tag = -1;
	cs->attr = ATTR_ID;
	cs->match = CSSSelector::Exact;
	cs->value = DOMString( curP + 1, endP - curP -1 );
    }
    else if (*curP == '.')
    {
	cs->tag = -1;
	cs->attr = ATTR_CLASS;
	cs->match = CSSSelector::List;
	cs->value = DOMString( curP + 1, endP - curP -1 );
    }
    else if (*curP == ':')
    {
	cs->tag = -1;
	cs->value = DOMString( curP, endP - curP );
	cs->match = CSSSelector::Pseudo;
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
		cs->attr = ATTR_ID;
		cs->match = CSSSelector::Exact;
		cs->value = DOMString( curP + 1, endP - curP - 1);
                break;
            }
            else if (*curP == '.')
            {
                tag = QString( startP, curP - startP );
		cs->attr = ATTR_CLASS;
		cs->match = CSSSelector::List;
		cs->value = DOMString( curP + 1, endP - curP - 1);
                break;
            }
            else if (*curP == ':')
            {
		// pseudo attributes (:link, :hover, ...)
                tag = QString( startP, curP - startP );
		cs->value = DOMString( curP, endP - curP);
		cs->match = CSSSelector::Pseudo;
                break;
            }
            else if (*curP == '[')
            {
                tag = QString( startP, curP - startP );
		curP++;
		//printf("tag = %s\n", tag.ascii());
		const QChar *equal = parseToChar(curP, endP, '=', false);
		QString attr;
		if(!equal)
		{
		    attr = QString( curP, endP - curP - 1 );
		    //printf("attr = '%s'\n", attr.ascii());
		    cs->match = CSSSelector::Set;
		}
		else
		{
		    // check relation: = / ~= / |=
		    if(*(equal-1) == '~')
		    {
			attr = QString( curP, equal - curP - 1 );
			cs->match = CSSSelector::List;
		    }
		    else if(*(equal-1) == '|')
		    {
			attr = QString( curP, equal - curP - 1 );
			cs->match = CSSSelector::Hyphen;
		    }
		    else
		    {
			attr = QString(curP, equal - curP );
			cs->match = CSSSelector::Exact;
		    }
		}
		cs->attr = khtml::getAttrID(attr.ascii(), attr.length());
		if(equal)
		{
		    equal++;
		    if(*equal == '\'')
		    {
			equal++;
			while(*endP != '\'' && endP > equal)
			    endP--;
		    }
		    else if(*equal == '\"')
		    {
			equal++;
			while(*endP != '\"' && endP > equal)
			    endP--;
		    }
		    else
			endP--;
		    cs->value = DOMString(equal, endP - equal);
		}
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
	{
	    //printf("found '*' selector\n");
	    cs->tag = -1;
	}
	else
	    cs->tag = khtml::getTagID(tag.lower().data(), tag.length());
   }
   if (cs->tag == 0)
   {
       delete cs;
       return(0);
   }
   //printf("[Selector: tag=%d Attribute=%d relation=%d value=%s]\n", cs->tag, cs->attr, cs->match, cs->value.string().ascii());
   return(cs);
}

CSSSelector *
StyleBaseImpl::parseSelector1(const QChar *curP, const QChar *endP)
{
    //printf("selector1 is \'%s\'\n", QString(curP, endP-curP).latin1());

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

QList<CSSSelector> *
StyleBaseImpl::parseSelector(const QChar *curP, const QChar *endP)
{
    //printf("selector is \'%s\'\n", QString(curP, endP-curP).latin1());

    QList<CSSSelector> *slist  = 0;
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
                slist = new QList<CSSSelector>;
		slist->setAutoDelete(true);
            }
            slist->append(selector);
        }
	else
	{
	    printf("invalid selector\n");
	    // invalid selector, delete
	    delete slist;
	    return 0;
	}
        curP++;
    }
    return slist;
}


void StyleBaseImpl::parseProperty(const QChar *curP, const QChar *endP, QList<CSSProperty> *propList)
{
    bool important = false;
    const QChar *colon;
    // Get rid of space in front of the declaration

    curP = parseSpace(curP, endP);
    if (!curP)
        return;

    // Search for the required colon or white space
    colon = parseToChar(curP, endP, ':', true);
    if (!colon)
        return;

    QString propName( curP, colon - curP );
    //printf("Property-name = \"%s\"\n", propName.data());

    // May have only reached white space before
    if (*colon != ':')
    {
        // Search for the required colon
        colon = parseToChar(curP, endP, ':', false);
        if (!colon)
            return;
    }
    // remove space in front of the value
    curP = parseSpace(colon+1, endP);
    if (!curP)
        return;

    // search for !important
    const QChar *exclam = parseToChar(curP, endP, '!', false);
    if(exclam)
    {
	const QChar *imp = parseSpace(exclam+1, endP);
	QString s(imp, endP - imp);
	s.lower();
	if(!s.contains("important"))
	    return;
	important = true;
	endP = exclam - 1;
	//printf("important property!\n");
    }

    // remove space after the value;
    while (endP > curP)
    {
        if (!isspace(*(endP-1)))
            break;
        endP--;
    }


    //    QString propVal( curP , endP - curP );
    //printf("Property-value = \"%s\"\n", propVal.data());

    const struct props *propPtr = findProp(propName.lower().ascii(), propName.length());
    if (!propPtr)
    {
         printf("Unknown property\n");
         return;
    }

    parseValue(curP, endP, propPtr->id, important, propList);

}

QList<CSSProperty> *StyleBaseImpl::parseProperties(const QChar *curP, const QChar *endP)
{
    QList<CSSProperty> *propList=0;
    propList = new QList<CSSProperty>;
    propList->setAutoDelete(true);

    while (curP < endP)
    {
        const QChar *startP = curP;
        curP = parseToChar(curP, endP, ';', false);
        if (!curP)
            curP = endP;

        parseProperty(startP, curP, propList);
        curP++;
    }
    if(!propList->isEmpty())
	return propList;

    printf("empty property list\n");
    delete propList;
    return 0;
}

bool StyleBaseImpl::parseValue(const QChar *curP, const QChar *endP, int propId, bool important,
					QList<CSSProperty> *propList)
{
    //printf("parseValue!\n");
    QString value(curP, endP - curP);
    value = value.lower();
    const char *val = value.ascii();

    CSSValueImpl *parsedValue = 0;

    if(!strcmp(val, "inherit"))
    {
	// inherited value
	parsedValue = new CSSInheritedValueImpl();
	goto end;
    }

    switch(propId)
    {
    case CSS_PROP_AZIMUTH:
    case CSS_PROP_BACKGROUND_ATTACHMENT:
    case CSS_PROP_BACKGROUND_COLOR:
    case CSS_PROP_BACKGROUND_POSITION:
    case CSS_PROP_BACKGROUND_REPEAT:
    case CSS_PROP_BORDER_TOP_STYLE:
    case CSS_PROP_BORDER_RIGHT_STYLE:
    case CSS_PROP_BORDER_BOTTOM_STYLE:
    case CSS_PROP_BORDER_LEFT_STYLE:
    case CSS_PROP_BORDER_COLLAPSE:
    case CSS_PROP_CAPTION_SIDE:
    case CSS_PROP_CLEAR:
    case CSS_PROP_CLIP:
    case CSS_PROP_CONTENT:
    case CSS_PROP_COUNTER_INCREMENT:
    case CSS_PROP_COUNTER_RESET:
    case CSS_PROP_CUE_AFTER:
    case CSS_PROP_CUE_BEFORE:
    case CSS_PROP_CURSOR:
    case CSS_PROP_DIRECTION:
    case CSS_PROP_DISPLAY:
    case CSS_PROP_ELEVATION:
    case CSS_PROP_EMPTY_CELLS:
    case CSS_PROP_FLOAT:
    case CSS_PROP_FONT_SIZE:
    case CSS_PROP_FONT_SIZE_ADJUST:
    case CSS_PROP_FONT_STRETCH:
    case CSS_PROP_FONT_STYLE:
    case CSS_PROP_FONT_VARIANT:
    case CSS_PROP_FONT_WEIGHT:
    case CSS_PROP_LETTER_SPACING:
    case CSS_PROP_LINE_HEIGHT:
    case CSS_PROP_LIST_STYLE_IMAGE:
    case CSS_PROP_LIST_STYLE_POSITION:
    case CSS_PROP_LIST_STYLE_TYPE:
    case CSS_PROP_MARGIN_TOP:
    case CSS_PROP_MARGIN_RIGHT:
    case CSS_PROP_MARGIN_BOTTOM:
    case CSS_PROP_MARGIN_LEFT:
    case CSS_PROP_MAX_HEIGHT:
    case CSS_PROP_MAX_WIDTH:
    case CSS_PROP_OUTLINE_COLOR:
    case CSS_PROP_OUTLINE_STYLE:
    case CSS_PROP_OUTLINE_WIDTH:
    case CSS_PROP_OVERFLOW:
    case CSS_PROP_PAGE:
    case CSS_PROP_PAGE_BREAK_AFTER:
    case CSS_PROP_PAGE_BREAK_BEFORE:
    case CSS_PROP_PAGE_BREAK_INSIDE:
    case CSS_PROP_PITCH:
    case CSS_PROP_PLAY_DURING:
    case CSS_PROP_POSITION:
    case CSS_PROP_QUOTES:
    case CSS_PROP_SIZE:
    case CSS_PROP_SPEAK:
    case CSS_PROP_SPEAK_HEADER:
    case CSS_PROP_SPEAK_NUMERAL:
    case CSS_PROP_SPEAK_PUNCTUATION:
    case CSS_PROP_SPEECH_RATE:
    case CSS_PROP_TABLE_LAYOUT:
    case CSS_PROP_TEXT_ALIGN:
	//case CSS_PROP_TEXT_DECORATION:
    case CSS_PROP_TEXT_TRANSFORM:
    case CSS_PROP_UNICODE_BIDI:
    case CSS_PROP_VERTICAL_ALIGN:
    case CSS_PROP_VISIBILITY:
    case CSS_PROP_VOICE_FAMILY:
    case CSS_PROP_VOLUME:
    case CSS_PROP_WHITE_SPACE:
    case CSS_PROP_WORD_SPACING:
    case CSS_PROP_Z_INDEX:
    {
	//printf("parseValue: value = %s\n", val);
	const struct css_value *cssval = findValue(val, value.length());
	if (cssval)
	{
	    //printf("got value %d\n", cssval->id);
	    parsedValue = new CSSPrimitiveValueImpl(cssval->id);
	    goto end;
	    // ### FIXME: should check if the identifier makes sense with the property
	}
    }	
    default:
	break;
    }

    // we don't have an identifier.

    switch(propId)
    {
// ident only properties
    case CSS_PROP_BACKGROUND_ATTACHMENT:
    case CSS_PROP_BACKGROUND_REPEAT:
    case CSS_PROP_BORDER_COLLAPSE:
    case CSS_PROP_BORDER_TOP_STYLE:
    case CSS_PROP_BORDER_RIGHT_STYLE:
    case CSS_PROP_BORDER_BOTTOM_STYLE:
    case CSS_PROP_BORDER_LEFT_STYLE:
    case CSS_PROP_CAPTION_SIDE:
    case CSS_PROP_CLEAR:
    case CSS_PROP_DIRECTION:
    case CSS_PROP_DISPLAY:
    case CSS_PROP_EMPTY_CELLS:
    case CSS_PROP_FLOAT:
    case CSS_PROP_FONT_STRETCH:
    case CSS_PROP_FONT_STYLE:
    case CSS_PROP_FONT_VARIANT:
    case CSS_PROP_FONT_WEIGHT:
    case CSS_PROP_LIST_STYLE_POSITION:
    case CSS_PROP_LIST_STYLE_TYPE:
    case CSS_PROP_OUTLINE_STYLE:
    case CSS_PROP_OVERFLOW:
    case CSS_PROP_PAGE:
    case CSS_PROP_PAGE_BREAK_AFTER:
    case CSS_PROP_PAGE_BREAK_BEFORE:
    case CSS_PROP_PAGE_BREAK_INSIDE:
    case CSS_PROP_PAUSE_AFTER:
    case CSS_PROP_PAUSE_BEFORE:
    case CSS_PROP_POSITION:
    case CSS_PROP_SPEAK:
    case CSS_PROP_SPEAK_HEADER:
    case CSS_PROP_SPEAK_NUMERAL:
    case CSS_PROP_SPEAK_PUNCTUATION:
    case CSS_PROP_TABLE_LAYOUT:
    case CSS_PROP_TEXT_TRANSFORM:
    case CSS_PROP_UNICODE_BIDI:
    case CSS_PROP_VISIBILITY:
    case CSS_PROP_WHITE_SPACE:
	break;

// special properties (css_extensions)
    case CSS_PROP_AZIMUTH:
	// CSS2Azimuth
    case CSS_PROP_BACKGROUND_POSITION:
	// CSS2BackgroundPosition
    case CSS_PROP_CURSOR:
	// CSS2Cursor
    case CSS_PROP_PLAY_DURING:
	// CSS2PlayDuring
    case CSS_PROP_TEXT_SHADOW:
	// list of CSS2TextShadow
	break;
    case CSS_PROP_BORDER_SPACING: // should be able to have two values
    	parsedValue = parseUnit(curP, endP, LENGTH);
	break;


// colors || inherit
    case CSS_PROP_BACKGROUND_COLOR:
    case CSS_PROP_BORDER_TOP_COLOR:
    case CSS_PROP_BORDER_RIGHT_COLOR:
    case CSS_PROP_BORDER_BOTTOM_COLOR:
    case CSS_PROP_BORDER_LEFT_COLOR:
    case CSS_PROP_COLOR:
    case CSS_PROP_OUTLINE_COLOR:
    {
	QString value(curP, endP - curP);
	QColor c;
	khtml::setNamedColor(c, value);
	if(!c.isValid()) return false;
	//printf("color is: %d, %d, %d\n", c.red(), c.green(), c.blue());
	parsedValue = new CSSPrimitiveValueImpl(c);
	break;
    }

// uri || inherit
    case CSS_PROP_BACKGROUND_IMAGE:
    case CSS_PROP_LIST_STYLE_IMAGE:
    {
	const struct css_value *cssval = findValue(val, value.length());
	if (cssval && cssval->id == CSS_VAL_NONE)
	{
	    parsedValue = new CSSImageValueImpl();
	    printf("empty image %p\n", static_cast<CSSImageValueImpl *>(parsedValue)->image());
	    break;
	}
	else
	{
	    DOMString value(curP, endP - curP);
	    value = khtml::parseURL(value);
	    printf("mage, url=%s\n", value.string().ascii());
	    parsedValue = new CSSImageValueImpl(value, baseUrl());
	    break;
	}
    }	
    case CSS_PROP_CUE_AFTER:
    case CSS_PROP_CUE_BEFORE:
    {
	DOMString value(curP, endP - curP);
	value = khtml::parseURL(value);
	parsedValue = new CSSPrimitiveValueImpl(value, CSSPrimitiveValue::CSS_URI);
	break;
    }

// length
    case CSS_PROP_BORDER_TOP_WIDTH:
    case CSS_PROP_BORDER_RIGHT_WIDTH:
    case CSS_PROP_BORDER_BOTTOM_WIDTH:
    case CSS_PROP_BORDER_LEFT_WIDTH:
    case CSS_PROP_MARKER_OFFSET:
    case CSS_PROP_LETTER_SPACING:
    case CSS_PROP_OUTLINE_WIDTH:
    case CSS_PROP_WORD_SPACING:
	parsedValue = parseUnit(curP, endP, LENGTH);
	break;

// length, percent
    case CSS_PROP_PADDING_TOP:
    case CSS_PROP_PADDING_RIGHT:
    case CSS_PROP_PADDING_BOTTOM:
    case CSS_PROP_PADDING_LEFT:
    case CSS_PROP_TEXT_INDENT:
    case CSS_PROP_BOTTOM:
    case CSS_PROP_FONT_SIZE:
    case CSS_PROP_HEIGHT:
    case CSS_PROP_LEFT:
    case CSS_PROP_MARGIN_TOP:
    case CSS_PROP_MARGIN_RIGHT:
    case CSS_PROP_MARGIN_BOTTOM:
    case CSS_PROP_MARGIN_LEFT:
    case CSS_PROP_MAX_HEIGHT:
    case CSS_PROP_MAX_WIDTH:
    case CSS_PROP_MIN_HEIGHT:
    case CSS_PROP_MIN_WIDTH:
    case CSS_PROP_RIGHT:
    case CSS_PROP_TOP:
    case CSS_PROP_VERTICAL_ALIGN:
    case CSS_PROP_WIDTH:
	parsedValue = parseUnit(curP, endP, LENGTH | PERCENT );
	break;

// angle
    case CSS_PROP_ELEVATION:
	parsedValue = parseUnit(curP, endP, ANGLE);
	break;
	
// number
    case CSS_PROP_FONT_SIZE_ADJUST:
    case CSS_PROP_ORPHANS:
    case CSS_PROP_PITCH_RANGE:
    case CSS_PROP_RICHNESS:
    case CSS_PROP_SPEECH_RATE:
    case CSS_PROP_STRESS:
    case CSS_PROP_WIDOWS:
    case CSS_PROP_Z_INDEX:
	parsedValue = parseUnit(curP, endP, NUMBER);
	break;

// length, percent, number
    case CSS_PROP_LINE_HEIGHT:
	parsedValue = parseUnit(curP, endP, LENGTH | PERCENT | NUMBER);
	break;

// number, percent
    case CSS_PROP_VOLUME:
	parsedValue = parseUnit(curP, endP, PERCENT | NUMBER);
	break;
	
// frequency
    case CSS_PROP_PITCH:
	parsedValue = parseUnit(curP, endP, FREQUENCY);
	break;

// string
    case CSS_PROP_TEXT_ALIGN:
	// only for table columns.
	break;

// rect
    case CSS_PROP_CLIP:
	// rect, ident
	break;
	
// lists
    case CSS_PROP_CONTENT:
	// list of string, uri, counter, attr, i
    case CSS_PROP_COUNTER_INCREMENT:
	// list of CSS2CounterIncrement
    case CSS_PROP_COUNTER_RESET:
	// list of CSS2CounterReset
    case CSS_PROP_FONT_FAMILY:
	// list of strings and ids
    {
	CSSValueListImpl *list = new CSSValueListImpl;
	QString str(curP, endP-curP);
	//printf("faces: '%s'\n", str.ascii());
	int pos=0, pos2;
	while( 1 )
	{
	    pos2 = str.find(',', pos);
	    QString face = str.mid(pos, pos2-pos);
	    face = face.stripWhiteSpace();
	    if(face[0] == '\"') face.remove(0, 1);
	    if(face[face.length()-1] == '\"') face = face.left(face.length()-1);
	    //printf("found face '%s'\n", face.ascii());
	    list->append(new CSSPrimitiveValueImpl(DOMString(face), CSSPrimitiveValue::CSS_STRING));
	    pos = pos2 + 1;
	    if(pos2 == -1) break;
	}		
	//printf("got %d faces\n", list->length());
	if(list->length())
	    parsedValue = list;
	else
	    delete list;
	break;
    }
    case CSS_PROP_QUOTES:
	// list of strings or i
    case CSS_PROP_SIZE:
	// ### look up
	break;
    case CSS_PROP_TEXT_DECORATION:
	// list of ident
    {
	CSSValueListImpl *list = new CSSValueListImpl;
	QString str(curP, endP-curP);
	str.simplifyWhiteSpace();
	//printf("text-decoration: '%s'\n", str.ascii());
	int pos=0, pos2;
	while( 1 )
	{
	    pos2 = str.find(' ', pos);
	    QString decoration = str.mid(pos, pos2-pos);
	    decoration = decoration.stripWhiteSpace();
	    //printf("found decoration '%s'\n", decoration.ascii());
	    const struct css_value *cssval = findValue(decoration.lower().ascii(),
						       decoration.length());
	    if (cssval)
		list->append(new CSSPrimitiveValueImpl(cssval->id));

	    pos = pos2 + 1;
	    if(pos2 == -1) break;
	}		
	//printf("got %ld decorations\n", list->length());
	if(list->length())
	    parsedValue = list;
	else
	    delete list;
	break;
    }
	
    case CSS_PROP_VOICE_FAMILY:
	// list of strings and i
	break;
	
// shorthand properties
    case CSS_PROP_BACKGROUND:
    {
	// add all shorthand properties to the list...
	//printf("parsing '%s'\n", QString(curP, endP - curP).ascii());
	bool last = false;
	while(!last)
	{
	    const QChar *nextP = curP;
	    while(*nextP != ' ' && *nextP != ';')
	    {
		if(nextP >= endP) {
                    last = true;
                    break;
                }
		nextP++;
	    }
	    bool found;
	    //printf("parsing '%s'\n", QString(curP, nextP - curP).ascii());
	    found = parseValue(curP, nextP, CSS_PROP_BACKGROUND_COLOR, important, propList);
	    if(!found) found = parseValue(curP, nextP, CSS_PROP_BACKGROUND_IMAGE, important, propList);
	    if(!found) found = parseValue(curP, nextP, CSS_PROP_BACKGROUND_POSITION, important, propList);
	    if(!found) found = parseValue(curP, nextP, CSS_PROP_BACKGROUND_REPEAT, important, propList);
	    if(!found) found = parseValue(curP, nextP, CSS_PROP_BACKGROUND_ATTACHMENT, important, propList);
	    if(!found) found = parseValue(curP, nextP, CSS_PROP_BACKGROUND_POSITION, important, propList);
	    curP = nextP+1;
	    if(curP >= endP) break;
	}		
	return true;
    }

// add all shorthand properties to the list...
    case CSS_PROP_BORDER:
    case CSS_PROP_BORDER_TOP:
    case CSS_PROP_BORDER_RIGHT:
    case CSS_PROP_BORDER_BOTTOM:
    case CSS_PROP_BORDER_LEFT:
    {
	const int *properties;
	const int properties_border[3] = {
	    CSS_PROP_BORDER_WIDTH, CSS_PROP_BORDER_STYLE, CSS_PROP_BORDER_COLOR };
	const int properties_border_top[3] = {
	    CSS_PROP_BORDER_TOP_WIDTH, CSS_PROP_BORDER_TOP_STYLE, CSS_PROP_BORDER_TOP_COLOR };
	const int properties_border_bottom[3] = {
	    CSS_PROP_BORDER_BOTTOM_WIDTH, CSS_PROP_BORDER_BOTTOM_STYLE, CSS_PROP_BORDER_BOTTOM_COLOR };
	const int properties_border_left[3] = {
	    CSS_PROP_BORDER_LEFT_WIDTH, CSS_PROP_BORDER_LEFT_STYLE, CSS_PROP_BORDER_LEFT_COLOR };
	const int properties_border_right[3] = {
	    CSS_PROP_BORDER_RIGHT_WIDTH, CSS_PROP_BORDER_RIGHT_STYLE, CSS_PROP_BORDER_RIGHT_COLOR };

	if(propId == CSS_PROP_BORDER)
	    properties = properties_border;
	else if(propId == CSS_PROP_BORDER_TOP)
	    properties = properties_border_top;
	else if(propId == CSS_PROP_BORDER_BOTTOM)
	    properties = properties_border_bottom;
	else if(propId == CSS_PROP_BORDER_LEFT)
	    properties = properties_border_left;
	else if(propId == CSS_PROP_BORDER_RIGHT)
	    properties = properties_border_right;	
	else return false;

	bool last = false;
	while(!last)
	{
	    const QChar *nextP = curP;
	    while(*nextP != ' ' && *nextP != ';')
	    {
		if(nextP >= endP) {
                    last = true;
                    break;
                } 
		nextP++;
	    }
	    bool found;
	    found = parseValue(curP, nextP, properties[0], important, propList);
	    if(!found) found = parseValue(curP, nextP, properties[1], important, propList);
	    if(!found) found = parseValue(curP, nextP, properties[2], important, propList);
	    curP = nextP+1;
	    if(curP >= endP) break;
	}		
	return true;
    }

    case CSS_PROP_BORDER_COLOR:
    {
	const struct css_value *cssval = findValue(val, value.length());
	if (cssval && cssval->id == CSS_VAL_TRANSPARENT)
	{
	    // set border colors to invalid
	    parsedValue = new CSSPrimitiveValueImpl(CSS_VAL_TRANSPARENT);
	    break;
	}
	const int properties[4] = {
	    CSS_PROP_BORDER_TOP_COLOR,
	    CSS_PROP_BORDER_RIGHT_COLOR,
	    CSS_PROP_BORDER_BOTTOM_COLOR,
	    CSS_PROP_BORDER_LEFT_COLOR
	    };
	return parse4Values(curP, endP, properties, important, propList);
    }
    case CSS_PROP_BORDER_WIDTH:
    {
	const int properties[4] = {
	    CSS_PROP_BORDER_TOP_WIDTH,
	    CSS_PROP_BORDER_RIGHT_WIDTH,
	    CSS_PROP_BORDER_BOTTOM_WIDTH,
	    CSS_PROP_BORDER_LEFT_WIDTH
	    };
	return parse4Values(curP, endP, properties, important, propList);
    }
    case CSS_PROP_BORDER_STYLE:
    {
	const int properties[4] = {
	    CSS_PROP_BORDER_TOP_STYLE,
	    CSS_PROP_BORDER_RIGHT_STYLE,
	    CSS_PROP_BORDER_BOTTOM_STYLE,
	    CSS_PROP_BORDER_LEFT_STYLE
	    };
	return parse4Values(curP, endP, properties, important, propList);
    }
    case CSS_PROP_MARGIN:
    {
	const int properties[4] = {
	    CSS_PROP_MARGIN_TOP,
	    CSS_PROP_MARGIN_RIGHT,
	    CSS_PROP_MARGIN_BOTTOM,
	    CSS_PROP_MARGIN_LEFT
	    };
	return parse4Values(curP, endP, properties, important, propList);
    }
    case CSS_PROP_PADDING:
    {
	const int properties[4] = {
	    CSS_PROP_PADDING_TOP,
	    CSS_PROP_PADDING_RIGHT,
	    CSS_PROP_PADDING_BOTTOM,
	    CSS_PROP_PADDING_LEFT
	    };
	return parse4Values(curP, endP, properties, important, propList);
    }

    case CSS_PROP_CUE:
    case CSS_PROP_FONT:
    case CSS_PROP_LIST_STYLE:
    case CSS_PROP_OUTLINE:
    case CSS_PROP_PAUSE:
	break;
    default:
	printf("illegal property!\n");
    }

 end:

    if(!parsedValue) return false;

    CSSProperty *prop = new CSSProperty();
    prop->m_id = propId;
    prop->setValue(parsedValue);
    prop->m_bImportant = important;

    propList->append(prop);
    //printf("added property %d\n", propId);

    return true;
}	

// used for shorthand properties xxx{1,4}
bool StyleBaseImpl::parse4Values(const QChar *curP, const QChar *endP, const int *properties,
				 bool important, QList<CSSProperty> *propList)
{
    bool last = false;
    QList<QChar> list;
    while(!last)
    {
	const QChar *nextP = curP;
	while(*nextP != ' ' && *nextP != ';')
	{
	    if(nextP >= endP) {
                last = true;
                break;
            }
	    nextP++;
	}
	list.append(curP);
	list.append(nextP);
	curP = nextP+1;
	if(curP >= endP) break;
    }
    switch(list.count())
    {
    case 2:
	// this is needed to get a correct reply for the border shorthand
	// property
	if(!parseValue(list.at(0), list.at(1), properties[0],
		   important, propList)) return false;
	parseValue(list.at(0), list.at(1), properties[1],
		   important, propList);
	parseValue(list.at(0), list.at(1), properties[2],
		   important, propList);
	parseValue(list.at(0), list.at(1), properties[3],
		   important, propList);
	return true;
    case 4:
	parseValue(list.at(0), list.at(1), properties[0],
		   important, propList);
	parseValue(list.at(2), list.at(3), properties[1],
		   important, propList);
	parseValue(list.at(0), list.at(1), properties[2],
		   important, propList);
	parseValue(list.at(2), list.at(3), properties[3],
		   important, propList);
	return true;
    case 6:
	parseValue(list.at(0), list.at(1), properties[0],
		   important, propList);
	parseValue(list.at(2), list.at(3), properties[1],
		   important, propList);
	parseValue(list.at(4), list.at(5), properties[2],
		   important, propList);
	parseValue(list.at(2), list.at(3), properties[3],
		   important, propList);
	return true;
    case 8:
	parseValue(list.at(0), list.at(1), properties[0],
		   important, propList);
	parseValue(list.at(2), list.at(3), properties[1],
		   important, propList);
	parseValue(list.at(4), list.at(5), properties[2],
		   important, propList);
	parseValue(list.at(6), list.at(7), properties[3],
		   important, propList);
	return true;
    default:
	return false;
    }
}

CSSValueImpl *
StyleBaseImpl::parseUnit(const QChar * curP, const QChar *endP, int allowedUnits)
{
    while(*endP == ' ' && endP > curP) endP--;
    const QChar *split = endP;
    // splt up number and unit
    while( (*split < '0' || *split > '9') && *split != '.' && split > curP)
	split--;
    split++;

    QString s(curP, split-curP);

    bool isInt = false;
    if(s.find('.') == -1) isInt = true;

    bool ok;
    float value = s.toFloat(&ok);
    if(!ok) return 0;


    if(split >= endP) // no unit
    {
	if(!(allowedUnits & (NUMBER | INTEGER)))
	    return 0;

	if(allowedUnits & NUMBER)
	    return new CSSPrimitiveValueImpl(value, CSSPrimitiveValue::CSS_NUMBER);
	
	if(allowedUnits & INTEGER && isInt) // ### DOM CSS doesn't seem to define something for integer
	    return new CSSPrimitiveValueImpl(value, CSSPrimitiveValue::CSS_NUMBER);

	if(allowedUnits & LENGTH && value == 0)
	    return new CSSPrimitiveValueImpl(0., CSSPrimitiveValue::CSS_UNKNOWN);

	return 0;
    }

    CSSPrimitiveValue::UnitTypes type = CSSPrimitiveValue::CSS_UNKNOWN;
    int unit = 0;

    switch(split->lower().latin1())
    {
    case '%':
	type = CSSPrimitiveValue::CSS_PERCENTAGE;
	unit = PERCENT;
    case 'e':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'm' || split->latin1() == 'M')
	{
	    type = CSSPrimitiveValue::CSS_EMS;
	    unit = LENGTH;
	}
	else if(split->latin1() == 'x' || split->latin1() == 'X')
	{
	    type = CSSPrimitiveValue::CSS_EXS;
	    unit = LENGTH;
	}
	break;
    case 'p':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'x' || split->latin1() == 'X')
	{
	    type = CSSPrimitiveValue::CSS_PX;
	    unit = LENGTH;
	}
	else if(split->latin1() == 't' || split->latin1() == 'T')
	{
	    type = CSSPrimitiveValue::CSS_PT;
	    unit = LENGTH;
	}
	else if(split->latin1() == 'c' || split->latin1() == 'C')
	{
	    type = CSSPrimitiveValue::CSS_PC;
	    unit = LENGTH;
	}
	break;
    case 'c':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'm' || split->latin1() == 'M')
	{
	    type = CSSPrimitiveValue::CSS_CM;
	    unit = LENGTH;
	}
	break;
    case 'm':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'm' || split->latin1() == 'M')
	{
	    type = CSSPrimitiveValue::CSS_MM;
	    unit = LENGTH;
	}
	else if(split->latin1() == 's' || split->latin1() == 'S')
	{
	    type = CSSPrimitiveValue::CSS_MS;
	    unit = TIME;
	}
	break;
    case 'i':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'n' || split->latin1() == 'N')
	{
	    type = CSSPrimitiveValue::CSS_IN;
	    unit = LENGTH;
	}
	break;
    case 'd':
	type = CSSPrimitiveValue::CSS_DEG;
	unit = ANGLE;
	break;
    case 'r':
	type = CSSPrimitiveValue::CSS_RAD;
	unit = ANGLE;
	break;
    case 'g':
	type = CSSPrimitiveValue::CSS_GRAD;
	unit = ANGLE;
	break;
    case 's':
	type = CSSPrimitiveValue::CSS_S;
	unit = TIME;
	break;
    case 'h':
        type = CSSPrimitiveValue::CSS_HZ;
	unit = FREQUENCY;
	break;
    case 'k':
	type = CSSPrimitiveValue::CSS_KHZ;
	unit = FREQUENCY;
	break;
    }

    if(unit & allowedUnits)
    {
	//printf("found allowed number %f, unit %d\n", value, type);
	return new CSSPrimitiveValueImpl(value, type);
    }

    return 0;
}

CSSStyleRuleImpl *
StyleBaseImpl::parseStyleRule(const QChar *&curP, const QChar *endP)
{
    //printf("style rule is \'%s\'\n", QString(curP, endP-curP).latin1());

    const QChar *startP;
    QList<CSSSelector> *slist;
    QList<CSSProperty> *plist;

    startP = curP;
    curP = parseToChar(startP, endP, '{', false);
    if (!curP)
        return(0);
    //printf("selector is \'%s\'\n", QString(startP, curP-startP).latin1());

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
	printf("bad style rule\n");
        return 0;
    }

    // return the newly created rule
    CSSStyleRuleImpl *rule = new CSSStyleRuleImpl(this);
    CSSStyleDeclarationImpl *decl = new CSSStyleDeclarationImpl(rule, plist);

    rule->setSelector(slist);
    rule->setDeclaration(decl);
    // ### set selector and value
    return rule;
}

CSSRuleImpl *
StyleBaseImpl::parseRule(const QChar *&curP, const QChar *endP)
{
    curP = parseSpace(curP, endP);
    CSSRuleImpl *rule = 0;

    if(!curP) return 0;
    //printf("parse rule: current = %c\n", curP->latin1());

    if (*curP == '@')
	rule = parseAtRule(curP, endP);
    else
	rule = parseStyleRule(curP, endP);

    if(curP) curP++;
    return rule;
}

// ------------------------------------------------------------------------------

StyleListImpl::~StyleListImpl()
{
    StyleBaseImpl *n;

    if(!m_lstChildren) return;

    for( n = m_lstChildren->first(); n != 0; n = m_lstChildren->next() )
    {
	n->setParent(0);
	if(n->deleteMe()) delete n;
    }
    delete m_lstChildren;
}

// --------------------------------------------------------------------------------

CSSSelector::CSSSelector(void)
: tag(0), tagHistory(0)
{
    attr = 0;
    match = None;
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
    //printf("[Selector: tag = %d, attr = \"%d\", value = \"%s\" relation = %d\n",
    //	tag, attr, value.string().data(), (int)relation);
}

int CSSSelector::specificity()
{
    int s = 0;
    if(tag != -1) s = 1;
    switch(match)
    {
    case Exact:
	if(attr == ATTR_ID)
	{
	    s += 100;
	    break;
	}
    case Set:
    case List:
    case Hyphen:
    case Pseudo:
	s += 10;
    case None:
	break;
    }
    if(tagHistory)
	s += tagHistory->specificity();
    return s;
}
// ----------------------------------------------------------------------------

CSSProperty::~CSSProperty()
{
    if(m_value) m_value->deref();
}

void CSSProperty::setValue(CSSValueImpl *val)
{
    if(m_value) m_value->deref();
    m_value = val;
    if(m_value) m_value->ref();
}

CSSValueImpl *CSSProperty::value()
{
    return m_value;
}
