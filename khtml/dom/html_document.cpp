/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
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
// --------------------------------------------------------------------------
#include "html_document.h"

#include "dom_node.h"
#include "dom_element.h"
#include "dom_doc.h"
#include "dom_string.h"
#include "dom_textimpl.h"
#include "html_misc.h"
#include "html_element.h"
#include "html_documentimpl.h"
#include "html_elementimpl.h"
#include "html_miscimpl.h"
#include "htmlhashes.h"
using namespace DOM;


HTMLDocument::HTMLDocument() : Document(false) // create the impl here
{
    impl = new HTMLDocumentImpl();
    impl->ref();

}

HTMLDocument::HTMLDocument(KHTMLView *parent)
    : Document(false) // create the impl here
{
    impl = new HTMLDocumentImpl( parent);
    impl->ref();

}

HTMLDocument::HTMLDocument(const HTMLDocument &other) : Document(other)
{
}

HTMLDocument::HTMLDocument(HTMLDocumentImpl *impl) : Document(impl)
{
}

HTMLDocument &HTMLDocument::operator = (const Node &other)
{
    if(other.nodeType() != DOCUMENT_NODE)
    {
	impl = 0;
	return *this;
    }
    Document d;
    d = other;
    if(!d.isHTMLDocument())
	impl = 0;
    else
	Node::operator =(other);
    return *this;
}

HTMLDocument &HTMLDocument::operator = (const HTMLDocument &other)
{
    Document::operator =(other);
    return *this;
}

HTMLDocument::~HTMLDocument()
{
}

DOMString HTMLDocument::title() const
{
    if(!impl) return 0;

    NodeImpl *e = static_cast<HTMLDocumentImpl *>(impl)->findElement(ID_TITLE);
    if(!e) return 0;

    NodeImpl *t = e->firstChild();
    if(!t) return 0;

    return static_cast<TextImpl *>(t)->data();
}

void HTMLDocument::setTitle( const DOMString &/*value*/ )
{
    // ###
}

DOMString HTMLDocument::referrer() const
{
    if(!impl) return 0;
    return ((HTMLDocumentImpl *)impl)->referrer();
}

DOMString HTMLDocument::domain() const
{
    if(!impl) return 0;
    return ((HTMLDocumentImpl *)impl)->domain();
}

DOMString HTMLDocument::URL() const
{
    if(!impl) return 0;
    return ((HTMLDocumentImpl *)impl)->URL();
}

HTMLElement HTMLDocument::body() const
{
    if(!impl) return 0;
    return ((HTMLDocumentImpl *)impl)->body();
}

HTMLCollection HTMLDocument::images() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::DOC_IMAGES);
}

HTMLCollection HTMLDocument::applets() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::DOC_APPLETS);
}

HTMLCollection HTMLDocument::links() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::DOC_LINKS);
}

HTMLCollection HTMLDocument::forms() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::DOC_FORMS);
}

HTMLCollection HTMLDocument::anchors() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::DOC_ANCHORS);
}

DOMString HTMLDocument::cookie() const
{
    // ###
    return 0;
}

void HTMLDocument::setCookie( const DOMString &/*value*/ )
{
    // ###
}

void HTMLDocument::open(  )
{
    if(impl)
        ((HTMLDocumentImpl *)impl)->open(  );
}

void HTMLDocument::close(  )
{
    if(impl)
        ((HTMLDocumentImpl *)impl)->close(  );
}

void HTMLDocument::write( const DOMString &text )
{
    if(impl)
        ((HTMLDocumentImpl *)impl)->write( text );
}

void HTMLDocument::write( const QString &text )
{
    if(impl)
        ((HTMLDocumentImpl *)impl)->write( text );
}

void HTMLDocument::writeln( const DOMString &text )
{
    if(impl)
        ((HTMLDocumentImpl *)impl)->writeln( text );
}

Element HTMLDocument::getElementById( const DOMString &elementId )
{
    if(!impl) return 0;
    return ((HTMLDocumentImpl *)impl)->getElementById( elementId );
}

NodeList HTMLDocument::getElementsByName( const DOMString &elementName )
{
    if(!impl) return 0;
    return ((HTMLDocumentImpl *)impl)->getElementsByName( elementName );
}

