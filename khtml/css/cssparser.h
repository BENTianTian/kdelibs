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
#ifndef _CSS_cssparser_h_
#define _CSS_cssparser_h_

#include "dom_string.h"
#include "dom_misc.h"
#include <qlist.h>

namespace DOM {

    class StyleSheetImpl;
    class MediaList;

    class CSSSelector;
    class CSSProperty;
    class CSSValueImpl;
    class CSSStyleDeclarationImpl;
    class CSSRuleImpl;
    class CSSStyleRuleImpl;


    int getPropertyID(const char *tagStr, int len);

    // a style class which has a parent (almost all have)
    class StyleBaseImpl : public DomShared
    {
    public:
	StyleBaseImpl() { m_parent = 0; }
	StyleBaseImpl(StyleBaseImpl *p) { m_parent = p; }
	virtual ~StyleBaseImpl() {}

	virtual bool deleteMe();

	virtual bool isStyleSheet() { return false; }
	virtual bool isCSSStyleSheet() { return false; }
	virtual bool isStyleSheetList() { return false; }
	virtual bool isMediaList() { return false; }
	virtual bool isRuleList() { return false; }
	virtual bool isRule() { return false; }
	virtual bool isStyleRule() { return false; }
	virtual bool isCharetRule() { return false; }
	virtual bool isImportRule() { return false; }
	virtual bool isMediaRule() { return false; }
	virtual bool isFontFaceRule() { return false; }
	virtual bool isPageRule() { return false; }
	virtual bool isUnknownRule() { return false; }
	virtual bool isStyleDeclaration() { return false; }
	virtual bool isValue() { return false; }
	virtual bool isPrimitiveValue() { return false; }
	virtual bool isValueList() { return false; }
	virtual bool isValueCustom() { return false; }

	void setParent(StyleBaseImpl *parent);

	const QChar *parseSpace(const QChar *curP, const QChar *endP);
	const QChar *parseToChar(const QChar *curP, const QChar *endP,
				 QChar c, bool chkws);

	CSSSelector *parseSelector2(const QChar *curP, const QChar *endP);
	CSSSelector *parseSelector1(const QChar *curP, const QChar *endP);
	QList<CSSSelector> *parseSelector(const QChar *curP, const QChar *endP);

	CSSProperty *parseProperty(const QChar *curP, const QChar *endP);
	QList<CSSProperty> *parseProperties(const QChar *curP, const QChar *endP);

	/* parses generic CSSValues */
	CSSValueImpl *parseValue(const QChar *curP, const QChar *endP, int propId);

	// defines units allowed for a certain property, used in parseUnit
	enum Units
	{
	    INTEGER   = 0x0001,
	    NUMBER    = 0x0002,  // real numbers
	    PERCENT   = 0x0004,
	    LENGTH    = 0x0008,
	    ANGLE     = 0x0010,
	    TIME      = 0x0020,
	    FREQUENCY = 0x0040
	};
	
	/* called by parseValue, parses numbers+units */
	CSSValueImpl *parseUnit(const QChar * curP, const QChar *endP, int allowedUnits);
	
	CSSRuleImpl *parseAtRule(const QChar *&curP, const QChar *endP);
	CSSStyleRuleImpl *parseStyleRule(const QChar *&curP, const QChar *endP);
	CSSRuleImpl *parseRule(const QChar *&curP, const QChar *endP);

	virtual bool parseString(const DOMString &/*cssString*/) { return false; }

    protected:
	StyleBaseImpl *m_parent;
    };

    // a style class which has a list of children (StyleSheets for example)
    class StyleListImpl : public StyleBaseImpl
    {
    public:
	StyleListImpl() : StyleBaseImpl() { m_lstChildren = 0; }
	StyleListImpl(StyleBaseImpl *parent) : StyleBaseImpl(parent) { m_lstChildren = 0; }

	virtual ~StyleListImpl();

    protected:
	QList<StyleBaseImpl> *m_lstChildren;
    };


// this class represents a selector for a StyleRule
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
    // ### change to DOMString
    QString      value;

    enum Relation
    {
	Descendant = 0,
	Child,
	Sibling
    };

    Relation relation;
    CSSSelector *tagHistory;

    int specificity();
};

// another helper class
class CSSProperty
{
public:
    CSSProperty()
    {
	m_id = -1;
	m_value = 0;
	m_bImportant = false;
    }
    ~CSSProperty();

    void setValue(CSSValueImpl *val);
    CSSValueImpl *value();

    int   m_id;
    bool m_bImportant;
protected:
    CSSValueImpl *m_value;
};

}; // namespace

#endif



