/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
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
#ifndef _DOM_ELEMENTImpl_h_
#define _DOM_ELEMENTImpl_h_

#include "dom_nodeimpl.h"
#include "dom/dom_element.h"

namespace DOM {

class ElementImpl;
class DocumentImpl;
class NamedAttrMapImpl;

class AttrImpl : public NodeImpl
{
    friend class ElementImpl;
    friend class NamedAttrMapImpl;

public:
    AttrImpl();
    AttrImpl(DocumentPtr *doc, const DOMString &name);
    AttrImpl(DocumentPtr *doc, int id);
    AttrImpl(DocumentPtr *doc, const DOMString &namespaceURI,
             const DOMString &qualifiedName );
    AttrImpl(const AttrImpl &other);

    AttrImpl &operator = (const AttrImpl &other);
    ~AttrImpl();

    // DOM methods & attributes for Attr
    DOMString name() const;
    bool specified() const { return m_specified; }
    Element ownerElement() const;
    virtual DOMString value() const;
    virtual void setValue( const DOMString &v, int &exceptioncode );

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual DOMString namespaceURI() const;
    virtual DOMString prefix() const;
    virtual void setPrefix(const DOMString &_prefix, int &exceptioncode );

    virtual DOMString nodeValue() const { return value(); }
    virtual void setNodeValue( const DOMString &, int &exceptioncode );
    virtual NodeImpl *parentNode() const;
    virtual NodeImpl *previousSibling() const;
    virtual NodeImpl *nextSibling() const;
    virtual NodeImpl *cloneNode ( bool deep, int &exceptioncode );

    // Other methods (not part of DOM)
    virtual bool isAttributeNode() const { return true; }
    virtual bool deleteMe();
    DOMStringImpl *val() { return _value; }
    virtual bool childAllowed( NodeImpl *newChild );
    virtual bool childTypeAllowed( unsigned short type );

protected:
    AttrImpl(const DOMString &name, const DOMString &value, DocumentPtr *doc);
    AttrImpl(int _id, const DOMString &value, DocumentPtr *doc);
    void setName(const DOMString &n);

    ElementImpl *_element;

#if 0
    NodeImpl::Id m_id;
//     DOMStringImpl *_name;
     DOMStringImpl *_value;
//     DOMStringImpl *_namespaceURI;

//    unsigned short m_prefixId;
#else
    DOMStringImpl* _name;
    DOMStringImpl* _value;
    DOMStringImpl* _namespaceURI;

public:
     unsigned short attrId;
#endif
};


class ElementImpl : public NodeBaseImpl
{
    friend class DocumentImpl;
    friend class NamedAttrMapImpl;
    friend class AttrImpl;

public:
    ElementImpl(DocumentPtr *doc);
    ~ElementImpl();

    // DOM methods & attributes for Element

    virtual DOMString tagName() const;
    virtual DOMString getAttribute ( const DOMString &name, int &exceptioncode ) const;
    virtual void setAttribute ( const DOMString &name, const DOMString &value, int &exceptioncode );
    virtual void removeAttribute ( const DOMString &name, int &exceptioncode );
    virtual AttrImpl *getAttributeNode ( const DOMString &name, int &exceptioncode );
    virtual Attr setAttributeNode ( AttrImpl *newAttr, int &exceptioncode );
    virtual Attr removeAttributeNode ( AttrImpl *oldAttr, int &exceptioncode );
    virtual DOMString getAttributeNS ( const DOMString &namespaceURI, const DOMString &localName,
                                       int &exceptioncode );
    virtual void setAttributeNS ( const DOMString &namespaceURI, const DOMString &qualifiedName,
                                  const DOMString &value, int &exceptioncode );
    virtual void removeAttributeNS ( const DOMString &namespaceURI, const DOMString &localName,
                                     int &exceptioncode );
    virtual AttrImpl *getAttributeNodeNS ( const DOMString &namespaceURI, const DOMString &localName,
                                           int &exceptioncode );
    virtual AttrImpl *setAttributeNodeNS ( AttrImpl *newAttr, int &exceptioncode );
    virtual bool hasAttribute ( const DOMString &name, int &exceptioncode ) const;
    virtual bool hasAttributeNS( const DOMString &namespaceURI, const DOMString &localName,
                                 int &exceptioncode );

    // DOM methods overridden from  parent classes
    virtual NodeImpl *cloneNode ( bool deep, int &exceptioncode );
    virtual DOMString nodeName() const;
    virtual DOMString prefix() const;
    virtual void setPrefix(const DOMString &_prefix, int &exceptioncode );

    // Other methods (not part of DOM)

    // convenience methods which ignore exceptions
    DOMString getAttribute ( const DOMString &name ) const;
    void setAttribute ( const DOMString &name, const DOMString &value);

    virtual bool isInline() const;

    virtual unsigned short nodeType() const;
    virtual bool isElementNode() const { return true; }

    virtual bool isHTMLElement() const { return false; }

    virtual NamedNodeMapImpl *attributes() const;
    virtual bool hasAttributes() const;

    /**
     * override this in subclasses if you need to parse
     * attributes. This is always called, whenever an attribute changed
      */
    virtual void parseAttribute(AttrImpl *) {}

    // not part of the DOM
    DOMString getAttribute ( int id ) const;
    AttrImpl *getAttributeNode ( int index ) const;
    int getAttributeCount() const;
    void setAttribute ( int id, const DOMString &value );
    void setAttributeMap ( NamedAttrMapImpl* list );

    // State of the element.
    virtual QString state() { return QString::null; }

    virtual void attach();
    virtual void detach();
    virtual void recalcStyle( StyleChange = NoChange );

    virtual void setFocus(bool = true);
    virtual void setActive(bool = true);
    virtual void mouseEventHandler( MouseEvent */*ev*/, bool /*inside*/ ) {};
    virtual khtml::FindSelectionResult findSelectionNode( int _x, int _y, int _tx, int _ty,
                                                   DOM::Node & node, int & offset );
    virtual bool isSelectable() const;
    virtual bool childAllowed( NodeImpl *newChild );
    virtual bool childTypeAllowed( unsigned short type );

    void createDecl();
    virtual DOM::CSSStyleDeclarationImpl *styleRules() {
      if (!m_styleDecls) createDecl();
      return m_styleDecls;
    }

    void dispatchAttrRemovalEvent(NodeImpl *attr);
    void dispatchAttrAdditionEvent(NodeImpl *attr);

    virtual void dump(QTextStream *stream, QString ind = "") const;

protected: // member variables

    friend class NodeImpl;
    mutable NamedAttrMapImpl *namedAttrMap;

    // map of default attributes. derived element classes are responsible
    // for setting this according to the corresponding element description
    // in the DTD
    virtual NamedAttrMapImpl* defaultMap() const;
    DOM::CSSStyleDeclarationImpl *m_styleDecls;
    DOMStringImpl *m_prefix;
};


class XMLElementImpl : public ElementImpl
{

public:
    XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_tagName);
    XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_qualifiedName, DOMStringImpl *_namespaceURI);
    ~XMLElementImpl();

    // DOM methods overridden from  parent classes

    virtual DOMString namespaceURI() const;
    virtual DOMString localName() const;
    virtual NodeImpl *cloneNode ( bool deep, int &exceptioncode );

    // Other methods (not part of DOM)
    virtual bool isXMLElementNode() const { return true; }
    virtual Id id() const { return m_id; }

protected:
    Id m_id;
};



class NamedAttrMapImpl : public NamedNodeMapImpl
{
    friend class ElementImpl;
public:
    NamedAttrMapImpl(ElementImpl *e);
    virtual ~NamedAttrMapImpl();
    NamedAttrMapImpl &operator =(const NamedAttrMapImpl &other);

    // DOM methods & attributes for NamedNodeMap

    NodeImpl *getNamedItem ( const DOMString &name, int &exceptioncode ) const;

    Node setNamedItem ( const Node &arg, int &exceptioncode );

    Node removeNamedItem ( const DOMString &name, int &exceptioncode );

    NodeImpl *item ( unsigned long index ) const;

    unsigned long length(  ) const;

    virtual NodeImpl *getNamedItemNS( const DOMString &namespaceURI, const DOMString &localName,
                                      int &exceptioncode ) const;

    virtual NodeImpl *setNamedItemNS( NodeImpl *arg, int &exceptioncode );

    virtual NodeImpl *removeNamedItemNS( const DOMString &namespaceURI, const DOMString &localName,
                                         int &exceptioncode );

    // Other methods (not part of DOM)

    AttrImpl *getIdItem ( int id ) const;
    Attr setIdItem ( AttrImpl *attr, int& exceptioncode );
    Attr removeIdItem ( int id );


    // only use this during parsing !
    void insertAttr(AttrImpl* newAtt);
    void clearAttrs();

    void detachFromElement();

protected:
    // generic functions for accessing attributes - can be used with different compare
    // types (id, name or name + namespace)
    enum AttrCompare {
        ID_COMPARE,
        NAME_COMPARE,
        NAME_NAMESPACE_COMPARE
    };

    AttrImpl *getItem ( int id, const DOMString &name, const DOMString &namespaceURI,
                        AttrCompare compareType, int &exceptioncode ) const;
    Attr setItem ( const Node &arg, AttrCompare compareType, int &exceptioncode );
    Attr removeItem ( int id, const DOMString &name, const DOMString &namespaceURI,
                      AttrCompare compareType, int &exceptioncode );
    int findAttr ( int id, const DOMString &name, const DOMString &namespaceURI,
                   AttrCompare compareType ) const;

    Attr replaceAttr(int i, AttrImpl *attr);
    void addAttr(AttrImpl *attr);
    Attr removeAttr(int index);

    Attr removeAttr( AttrImpl *oldAttr, int &exceptioncode );

    ElementImpl *element;
    AttrImpl **attrs;
    uint len;
};

}; //namespace

#endif
