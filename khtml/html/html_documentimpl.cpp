/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
#include "html_documentimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include "misc/khtmldata.h"

#include "htmlparser.h"
#include "htmltokenizer.h"
#include "htmlhashes.h"

#include "dom_exception.h"
#include "html_headimpl.h"

#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>

#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
#include "rendering/render_style.h"
#include "rendering/render_root.h"

#include <qstack.h>


using namespace DOM;
using namespace khtml;

template class QStack<DOM::NodeImpl>;

HTMLDocumentImpl::HTMLDocumentImpl() : DocumentImpl()
{
    kdDebug( 6030 ) << "HTMLDocumentImpl constructor this = " << this << endl;
    parser = 0;
    tokenizer = 0;

    bodyElement = 0;

    m_loadingSheet = false;

    m_elemSheet=0;
}

HTMLDocumentImpl::HTMLDocumentImpl(KHTMLView *v)
    : DocumentImpl(v)
{
    kdDebug( 6030 ) << "HTMLDocumentImpl constructor2 this = " << this << endl;
    parser = 0;
    tokenizer = 0;

    bodyElement = 0;

    m_styleSelector = new CSSStyleSelector(this);

    m_loadingSheet = false;

    m_elemSheet=0;
}

HTMLDocumentImpl::~HTMLDocumentImpl()
{
    kdDebug( 6030 ) << "HTMLDocumentImpl destructor this = " << this << endl;
}

DOMString HTMLDocumentImpl::referrer() const
{
    // ### should we fix that? I vote against for privacy reasons
    return DOMString();
}

DOMString HTMLDocumentImpl::domain() const
{
    // ### do they want the host or the domain????
    KURL u(url.string());
    return u.host();
}

DOMString HTMLDocumentImpl::baseURL() const
{
    if(!view()->part()->baseURL().isEmpty()) return view()->part()->baseURL().url();
    return url;
}


HTMLElementImpl *HTMLDocumentImpl::body()
{
    if(bodyElement) return bodyElement;
    if(!_first) return 0;
    NodeImpl *test = _first->firstChild();
    if(!test) return 0;
    while(test && (test->id() != ID_BODY && test->id() != ID_FRAMESET))
	test = test->nextSibling();
    if(!test) return 0;
    bodyElement = static_cast<HTMLElementImpl *>(test);
    return bodyElement;
}

void HTMLDocumentImpl::open(  )
{
    //kdDebug( 6030 ) << "HTMLDocumentImpl::open()" << endl;
    clear();
    parser = new KHTMLParser(m_view, this);
    tokenizer = new HTMLTokenizer(parser, m_view);
    tokenizer->begin();
}

void HTMLDocumentImpl::close(  )
{
    if (m_render)
    	m_render->close();

    if(parser) delete parser;
    parser = 0;
    if(tokenizer) delete tokenizer;
    tokenizer = 0;
}

void HTMLDocumentImpl::write( const DOMString &text )
{
    if(tokenizer)
	tokenizer->write(text.string());
}

void HTMLDocumentImpl::write( const QString &text )
{
    if(tokenizer)
	tokenizer->write(text);
}

void HTMLDocumentImpl::writeln( const DOMString &text )
{
    write(text);
    write(DOMString("\n"));
}

ElementImpl *HTMLDocumentImpl::getElementById( const DOMString &elementId )
{
    QStack<NodeImpl> nodeStack;
    NodeImpl *current = _first;

    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	    current = current->nextSibling();
	}
	else
	{
	    if(current->isElementNode())
	    {
		ElementImpl *e = static_cast<ElementImpl *>(current);
		if(e->getAttribute(ATTR_ID) == elementId)
		    return e;
	    }
	
	    NodeImpl *child = current->firstChild();
	    if(child)
	    {	
		nodeStack.push(current);
		current = child;
	    }
	    else
	    {
		current = current->nextSibling();
	    }
	}
    }

    return 0;
}


NodeListImpl *HTMLDocumentImpl::getElementsByName( const DOMString &elementName )
{
    return new NameNodeListImpl( this, elementName );
}

// internal. finds the first element with tagid id
NodeImpl *HTMLDocumentImpl::findElement( int id )
{
    QStack<NodeImpl> nodeStack;
    NodeImpl *current = _first;

    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	    current = current->nextSibling();
	}
	else
	{
	    if(current->id() == id)
		return current;
	
	    NodeImpl *child = current->firstChild();
	    if(child)
	    {	
		nodeStack.push(current);
		current = child;
	    }
	    else
	    {
		current = current->nextSibling();
	    }
	}
    }

    return 0;
}


StyleSheetListImpl *HTMLDocumentImpl::styleSheets()
{
    // ### implement for html
    return 0;
}


// --------------------------------------------------------------------------
// not part of the DOM
// --------------------------------------------------------------------------

void HTMLDocumentImpl::clear()
{
    if(parser) delete parser;
    if(tokenizer) delete tokenizer;
    parser = 0;
    tokenizer = 0;

    // #### clear tree
}


NodeImpl *HTMLDocumentImpl::addChild(NodeImpl *newChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6030 ) << "Document::addChild( " << newChild->nodeName().string() << " )" << endl;
#endif

    // short check for consistency with DTD
    if(newChild->id() != ID_HTML)
    {
	kdDebug( 6030 ) << "AddChild failed! id=#document, child->id=" << newChild->id() << endl;
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
    }

    if(_first)
    {
	kdDebug( 6030 ) << "AddChild failed! id=#document, child->id=" << newChild->id() << ". Already have a HTML element!" << endl;
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
    }

    // just add it...
    newChild->setParent(this);
    _first = _last = newChild;
    return newChild;
}

bool HTMLDocumentImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int, int, DOMString &url,
                                   NodeImpl *&innerNode, long &offset)
{
    if(body())
    {
	return bodyElement->mouseEvent(_x, _y, button, type, 0, 0, url, innerNode, offset);
    }
    return false;
}

void HTMLDocumentImpl::attach(KHTMLView *w)
{
    m_view = w;
    if(!m_styleSelector) createSelector();
    m_style = new RenderStyle();
    m_style->setDisplay(BLOCK);
    // ### make the font stuff _really_ work!!!!
    const QString *families = w->part()->settings()->families();
    QValueList<int> fs = w->part()->settings()->fontSizes();
    QFont f = KGlobal::generalFont();
    f.setFamily(families[0]);
    f.setPointSize(fs[3]);
    f.setCharSet(w->part()->settings()->charset);
    m_style->setFont(f);

    m_style->setHtmlHacks(true); // enable html specific rendering tricks

    m_render = new RenderRoot(w);
    m_render->setStyle(m_style);
    m_render->ref();

    NodeBaseImpl::attach(w);
}

void HTMLDocumentImpl::detach()
{
    m_view = 0;

    NodeBaseImpl::detach();
}

void HTMLDocumentImpl::setVisuallyOrdered()
{
    if(!m_style) return;
    m_style->setVisuallyOrdered(true);
}

void HTMLDocumentImpl::createSelector()
{
    applyChanges();
}

// ### this function should not be needed in the long run. The one in
// DocumentImpl should be enough.
void HTMLDocumentImpl::applyChanges(bool)
{
    if(m_styleSelector) delete m_styleSelector;
    m_styleSelector = new CSSStyleSelector(this);
    if(!m_render) return;

    const QString *families = m_view->part()->settings()->families();
    QValueList<int> fs = m_view->part()->settings()->fontSizes();
    QFont f = KGlobal::generalFont();
    f.setFamily(families[0]);
    f.setPointSize(fs[3]);
    f.setCharSet(m_view->part()->settings()->charset);
    m_style->setFont(f);

    // a style change can influence the children, so we just go
    // through them and trigger an appplyChanges there too
    NodeImpl *n = _first;
    while(n) {
	n->applyChanges();
	n = n->nextSibling();
    }

    // force a relayout of this part of the document
    m_render->layout(true);
    // force a repaint of this part.
    // ### if updateSize() changes any size, it will already force a
    // repaint, so we might do double work here...
    m_render->repaint();
}

void HTMLDocumentImpl::setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet)
{
    kdDebug( 6030 ) << "HTMLDocument::setStyleSheet()" << endl;
    m_sheet = new CSSStyleSheetImpl(this, url);
    m_sheet->ref();
    m_sheet->parseString(sheet);
    m_loadingSheet = false;

    createSelector();
}

CSSStyleSheetImpl* HTMLDocumentImpl::elementSheet()
{
    if (!m_elemSheet)
    	m_elemSheet = new CSSStyleSheetImpl(this, url);
    return m_elemSheet;
}


void HTMLDocumentImpl::setSelection(NodeImpl* s, int sp, NodeImpl* e, int ep)
{
    static_cast<RenderRoot*>(m_render)
    	->setSelection(s->renderer(),sp,e->renderer(),ep);
}

void HTMLDocumentImpl::clearSelection()
{
    static_cast<RenderRoot*>(m_render)
    	->clearSelection();
}
