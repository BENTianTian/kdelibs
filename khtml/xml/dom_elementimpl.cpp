/**
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
 */

//#define EVENT_DEBUG
#include "dom/dom_exception.h"
#include "dom/dom_node.h"
#include "xml/dom_textimpl.h"
#include "xml/dom_docimpl.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_elementimpl.h"

#include "html/dtd.h"
#include "html/htmlparser.h"

#include "rendering/render_object.h"
#include "misc/htmlhashes.h"
#include "css/css_valueimpl.h"
#include "css/css_stylesheetimpl.h"
#include "css/cssstyleselector.h"
#include "xml/dom_xmlimpl.h"

#include <qtextstream.h>
#include <kdebug.h>

using namespace DOM;
using namespace khtml;

/*
 * NOTE:
 * According to the DOM docs, an Attr stores the value directly in it's parsed
 * form, but for values containing entities, creates a subtree with nodes
 * containing the unexpanded form (for XML). On read, the value is always
 * returned entity-free, so we decided for HTML we could only store a
 * parsed DOMString and have no child-nodes.
 */

AttrImpl::AttrImpl()
    : NodeImpl(0),
     _element(0),
    _name(0),
    _value(0),
    _namespaceURI(0),
    attrId(0)
{
}


AttrImpl::AttrImpl(DocumentPtr *doc, const DOMString &name)
    : NodeImpl(doc),
    _element(0),
    _name(0),
    _value(0),
    _namespaceURI(0),
    attrId(0)
{
    setName(name);
}

AttrImpl::AttrImpl(DocumentPtr *doc, int id)
    : NodeImpl(doc),
    _element(0),
    _name(0),
    _value(0),
    _namespaceURI(0),
    attrId(id)
{
}

AttrImpl::AttrImpl(DocumentPtr *doc, const DOMString &/*namespaceURI*/,
                   const DOMString &/*qualifiedName*/)
    : NodeImpl(doc),
    _element(0),
    _name(0),
    _value(0),
    _namespaceURI(0),
    attrId(0)
{
    // ### implement properly!
}

AttrImpl::AttrImpl(const AttrImpl &other) : NodeImpl(other.docPtr())
{
    m_specified = other.specified();
    _element = other._element;
    _name = other._name;
    if (_name) _name->ref();
    _value = other._value;
    if (_value) _value->ref();
    _namespaceURI = other._namespaceURI;
    if (_namespaceURI) _namespaceURI->ref();
    attrId = other.attrId;
}

AttrImpl &AttrImpl::operator = (const AttrImpl &other)
{
    NodeImpl::operator =(other);
    m_specified = other.specified();
    _element = other._element;

    if (_name) _name->deref();
    _name = other._name;
    if (_name) _name->ref();
    if (_value) _value->deref();
    _value = other._value;
    if (_value) _value->ref();
    _namespaceURI = other._namespaceURI;
    if (_namespaceURI) _namespaceURI->ref();
    attrId = other.attrId;
    return *this;
}

AttrImpl::~AttrImpl()
{
    if(_name) _name->deref();
    if(_value) _value->deref();
    if(_namespaceURI) _namespaceURI->deref();
}

DOMString AttrImpl::nodeName() const
{
    return name();
}

unsigned short AttrImpl::nodeType() const
{
    return Node::ATTRIBUTE_NODE;
}

DOMString AttrImpl::namespaceURI() const
{
    // ### make sure it is copied properly during a clone
    return _namespaceURI;
}

DOMString AttrImpl::prefix() const
{
    // ### implement
    return DOMString();
}

void AttrImpl::setPrefix(const DOMString &_prefix, int &exceptioncode )
{
    checkSetPrefix(_prefix, exceptioncode);
    if (exceptioncode)
        return;

    // ### implement
}

Element AttrImpl::ownerElement() const
{
    return _element;
}

DOMString AttrImpl::name() const
{
    // For XML/XHTML documents, XHTML attribute names are all lowercase and case sensitive.
    // For HTML documents,attribute names are case-insensitive but are stored using
    // their uppercase form

    if(attrId) {
        if (ownerDocument()->htmlMode() != DocumentImpl::XHtml)
            return getAttrName(attrId);
        else
            return getAttrName(attrId).lower();
    }
    else if (_name)
        return _name;
    else
        return DOMString();
}

void AttrImpl::setName(const DOMString &n)
{
    if(_name) _name->deref();
    _name = n.implementation();
    if(!_name) {
        attrId = 0;
        return;
    }

    // ### this will break for XML! we need to store the actual name
    attrId = khtml::getAttrID(QConstString(_name->s, _name->l).string().lower().ascii(), _name->l);
    if (attrId)
        _name = 0;
    else
        _name->ref();
}

DOMString AttrImpl::value() const {
    return _value;
}

void AttrImpl::setValue( const DOMString &v, int &exceptioncode )
{
    exceptioncode = 0;

    // according to the DOM docs, we should create an unparsed Text child
    // node here; we decided this was not necessary for HTML


    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }
    // ### TODO: parse value string, interprete entities (not sure if we are supposed to do this)

    DOMStringImpl *prevValue = _value;
    _value = v.implementation();
    if (_value) _value->ref();
    m_specified = true;

    if (_element) {
        _element->parseAttribute(this);
        _element->setChanged(true);
	if (getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER)) {
	    int exceptioncode = 0;
	    _element->dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,this,prevValue,
				    _value,_name,MutationEvent::MODIFICATION),exceptioncode,true);
	}
    }
    if (prevValue) prevValue->deref();
}

void AttrImpl::setNodeValue( const DOMString &v, int &exceptioncode )
{
    exceptioncode = 0;
    // NO_MODIFICATION_ALLOWED_ERR: taken care of by setValue()
    setValue(v, exceptioncode);
}

AttrImpl::AttrImpl(const DOMString &name, const DOMString &value, DocumentPtr *doc)
    : NodeImpl(doc)
{
    attrId = 0;
    _name = 0;
    setName(name);
    _value = value.implementation();
    if (_value) _value->ref();
    _namespaceURI = 0;
    _element = 0;
    m_specified = true;
}

AttrImpl::AttrImpl(int _id, const DOMString &value, DocumentPtr *doc)
    : NodeImpl(doc)
{
    attrId = _id;
    _name = 0;
    _value = value.implementation();
    if (_value) _value->ref();
    _namespaceURI = 0;
    _element = 0;
    m_specified = false;
}

NodeImpl *AttrImpl::parentNode() const
{
    return 0;
}

NodeImpl *AttrImpl::previousSibling() const
{
    return 0;
}

NodeImpl *AttrImpl::nextSibling() const
{
    return 0;
}

NodeImpl *AttrImpl::cloneNode ( bool /*deep*/, int &/*exceptioncode*/ )
{
    AttrImpl *newImpl = new AttrImpl(*this);
    newImpl->_element = 0; // can't have two attributes with the same name/id attached to an element
    return newImpl;
}

bool AttrImpl::deleteMe()
{
    if(!_element && _ref <= 0) return true;
    return false;
}

// DOM Section 1.1.1
bool AttrImpl::childAllowed( NodeImpl *newChild )
{
    if(!newChild)
        return false;

    return childTypeAllowed(newChild->nodeType());
}

bool AttrImpl::childTypeAllowed( unsigned short type )
{
    switch (type) {
        case Node::TEXT_NODE:
        case Node::ENTITY_REFERENCE_NODE:
            return true;
            break;
        default:
            return false;
    }
}

// -------------------------------------------------------------------------

ElementImpl::ElementImpl(DocumentPtr *doc) : NodeBaseImpl(doc)
{
    namedAttrMap = 0;
    m_styleDecls = 0;
    m_prefix = 0;
}

ElementImpl::~ElementImpl()
{
    if (m_render)
        detach();

    if(namedAttrMap) {
        namedAttrMap->detachFromElement();
        namedAttrMap->deref();
    }

    if (m_styleDecls) {
        m_styleDecls->setNode(0);
        m_styleDecls->setParent(0);
        m_styleDecls->deref();
    }

    if (m_prefix)
        m_prefix->deref();
}

DOMString ElementImpl::getAttribute( const DOMString &name, int &exceptioncode ) const
{
  // search in already set attributes first
    if(!namedAttrMap) return DOMString();
    AttrImpl *attr = static_cast<AttrImpl*>(namedAttrMap->getNamedItem(name,exceptioncode));
    if (attr) return attr->value();

    // then search in default attr in case it is not yet set
    NamedAttrMapImpl* dm = defaultMap();
    if(!dm) return DOMString();
    AttrImpl* defattr = static_cast<AttrImpl*>(dm->getNamedItem(name, exceptioncode));
    if(!defattr || exceptioncode) return DOMString();

    return defattr->value();
}

void ElementImpl::setAttribute( const DOMString &name, const DOMString &value, int &exceptioncode )
{
    // INVALID_CHARACTER_ERR: Raised if the specified name contains an illegal character.
    if (!validAttrName(name)) {
        exceptioncode = DOMException::INVALID_CHARACTER_ERR;
        return;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // If we don't already have an attribute map, create one
    if(!namedAttrMap) {
        namedAttrMap = new NamedAttrMapImpl(this);
        namedAttrMap->ref();
    }

    // Special case: if the value is null, remove the attribute
    if (value.isNull())
        namedAttrMap->removeNamedItem(name,exceptioncode);
    else {
        AttrImpl *a = static_cast<AttrImpl*>(namedAttrMap->getNamedItem(name,exceptioncode));
        if (a)
            a->setValue(value,exceptioncode);
        else
            namedAttrMap->setNamedItem(new AttrImpl(name,value,docPtr()),exceptioncode);
    }
}


void ElementImpl::removeAttribute( const DOMString &name, int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    if(!namedAttrMap) return;
    // If the attribute doesn't exist, removeAttribute does NOT emit an exception
    int dummyExceptionCode = 0;
    namedAttrMap->removeNamedItem(name,dummyExceptionCode);
}

AttrImpl *ElementImpl::getAttributeNode( const DOMString &name, int &exceptioncode )
{
    // ### do we return attribute node if it is in the default map but not specified?
    if(!namedAttrMap) return 0;
    return static_cast<AttrImpl*>(namedAttrMap->getNamedItem(name,exceptioncode));

}

Attr ElementImpl::setAttributeNode( AttrImpl *newAttr, int &exceptioncode )
{
    exceptioncode = 0;

    // Note mentioned in spec: throw NOT_FOUND_ERR is null node passed in
    if (!newAttr) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    // WRONG_DOCUMENT_ERROR and NO_MODIFICATION_ALLOWED_ERR will be handled by namedAttrMap

    // If we don't already have an attribute map, create one
    if(!namedAttrMap) {
        namedAttrMap = new NamedAttrMapImpl(this);
        namedAttrMap->ref();
    }

    // Set the value in the map
    if (newAttr->attrId)
        return namedAttrMap->setIdItem(newAttr, exceptioncode);
    else
        return namedAttrMap->setNamedItem(newAttr, exceptioncode);
}

Attr ElementImpl::removeAttributeNode( AttrImpl *oldAttr, int &exceptioncode )
{
    // ### should we replace with default in map? currently default attrs don't exist in map
    exceptioncode = 0;
    if(!namedAttrMap) return 0;
    return namedAttrMap->removeAttr(oldAttr, exceptioncode);
}

DOMString ElementImpl::getAttributeNS( const DOMString &namespaceURI, const DOMString &localName,
                                       int &exceptioncode )
{
    if (!namedAttrMap)
        return "";

    // Retrieve the attribute from the map. If there is no such attribute, we return the empty
    // string. Otherwise, return the attribute's value
    NodeImpl *attr = namedAttrMap->getNamedItemNS(namespaceURI, localName, exceptioncode);
    if (attr)
        return attr->nodeValue();
    else
        return "";
}

void ElementImpl::setAttributeNS( const DOMString &namespaceURI, const DOMString &qualifiedName,
                                  const DOMString &value, int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // WRONG_DOCUMENT_ERROR and NO_MODIFICATION_ALLOWED_ERR will be handled by namedAttrMap

    // If we don't already have an attribute map, create one
    if(!namedAttrMap) {
        namedAttrMap = new NamedAttrMapImpl(this);
        namedAttrMap->ref();
    }

    // Create the attribute and add it to the map. We keep a refcount while we're adding, so if
    // for some reason the attribute does not get added to the map (e.g. because of an exception),
    // it will be deleted afterwards
    AttrImpl *attr = new AttrImpl(docPtr(), namespaceURI, qualifiedName);
    attr->setValue(value,exceptioncode);
    if (exceptioncode) {
        delete attr;
        return;
    }
    attr->ref();
    namedAttrMap->setNamedItemNS(attr,exceptioncode);
    attr->deref();
}

void ElementImpl::removeAttributeNS( const DOMString &namespaceURI, const DOMString &localName,
                                     int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // NOT_FOUND_ERR: Raised if oldAttr is not an attribute of the element.
    // removeNamedItemNS() will take care of throwing an exception where we have a map but the
    // attribute doesn't exist in it
    // - WRONG. The DOM2 spec doesn't say that removeAttribute[NS] throws NOT_FOUND_ERR - David
    if (!namedAttrMap) {
        //exceptioncode = DOMException::NOT_FOUND_ERR;
        return;
    }

    // Remove the attribute from the map
    int dummyExceptionCode = 0;
    namedAttrMap->removeNamedItemNS(namespaceURI,localName,dummyExceptionCode);
}

AttrImpl *ElementImpl::getAttributeNodeNS ( const DOMString &namespaceURI, const DOMString &localName,
                                            int &exceptioncode )
{
    if (!namedAttrMap)
        return 0;

    return static_cast<AttrImpl*>(namedAttrMap->getNamedItemNS(namespaceURI,localName,exceptioncode));
}

AttrImpl *ElementImpl::setAttributeNodeNS ( AttrImpl *newAttr, int &exceptioncode )
{
    exceptioncode = 0;

    // Note mentioned in spec: throw NOT_FOUND_ERR is null node passed in
    if (!newAttr) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    // WRONG_DOCUMENT_ERROR and NO_MODIFICATION_ALLOWED_ERR will be handled by namedAttrMap

    // If we don't already have an attribute map, create one
    if(!namedAttrMap) {
        namedAttrMap = new NamedAttrMapImpl(this);
        namedAttrMap->ref();
    }

    // Set the value in the map
    return static_cast<AttrImpl*>(namedAttrMap->setNamedItemNS(newAttr, exceptioncode));
}


bool ElementImpl::hasAttribute( const DOMString &name, int &exceptioncode ) const
{
  // search in already set attributes first
    if(!namedAttrMap) return false;
    AttrImpl *attr = static_cast<AttrImpl*>(namedAttrMap->getNamedItem(name,exceptioncode));
    if (exceptioncode)
        return false;
    if (attr) return true;

    // then search in default attr in case it is not yet set
    NamedAttrMapImpl* dm = defaultMap();
    if(!dm) return false;
    AttrImpl* defattr = static_cast<AttrImpl*>(dm->getNamedItem(name, exceptioncode));
    if(!defattr || exceptioncode) return false;
    return true;
}


bool ElementImpl::hasAttributeNS( const DOMString &namespaceURI, const DOMString &localName, int &exceptioncode )
{
    return (getAttributeNS(namespaceURI, localName, exceptioncode) != 0);
}

DOMString ElementImpl::getAttribute ( const DOMString &name ) const
{
    int exceptioncode = 0;
    return getAttribute(name,exceptioncode);
}

void ElementImpl::setAttribute ( const DOMString &name, const DOMString &value)
{
    int exceptioncode = 0;
    setAttribute(name,value,exceptioncode);
}

bool ElementImpl::isInline() const
{
    if(!m_style) return false;
    return (m_style->display() == khtml::INLINE);
}

unsigned short ElementImpl::nodeType() const
{
    return Node::ELEMENT_NODE;
}

DOMString ElementImpl::getAttribute( int id ) const
{
    // search in already set attributes first
    if(!namedAttrMap) return DOMString();
    AttrImpl *attr = static_cast<AttrImpl*>(namedAttrMap->getIdItem(id));
    if (attr) return attr->value();

    // then search in default attr in case it is not yet set
    NamedAttrMapImpl* dm = defaultMap();
    if(!dm) return DOMString();

    AttrImpl* defattr = static_cast<AttrImpl*>(dm->getIdItem(id));
    if(!defattr) return DOMString();

    return defattr->value();
}

int ElementImpl::getAttributeCount() const
{
    return namedAttrMap ? namedAttrMap->length() : 0;
}

void ElementImpl::setAttribute( int id, const DOMString &value )
{
    if(!namedAttrMap) {
        namedAttrMap = new NamedAttrMapImpl(this);
        namedAttrMap->ref();
    }
    if (value.isNull())
        namedAttrMap->removeIdItem(id);
    else {
        int exceptioncode = 0;
        AttrImpl* a = static_cast<AttrImpl*>(namedAttrMap->getIdItem(id));
        if(a)
            a->setValue(value,exceptioncode);
        else
            namedAttrMap->setIdItem(new AttrImpl(id,value,docPtr()),exceptioncode);
    }
}

void ElementImpl::setAttributeMap( NamedAttrMapImpl* list )
{
    if(namedAttrMap)
        namedAttrMap->deref();

    namedAttrMap = list;

    if(namedAttrMap) {
        namedAttrMap->ref();
        namedAttrMap->element = this;
        unsigned int len = namedAttrMap->length();

        for(unsigned int i = 0; i < len; i++) {
            AttrImpl* a = namedAttrMap->attrs[i];
            if(a && !a->_element) {
                a->_element = this;

                parseAttribute(a);
            }
        }
    }
}

NodeImpl *ElementImpl::cloneNode ( bool deep, int &exceptioncode )
{
    exceptioncode = 0;
    ElementImpl *clone = ownerDocument()->createElement(tagName()); // ### pass exceptioncode
    if (!clone)
      return 0;

    if (exceptioncode)
        return 0;

    // clone attributes
    if(namedAttrMap)
        *(static_cast<NamedAttrMapImpl*>(clone->attributes())) = *namedAttrMap;

    if (deep)
        cloneChildNodes(clone,exceptioncode);
    return clone;
}

DOMString ElementImpl::nodeName() const
{
    return tagName();
}

DOMString ElementImpl::tagName() const
{
    DOMString tn = ownerDocument()->tagName(id());
    if (ownerDocument()->htmlMode() == DocumentImpl::XHtml)
        tn = tn.lower();

    if (m_prefix)
        return DOMString(m_prefix) + ":" + tn;

    return tn;
}

DOMString ElementImpl::prefix() const
{
    return m_prefix;
}

void ElementImpl::setPrefix( const DOMString &_prefix, int &exceptioncode )
{
    checkSetPrefix(_prefix, exceptioncode);
    if (exceptioncode)
        return;

    if (m_prefix)
        m_prefix->deref();
    m_prefix = _prefix.implementation();
    if (m_prefix)
        m_prefix->ref();
}

NamedNodeMapImpl *ElementImpl::attributes() const
{
    if(!namedAttrMap) {
        namedAttrMap = new NamedAttrMapImpl(const_cast<ElementImpl*>(this));
        namedAttrMap->ref();
    }
    return namedAttrMap;
}

bool ElementImpl::hasAttributes() const
{
    return namedAttrMap ? (namedAttrMap->length() > 0) : false;
}

AttrImpl *ElementImpl::getAttributeNode ( int index ) const
{
    return namedAttrMap ? namedAttrMap->getIdItem(index) : 0;
}

NamedAttrMapImpl* ElementImpl::defaultMap() const
{
    return 0;
}

RenderObject *ElementImpl::createRenderer()
{
    return RenderObject::createObject(this);
}

void ElementImpl::attach()
{
    if (!m_render)
    {
#if SPEED_DEBUG < 2
        setStyle(ownerDocument()->styleSelector()->styleForElement(this));
#if SPEED_DEBUG < 1
        if(_parent && _parent->renderer())
        {
            m_render = createRenderer();
            if(m_render)
            {
                m_render->setStyle(m_style);
                _parent->renderer()->addChild(m_render, nextRenderer());
            }
        }
#endif
#endif
    }

    NodeBaseImpl::attach();
}

void ElementImpl::detach()
{
    NodeBaseImpl::detach();

    if ( m_render )
        m_render->detach();

    m_render = 0;
}

void ElementImpl::recalcStyle( StyleChange change )
{
    if ( m_style ) {
	// ### should go away and be done in renderobject
	bool needsUpdate = false;
//  	qDebug("recalcStyle(%p: %s)", this, tagName().string().latin1());
	if ( change >= Inherit || changed() ) {
	    EDisplay oldDisplay = m_style ? m_style->display() : INLINE;
	
	    int dynamicState = StyleSelector::None;
	    if ( m_mouseInside )
		dynamicState |= StyleSelector::Hover;
	    if ( m_focused )
		dynamicState |= StyleSelector::Focus;
	    if ( m_active )
		dynamicState |= StyleSelector::Active;

	    RenderStyle *newStyle = ownerDocument()->styleSelector()->styleForElement(this, dynamicState);
	    StyleChange ch = diff( m_style, newStyle );
	    if ( ch != NoChange ) {
		setStyle( newStyle );

		if (oldDisplay != m_style->display()) {
		    // ### doesn't this already take care of changing the style
		    // for all children?
		    detach();
		    attach();
		    needsUpdate = true;
		}
		if( m_render && m_style ) {
//  		    qDebug("--> setting style on render element bgcolor=%s", m_style->backgroundColor().name().latin1());
		    m_render->setStyle(m_style);
		    needsUpdate = true;
		}
	    }
	    if ( change != Force )
		change = ch;
	}

	NodeImpl *n;
	for (n = _first; n; n = n->nextSibling()) {
// 	    qDebug("    (%p) calling recalcStyle on child %s, change=%d", this, n->isElementNode() ? ((ElementImpl *)n)->tagName().string().latin1() : n->isTextNode() ? "text" : "unknown", change );
	    if ( change >= Inherit || n->isTextNode() ||
		 n->hasChangedChild() || n->changed() )
		n->recalcStyle( change );
	}

	// ### should go away and be done by the renderobjects in
	// a more intelligent way
	if ( needsUpdate && m_render && !parentNode()->changed() ) {
	    RenderObject *r = m_render;
	    if ( !m_render->layouted() ) {
		if ( m_render->isInline() )
		    r = m_render->containingBlock();
		r->updateSize();
	    }
	    r->repaint();
	}
    }    
    setChanged( false );
    setHasChangedChild( false );
}

void ElementImpl::setFocus(bool received)
{
    NodeBaseImpl::setFocus(received);
    recalcStyle( Inherit );
}

void ElementImpl::setActive(bool down)
{
    NodeBaseImpl::setActive(down);
    recalcStyle( Inherit );
}

khtml::FindSelectionResult ElementImpl::findSelectionNode( int _x, int _y, int _tx, int _ty, DOM::Node & node, int & offset )
{
    //kdDebug(6030) << "ElementImpl::findSelectionNode " << this << " _x=" << _x << " _y=" << _y
    //           << " _tx=" << _tx << " _ty=" << _ty << endl;

    // ######### Duplicated code from mouseEvent
    // TODO put the code above (getting _tx,_ty) in a common place and call it from here

    if (!m_render) return SelectionPointAfter;

    RenderObject *p = m_render->parent();
    while( p && p->isAnonymousBox() ) {
//      kdDebug( 6030 ) << "parent is anonymous!" << endl;
        // we need to add the offset of the anonymous box
        _tx += p->xPos();
        _ty += p->yPos();
        p = p->parent();
    }

    if(!m_render->isInline() || !m_render->firstChild() || m_render->isFloating() )
    {
        m_render->absolutePosition(_tx, _ty);
    }

    int off=0, lastOffset=0;
    DOM::Node nod;
    DOM::Node lastNode;
    NodeImpl *child = firstChild();
    while(child != 0)
    {
        khtml::FindSelectionResult pos = child->findSelectionNode(_x, _y, _tx, _ty, nod, off);
        //kdDebug(6030) << this << " child->findSelectionNode returned " << pos << endl;
        if ( pos == SelectionPointInside ) // perfect match
        {
            node = nod;
            offset = off;
            //kdDebug(6030) << "ElementImpl::findSelectionNode " << this << " match offset=" << offset << endl;
            return SelectionPointInside;
        } else if ( pos == SelectionPointBefore )
        {
            //x,y is before this element -> stop here
            if ( !lastNode.isNull() ) {
                node = lastNode;
                offset = lastOffset;
                //kdDebug(6030) << "ElementImpl::findSelectionNode " << this << " before this child -> returning offset=" << offset << endl;
                return SelectionPointInside;
            } else {
                //kdDebug(6030) << "ElementImpl::findSelectionNode " << this << " before us -> returning -2" << endl;
                return SelectionPointBefore;
            }
        }
        // SelectionPointAfter -> keep going
        if ( !nod.isNull() )
        {
            lastNode = nod;
            lastOffset = off;
        }
        child = child->nextSibling();
    }
    node = lastNode;
    offset = lastOffset;
    return SelectionPointAfter;
}

bool ElementImpl::isSelectable() const
{
    return false;
}

// DOM Section 1.1.1
bool ElementImpl::childAllowed( NodeImpl *newChild )
{
    if (!childTypeAllowed(newChild->nodeType()))
        return false;

    // ### check xml element allowedness according to DTD
    if (id() && newChild->id()) // if one if these is 0 then it is an xml element and we allow it anyway
        return checkChild(id(), newChild->id());
    else
        return true;
}

bool ElementImpl::childTypeAllowed( unsigned short type )
{
    switch (type) {
        case Node::ELEMENT_NODE:
        case Node::TEXT_NODE:
        case Node::COMMENT_NODE:
        case Node::PROCESSING_INSTRUCTION_NODE:
        case Node::CDATA_SECTION_NODE:
        case Node::ENTITY_REFERENCE_NODE:
            return true;
            break;
        default:
            return false;
    }
}

void ElementImpl::createDecl( )
{
    m_styleDecls = new CSSStyleDeclarationImpl(0);
    m_styleDecls->ref();
    m_styleDecls->setParent(ownerDocument()->elementSheet());
    m_styleDecls->setNode(this);
    m_styleDecls->setStrictParsing( ownerDocument()->parseMode() == DocumentImpl::Strict );
}

void ElementImpl::dispatchAttrRemovalEvent(NodeImpl *attr)
{
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
    int exceptioncode = 0;
    AttrImpl *att = static_cast<AttrImpl*>(attr);
    dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,att->value(),
		  att->value(),att->name(),MutationEvent::REMOVAL),exceptioncode);
}

void ElementImpl::dispatchAttrAdditionEvent(NodeImpl *attr)
{
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
    int exceptioncode = 0;
    AttrImpl *att = static_cast<AttrImpl*>(attr);
    dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,att->value(),
		  att->value(),att->name(),MutationEvent::ADDITION),exceptioncode);
}

void ElementImpl::dump(QTextStream *stream, QString ind) const
{
    if (namedAttrMap) {
	for (uint i = 0; i < namedAttrMap->length(); i++) {
	    AttrImpl *attr = static_cast<AttrImpl*>(namedAttrMap->item(i));
	    *stream << " " << DOMString(attr->name()).string().ascii()
		    << "=\"" << DOMString(attr->value()).string().ascii() << "\"";
	}
    }

    NodeBaseImpl::dump(stream,ind);
}

// -------------------------------------------------------------------------

XMLElementImpl::XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_tagName)
    : ElementImpl(doc)
{
    m_id = doc->document()->tagId(0 /* no namespace */, _tagName,  false /* allocate */);
}

XMLElementImpl::XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_qualifiedName, DOMStringImpl *_namespaceURI)
    : ElementImpl(doc)
{
    int colonpos = -1;
    for (uint i = 0; i < _qualifiedName->l; ++i)
        if (_qualifiedName->s[i] == ':') {
            colonpos = i;
            break;
        }

    if (colonpos >= 0) {
        // we have a prefix
        DOMStringImpl* localName = _qualifiedName->copy();
        localName->ref();
        localName->remove(0,colonpos+1);
        m_id = doc->document()->tagId(_namespaceURI, localName, false /* allocate */);
        localName->deref();
        m_prefix = _qualifiedName->copy();
        m_prefix->ref();
        m_prefix->truncate(colonpos);
    }
    else {
        // no prefix
        m_id = doc->document()->tagId(_namespaceURI, _qualifiedName, false /* allocate */);
        m_prefix = 0;
    }
}

XMLElementImpl::~XMLElementImpl()
{
}

DOMString XMLElementImpl::namespaceURI() const
{
    return ownerDocument()->namespaceURI(m_id);
}

DOMString XMLElementImpl::localName() const
{
    return ownerDocument()->tagName(m_id);
}

NodeImpl *XMLElementImpl::cloneNode ( bool deep, int &exceptioncode )
{
    XMLElementImpl *clone = new XMLElementImpl(docPtr(), ownerDocument()->tagName(m_id).implementation());
    clone->m_id = m_id;

    // clone attributes
    if(namedAttrMap)
        *(static_cast<NamedAttrMapImpl*>(clone->attributes())) = *namedAttrMap;

    if (deep)
        cloneChildNodes(clone,exceptioncode);

    return clone;
}

// -------------------------------------------------------------------------

NamedAttrMapImpl::NamedAttrMapImpl(ElementImpl *e)
    : element(e)
{
    attrs = 0;
    len = 0;
}

NamedAttrMapImpl::~NamedAttrMapImpl()
{
    clearAttrs();
}

NamedAttrMapImpl &NamedAttrMapImpl::operator =(const NamedAttrMapImpl &other)
{
    // clone all attributes in the other map, but attach to our element
    if (!element)
        return *this;

    clearAttrs();
    len = other.len;
    attrs = new AttrImpl* [len];
    uint i;

    // first initialize attrs vector, then call parseAttribute on it
    // this allows parseAttribute to use getAttribute
    for (i = 0; i < len; i++) {
        int exceptioncode = 0; // ### propogate
        attrs[i] = static_cast<AttrImpl*>(other.attrs[i]->cloneNode(true,exceptioncode));
        attrs[i]->_element = element;
        attrs[i]->ref();
    }

    for(i = 0; i < len; i++)
        element->parseAttribute(attrs[i]);

    element->setChanged(true);
    return *this;
}

// DOM methods & attributes for NamedNodeMap

NodeImpl *NamedAttrMapImpl::getNamedItem ( const DOMString &name, int &exceptioncode ) const
{
    DOMString nullstr;
    return getItem(0,name,nullstr,NAME_COMPARE,exceptioncode);
}

Node NamedAttrMapImpl::setNamedItem ( const Node &arg, int &exceptioncode )
{
    return setItem(arg,NAME_COMPARE,exceptioncode);
}

Node NamedAttrMapImpl::removeNamedItem ( const DOMString &name, int &exceptioncode )
{
    DOMString nullstr;
    return removeItem(0,name,nullstr,NAME_COMPARE,exceptioncode);
}

NodeImpl *NamedAttrMapImpl::item ( unsigned long index ) const
{
    if (index >= len)
        return 0;
    else
        return attrs[index];
}

unsigned long NamedAttrMapImpl::length(  ) const
{
    return len;
}

NodeImpl *NamedAttrMapImpl::getNamedItemNS( const DOMString &namespaceURI,
                                            const DOMString &localName,
                                            int &exceptioncode ) const
{
    return getItem(0,localName,namespaceURI,NAME_NAMESPACE_COMPARE,exceptioncode);
}

NodeImpl *NamedAttrMapImpl::setNamedItemNS( NodeImpl *arg, int &exceptioncode )
{
    return setItem(arg,NAME_NAMESPACE_COMPARE,exceptioncode).handle();
}

NodeImpl *NamedAttrMapImpl::removeNamedItemNS( const DOMString &namespaceURI,
                                               const DOMString &localName,
                                               int &exceptioncode )
{
    return removeItem(0,localName,namespaceURI,NAME_NAMESPACE_COMPARE,exceptioncode).handle();
}

// Other methods (not part of DOM)

AttrImpl *NamedAttrMapImpl::getIdItem ( int id ) const
{
    int exceptioncode = 0; // will be ignored
    DOMString nullstr;
    return getItem(id,nullstr,nullstr,ID_COMPARE,exceptioncode);
}


Attr NamedAttrMapImpl::setIdItem ( AttrImpl *attr, int &exceptioncode )
{
    DOMString nullstr;
    return setItem(attr,ID_COMPARE,exceptioncode);
}

Attr NamedAttrMapImpl::removeIdItem ( int id )
{
    int exceptioncode = 0; // will be ignored
    DOMString nullstr;
    return removeItem(id,nullstr,nullstr,ID_COMPARE,exceptioncode);
}

void NamedAttrMapImpl::insertAttr( AttrImpl *a )
{
    // only add if not already there
    // ### delete/deref attr if we're not going to add it? (potential memory leak)
    if( !a->attrId || !getIdItem(a->attrId))
        addAttr(a);
}

void NamedAttrMapImpl::clearAttrs()
{
    if (attrs) {
        uint i;
        for (i = 0; i < len; i++) {
            attrs[i]->_element = 0;
            attrs[i]->deref();
        }
        delete [] attrs;
        attrs = 0;
    }
    len = 0;
}

void NamedAttrMapImpl::detachFromElement()
{
    // we allow a NamedAttrMapImpl w/o an element in case someone still has a reference
    // to if after the element gets deleted - but the map is now invalid
    element = 0;
    clearAttrs();
}

AttrImpl *NamedAttrMapImpl::getItem ( int id, const DOMString &name, const DOMString &namespaceURI,
                                      AttrCompare compareType, int &/*exceptioncode*/ ) const
{
    int index = findAttr(id,name,namespaceURI,compareType);
    if (index >= 0)
        return attrs[index];
    else
        return 0;
}

Attr NamedAttrMapImpl::setItem ( const Node &arg, AttrCompare compareType, int &exceptioncode )
{
    if (!element) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    // Not mentioned in spec: throw a HIERARCHY_REQUEST_ERROR if the user passes in a non-attribute node
    if (arg.nodeType() != Node::ATTRIBUTE_NODE) {
        exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
        return 0;
    }
    AttrImpl *attr = static_cast<AttrImpl*>(arg.handle());

    // WRONG_DOCUMENT_ERR: Raised if arg was created from a different document than the one that created this map.
    if (attr->getDocument() != element->getDocument()) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return 0;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    // INUSE_ATTRIBUTE_ERR: Raised if arg is an Attr that is already an attribute of another Element object.
    // The DOM user must explicitly clone Attr nodes to re-use them in other elements.
    if (attr->_element) {
        exceptioncode = DOMException::INUSE_ATTRIBUTE_ERR;
        return 0;
    }

    int index = findAttr(attr->attrId,attr->name(),attr->namespaceURI(),compareType);
    if (index >= 0) {
        // attribute with this id already in list
        return replaceAttr(index,attr);
    }
    else {
        // attribute with this name not yet in list
        addAttr(attr);
        return 0;
    }
}

Attr NamedAttrMapImpl::removeItem ( int id, const DOMString &name, const DOMString &namespaceURI,
                                    AttrCompare compareType, int &exceptioncode )
{
    // ### replace with default attribute (if any)

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    int index = findAttr(id,name,namespaceURI,compareType);

    // NOT_FOUND_ERR: Raised if there is no node with the specified namespaceURI and localName in this map.
    if (index < 0) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    Attr ret = removeAttr(index);

    return ret;
}


int NamedAttrMapImpl::findAttr ( int id, const DOMString &name, const DOMString &namespaceURI,
                                 AttrCompare compareType ) const
{
    uint i;
    if (compareType == ID_COMPARE) {
        // compare attributes by id
        for (i = 0; i < len; i++) {
            if (attrs[i]->attrId == id)
                return i;
        }
    }
    else if (compareType == NAME_COMPARE) {
        // compare attributes by name

        if (element->getDocument()->isHTMLDocument()) {
            // HTML document - do a case insensitive compare on attributes
            for (i = 0; i < len; i++) {
                if (!strcasecmp(attrs[i]->name(),name))
                    return i;
            }
        }
        else {
            // XML document - do a case sensitive compare on attributes
            for (i = 0; i < len; i++) {
                if (attrs[i]->name() == name)
                    return i;
            }
        }
    }
    else {
        // compare attributes by name & namespaceURI (case sensitive since this is only
        // relevant for XML documents)
        for (i = 0; i < len; i++) {
            if (attrs[i]->name() == name &&
                attrs[i]->namespaceURI() == namespaceURI)
                return i;
        }
    }

    // attribute not found
    return -1;
}

Attr NamedAttrMapImpl::replaceAttr(int i, AttrImpl *attr)
{
    // Replace the attribute
    Attr oldAttr = attrs[i];
    attrs[i]->_element = 0;
    attrs[i]->deref();
    attrs[i] = attr;
    attrs[i]->ref();
    attr->_element = element;

    // Notify the element that the attribute has been replaced, and dispatch appropriate mutation events
    element->parseAttribute(attr);
    element->setChanged(true);
    element->dispatchAttrRemovalEvent(oldAttr.handle());
    element->dispatchAttrAdditionEvent(attrs[i]);
    element->dispatchSubtreeModifiedEvent();
    return oldAttr;
}

void NamedAttrMapImpl::addAttr(AttrImpl *attr)
{
    // Add the attribute tot he list
    AttrImpl **newAttrs = new AttrImpl* [len+1];
    uint i;
    if (attrs) {
      for (i = 0; i < len; i++)
        newAttrs[i] = attrs[i];
      delete [] attrs;
    }
    attrs = newAttrs;
    attrs[len] = attr;
    attr->ref();
    len++;
    attr->_element = element;

    // Notify the element that the attribute has been added, and dispatch appropriate mutation events
    // Note that element may be null here if we are called from insertAttr() during parsing
    if (element) {
        element->parseAttribute(attr);
        element->setChanged(true);
        element->dispatchAttrAdditionEvent(attr);
        element->dispatchSubtreeModifiedEvent();
    }
}

Attr NamedAttrMapImpl::removeAttr(int index)
{
    // Remove the attribute from the list
    AttrImpl* ret = attrs[index];
    attrs[index]->_element = 0;
    if (len == 1) {
        delete [] attrs;
        attrs = 0;
        len = 0;
    }
    else {
        AttrImpl **newAttrs = new AttrImpl* [len-1];
        uint i;
        for (i = 0; i < uint(index); i++)
            newAttrs[i] = attrs[i];
        len--;
        for (; i < len; i++)
            newAttrs[i] = attrs[i+1];
        delete [] attrs;
        attrs = newAttrs;
    }

    // Notify the element that the attribute has been removed
    // dispatch appropriate mutation events
    if (ret->_value) {
        ret->_value->deref();
        ret->_value = 0;
        element->parseAttribute(ret);
        //element->setChanged(true);
    }
    element->dispatchAttrRemovalEvent(ret);
    element->dispatchSubtreeModifiedEvent();

    // now some refcounting hackery
    Attr a(ret);
    ret->deref();
    return a;
}

Attr NamedAttrMapImpl::removeAttr( AttrImpl *oldAttr, int &exceptioncode )
{
    exceptioncode = 0;
    uint i;
    for (i = 0; i < len; i++) {
        if (attrs[i] == oldAttr) {
            Attr ret = removeAttr(i);
            return ret;
        }
    }

    exceptioncode = DOMException::NOT_FOUND_ERR;
    return 0;
}
