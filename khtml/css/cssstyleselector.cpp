/**
 * This file is part of the CSS implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
#include "cssstyleselector.h"
#include "rendering/render_style.h"
#include "css_stylesheetimpl.h"
#include "css_ruleimpl.h"
#include "css_valueimpl.h"
#include "csshelper.h"
#include "html/html_documentimpl.h"
#include "html/html_baseimpl.h"
#include "dom/css_rule.h"
#include "dom/css_value.h"
using namespace khtml;
using namespace DOM;

#include "cssproperties.h"
#include "cssvalues.h"

#include "misc/khtmllayout.h"
#include "misc/khtmldata.h"
#include "misc/htmlhashes.h"

#include "html/html_headimpl.h"
#include "khtmlview.h"
#include "khtml_part.h"

#include <kstddirs.h>
#include <kcharsets.h>
#include <kglobal.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qfontdatabase.h>
#include <qfontinfo.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <kdebug.h>

#include <assert.h>

CSSStyleSelectorList *CSSStyleSelector::defaultStyle = 0;
CSSStyleSelectorList *CSSStyleSelector::userStyle = 0;


CSSStyleSelector::CSSStyleSelector(DocumentImpl * /*doc*/)
{
    // ### parse the xml for processing instructions containing style sheet info
    authorStyle = new CSSStyleSelectorList();
}

CSSStyleSelector::CSSStyleSelector(HTMLDocumentImpl *doc)
{
    if(!defaultStyle) loadDefaultStyle();

    authorStyle = new CSSStyleSelectorList();
    // ### go through DOM tree for style elements

    NodeImpl *test = doc->firstChild(); // should be html element
    if(!test) return;
    test = test->firstChild();
    if(!test) return;
    while(test && (test->id() != ID_HEAD))
	test = test->nextSibling();
    if(!test) return;
    HTMLHeadElementImpl *head = static_cast<HTMLHeadElementImpl *>(test);

    // all LINK and STYLE elements have to be direct children of the HEAD element
    test = head->firstChild();
    while(test)
    {
	if(test->id() == ID_LINK)
	{
	    HTMLLinkElementImpl *link = static_cast<HTMLLinkElementImpl *>(test);
	    authorStyle->append(link->sheet());
	}
	else if(test->id() == ID_STYLE)
	{
	    HTMLStyleElementImpl *style = static_cast<HTMLStyleElementImpl *>(test);
	    authorStyle->append(style->sheet());
	}
	test = test->nextSibling();
    }

    HTMLElementImpl *e = doc->body();
    if(e && e->id() == ID_BODY)
    {
	kdDebug( 6080 ) << "found body element" << endl;
	HTMLBodyElementImpl *body = static_cast<HTMLBodyElementImpl *>(e);
	if(body->sheet())
	{
	    kdDebug( 6080 ) << "body has style sheet " << body->sheet() << endl;
	    authorStyle->append(body->sheet());
	}
    }
}

CSSStyleSelector::CSSStyleSelector(StyleSheetImpl *sheet)
{
    if(!defaultStyle) loadDefaultStyle();

    authorStyle = new CSSStyleSelectorList();
    authorStyle->append(sheet);
}

CSSStyleSelector::~CSSStyleSelector()
{
    delete authorStyle;
}

void CSSStyleSelector::addSheet(StyleSheetImpl *sheet)
{
    authorStyle->append(sheet);
}

void CSSStyleSelector::setUserStyle(StyleSheetImpl *sheet)
{
    if(userStyle) delete userStyle;
    userStyle = new CSSStyleSelectorList();
    userStyle->append(sheet);
}

void CSSStyleSelector::loadDefaultStyle()
{
    if(defaultStyle) return;

    QFile f(locate( "data", "khtml/css/html4.css" ) );
    f.open(IO_ReadOnly);

    QTextStream t( &f );
    QString style = t.read();
    DOMString str(style);

    // ### memory leak!
    DOM::CSSStyleSheetImpl *sheet = new DOM::CSSStyleSheetImpl((DOM::CSSStyleSheetImpl *)0);
    sheet->parseString( str );

    defaultStyle = new CSSStyleSelectorList();
    defaultStyle->append(sheet);
}

#include <qmap.h>

static QList<RenderStyle> lastStyles;

RenderStyle *CSSStyleSelector::styleForElement(ElementImpl *e)
{
    CSSOrderedPropertyList *propsToApply = new CSSOrderedPropertyList();

    // the higher the offset or important number, the later the rules get applied.

    if(defaultStyle) defaultStyle->collect(propsToApply, e, 0x00100000); // no important rules here
    // important rules from user styles are higher than important rules from author styles.
    // for non important rules the order is reversed
    if(userStyle) userStyle->collect(propsToApply, e, 0x00200000, 0x04000000);
    if(authorStyle) authorStyle->collect(propsToApply, e, 0x00400000, 0x01000000);
    // these count as author rules, and come after all other style sheets
    if(e->styleRules()) propsToApply->append(e->styleRules(), 0x00800000, 0x02000000);

    propsToApply->sort();

    RenderStyle *style;
    if(e->parentNode())
    {
	assert(e->parentNode()->style() != 0);
	style = new RenderStyle(e->parentNode()->style());
    }
    else
	style = new RenderStyle();	

    for(int i = 0; i < (int)propsToApply->count(); i++)
	applyRule(style, propsToApply->at(i)->prop, e);
	
	
//    kdDebug( 6080 ) << "STYLE count=" << RenderStyle::counter << ", DATA count=" << SharedData::counter << endl;

    // experimental -antti
#if 0
    for ( int n=0; n<(int)lastStyles.count(); n++)
    {
    	style->mergeData(lastStyles.at(n));
    }

    lastStyles.append(style);

    if (lastStyles.count()>5)
    	lastStyles.removeFirst();
#endif

    delete propsToApply;

    return style;
}

// ----------------------------------------------------------------------


CSSOrderedRule::CSSOrderedRule(DOM::CSSStyleRuleImpl *r, DOM::CSSSelector *s, int _index)
{
    rule = r;
    if(rule) r->ref();
    index = _index;
    selector = s;
}

CSSOrderedRule::~CSSOrderedRule()
{
    if(rule) rule->deref();
}

bool CSSOrderedRule::checkSelector(DOM::ElementImpl *e)
{
    CSSSelector *sel = selector;
    NodeImpl *n = e;
    // first selector has to match
    if(!checkOneSelector(sel, e)) return false;

    // check the subselectors
    CSSSelector::Relation relation = sel->relation;
    while((sel = sel->tagHistory))
    {
	if(!n->isElementNode()) return false;
	switch(relation)
	{
	case CSSSelector::Descendant:
	{
	    bool found = false;
	    while(!found)
	    {
		n = n->parentNode();
		if(!n || !n->isElementNode()) return false;
		ElementImpl *elem = static_cast<ElementImpl *>(n);
		if(checkOneSelector(sel, elem)) found = true;
	    }
	    break;
	}
	case CSSSelector::Child:
	{
	    n = n->parentNode();
	    if(!n || !n->isElementNode()) return false;
	    ElementImpl *elem = static_cast<ElementImpl *>(n);
	    if(!checkOneSelector(sel, elem)) return false;
	    break;
	}
	case CSSSelector::Sibling:
	{
	    n = n->previousSibling();
	    if(!n || !n->isElementNode()) return false;
	    ElementImpl *elem = static_cast<ElementImpl *>(n);
	    if(!checkOneSelector(sel, elem)) return false;
	    break;
	}
	}
	relation = sel->relation;
    }
    return true;
}

bool CSSOrderedRule::checkOneSelector(DOM::CSSSelector *sel, DOM::ElementImpl *e)
{
    if(!e || !e->isHTMLElement())
    {
	// ### no support for xml elements at the moment
	return false;
    }

    if(e->id() != sel->tag && sel->tag != -1) return false;

    if(sel->attr)
    {
	DOMString value = e->getAttribute(sel->attr);
	if(value.isNull()) return false; // attribute is not set
	
	switch(sel->match)
	{
	case CSSSelector::Exact:
	    if(sel->value != value) return false;
	    break;
	case CSSSelector::Set:
	    break;
	case CSSSelector::List:
	{
	    //kdDebug( 6080 ) << "checking for list match" << endl;
	    QString str = value.string();
	    QString selStr = sel->value.string();
	    int pos = str.find(selStr);
	    if(pos == -1) return false;
	    if(pos && str[pos-1] != ' ') return false;
	    pos += selStr.length();
	    if(pos < (int)str.length() && str[pos] != ' ') return false;
	    break;
	}
	case CSSSelector::Hyphen:
	{
	    // ### still doesn't work. FIXME
	    //kdDebug( 6080 ) << "checking for hyphen match" << endl;
	    QString str = value.string();
	    if(str.find(sel->value.string()) != 0) return false;
	    // ### could be "bla , sdfdsf" too. Parse out spaces
	    int pos = sel->value.length() + 1;
	    while(pos < (int)str.length() && sel->value[pos] == ' ') pos++;
	    if(pos < (int)str.length() && sel->value[pos] != ',') return false;
	    break;
	}
	case CSSSelector::Pseudo:
	case CSSSelector::None:
	    break;
	}
    }
    if(sel->match == CSSSelector::Pseudo)
    {
	// Pseudo elements. We need to check first child here. No dynamic pseudo
	// elements for the moment
	if(sel->value == ":first-child")
	{
	    if(e->parentNode()->firstChild() != e)
		return false;
	}
	else
	    return false;
    }		
    // ### add the rest of the checks...
    return true;
}


// -----------------------------------------------------------------

CSSStyleSelectorList::CSSStyleSelectorList()
    : QList<CSSOrderedRule>()
{
    setAutoDelete(true);
}
CSSStyleSelectorList::~CSSStyleSelectorList()
{
}

int CSSStyleSelectorList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
    CSSOrderedRule *r1 = static_cast<CSSOrderedRule *>(i1);
    CSSOrderedRule *r2 = static_cast<CSSOrderedRule *>(i2);
    int d = r1->selector->specificity() - r2->selector->specificity();
    if(d) return d;
    return r1->index - r2->index;
}

void CSSStyleSelectorList::append(StyleSheetImpl *sheet)
{

    if(!sheet || !sheet->isCSSStyleSheet()) return;

    int len = sheet->length();

    for(int i = 0; i< len; i++)
    {
	StyleBaseImpl *item = sheet->item(i);
	if(item->isStyleRule())
	{
	    CSSStyleRuleImpl *r = static_cast<CSSStyleRuleImpl *>(item);
	    QList<CSSSelector> *s = r->selector();
	    for(int j = 0; j < (int)s->count(); j++)
	    {
		CSSOrderedRule *rule = new CSSOrderedRule(r, s->at(j), count());
		QList<CSSOrderedRule>::append(rule);
		//kdDebug( 6080 ) << "appending StyleRule!" << endl;
	    }
	}
	else if(item->isImportRule())
	{
	    CSSImportRuleImpl *import = static_cast<CSSImportRuleImpl *>(item);
	    // ### check media type
	    StyleSheetImpl *importedSheet = import->styleSheet();
	    append(importedSheet);
	}
	// ### include media, import rules and other
    }
    sort();
}

void CSSStyleSelectorList::append(CSSStyleRuleImpl *rule)
{
    QList<CSSSelector> *s = rule->selector();
    for(int j = 0; j < (int)s->count(); j++)
    {
	CSSOrderedRule *r = new CSSOrderedRule(rule, s->at(j), count());
	inSort(r);
    }
}

void CSSStyleSelectorList::collect(CSSOrderedPropertyList *propsToApply, DOM::ElementImpl *e,
				   int offset, int important)
{
    int i;
    int num = count();
    for(i = 0; i< num; i++)
    {
	if(at(i)->checkSelector(e))
	{
	    //kdDebug( 6080 ) << "found matching rule for element " << e->id() << endl;
	    CSSStyleDeclarationImpl *decl = at(i)->rule->declaration();
	    propsToApply->append(decl, offset + at(i)->index, important);
	}
    }
}

// -------------------------------------------------------------------------

CSSOrderedPropertyList::CSSOrderedPropertyList()
{
    setAutoDelete(true);
}

int CSSOrderedPropertyList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
    return static_cast<CSSOrderedProperty *>(i1)->priority
	- static_cast<CSSOrderedProperty *>(i2)->priority;
}

void CSSOrderedPropertyList::append(DOM::CSSStyleDeclarationImpl *decl, int offset, int important)
{
    QList<CSSProperty> *values = decl->values();
    if(!values) return;
    int len = values->count();
    for(int i = 0; i < len; i++)
    {
	int thisOffset = offset;
	CSSProperty *prop = values->at(i);
	if(prop->m_bImportant) thisOffset += important;
	// give special priority to font-xxx, color properties
	switch(prop->m_id)
	{
	case CSS_PROP_FONT_SIZE:
	case CSS_PROP_FONT:
	case CSS_PROP_COLOR:
	    // these have to be applied first, because other properties use the computed
	    // values of these porperties.
	    break;
	default:
	    // don't use 0x80000000, that is negative usually
	    thisOffset += 0x40000000;
	    break;
	}	
	
	append(prop, thisOffset);
    }
}

void CSSOrderedPropertyList::append(DOM::CSSProperty *prop, int priority)
{
    QList<CSSOrderedProperty>::append(new CSSOrderedProperty(prop, priority));
}

// -------------------------------------------------------------------------------------
// this is mostly boring stuff on how to apply a certain rule to the renderstyle...

void khtml::applyRule(khtml::RenderStyle *style, DOM::CSSProperty *prop, DOM::ElementImpl *e)
{
    CSSValueImpl *value = prop->value();

    //kdDebug( 6080 ) << "applying property " << prop->m_id << endl;

    CSSPrimitiveValueImpl *primitiveValue = 0;
    if(value->isPrimitiveValue()) primitiveValue = static_cast<CSSPrimitiveValueImpl *>(value);

    Length l;
    bool apply = false;
	
    // here follows a long list, defining how to aplly certain properties to the style object.
    // rather boring stuff...
    switch(prop->m_id)
    {
// ident only properties
    case CSS_PROP_BACKGROUND_ATTACHMENT:
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    // ###
	    //style->setBackgroundAttachment(e->parentNode->style()->backgroundAttachment());
	    break;
	}
	if(!primitiveValue) break;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_FIXED:
	    style->setBackgroundAttachment(false);
	    break;
	case CSS_VAL_SCROLL:
	    style->setBackgroundAttachment(true);
	    break;
	default:
	    return;
	}	
    case CSS_PROP_BACKGROUND_REPEAT:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    // ###
	    //style->setDisplay(e->parentNode->style()->display());
	    return;
	}
	if(!primitiveValue) return;

	EBackgroundRepeat r = REPEAT;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_REPEAT:
	    r = REPEAT; break;
	case CSS_VAL_REPEAT_X:
	    r = REPEAT_X; break;
	case CSS_VAL_REPEAT_Y:
	    r = REPEAT_Y; break;
	case CSS_VAL_NO_REPEAT:
	    r = NO_REPEAT; break;
	default:
	    return;
	}
	style->setBackgroundRepeat(r);
	break;
    }
    case CSS_PROP_BORDER_COLLAPSE:
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    // ###
	    //style->setBackgroundAttachment(e->parentNode->style()->backgroundAttachment());
	    break;
	}
	if(!primitiveValue) break;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_COLLAPSE:
	    style->setBorderCollapse(true);
	    break;
	case CSS_VAL_SCROLL:
	    style->setBorderCollapse(false);
	    break;
	default:
	    return;
	}	
	
    case CSS_PROP_BORDER_TOP_STYLE:
    case CSS_PROP_BORDER_RIGHT_STYLE:
    case CSS_PROP_BORDER_BOTTOM_STYLE:
    case CSS_PROP_BORDER_LEFT_STYLE:
    {
	//kdDebug( 6080 ) << "applying border style!" << endl;
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    switch(prop->m_id)
	    {
	    case CSS_PROP_BORDER_TOP_STYLE:
		style->setBorderTopStyle(e->parentNode()->style()->borderTopStyle());
		return;
	    case CSS_PROP_BORDER_RIGHT_STYLE:
		style->setBorderRightStyle(e->parentNode()->style()->borderRightStyle());
		return;
	    case CSS_PROP_BORDER_BOTTOM_STYLE:
		style->setBorderBottomStyle(e->parentNode()->style()->borderBottomStyle());
		return;
	    case CSS_PROP_BORDER_LEFT_STYLE:
		style->setBorderLeftStyle(e->parentNode()->style()->borderLeftStyle());
		return;
	    }
	}
	if(!primitiveValue) return;
	EBorderStyle s = BNONE;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_NONE:
	    s = BNONE; break;
	case CSS_VAL_HIDDEN:
	    s = BHIDDEN; break;
	case CSS_VAL_DOTTED:
	    s = DOTTED; break;
	case CSS_VAL_DASHED:
	    s = DASHED; break;
	case CSS_VAL_SOLID:
	    s = SOLID; break;
	case CSS_VAL_DOUBLE:
	    s = DOUBLE; break;
	case CSS_VAL_GROOVE:
	    s = GROOVE; break;
	case CSS_VAL_RIDGE:
	    s = RIDGE; break;
	case CSS_VAL_INSET:
	    s = INSET; break;
	case CSS_VAL_OUTSET:
	    s = OUTSET; break;
	default:
	    return;
	}
	switch(prop->m_id)
	{
	case CSS_PROP_BORDER_TOP_STYLE:
	    style->setBorderTopStyle(s); return;
	case CSS_PROP_BORDER_RIGHT_STYLE:
	    style->setBorderRightStyle(s); return;
	case CSS_PROP_BORDER_BOTTOM_STYLE:
	    style->setBorderBottomStyle(s); return;
	case CSS_PROP_BORDER_LEFT_STYLE:
	    style->setBorderLeftStyle(s); return;
	default:
	    return;
	}
	return;
    }
    case CSS_PROP_CAPTION_SIDE:
    case CSS_PROP_CLEAR:
    {
  	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setClear(e->parentNode()->style()->clear());
	    break;
	}
	if(!primitiveValue) break;
	EClear c = CNONE;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_LEFT:
	    c = CLEFT; break;
	case CSS_VAL_RIGHT:
	    c = CRIGHT; break;
	case CSS_VAL_BOTH:
	    c = CBOTH; break;
	default:
	    return;
	}
	style->setClear(c);
	return;
    }
    case CSS_PROP_DIRECTION:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setDirection(e->parentNode()->style()->direction());
	    break;
	}
	if(!primitiveValue) break;
	EDirection d = LTR;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_LTR:
	    d = LTR; break;
	case CSS_VAL_RTL:
	    d = RTL; break;
	default:
	    return;
	}
	style->setDirection(d);
	return;
    }
    case CSS_PROP_DISPLAY:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setDisplay(e->parentNode()->style()->display());
	    break;
	}
	if(!primitiveValue) break;
	EDisplay d = INLINE;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_INLINE:
	    break;
	case CSS_VAL_BLOCK:
	    d = BLOCK; break;
	case CSS_VAL_LIST_ITEM:
	    d = LIST_ITEM; break;
	case CSS_VAL_RUN_IN:
	    d = RUN_IN; break;
	case CSS_VAL_COMPACT:
	    d = COMPACT; break;
	case CSS_VAL_MARKER:
	    d = MARKER; break;
	case CSS_VAL_TABLE:
	    d = TABLE; break;
	case CSS_VAL_INLINE_TABLE:
	    d = INLINE_TABLE; break;
	case CSS_VAL_TABLE_ROW_GROUP:
	    d = TABLE_ROW_GROUP; break;
	case CSS_VAL_TABLE_HEADER_GROUP:		
	    d = TABLE_HEADER_GROUP; break;
	case CSS_VAL_TABLE_FOOTER_GROUP:
	    d = TABLE_FOOTER_GROUP; break;
	case CSS_VAL_TABLE_ROW:
	    d = TABLE_ROW; break;
	case CSS_VAL_TABLE_COLUMN_GROUP:
	    d = TABLE_COLUMN_GROUP; break;
	case CSS_VAL_TABLE_COLUMN:
	    d = TABLE_COLUMN; break;
	case CSS_VAL_TABLE_CELL:
	    d = TABLE_CELL; break;
	case CSS_VAL_TABLE_CAPTION:
	    d = TABLE_CAPTION; break;
	case CSS_VAL_NONE:
	    d = NONE; break;
	default:
	    break;
	}
	//kdDebug( 6080 ) << "setting display to " << d << endl;
	style->setDisplay(d);
	break;
    }
	
    case CSS_PROP_EMPTY_CELLS:
	break;
    case CSS_PROP_FLOAT:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setFloating(e->parentNode()->style()->floating());
	    return;
	}
	if(!primitiveValue) return;
	EFloat f;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_LEFT:
	    f = FLEFT; break;
	case CSS_VAL_RIGHT:
	    f = FRIGHT; break;
// ### we're not complying 100% to CSS standards here. There is no center value for the
//     float property. Still it's the easiest way to get <table align=center> and <center>
//     working as expected
	case CSS_VAL_CENTER:
	    f = FCENTER; break;
	case CSS_VAL_NONE:
	    f = FNONE; break;
	default:
	    return;
	}
	style->setFloating(f);
	break;
    }

	break;
    case CSS_PROP_FONT_STRETCH:
	break;

    case CSS_PROP_FONT_STYLE:
    {
	QFont f = style->font();
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    f.setItalic(e->parentNode()->style()->font().italic());
	    //KGlobal::charsets()->setQFont(f, e->ownerDocument()->view()->part()->settings()->charset);
	    style->setFont(f);
	    return;
	}
	if(!primitiveValue) return;
	switch(primitiveValue->getIdent())
	{
	    // ### oblique is the same as italic for the moment...
	case CSS_VAL_OBLIQUE:
	case CSS_VAL_ITALIC:
	    f.setItalic(true);
	    break;
	case CSS_VAL_NORMAL:
	    f.setItalic(false);
	    break;
	default:
	    return;
	}
	//KGlobal::charsets()->setQFont(f, e->ownerDocument()->view()->part()->settings()->charset);
	style->setFont(f);
	break;
    }

	
    case CSS_PROP_FONT_VARIANT:
	// ### no small caps at the moment...
	break;
	
    case CSS_PROP_FONT_WEIGHT:
    {
	QFont f = style->font();
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    f.setWeight(e->parentNode()->style()->font().weight());
	    //KGlobal::charsets()->setQFont(f, e->ownerDocument()->view()->part()->settings()->charset);
	    style->setFont(f);
	    return;
	}
	if(!primitiveValue) return;
	if(primitiveValue->getIdent())
	{
	    switch(primitiveValue->getIdent())
	    {
		// ### we just support normal and bold fonts at the moment...
		// setWeight can actually accept values between 0 and 99...
	    case CSS_VAL_BOLD:
	    case CSS_VAL_BOLDER:
		f.setWeight(QFont::Bold);
		break;
	    case CSS_VAL_NORMAL:
	    case CSS_VAL_LIGHTER:
		f.setWeight(QFont::Normal);
		break;
	    default:
		return;
	    }
	}
	else
	{
	    // ### fix parsing of 100-900 values in parser, apply them here
	}
	//KGlobal::charsets()->setQFont(f, e->ownerDocument()->view()->part()->settings()->charset);
	style->setFont(f);
	break;
    }
	
    case CSS_PROP_LIST_STYLE_POSITION:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setListStylePosition(e->parentNode()->style()->listStylePosition());
	    return;
	}
	if(!primitiveValue) return;
	if(primitiveValue->getIdent())
	{
	    EListStylePosition p;
	    switch(primitiveValue->getIdent())
	    {
	    case CSS_VAL_INSIDE:
		p = INSIDE; break;
	    case CSS_VAL_BOLDER:
		p = OUTSIDE; break;
	    default:
		return;
	    }
	    style->setListStylePosition(p);
	}	
	return;
    }

    case CSS_PROP_LIST_STYLE_TYPE:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setListStyleType(e->parentNode()->style()->listStyleType());
	    return;
	}
	if(!primitiveValue) return;
	if(primitiveValue->getIdent())
	{
	    EListStyleType t;
	    switch(primitiveValue->getIdent())
	    {
	    case CSS_VAL_DISC:
		t = DISC; break;
	    case CSS_VAL_CIRCLE:
		t = CIRCLE; break;
	    case CSS_VAL_SQUARE:
		t = SQUARE; break;
	    case CSS_VAL_HEBREW:
		t = HEBREW; break;
	    case CSS_VAL_ARMENIAN:
		t = ARMENIAN; break;
	    case CSS_VAL_GEORGIAN:
		t = GEORGIAN; break;
	    case CSS_VAL_CJK_IDEOGRAPHIC:
		t = CJK_IDEOGRAPHIC; break;
	    case CSS_VAL_HIRAGANA:
		t = HIRAGANA; break;
	    case CSS_VAL_KATAKANA:
		t = KATAKANA; break;
	    case CSS_VAL_HIRAGANA_IROHA:
		t = HIRAGANA_IROHA; break;
	    case CSS_VAL_KATAKANA_IROHA:
		t = KATAKANA_IROHA; break;
	    case CSS_VAL_DECIMAL_LEADING_ZERO:
		t = DECIMAL_LEADING_ZERO; break;
	    case CSS_VAL_DECIMAL:
		t = LDECIMAL; break;
	    case CSS_VAL_LOWER_ROMAN:
		t = LOWER_ROMAN; break;
	    case CSS_VAL_UPPER_ROMAN:
		t = UPPER_ROMAN; break;
	    case CSS_VAL_LOWER_GREEK:
		t = LOWER_GREEK; break;
	    case CSS_VAL_LOWER_ALPHA:
		t = LOWER_ALPHA; break;
	    case CSS_VAL_LOWER_LATIN:
		t = LOWER_LATIN; break;
	    case CSS_VAL_UPPER_ALPHA:
		t = UPPER_ALPHA; break;
	    case CSS_VAL_UPPER_LATIN:
		t = UPPER_LATIN; break;
	    case CSS_VAL_NONE:
		t = LNONE; break;
	    default:
		return;
	    }
	    style->setListStyleType(t);
	}	
	return;
    }

    case CSS_PROP_OUTLINE_STYLE:
    case CSS_PROP_OVERFLOW:
    case CSS_PROP_PAGE:
    case CSS_PROP_PAGE_BREAK_AFTER:
    case CSS_PROP_PAGE_BREAK_BEFORE:
    case CSS_PROP_PAGE_BREAK_INSIDE:
    case CSS_PROP_PAUSE_AFTER:
    case CSS_PROP_PAUSE_BEFORE:
	break;

    case CSS_PROP_POSITION:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    style->setPosition(e->parentNode()->style()->position());
	    return;
	}
	if(!primitiveValue) return;
	EPosition p;
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_STATIC:
	    p = STATIC; break;
	case CSS_VAL_RELATIVE:
	    p = RELATIVE; break;
	case CSS_VAL_ABSOLUTE:
	    p = ABSOLUTE; break;
	case CSS_VAL_FIXED:
	    p = FIXED; break;
	default:
	    return;
	}
	style->setPosition(p);
	return;
    }

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
	break;
    case CSS_PROP_BORDER_SPACING:
    	{
    	if(!primitiveValue) break;
	short spacing = 0;
	spacing = computeLength(primitiveValue, style);
	style->setBorderSpacing(spacing);
	break;
	}	
	// CSS2BorderSpacing
    case CSS_PROP_CURSOR:
	// CSS2Cursor
    case CSS_PROP_PLAY_DURING:
	// CSS2PlayDuring
    case CSS_PROP_TEXT_SHADOW:
	// list of CSS2TextShadow
	break;

// colors || inherit
    case CSS_PROP_BACKGROUND_COLOR:
    case CSS_PROP_BORDER_TOP_COLOR:
    case CSS_PROP_BORDER_RIGHT_COLOR:
    case CSS_PROP_BORDER_BOTTOM_COLOR:
    case CSS_PROP_BORDER_LEFT_COLOR:
    case CSS_PROP_COLOR:
    case CSS_PROP_OUTLINE_COLOR:
	// this property is an extension used to get HTML4 <font> right.
    case CSS_PROP_TEXT_DECORATION_COLOR:
    {
	QColor col;
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    switch(prop->m_id)
	    {
	    case CSS_PROP_BACKGROUND_COLOR:
		style->setBackgroundColor(e->parentNode()->style()->backgroundColor()); break;
	    case CSS_PROP_BORDER_TOP_COLOR:
		style->setBorderTopColor(e->parentNode()->style()->borderTopColor()); break;
	    case CSS_PROP_BORDER_RIGHT_COLOR:
		style->setBorderRightColor(e->parentNode()->style()->borderRightColor()); break;
	    case CSS_PROP_BORDER_BOTTOM_COLOR:
		style->setBorderBottomColor(e->parentNode()->style()->borderBottomColor()); break;
	    case CSS_PROP_BORDER_LEFT_COLOR:
		style->setBorderLeftColor(e->parentNode()->style()->borderLeftColor()); break;
	    case CSS_PROP_COLOR:
		style->setColor(e->parentNode()->style()->color()); break;
	    case CSS_PROP_OUTLINE_COLOR:
	    default:
		// ###
		break;
	    }
	    return;
	}
	if(!primitiveValue) return;
	if(primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_RGBCOLOR)
	    col = primitiveValue->getRGBColorValue()->color();
	else
	    return;
	//kdDebug( 6080 ) << "applying color " << col.isValid() << endl;
	switch(prop->m_id)
	{
	case CSS_PROP_BACKGROUND_COLOR:
	    style->setBackgroundColor(col); break;
	case CSS_PROP_BORDER_TOP_COLOR:
	    style->setBorderTopColor(col); break;
	case CSS_PROP_BORDER_RIGHT_COLOR:
	    style->setBorderRightColor(col); break;
	case CSS_PROP_BORDER_BOTTOM_COLOR:
	    style->setBorderBottomColor(col); break;
	case CSS_PROP_BORDER_LEFT_COLOR:
	    style->setBorderLeftColor(col); break;
	case CSS_PROP_COLOR:
	    style->setColor(col); break;
	case CSS_PROP_TEXT_DECORATION_COLOR:
	    style->setTextDecorationColor(col); break;
	case CSS_PROP_OUTLINE_COLOR:
	    // ###
	default:
	    return;
	}		
	return;
    }
    break;
// uri || inherit
    case CSS_PROP_BACKGROUND_IMAGE:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setBackgroundImage(e->parentNode()->style()->backgroundImage());
	    break;
	}
	if(!primitiveValue) return;
	CSSImageValueImpl *image = static_cast<CSSImageValueImpl *>(primitiveValue);
	style->setBackgroundImage(image->image());
	kdDebug( 6080 ) << "setting image in style to " << image->image() << endl;
	break;
    }
    case CSS_PROP_CUE_AFTER:
    case CSS_PROP_CUE_BEFORE:
	break;
    case CSS_PROP_LIST_STYLE_IMAGE:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setListStyleImage(e->parentNode()->style()->listStyleImage());
	    break;
	}
	if(!primitiveValue) return;
	CSSImageValueImpl *image = static_cast<CSSImageValueImpl *>(primitiveValue);
	style->setListStyleImage(image->image());
	kdDebug( 6080 ) << "setting image in list to " << image->image() << endl;
	break;
    }

// length
    case CSS_PROP_BORDER_TOP_WIDTH:
    case CSS_PROP_BORDER_RIGHT_WIDTH:
    case CSS_PROP_BORDER_BOTTOM_WIDTH:
    case CSS_PROP_BORDER_LEFT_WIDTH:
    case CSS_PROP_OUTLINE_WIDTH:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    // ###
	    break;
	}
	if(!primitiveValue) break;
	short width = 3; // medium is default value
	switch(primitiveValue->getIdent())
	{
	case CSS_VAL_THIN:
	    width = 1;
	    break;
	case CSS_VAL_MEDIUM:
	    width = 3;
	    break;
	case CSS_VAL_THICK:
	    width = 5;
	    break;
	case CSS_VAL_INVALID:
	    width = computeLength(primitiveValue, style);
	    break;
	default:
	    return;
	}	
	if(width < 0) return;
	switch(prop->m_id)
	{
	case CSS_PROP_BORDER_TOP_WIDTH:
	    style->setBorderTopWidth(width);
	    break;
	case CSS_PROP_BORDER_RIGHT_WIDTH:
	    style->setBorderRightWidth(width);
	    break;
	case CSS_PROP_BORDER_BOTTOM_WIDTH:
	    style->setBorderBottomWidth(width);
	    break;
	case CSS_PROP_BORDER_LEFT_WIDTH:
	    style->setBorderLeftWidth(width);
	    break;
	case CSS_PROP_OUTLINE_WIDTH:
	    // ###
	    break;
	default:
	    return;
	}
	return;
    }
	
    case CSS_PROP_MARKER_OFFSET:
    case CSS_PROP_LETTER_SPACING:
    case CSS_PROP_WORD_SPACING:
    {
	if(!primitiveValue) return;
	int width = computeLength(primitiveValue, style);
	if( width < 0 ) return;
	switch(prop->m_id)
	{
	case CSS_PROP_MARKER_OFFSET:
	case CSS_PROP_LETTER_SPACING:
	case CSS_PROP_WORD_SPACING:
	    // ### needs the definitions in renderstyle
	default:
	    return;
	}
    }

// length, percent
    case CSS_PROP_MAX_WIDTH:
	// +none +inherit
	if(primitiveValue && primitiveValue->getIdent() == CSS_VAL_NONE)
	    apply = true;
    case CSS_PROP_BOTTOM:
    case CSS_PROP_TOP:
    case CSS_PROP_LEFT:
    case CSS_PROP_RIGHT:
    case CSS_PROP_WIDTH:
    case CSS_PROP_MIN_WIDTH:
    case CSS_PROP_MARGIN_TOP:
    case CSS_PROP_MARGIN_RIGHT:
    case CSS_PROP_MARGIN_BOTTOM:
    case CSS_PROP_MARGIN_LEFT:
	// +inherit +auto
	if(prop->m_id != CSS_PROP_MAX_WIDTH && primitiveValue &&
	   primitiveValue->getIdent() == CSS_VAL_AUTO)
	{
	    //kdDebug( 6080 ) << "found value=auto" << endl;
	    apply = true;
	}
    case CSS_PROP_PADDING_TOP:
    case CSS_PROP_PADDING_RIGHT:
    case CSS_PROP_PADDING_BOTTOM:
    case CSS_PROP_PADDING_LEFT:
    case CSS_PROP_TEXT_INDENT:
	// +inherit
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    // ###
	    //l = e->parentNode->style()->....());
	}
	if(primitiveValue && !apply)
	{
	    int type = primitiveValue->primitiveType();
	    if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
		l = Length(computeLength(primitiveValue, style), Fixed);
	    else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
		l = Length((int)primitiveValue->getFloatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
	    else
		return;
	    apply = true;
	}
	if(!apply) return;
	switch(prop->m_id)
	{
	case CSS_PROP_MAX_WIDTH:
	    style->setMaxWidth(l); break;
	case CSS_PROP_BOTTOM:
	    style->setBottom(l); break;
	case CSS_PROP_TOP:
	    style->setTop(l); break;
	case CSS_PROP_LEFT:
	    style->setLeft(l); break;
	case CSS_PROP_RIGHT:
	    style->setRight(l); break;
	case CSS_PROP_WIDTH:
	    style->setWidth(l); break;
	case CSS_PROP_MIN_WIDTH:
	    style->setMinWidth(l); break;
	case CSS_PROP_PADDING_TOP:
	    style->setPaddingTop(l); break;
	case CSS_PROP_PADDING_RIGHT:
	    style->setPaddingRight(l); break;
	case CSS_PROP_PADDING_BOTTOM:
	    style->setPaddingBottom(l); break;
	case CSS_PROP_PADDING_LEFT:
	    style->setPaddingLeft(l); break;
	case CSS_PROP_MARGIN_TOP:
	    style->setMarginTop(l); break;
	case CSS_PROP_MARGIN_RIGHT:
	    style->setMarginRight(l); break;
	case CSS_PROP_MARGIN_BOTTOM:
	    style->setMarginBottom(l); break;
	case CSS_PROP_MARGIN_LEFT:
	    style->setMarginLeft(l); break;
	case CSS_PROP_TEXT_INDENT:
	    style->setTextIndent(l); break;
	default:
	    return;
	}
    }

    case CSS_PROP_MAX_HEIGHT:
	// +inherit +none !can be calculted directly!
	if(primitiveValue && primitiveValue->getIdent() == CSS_VAL_NONE)
	    apply = true;
    case CSS_PROP_HEIGHT:
    case CSS_PROP_MIN_HEIGHT:
	// +inherit +auto !can be calculted directly!
	if(!prop->m_id == CSS_PROP_MAX_HEIGHT && primitiveValue &&
	   primitiveValue->getIdent() == CSS_VAL_AUTO)
	    apply = true;
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    // ###
	    //l = e->parentNode->style()->....());
	}
	if(primitiveValue && !apply)
	{
	    int type = primitiveValue->primitiveType();
	    if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
		l = Length(computeLength(primitiveValue, style), Fixed);
	    else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
	    {
		// ### compute from parents height!!!
		l = Length((int)primitiveValue->getFloatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
	    }
	    else
		return;
	    apply = true;
	}
	if(!apply) return;
	switch(prop->m_id)
	{
	case CSS_PROP_MAX_HEIGHT:
	    style->setMaxHeight(l); break;
	case CSS_PROP_HEIGHT:
	    style->setHeight(l); break;
	case CSS_PROP_MIN_HEIGHT:
	    style->setMinHeight(l); break;
	default:
	    return;
	}
	return;

	break;
	
    case CSS_PROP_VERTICAL_ALIGN:
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setVerticalAlign(e->parentNode()->style()->verticalAlign());
	    return;
	}
	if(!primitiveValue) return;
	if(primitiveValue->getIdent())
	{
	    khtml::EVerticalAlign align;
	    switch(primitiveValue->getIdent())
	    {
	    case CSS_VAL_TOP:
		align = TOP; break;
	    case CSS_VAL_BOTTOM:
		align = BOTTOM; break;
	    case CSS_VAL_MIDDLE:
		align = MIDDLE; break;
	    case CSS_VAL_BASELINE:
		align = BASELINE; break;
	    case CSS_VAL_TEXT_BOTTOM:
		align = TEXT_BOTTOM; break;
	    case CSS_VAL_TEXT_TOP:
		align = TEXT_TOP; break;
	    case CSS_VAL_SUB:
		align = SUB; break;
	    case CSS_VAL_SUPER:
		align = SUPER; break;
	    default:
		return;
	    }
	    style->setVerticalAlign(align);
	    return;
	}
	break;
	
    case CSS_PROP_FONT_SIZE:
    {
	QFont f = style->font();
	QFontDatabase db;
	int oldSize;
	float size = 0;
	QValueList<int> standardSizes = e->ownerDocument()->view()->part()->fontSizes();
	if(e->parentNode())
	{
	    QFontInfo fi(e->parentNode()->style()->font());
	    oldSize = fi.pointSize();
	}
	else
	    oldSize = standardSizes[3];
	
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    size = oldSize;
	}
	else if(primitiveValue->getIdent())
	{
	    switch(primitiveValue->getIdent())
	    {
	    case CSS_VAL_XX_SMALL:
		size = standardSizes[0]; break;
	    case CSS_VAL_X_SMALL:
		size = standardSizes[1]; break;
	    case CSS_VAL_SMALL:
		size = standardSizes[2]; break;
	    case CSS_VAL_MEDIUM:
		size = standardSizes[3]; break;
	    case CSS_VAL_LARGE:
		size = standardSizes[4]; break;
	    case CSS_VAL_X_LARGE:
		size = standardSizes[5]; break;
	    case CSS_VAL_XX_LARGE:
		size = standardSizes[6]; break;
	    case CSS_VAL_LARGER:
		// ### use the next bigger standardSize!!!
		size = oldSize * 1.2;
		break;
	    case CSS_VAL_SMALLER:
		size = oldSize / 1.2;
		break;
	    default:
		return;
	    }

     	}
	else
	{
	    int type = primitiveValue->primitiveType();
	    RenderStyle *parentStyle = style; // use the current style as fallback in case we have no parent
	    if(e->parentNode()) parentStyle = e->parentNode()->style();
	    if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
		size = computeLength(primitiveValue, parentStyle);
	    else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
		size = (int)(primitiveValue->getFloatValue(CSSPrimitiveValue::CSS_PERCENTAGE)
				  * parentStyle->font().pointSize() / 100.);
	    else
		return;
	}

	if(size <= 0) return;

	// we never want to get smaller than 7 points to keep fonts readable
	if(size < 7 ) size = 7;
	
	//kdDebug( 6080 ) << "computed raw font size: " << size << endl;

	// ok, now some magic to get a nice unscaled font
	// ### all other font properties should be set before this one!!!!
	// ####### make it use the charset needed!!!!
	if( !db.isSmoothlyScalable(f.family(), db.styleString(f), "iso8859-1") )
	{
	    QValueList<int> pointSizes = db.smoothSizes(f.family(), db.styleString(f), "iso8859-1");
	    // lets see if we find a nice looking font, which is not too far away
	    // from the requested one.
	
	    QValueList<int>::Iterator it;
	    float diff = 1; // ### 100% deviation
	    int bestSize = 0;
	    for( it = pointSizes.begin(); it != pointSizes.end(); ++it )
	    {
		float newDiff = ((*it) - size)/size;
		//kdDebug( 6080 ) << "smooth font size: " << *it << " diff=" << newDiff << endl;
		if(newDiff < 0) newDiff = -newDiff;
		if(newDiff < diff)
		{
		    diff = newDiff;
		    bestSize = *it;
		}
	    }
	    //kdDebug( 6080 ) << "best smooth font size: " << bestSize << " diff=" << diff << endl;
	    if(diff < .15) // 15% deviation, otherwise we use a scaled font...
		size = bestSize;
	}	
	f.setPointSize(size);
	//KGlobal::charsets()->setQFont(f, e->ownerDocument()->view()->part()->settings()->charset);
	style->setFont(f);
	return;
    }
	
// angle
    case CSS_PROP_ELEVATION:
	
// number
    case CSS_PROP_FONT_SIZE_ADJUST:
    case CSS_PROP_ORPHANS:
    case CSS_PROP_PITCH_RANGE:
    case CSS_PROP_RICHNESS:
    case CSS_PROP_SPEECH_RATE:
    case CSS_PROP_STRESS:
    case CSS_PROP_WIDOWS:
    case CSS_PROP_Z_INDEX:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setZIndex(e->parentNode()->style()->zIndex());
	    return;
	}
	if(!primitiveValue ||
	   primitiveValue->primitiveType() != CSSPrimitiveValue::CSS_NUMBER)
	    return;
	style->setZIndex((int)primitiveValue->getFloatValue(CSSPrimitiveValue::CSS_NUMBER));
	return;
    }
	
// length, percent, number
    case CSS_PROP_LINE_HEIGHT:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setLineHeight(e->parentNode()->style()->lineHeight());
	    return;
	}
	Length lineHeight;
	if(!primitiveValue) return;
	int type = primitiveValue->primitiveType();
	if(primitiveValue->getIdent() == CSS_VAL_NORMAL)
	    lineHeight = Length(100, Percent);
	else if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
		lineHeight = Length(computeLength(primitiveValue, style), Fixed);
	else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
	    lineHeight = Length(primitiveValue->getFloatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
	else if(type == CSSPrimitiveValue::CSS_NUMBER)
	    lineHeight = Length(primitiveValue->getFloatValue(CSSPrimitiveValue::CSS_NUMBER)*100, Percent);
	else
	    return;
	style->setLineHeight(lineHeight);
	return;
    }

// number, percent
    case CSS_PROP_VOLUME:
	
// frequency
    case CSS_PROP_PITCH:

// string
    case CSS_PROP_TEXT_ALIGN:
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setTextAlign(e->parentNode()->style()->textAlign());
	    return;
	}
	if(!primitiveValue) return;
	if(primitiveValue->getIdent())
	{
	    khtml::ETextAlign align;
	    switch(primitiveValue->getIdent())
	    {
	    case CSS_VAL_LEFT:
		align = LEFT; break;
	    case CSS_VAL_RIGHT:
		align = RIGHT; break;
	    case CSS_VAL_CENTER:
		align = CENTER; break;
	    case CSS_VAL_JUSTIFY:
		align = JUSTIFY; break;
	    default:
		return;
	    }
	    style->setTextAlign(align);
	    return;
	}
    }
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
	if(!value->isValueList()) return;
	CSSValueListImpl *list = static_cast<CSSValueListImpl *>(value);
	int len = list->length();
	for(int i = 0; i < len; i++)
	{
	    // yuck, what an ugly construct...
	    const QString *families = e->ownerDocument()->view()->part()->settings()->families();
	    QFont f = style->font();
	    CSSValueImpl *item = list->item(i);
	    if(!item->isPrimitiveValue()) continue;
	    CSSPrimitiveValueImpl *val = static_cast<CSSPrimitiveValueImpl *>(item);
	    if(!val->primitiveType() == CSSPrimitiveValue::CSS_STRING) return;
	    DOMStringImpl *str = val->getStringValue();
	    QString face(str->s, str->l);
	    // ### use the fonts in settings!!!!
	    if(face == "serif")
		face = families[0];
	    else if(face == "sans-serif")
		face = families[2];
	    else if( face == "cursive")
		face = families[3];
	    else if( face == "fantasy")
		face = families[4];
	    else if( face == "monospace")
		face = families[5];
	    f.setFamily(face);
	    QFontInfo fi(f);
	    if(!strcasecmp(fi.family().ascii(), face.ascii()))
	    {
		//kdDebug( 6080 ) << "=====> setting font family to " << face << endl;
		//KGlobal::charsets()->setQFont(f, e->ownerDocument()->view()->part()->settings()->charset);
		style->setFont(f);
		break;
	    }
	    //kdDebug( 6080 ) << "no match for font family " << face << ", got " << fi.family() << endl;
	}
	break;
    }
    case CSS_PROP_QUOTES:
	// list of strings or i
    case CSS_PROP_SIZE:
	// ### look up
      break;
    case CSS_PROP_TEXT_DECORATION:
	// list of ident
	// ### no list at the moment
    {
	if(value->valueType() == CSSValue::CSS_INHERIT)
	{
	    if(!e->parentNode()) return;
	    style->setTextDecoration(e->parentNode()->style()->textDecoration());
	    style->setTextDecorationColor(e->parentNode()->style()->textDecorationColor());
	    return;
	}
	int t = TDNONE;
	if(!value->isValueList()) return;
	CSSValueListImpl *list = static_cast<CSSValueListImpl *>(value);
	int len = list->length();
	for(int i = 0; i < len; i++)
	{
	    CSSValueImpl *item = list->item(i);
	    if(!item->isPrimitiveValue()) continue;
	    primitiveValue = static_cast<CSSPrimitiveValueImpl *>(item);
	    switch(primitiveValue->getIdent())
	    {
	    case CSS_VAL_NONE:
		t = TDNONE; break;
	    case CSS_VAL_UNDERLINE:
		t |= UNDERLINE; break;
	    case CSS_VAL_OVERLINE:
		t |= OVERLINE; break;
	    case CSS_VAL_LINE_THROUGH:
		t |= LINE_THROUGH; break;
	    case CSS_VAL_BLINK:
		t |= BLINK; break;
	    default:
		return;
	    }
	    style->setTextDecoration(t);
	    style->setTextDecorationColor(style->color());
	}
	break;
    }
	
    case CSS_PROP_VOICE_FAMILY:
	// list of strings and i
	break;
	
// shorthand properties
    case CSS_PROP_BACKGROUND:
	// ### inherit
	if(value->valueType() != CSSValue::CSS_INHERIT || !e->parentNode()) return;	
	style->setBackgroundColor(e->parentNode()->style()->backgroundColor());
	style->setBackgroundImage(e->parentNode()->style()->backgroundImage());
	style->setBackgroundRepeat(e->parentNode()->style()->backgroundRepeat());
	style->setBackgroundAttachment(e->parentNode()->style()->backgroundAttachment());
//	style->setBackgroundPosition(e->parentNode()->style()->backgroundPosition());
	
	break;
    case CSS_PROP_BORDER_COLOR:
	if(primitiveValue && primitiveValue->getIdent() == CSS_VAL_TRANSPARENT)
	{
	    style->setBorderTopColor(QColor());
	    style->setBorderBottomColor(QColor());
	    style->setBorderLeftColor(QColor());
	    style->setBorderRightColor(QColor());
	    return;
	}
    case CSS_PROP_BORDER:
    case CSS_PROP_BORDER_STYLE:
    case CSS_PROP_BORDER_WIDTH:
	if(value->valueType() != CSSValue::CSS_INHERIT || !e->parentNode()) return;
	
	if(prop->m_id == CSS_PROP_BORDER || prop->m_id == CSS_PROP_BORDER_COLOR)
	{
	    style->setBorderTopColor(e->parentNode()->style()->borderTopColor());
	    style->setBorderBottomColor(e->parentNode()->style()->borderBottomColor());
	    style->setBorderLeftColor(e->parentNode()->style()->borderLeftColor());
	    style->setBorderRightColor(e->parentNode()->style()->borderRightColor());
	}
	if(prop->m_id == CSS_PROP_BORDER || prop->m_id == CSS_PROP_BORDER_STYLE)
	{
	    style->setBorderTopStyle(e->parentNode()->style()->borderTopStyle());
	    style->setBorderBottomStyle(e->parentNode()->style()->borderBottomStyle());
	    style->setBorderLeftStyle(e->parentNode()->style()->borderLeftStyle());
	    style->setBorderRightStyle(e->parentNode()->style()->borderRightStyle());
	}
	if(prop->m_id == CSS_PROP_BORDER || prop->m_id == CSS_PROP_BORDER_WIDTH)
	{
	    style->setBorderTopWidth(e->parentNode()->style()->borderTopWidth());
	    style->setBorderBottomWidth(e->parentNode()->style()->borderBottomWidth());
	    style->setBorderLeftWidth(e->parentNode()->style()->borderLeftWidth());
	    style->setBorderRightWidth(e->parentNode()->style()->borderRightWidth());
	}
	return;
    case CSS_PROP_BORDER_TOP:
	if(value->valueType() != CSSValue::CSS_INHERIT || !e->parentNode()) return;
	style->setBorderTopColor(e->parentNode()->style()->borderTopColor());
	style->setBorderTopStyle(e->parentNode()->style()->borderTopStyle());
	style->setBorderTopWidth(e->parentNode()->style()->borderTopWidth());
	return;
    case CSS_PROP_BORDER_RIGHT:
	if(value->valueType() != CSSValue::CSS_INHERIT || !e->parentNode()) return;
	style->setBorderRightColor(e->parentNode()->style()->borderRightColor());
	style->setBorderRightStyle(e->parentNode()->style()->borderRightStyle());
	style->setBorderRightWidth(e->parentNode()->style()->borderRightWidth());
	return;
    case CSS_PROP_BORDER_BOTTOM:
	if(value->valueType() != CSSValue::CSS_INHERIT || !e->parentNode()) return;
	style->setBorderBottomColor(e->parentNode()->style()->borderBottomColor());
	style->setBorderBottomStyle(e->parentNode()->style()->borderBottomStyle());
	style->setBorderBottomWidth(e->parentNode()->style()->borderBottomWidth());
	return;
    case CSS_PROP_BORDER_LEFT:
	if(value->valueType() != CSSValue::CSS_INHERIT || !e->parentNode()) return;
	style->setBorderLeftColor(e->parentNode()->style()->borderLeftColor());
	style->setBorderLeftStyle(e->parentNode()->style()->borderLeftStyle());
	style->setBorderLeftWidth(e->parentNode()->style()->borderLeftWidth());
	return;
    case CSS_PROP_MARGIN:
	if(value->valueType() != CSSValue::CSS_INHERIT || !e->parentNode()) return;
	style->setMarginTop(e->parentNode()->style()->marginTop());
	style->setMarginBottom(e->parentNode()->style()->marginBottom());
	style->setMarginLeft(e->parentNode()->style()->marginLeft());
	style->setMarginRight(e->parentNode()->style()->marginRight());
	return;
    case CSS_PROP_PADDING:
	if(value->valueType() != CSSValue::CSS_INHERIT || !e->parentNode()) return;
	style->setPaddingTop(e->parentNode()->style()->paddingTop());
	style->setPaddingBottom(e->parentNode()->style()->paddingBottom());
	style->setPaddingLeft(e->parentNode()->style()->paddingLeft());
	style->setPaddingRight(e->parentNode()->style()->paddingRight());
	return;


    case CSS_PROP_CUE:
    case CSS_PROP_FONT:
    case CSS_PROP_LIST_STYLE:
    case CSS_PROP_OUTLINE:
    case CSS_PROP_PAUSE:
	break;
    default:
	return;
    }
}

