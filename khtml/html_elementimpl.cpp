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
// -------------------------------------------------------------------------
//#define DEBUG
//#define DEBUG_LAYOUT
//#define PAR_DEBUG

#include "dom_string.h"

#include <qpainter.h>
#include <qfontmetrics.h>
#include <qstack.h>
#include <qlist.h>

#include "html_elementimpl.h"
#include "html_inlineimpl.h"
#include "html_blockimpl.h"
#include "html_imageimpl.h"
#include "html_documentimpl.h"
#include "dom_node.h"
#include "dom_textimpl.h"
#include "dom_stringimpl.h"
#include "dom_exception.h"

#include "khtmltags.h"
#include "khtmlstyle.h"
#include "khtmlfont.h"
#include "khtmltext.h"
#include "khtmlattrs.h"
#include "khtml.h"

#include <stdio.h>
#include <assert.h>

using namespace DOM;


inline int MAX(int a, int b)
{
    return a > b ? a : b;
}

inline int MIN(int a, int b)
{
    return a < b ? a : b;
}

HTMLElementImpl::HTMLElementImpl(DocumentImpl *doc) : ElementImpl(doc)
{
    _style = 0;
    halign = HNone;
    minWidth = -1;
    maxWidth = -1;
    availableWidth = 0;
}

HTMLElementImpl::~HTMLElementImpl()
{
    //printf("Element destructor: this=%s\n", nodeName().string().ascii());
    delete _style;
}

void HTMLElementImpl::setStyle(CSSStyle *s)
{
    _style = new CSSStyle(*s);
}

NodeImpl *HTMLElementImpl::addChild(NodeImpl *newChild)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Element)::addChild( %s )\n", nodeName().string().ascii(), newChild->nodeName().string().ascii());
#endif

    newChild->setAvailableWidth(availableWidth);

    return NodeBaseImpl::addChild(newChild);
}

void HTMLElementImpl::print(QPainter *p, int _x, int _y, int _w, int _h,
				  int _tx, int _ty)
{
    // default implementation. Just pass things through to the children
    NodeImpl *child = firstChild();
    while(child != 0)
    {
	child->print(p, _x, _y, _w, _h, _tx, _ty);
	child = child->nextSibling();
    }
}

static QChar nbsp = KGlobal::charsets()->fromEntity("nbsp");

void HTMLElementImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("%s(Element)::calcMinMaxWidth() known=%d\n", nodeName().string().ascii(), minMaxKnown());
#endif

    if(minMaxKnown()) return;

    minWidth = 0;
    maxWidth = 0;

    int inlineMax=0;
    int inlineMin=0;    
    
    bool noBreak=false;        

    NodeImpl *child = firstChild();
    while(child != 0)
    {
	if((child->isInline() || child->isFloating() ) &&
	    !( child->id()==ID_BR || child->id()==ID_P))  // any more?
	{
	    // we have to take care about nbsp's, and places were
	    // we can't break between two inline objects...
	    // But for the moment, this will do...
	    
	    // mostly done -antti
	    

	    if (child->isTextNode())
	    {	    
	    	bool hasNbsp=false;
	    	TextImpl* t = static_cast<TextImpl*>(child);
		if (t->data()[0]==nbsp) //inline starts with nbsp
		{
		    inlineMin+=child->getMinWidth();
		    inlineMax+=child->getMaxWidth();
		    hasNbsp=true;
		} 
		if (hasNbsp && t->data()[t->length()-1]==nbsp)
		{   	    	    	//inline starts and ends with nbsp
		    noBreak=true;
		}
		else if (t->data()[t->length()-1]==nbsp)
		{   	    	    	//inline only ends with nbsp
		    int w = child->getMinWidth();
		    if(inlineMin < w) inlineMin = w;
		    w = child->getMaxWidth();
		    inlineMax += w;
		    noBreak=true;
		    hasNbsp=true;
		}
		if (hasNbsp)
		{
		    child=child->nextSibling();
		    hasNbsp=false;		    
		    continue;
		}
	    }
	    if (noBreak)
	    {
	    	inlineMin+=child->getMinWidth();
		inlineMax+=child->getMaxWidth();
		noBreak=false;
	    }
	    else	    
	    {
		int w = child->getMinWidth();
		if(inlineMin < w) inlineMin = w;
		w = child->getMaxWidth();
		inlineMax += w;	
		
	    }	    
	}
	else
	{
	    int w = child->getMinWidth();
	    if(minWidth < w) minWidth = w;
	    w = child->getMaxWidth();
	    if(maxWidth < w) maxWidth = w;

    	    if(minWidth < inlineMin) minWidth = inlineMin;
	    if(maxWidth < inlineMax) maxWidth = inlineMax;
	    inlineMin=0;
            inlineMax=0;
	    
	    noBreak=false;
	}
	child = child->nextSibling();
    }
    if(minWidth < inlineMin) minWidth = inlineMin;
    if(maxWidth < inlineMax) maxWidth = inlineMax;
    if(maxWidth < minWidth) maxWidth = minWidth;
}

bool HTMLElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &url)
{
#ifdef EVENT_DEBUG
    printf("%s::mouseEvent\n", nodeName().string().ascii());
#endif
    bool inside = false;

    NodeImpl *child = firstChild();
    while(child != 0)
    {
	if(child->mouseEvent(_x, _y, button, type, _tx, _ty, url))
	{
	    inside = true;
	    break;
	}
	child = child->nextSibling();
    }

#ifdef EVENT_DEBUG
    if(inside) printf("    --> inside\n");
#endif
    // dynamic HTML...
    if(inside) mouseEventHandler(button, type);

    return inside;
}

void HTMLElementImpl::mouseEventHandler( int /*button*/, MouseEventType type )
{
    if(!hasEvents()) return;

    KHTMLWidget *htmlwidget = (KHTMLWidget *) static_cast<HTMLDocumentImpl *>(document)->HTMLWidget();
    if(!htmlwidget) return;

    int id;
    bool click = false;

    switch(type)
    {
    case MousePress:
	id = ATTR_ONMOUSEDOWN;
	setPressed();
	break;
    case MouseRelease:
	id = ATTR_ONMOUSEUP;
	if(pressed()) click = true;
	setPressed(false);
	break;
    case MouseClick:
	// handled differently at the moment
	break;
    case MouseDblClick:
	id = ATTR_ONDBLCLICK;
	break;
    case MouseMove:
	id = ATTR_ONMOUSEMOVE;
	break;
    default:
	break;
    }

    DOMString script = getAttribute(id);
    if(script.length())
    {
	printf("emit executeScript( %s )\n", script.string().ascii());
	htmlwidget->executeScript( script.string() );
    }

    if(click)
    {
	script = getAttribute(ATTR_ONCLICK);
	if(script.length())
	{
	    htmlwidget->executeScript( script.string() );
	    printf("emit executeScript( %s )\n", script.string().ascii());
	}
    }
}

void HTMLElementImpl::getAbsolutePosition(int &xPos, int &yPos)
{
    if(_parent)
	_parent->getAbsolutePosition( xPos, yPos );
    else
	xPos = yPos = -1;
}

void HTMLElementImpl::setAvailableWidth(int w)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Element)::setAvailableWidth(%d)\n", nodeName().string().ascii(), w);
#endif

    if (w != -1 && w != availableWidth)
    	setLayouted(false);

    if(w != -1) availableWidth = w;

    NodeImpl *child = firstChild();
    while(child != 0)
    {
    	if (child->getMinWidth() > availableWidth)
	{
	    printf("ERROR: %d too narrow availableWidth=%d minWidth=%d\n",
	    id(), availableWidth, child->getMinWidth());
	    calcMinMaxWidth();
	    setLayouted(false);
	}
	child->setAvailableWidth(availableWidth);
	child = child->nextSibling();
    }
}


void HTMLElementImpl::close()
{
    setParsing(false);
    calcMinMaxWidth();
    if(!availableWidth) return;
    if(availableWidth < minWidth)
    	_parent->updateSize();
    else if(!isInline())
    {
	layout(true);
	if(layouted())
	    static_cast<HTMLDocumentImpl *>(document)->print(this);
    }
}

void HTMLElementImpl::updateSize()
{
  //    printf("element::updateSize()\n");
    setLayouted(false);
    calcMinMaxWidth();
    if(_parent)
    	_parent->updateSize();
}

void HTMLElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
// the core attributes...
    case ATTR_ID:
	// unique id
	setHasID();
	break;
    case ATTR_CLASS:
	// class
	setHasClass();
	break;
    case ATTR_STYLE:
	// stylesheet info
	setHasStyle();
	break;
    case ATTR_TITLE:
	// additional title for the element, may be displayed as tooltip
	setHasTooltip();
	break;
// i18n attributes
    case ATTR_LANG:
	// language info
	break;
    case ATTR_DIR:
	// BiDi info
	break;
// standard events
    case ATTR_ONCLICK:
    case ATTR_ONDBLCLICK:
    case ATTR_ONMOUSEDOWN:
    case ATTR_ONMOUSEMOVE:
    case ATTR_ONMOUSEOUT:
    case ATTR_ONMOUSEOVER:
    case ATTR_ONMOUSEUP:
    case ATTR_ONKEYDOWN:
    case ATTR_ONKEYPRESS:
    case ATTR_ONKEYUP:
	setHasEvents();
	break;
// other misc attributes
    case ATTR_ALIGN:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;
	break;
    default:
	break;
    }
}

// -------------------------------------------------------------------------
HTMLInlineElementImpl::HTMLInlineElementImpl(DocumentImpl *doc, ushort i)
    : HTMLElementImpl(doc)
{
    _id = i;
}

HTMLInlineElementImpl::~HTMLInlineElementImpl()
{
}

const DOMString HTMLInlineElementImpl::nodeName() const
{
    return getTagName(_id);
}


void HTMLInlineElementImpl::setStyle(CSSStyle *currentStyle)
{
    switch(_id)
    {
    case ID_TT:
	currentStyle->font.family = pSettings->fixedFontFace;
	currentStyle->font.fixed = true;
	currentStyle->font.style = CSSStyleFont::stNormal;
	currentStyle->font.weight = CSSStyleFont::Normal;
	break;
    case ID_U:
	currentStyle->font.decoration = CSSStyleFont::decUnderline;
	break;
    case ID_B:
	currentStyle->font.weight = CSSStyleFont::Bold;
	break;
    case ID_I:
	currentStyle->font.style = CSSStyleFont::stItalic;
	break;
    case ID_S:
    case ID_STRIKE:
	currentStyle->font.decoration = CSSStyleFont::decLineThrough;
	break;
    case ID_BIG:
	currentStyle->font.size += 1;
	break;
    case ID_SMALL:
	currentStyle->font.size = pSettings->fontBaseSize - 1;
	break;

    case ID_EM:
	// ### alternate between normal and italic
	currentStyle->font.style = CSSStyleFont::stItalic;
	break;	
    case ID_STRONG:
	currentStyle->font.weight = CSSStyleFont::Bold;
	break;
    case ID_DFN:
	// ###
	break;
    case ID_CODE:
    case ID_SAMP:
    case ID_KBD:
	currentStyle->font.family = pSettings->fixedFontFace;
	currentStyle->font.fixed = true;
	currentStyle->font.style = CSSStyleFont::stNormal;
	currentStyle->font.weight = CSSStyleFont::Normal;
	break;
    case ID_VAR:
	currentStyle->font.style = CSSStyleFont::stItalic;
	break;
    case ID_CITE:
	currentStyle->font.style = CSSStyleFont::stItalic;
	currentStyle->font.weight = CSSStyleFont::Normal;
	break;
    case ID_ABBR:
	// ###
	break;
    case ID_ACRONYM:
	// ###
	break;
    case ID_SUB:
	currentStyle->font.size -= 1;
	break;
    case ID_SUP:
	currentStyle->font.size -= 1;
	break;
    default:
	break;
    }

    HTMLElementImpl::setStyle(currentStyle);
}

void HTMLInlineElementImpl::print(QPainter *p, int _x, int _y, int _w, int _h,
				  int _tx, int _ty)
{
    switch(_id)
    {	
    case ID_SUB:
    {
	int h = ((QFont)*getFont()).pixelSize()/2;
	_ty += h;
	break;
    }
    case ID_SUP:
    {
	int h = ((QFont)*getFont()).pixelSize()/2;
	_ty -= h;
	break;
    }
    default:
	break;
    }

    NodeImpl *child = firstChild();
    while(child != 0)
    {
	child->print(p, _x, _y, _w, _h, _tx, _ty);
	child = child->nextSibling();
    }
}

// -------------------------------------------------------------------------

bool HTMLPositionedElementImpl::mouseEvent( int _x, int _y, int button,
					    MouseEventType type,
					    int _tx, int _ty, DOMString &url)
{
#ifdef EVENT_DEBUG
    printf("%s::mouseEvent\n", nodeName().string().ascii());
#endif

    // check if x/y is inside the element
    _ty += y;
    if((_ty - ascent > _y) || (_ty + descent < _y)) return false;
    _tx += x;
    if((_tx > _x) || (_tx + width < _x)) return false;

    NodeImpl *child = firstChild();
    while(child != 0)
    {
	if(child->mouseEvent(_x, _y, button, type, _tx, _ty, url)) break;
	child = child->nextSibling();
    }

#ifdef EVENT_DEBUG
    printf("    --> inside\n");
#endif

    // dynamic HTML...
    mouseEventHandler(button, type);

    return true;
}

void HTMLPositionedElementImpl::getAbsolutePosition(int &xPos, int &yPos)
{
    if(_parent)
    {
	_parent->getAbsolutePosition(xPos, yPos);
	if(xPos != -1) xPos += x, yPos += y;
    }
    else
	xPos = yPos = -1;
}

NodeImpl *HTMLPositionedElementImpl::addChild(NodeImpl *newChild)
{
#ifdef DEBUG_LAYOUT
    printf("%s(PositionedElement)::addChild( %s )\n", nodeName().string().ascii(), newChild->nodeName().string().ascii());
#endif

    newChild->setAvailableWidth(availableWidth);
    if(!newChild->isInline())
    {
	if(_last)
	    newChild->setYPos(_last->getYPos() + _last->getDescent());
	else
	    newChild->setYPos(0);
    }
    return NodeBaseImpl::addChild(newChild);
}

void HTMLPositionedElementImpl::updateSize()
{
    //printf("positionelement::updateSize()\n");
    int oldAscent = ascent;
    int oldDescent = descent;
    int oldMin = minWidth;
    setLayouted(false);
    calcMinMaxWidth();
    if(minWidth > availableWidth || minWidth!=oldMin)
    {
    	//printf("parent id=%d\n",_parent->id());
	if(_parent) _parent->updateSize();
    }
    else
    {
    	//printf("layout\n");
	setAvailableWidth();
	layout(true);	
	if(ascent != oldAscent || descent != oldDescent)
	{
	    setLayouted(false);
	    if(_parent) _parent->updateSize();
	} else {
	    //printf("HACK!\n");
	    // ###FIXME   		
	    KHTMLWidget* kh = const_cast<KHTMLWidget*>(
		static_cast<HTMLDocumentImpl*>(document)->HTMLWidget());
	    if (kh)
		kh->updateContents(getXPos(),getYPos()-getAscent(),getWidth(),getHeight());
	}
    }
}
// -------------------------------------------------------------------------

HTMLBlockElementImpl::HTMLBlockElementImpl(DocumentImpl *doc)
    : HTMLPositionedElementImpl(doc)
{
    aligned = 0;
    startPar = 0;
    leftMargin=0L;
    rightMargin=0L;
}

HTMLBlockElementImpl::~HTMLBlockElementImpl()
{
    if (leftMargin)
    	delete leftMargin;
    if (rightMargin)
    	delete rightMargin;

}

void HTMLBlockElementImpl::print(QPainter *p, int _x, int _y, int _w, int _h,
				 int _tx, int _ty)
{
    _tx += x;
    _ty += y;

#ifdef DEBUG_LAYOUT
    printf("%s(BlockElement)::print()\n", nodeName().string().ascii());
#endif

    // check if we need to do anything at all...
    if((_ty - ascent > _y + _h) || (_ty + descent < _y)) return;
    if((_tx > _x + _w) || (_tx + width < _x)) return;
    //if(!layouted()) return;

#ifdef DEBUG_LAYOUT
    printf("print 2\n");
#endif

    // default implementation. Just pass things through to the children
    // and paint paragraphs (groups of inline elements)
    NodeImpl *child;

    _ty += ascent;
    child = firstChild();
    while(child != 0)
    {
	child->print(p, _x, _y, _w, _h, _tx, _ty);
	child = child->nextSibling();
    }
}

void HTMLBlockElementImpl::printObject(QPainter *p, int _x, int _y,
				       int _w, int _h, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(BlockElement)::printObject()\n", nodeName().string().ascii());
#endif
    // check if we need to do anything at all...
    if((_ty - ascent > _y + _h) || (_ty + descent < _y)) return;
    if(!layouted()) return;

    // default implementation. Just pass things through to the children
    // and paint paragraphs (groups of inline elements)
    NodeImpl *child;

    _ty += ascent;
    child = firstChild();
    while(child != 0)
    {
	if(child->isInline())
	    child->print(p, _x, _y, _w, _h, _tx, _ty);
	child = child->nextSibling();
    }
}

void HTMLBlockElementImpl::layout( bool deep )
{
   //if (layouted())
    //	return;

    width = availableWidth;
#ifdef DEBUG_LAYOUT
    printf("%s(BlockElement)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    if(width<=0) return;
    clearMargins();

    bool layouted_ = true;

    // Block elements usually just have descent.
    // ascent != 0 will give a separation.
    ascent = descent = 0;

    NodeImpl *child = firstChild();
    while( child != 0 )
    {

    	if(child->isInline())
	{
	    child = calcParagraph(child);	
	}
	else if(child->isFloating())
	{	
	    calcFloating(child,descent);
	    if(deep)
		child->layout(deep);
	    else if (!child->layouted())
	    	layouted_=false;
	    child = child->nextSibling();
	}
	else
	{
	    int x = getLeftMargin(descent);
	    int w = getWidth(descent);
	    child->setYPos(descent);
 	    child->setAvailableWidth(w);
	    if(deep)
		child->layout(deep);
	    else if (!child->layouted())
	    	layouted_=false;
	    switch(hAlign())
	    {
	    case Left:
		break;
	    case Right:
		x = x + w - child->getWidth();
		break;
	    case HCenter:
		x = x + (w - child->getWidth())/2;
		break;
	    default:
		break;
	    }
 	    child->setXPos(x);
	    descent += child->getDescent()+child->getAscent();
	    child = child->nextSibling();
	}
    }
    descent = MAX (descent, getLeftBottom());
    descent = MAX (descent, getRightBottom());
    setLayouted(layouted_);

    // printf("layouted = %d\n", layouted_);
}



void
HTMLBlockElementImpl::insertMarginElement(HAlign align_, int y_,
    NodeImpl* node_)
{

    int height_ = node_->getHeight() + node_->vSpace()*2;;
    int width_ = node_->getWidth() + node_->hSpace()*2;

    MarginRange* range=0;	
    QList<MarginRange>* marg=0;
    if (align_==Right) {
    	if(!rightMargin) {
	    rightMargin = new QList<MarginRange>;
	    rightMargin->setAutoDelete(true);	
	}
    	marg = rightMargin;
    } else {
    	if(!leftMargin) {
	    leftMargin = new QList<MarginRange>;
	    leftMargin->setAutoDelete(true);
	}
    	marg = leftMargin;
    }
	
    QListIterator<MarginRange> it(*marg);
    MarginRange* r;	
    while ( (r = it.current()) ) {
	if (r->node == node_) {
	    range = r;
	    break;
	}
	++it;
    }

    if (range) return;

    range = new MarginRange;

    range->startY=y_;
    range->endY=y_+height_;
    range->width=width_;
    range->node=node_;

    marg->append(range);

}
int
HTMLBlockElementImpl::getLeftMargin(int y) {
    if (!leftMargin)
	return 0;;
    QListIterator<MarginRange> lIt(*leftMargin);
    int res=0;
    MarginRange* r;	
    while ( (r = lIt.current()) )
    {
	if (r->startY<=y && r->endY>y)
	    res+=r->width;
	++lIt;
    }
    return res;

}
int
HTMLBlockElementImpl::getRightMargin(int y) {
    if (!rightMargin)
	return width;
    QListIterator<MarginRange> rIt(*rightMargin);
    int res=width;
    MarginRange* r;	
    while ( (r = rIt.current()) )
    {
	if (r->startY<=y && r->endY>y)
	    res-=r->width;
	++rIt;
    }
    return res;

}

int
HTMLBlockElementImpl::getLeftBottom() {
    if (!leftMargin)
	return 0;;
    QListIterator<MarginRange> lIt(*leftMargin);
    int bottom=0;
    MarginRange* r;	
    while ( (r = lIt.current()) )
    {
	if (r->endY>bottom)
	    bottom=r->endY;
	++lIt;
    }
    return bottom;

}

int
HTMLBlockElementImpl::getRightBottom() {
    if (!rightMargin)
	return 0;
    QListIterator<MarginRange> rIt(*rightMargin);
    int bottom=0;
    MarginRange* r;	
    while ( (r = rIt.current()) )
    {
	if (r->endY>bottom)
	    bottom=r->endY;
	++rIt;
    }
    return bottom;
}


void
HTMLBlockElementImpl::clearMargins()
{
    if (leftMargin)
    	leftMargin->clear();
    if (rightMargin)
    	rightMargin->clear();
}

int
HTMLBlockElementImpl::getWidth(int y) {
    int res;
    res = getRightMargin(y)-getLeftMargin(y);
//    printf("y:%d, r:%d, l:%d\n",y,getRightMargin(y),getLeftMargin(y));
    if (res<0) res=0;
    return res;
}

void
HTMLBlockElementImpl::calcFloating(NodeImpl *child, int elemY)
{
    // a floating element

    if (child->hAlign()==Left)
    {
    	int fx = getLeftMargin(elemY);
	int fy = elemY;
	if (getRightMargin(elemY)-fx < child->getWidth())
	{
	    fx=0;
	    fy=getLeftBottom()+1;
	}
	child->setXPos(fx+child->hSpace());			
	child->setYPos(fy+child->vSpace());
	insertMarginElement(Left,fy,child);
    }
    else
    {
	child->setXPos(width-child->getWidth() - child->hSpace());
	child->setYPos(elemY+child->vSpace());
	insertMarginElement(Right,elemY,child);
    }	

}


HTMLBlockElementImpl::HTMLParagraphClose
    HTMLBlockElementImpl::pElemClose = HTMLParagraphClose();

NodeImpl *HTMLBlockElementImpl::calcParagraph(NodeImpl *_start, bool pre)
{
#ifdef PAR_DEBUG
    printf("calcParagraph\n");
#endif

    QStack<NodeImpl> nodeStack;

    DOMStringImpl *text;

    NodeImpl *current = _start;
    NodeImpl *retval = 0;

    QList<NodeImpl> renderedNodes;


    // lets get all parts of the paragraph which get rendered
    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	
	    // Mark paragraph endings with pseudo nodes.
	    // Paragraphs are really block elements but are
	    // treated as inline. </P> implies line break,
	    // so it must also be rendered. This is a trick
	    // to do it.
 	    if (current->id() == ID_P && current != _first &&
 	    	(!current->nextSibling() || current->nextSibling()->id()!=ID_P))
    	    	renderedNodes.append(&pElemClose);
	    current = current->nextSibling();
	    continue;
	}
	else if(current->isFloating())
	{
	    renderedNodes.append(current);
	    current = current->nextSibling();
	    continue;
	}
	else if(!current->isInline())
	{
#ifdef DEBUG // ### debugging...	
	    if(!nodeStack.isEmpty())
		printf("Error in calcParagrph!\n");
#endif		
	    retval = current;
	    break;
	}
	else if(current->isRendered())
	{
	    renderedNodes.append(current);	
	}	

	if(current->isTextNode())
	    (static_cast<TextImpl *>(current))->deleteSlaves();
	
	NodeImpl *child = current->firstChild();
	if(child && current->childrenRendered())
	{	
	    nodeStack.push(current);
	    current = child;
	}
	else
	{
	    current = current->nextSibling();
	}
    }

    // let's put the lines together now
#ifdef PAR_DEBUG
    printf("got %d nodes in paragraph\n", renderedNodes.count());
#endif

    QListIterator<NodeImpl> startNode(renderedNodes);
    QListIterator<NodeImpl> endNode(renderedNodes);
    int startPos = 0;
    int endPos = 0;

    HAlign elemPAlign=HNone;

    QFontMetrics fm(*getFont());
    int defAscent=fm.ascent();
    int defDescent=fm.descent();

    int lineAscent = 0;
    int lineDescent = 0;

    TextSlave *slave = 0;

    while(endNode.current())
    {
	QListIterator<NodeImpl> testNode = endNode;
	int testPos = endPos;
	startNode = endNode;
	startPos = endPos;

	lineAscent = 0;
	lineDescent = 0;
	
	int currentAscent = 0;
	int currentDescent = 0;
    	int secondaryAscent = 0;

	bool startOfLine = true;
	bool skipBlank = true;
	bool skipNewline = false;
	int w = 0;
	bool lineFull = false;
	VAlign lineAlign = VNone;
	bool breakPosFound = false;
	
#ifdef PAR_DEBUG
	printf("looking for line from %p/%d\n", startNode.current(), startPos);
#endif

	HAlign align;
	if (elemPAlign!=HNone)
	    align = elemPAlign;
	else
	    align = hAlign();


	// accumulate until we have enough to fill the line
	while((current = testNode.current()))
	{
	    if(current->isTextNode())
	    {
#ifdef PAR_DEBUG
		printf("found textNode\n");
#endif	    	    	    	
		TextImpl *t = static_cast<TextImpl *>(current);
		
		QFontMetrics fm(*t->getFont());
		currentAscent = fm.ascent();
		currentDescent = fm.descent();		
		if(currentAscent > lineAscent) {
    	    	    secondaryAscent = lineAscent;
	            lineAscent = currentAscent;
		} else if (currentAscent>secondaryAscent)
		    secondaryAscent = currentAscent;
		if(currentDescent > lineDescent) lineDescent = currentDescent;

		text = t->string();
		// only if are not in <pre>!!!
		if(startOfLine && !pre && (*text)[testPos] == QChar(' ') )
		{
		    testPos += 1;
		
		    if ( skipBlank )
		    {
		      	startPos += 1;
		      	if (endPos<startPos)
		      	    endPos=startPos;
		      	skipBlank = (*text)[testPos] == QChar(' ');
		    }
		}
		int dw = getWidth(descent);
		int len = text->l;
		while(testPos<len)
		{
		    w += fm.width((*text)[testPos]);		
		    if(w > dw)
		    {
			if(!breakPosFound)
			{
			    // ### we don't get a fit... break it, even
			    // if it looks ugly!
			
			    if (dw==width)
			    {
			    	endNode = testNode;
				// are we advancing at all?
				if (startPos==testPos && startNode==testNode)
				    endPos = ++testPos;
				else
			    	    endPos = testPos;
			    }
			    else
			    {
			    	endNode = testNode;
			    	endPos = startPos;
			    }
#ifdef PAR_DEBUG
			    printf("forced break!!!\n");
#endif
			}
			lineFull = true;
#ifdef PAR_DEBUG
			printf("line full at %p/%d w/width: %d/%d/%d\n",
			    testNode.current(), testPos, w, width, dw);
#endif
			break;
		    }
		    else if(!pre && (*text)[testPos] == QChar(' '))
		    {
			// we found the position of a possible line break
			endNode = testNode;
			endPos = testPos;
			startOfLine = false;
			breakPosFound = true;
		    }
		    else if(!pre && (*text)[testPos] == QChar('-'))
		    {
			// we found the position of a possible line break
			endNode = testNode;
			endPos = testPos+1;
			startOfLine = false;
			breakPosFound = true;
		    }
		    else if((*text)[testPos] == QChar('\n'))
		    {
#ifdef PAR_DEBUG
			printf("got newline\n");
#endif
			endNode = testNode;
			endPos = testPos;
			if(currentAscent > lineAscent)
			    lineAscent = currentAscent;
			if(currentDescent > lineDescent)
			    lineDescent = currentDescent;
			lineFull = true;
			skipNewline = true;
			break;
		    }
		    testPos++;
		}
	    	
	    }
	    else if (current->isFloating())
	    {		    	
	    	// just layout floating elements
#ifdef PAR_DEBUG		
		printf("floating\n");
#endif		
		current->layout();
		calcFloating(current,descent);
	    }
	    else
	    {	    	
		switch(current->id())
		{
		case ID_P:
		    elemPAlign = current->hAlign();
		case ID_BR:
#ifdef PAR_DEBUG
		    printf("got linebreaking element\n");
#endif
		    endNode = testNode;
		    endPos = testPos;
		    if(currentAscent > lineAscent) {
		    	secondaryAscent = lineAscent;
		    	lineAscent = currentAscent;
		    } else if (currentAscent>secondaryAscent)
		    	secondaryAscent = currentAscent;
		    if(currentDescent > lineDescent) lineDescent = currentDescent;
		    lineFull = true;
		    break;
		case ID_P + ID_CLOSE_TAG:
		    endNode = testNode;
	    	    endPos = testPos;
		    lineFull=true;
		    elemPAlign = HNone;
		    break;
 		case ID_IMG:
		    if(!current->layouted())
		      	current->layout();	
  		    if(current->getDescent()+current->vSpace() > lineDescent)
  		    {
         	    	if (current->vAlign()==Top)
	    	    	    lineAlign=Top;
    	 		else if (current->vAlign()==VCenter)
     	    		    lineAlign=VCenter;
  	    	    }

		    // continue with default to avoid duplicated code
		default:		
#ifdef PAR_DEBUG		    	
		    printf("layouting element %d\n", current->id());
#endif
		    if(!current->layouted())
		      	current->layout();

		    if( w + current->getWidth() > getWidth(descent)
			&& w != 0 && !startOfLine)
		    {	
#ifdef PAR_DEBUG		    	
		     	printf("too wide width=%d, current=%d, max=%d\n",
			       w, current->getWidth(), getWidth(descent));
			printf("current->id = %d\n", current->id());
#endif			
 			if (endNode.current()->isTextNode())
 			{
			    endPos=-1; // -1=break after this text element
 			}
			lineFull = true;
			break;
		    }
		    else
			w += current->getWidth()+current->hSpace()*2;
		    currentAscent = current->getAscent()+current->vSpace();
		    currentDescent = current->getDescent()+current->vSpace();
		    if(currentAscent > lineAscent) {
		    	secondaryAscent = lineAscent;
		    	lineAscent = currentAscent;
		    } else if (currentAscent>secondaryAscent)
		    	secondaryAscent = currentAscent;
		    if(currentDescent > lineDescent)
		    	lineDescent = currentDescent;
			
		    endNode = testNode;
	    	    endPos = testPos;			
		}

	    }

	
	    if(lineFull) {
	    	// correct the line length for text lines
	    	if ( (align==Right || align==HCenter)
		    && current->isTextNode() && testPos>endPos)
		{
		    TextImpl *t = static_cast<TextImpl *>(current);
		    text = t->string();
		
		    QFontMetrics fm(*t->getFont());
		    QConstString str(text->s+endPos, testPos-endPos+1);

		    w -= fm.width(str.string());
		
		}
		// got one line
	    	break;
	    }
	
	    startOfLine = false;
	    ++testNode;	
	
	    testPos = 0;
	}
	// now we know from where to were we have the line
	if(!testNode.current())
	{
#ifdef PAR_DEBUG
	    printf("set end to test\n");
#endif
	    endNode=testNode;
	    endPos = 0;
	}
	
	if (lineAlign==VCenter)
	{
	    lineAscent+=secondaryAscent/2;
	    lineDescent-=secondaryAscent/2;
	}
	
#ifdef PAR_DEBUG
	printf("got line from %p/%d to %p/%d\n", startNode.current(), startPos,
	       endNode.current(), endPos);
	printf("ascent/descent %d/%d\n", lineAscent, lineDescent);
#endif	

	// alignment		
		
    	int xPos;
	
	switch(align)
	{
	case HNone:
	case Left:
	    xPos = getLeftMargin(descent);
	    break;
	case Right:
	    xPos = getRightMargin(descent) - w;
	    break;
	case HCenter:
	    xPos = (getRightMargin(descent) - w)/2;
	default:
	    break;
	}
	
	while(1)
	{
	    current = startNode.current();
	    bool nextNode = false;
	    bool endOfLine = false;
	
	    if(!current) break;

	    if(current->isTextNode())
	    {
#ifdef PAR_DEBUG
		printf("isTextNode() %p\n", current);
#endif
		TextImpl *t = static_cast<TextImpl *>(current);
		text = t->string();

		TextSlave *s;
		// take care of startNode==endNode
		if(current != endNode.current())
		{
		    s = new TextSlave(xPos, descent+lineAscent,
				      startPos, text->l-startPos,
				      text, lineAscent, lineDescent);
		    QFontMetrics fm(*t->getFont());
		    QConstString str(text->s+startPos, text->l-startPos);

		    xPos += fm.width(str.string());
#ifdef PAR_DEBUG
		    printf("current != endNode.current\n");
#endif
		    nextNode = true;
		}
		else
		{
#ifdef PAR_DEBUG
		    printf("startPos=%d,endPos=%d\n",startPos,endPos);
#endif
		    if (endPos==-1)
		    {
		    	endPos = text->l;
			nextNode = true;
		    }		
		    //assert(endPos-startPos>0);
		
		    endOfLine = true;
		
		    if (endPos==startPos)		    	
		    	break;		
		
		    s = new TextSlave(xPos, descent+lineAscent,
				      startPos, endPos-startPos,
				      text, lineAscent, lineDescent);
#ifdef PAR_DEBUG
		    printf("enpos = %d, len = %d\n", endPos, text->l);
#endif
		    startPos = endPos;		
		}
		if(slave)
		    slave->setNext(s);
		else
		    t->first = s;
#ifdef PAR_DEBUG
		printf("added slave %p\n", s);
#endif
		slave = s;
	    }
	    else if (current->isFloating())
    	    {
		nextNode=true;
		if(current == endNode.current())
		    endOfLine = true;
	    }	
	    else
	    { 	
#ifdef PAR_DEBUG	    	
	    	printf("current id=%d\n", current->id());
#endif		
		switch(current->id())
		{
		case ID_P:
		    // ### should be defined by style sheet...
		    // ### add indent for next line acc to style
		    if (current!=_start)
		    	lineDescent += 8;		
		    endOfLine = true;
		    nextNode = true;
		    break;
 		case ID_P + ID_CLOSE_TAG:				
		    lineDescent += 8;
		    if (lineAscent==0)
		    	lineAscent=defAscent;
		    endOfLine = true;
		    nextNode = true;
		    break;		
		case ID_BR:
		    {
		    HTMLBRElementImpl *br =
		    	static_cast<HTMLBRElementImpl*>(current);
			
		    if (lineAscent==0)
		    	lineAscent=defAscent;			
			
		    switch(br->clear())
		    {
		    	case BRNone:
			    break;			
			case BRLeft:
			    descent = MAX(descent,getLeftBottom());
			    break;
			case BRRight:
			    descent = MAX(descent,getRightBottom());
			    break;
			case BRAll:
			    descent = MAX(descent,
			    	MAX(getLeftBottom(),getRightBottom()));
				
		    }
		    endOfLine = true;
		    nextNode = true;
		    }
		    break;
 		default:	    		
 		    current->setXPos(xPos+current->hSpace());
  		    int asc;
 		    if (lineAlign==Top && current->vAlign()==Top)
  		    	asc=0;
 		    else if (lineAlign==VCenter && current->vAlign()==VCenter)
  		    	asc=lineAscent-secondaryAscent/2;
  		    else
  		    	asc=lineAscent;
 		    current->setYPos(descent + asc + current->vSpace());
 		    xPos += current->getWidth()+current->hSpace()*2;
		    if(current == endNode.current())
		    	endOfLine = true;
		    nextNode = true;
 		    break;				
  		}		
	    }
	    if(nextNode)
	    {
		++startNode;
		startPos = 0;
		slave = 0;
	    }
	    if(endOfLine) break;
	}

	endNode = startNode;
	endPos = startPos;
	if (lineAlign==Top)
	{
	    if (lineDescent>lineAscent)
	    	descent += lineDescent;
	    else
	    	descent += lineAscent;
	}
	else
	    descent += lineAscent + lineDescent;

#ifdef PAR_DEBUG
	printf("descent = %d\n", descent);
#endif
	if(skipNewline)
	{
	    endPos++;
	    assert(endNode.current()->id() == ID_TEXT);
	    TextImpl *t = static_cast<TextImpl *>(endNode.current());
	    if(endPos > (int)t->string()->l)
	    {
		endPos = 0;
		++endNode;
	    }
	}
    }
    if (lineDescent)
    	descent+=defDescent;	

#ifdef PAR_DEBUG
    printf("calcParagraph end\n");
#endif
    return retval;
}

NodeImpl *HTMLBlockElementImpl::addChild(NodeImpl *newChild)
{
#ifdef DEBUG_LAYOUT
    printf("%s(BlockElement)::addChild( %s )\n", nodeName().string().ascii(), newChild->nodeName().string().ascii());
#endif

    newChild->setAvailableWidth(availableWidth);
    width=availableWidth;

    if(_last && !_last->isInline() && !_last->isFloating())
	descent += _last->getDescent() +_last->getAscent();;

    NodeImpl* child = NodeBaseImpl::addChild(newChild);

    if(newChild->isInline())
    {
    	if(!startPar)
	    startPar = newChild;
    }
    else
    {
	if(availableWidth)
	{
	    if(startPar)
	    {	    		    	
		calcParagraph(startPar);
		// printme...
		// ### not very efficient...
		static_cast<HTMLDocumentImpl *>(document)->print(this);
		startPar = 0;
	    }
	    if(child->isFloating())
	    {
    		calcFloating(newChild,descent);	
	    }
	    else
	    {		
		int x = getLeftMargin(descent);
		int w = getWidth(descent);
		child->setYPos(descent);
 		child->setAvailableWidth(w);
		switch(hAlign())
		{
		case Left:
		    break;
		case Right:
		    x = x + w - child->getWidth();
		    break;
		case HCenter:
		    x = x + (w - child->getWidth())/2;
		    break;
		default:
		    break;
		}
		child->setXPos(x);	
	    }
	}	
    }

    return newChild;
}



// --------------------------------------------------------------------------

const DOMString HTMLGenericBlockElementImpl::nodeName() const
{
    return getTagName(_id);
}

void HTMLGenericBlockElementImpl::setStyle(CSSStyle *currentStyle)
{
    switch(_id)
    {
    case ID_CENTER:
	halign = HCenter;
	break;
    case ID_ADDRESS:
	currentStyle->font.family = pSettings->fixedFontFace;
	currentStyle->font.fixed = true;
	currentStyle->font.style = CSSStyleFont::stNormal;
	currentStyle->font.weight = CSSStyleFont::Normal;
	break;
    default:
	break;
    }

    HTMLElementImpl::setStyle(currentStyle);
}

