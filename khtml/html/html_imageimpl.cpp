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
 */

#include "html/html_imageimpl.h"
#include "html/html_documentimpl.h"

#include "misc/htmlhashes.h"
#include "khtmlview.h"
#include "khtml_part.h"

#include <kstringhandler.h>
#include <kdebug.h>

#include "rendering/render_image.h"
#include "rendering/render_flow.h"
#include "css/cssstyleselector.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "css/csshelper.h"
#include "xml/dom2_eventsimpl.h"

#include <qstring.h>
#include <qpoint.h>
#include <qregion.h>
#include <qptrstack.h>
#include <qimage.h>
#include <qpointarray.h>

using namespace DOM;
using namespace khtml;

// -------------------------------------------------------------------------

HTMLImageElementImpl::HTMLImageElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
    ismap = false;
}

HTMLImageElementImpl::~HTMLImageElementImpl()
{
}


// DOM related

NodeImpl::Id HTMLImageElementImpl::id() const
{
    return ID_IMG;
}

bool HTMLImageElementImpl::prepareMouseEvent( int _x, int _y,
                                              int _tx, int _ty,
                                              MouseEvent *ev )
{
    //kdDebug( 6030 ) << "_x=" << _x << " _tx=" << _tx << " _y=" << _y << ", _ty=" << _ty << endl;
    bool inside = HTMLElementImpl::prepareMouseEvent(_x, _y, _tx, _ty, ev);
    if ( inside && usemap.length() > 0 &&
         ( ! (m_render && m_render->style()->visibility() == HIDDEN) ) )
    {
        RenderObject* p = m_render->parent();
        while( p && p->isAnonymousBox() )
        {
            //kdDebug( 6030 ) << "parent is anonymous!" << endl;
            // we need to add the offset of the anonymous box
            _tx += p->xPos();
            _ty += p->yPos();
            p = p->parent();
        }

        //cout << "usemap: " << usemap.string() << endl;
        HTMLMapElementImpl* map;
        DocumentImpl* doc = getDocument();

        if(doc && doc->isHTMLDocument() &&
           (map = static_cast<HTMLDocumentImpl*>(doc)->getMap(usemap)))
        {
            //kdDebug( 6030 ) << "have map" << endl;
            return map->mapMouseEvent(_x-renderer()->xPos()-_tx,
                                      _y-renderer()->yPos()-_ty,
                                      renderer()->width(), renderer()->height(), ev);
        }
    }
    return inside;
}

void HTMLImageElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch (attr->id())
    {
    case ATTR_SRC:
        m_imageURL = khtml::parseURL(attr->value());
        break;
    case ATTR_WIDTH:
        addCSSLength(CSS_PROP_WIDTH, attr->value());
        break;
    case ATTR_HEIGHT:
        addCSSLength(CSS_PROP_HEIGHT, attr->value());
        break;
    case ATTR_BORDER:
        // border="noborder" -> border="0"
        if(attr->value().toInt()) {
            addCSSLength(CSS_PROP_BORDER_WIDTH, attr->value());
            addCSSProperty( CSS_PROP_BORDER_TOP_STYLE, CSS_VAL_SOLID );
            addCSSProperty( CSS_PROP_BORDER_RIGHT_STYLE, CSS_VAL_SOLID );
            addCSSProperty( CSS_PROP_BORDER_BOTTOM_STYLE, CSS_VAL_SOLID );
            addCSSProperty( CSS_PROP_BORDER_LEFT_STYLE, CSS_VAL_SOLID );
        }
        break;
    case ATTR_VSPACE:
        addCSSLength(CSS_PROP_MARGIN_TOP, attr->value());
        addCSSLength(CSS_PROP_MARGIN_BOTTOM, attr->value());
        break;
    case ATTR_HSPACE:
        addCSSLength(CSS_PROP_MARGIN_LEFT, attr->value());
        addCSSLength(CSS_PROP_MARGIN_RIGHT, attr->value());
        break;
    case ATTR_ALIGN:
	addHTMLAlignment( attr->value() );
	break;
    case ATTR_VALIGN:
	addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value());
        break;
    case ATTR_USEMAP:
        if ( attr->value()[0] == '#' )
            usemap = attr->value();
        else {
            QString url = getDocument()->completeURL( khtml::parseURL( attr->value() ).string() );
            // ### we remove the part before the anchor and hope
            // the map is on the same html page....
            usemap = url;
        }
    case ATTR_ISMAP:
        ismap = true;
        break;
    case ATTR_ALT:
        break;
    case ATTR_ONABORT: // ### add support for this
        setHTMLEventListener(EventImpl::ABORT_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONERROR: // ### add support for this
        setHTMLEventListener(EventImpl::ERROR_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONLOAD:
        setHTMLEventListener(EventImpl::LOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_NAME:
    case ATTR_NOSAVE:
	break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

DOMString HTMLImageElementImpl::altText() const
{
    // lets figure out the alt text.. magic stuff
    // http://www.w3.org/TR/1998/REC-html40-19980424/appendix/notes.html#altgen
    // also heavily discussed by Hixie on bugzilla
    DOMString alt( getAttribute( ATTR_ALT ) );
    // fall back to title attribute
    if ( alt.isNull() )
        alt = getAttribute( ATTR_TITLE );
    if ( alt.isNull() ) {
        QString p = KURL( getDocument()->completeURL( m_imageURL.string() ) ).prettyURL();
        int pos;
        if ( ( pos = p.findRev( '.' ) ) > 0 )
            p.truncate( pos );
        alt = DOMString( KStringHandler::csqueeze( p ) );
    }

    return alt;
}

void HTMLImageElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

    if (parentNode()->renderer()) {
        m_render = new RenderImage(this);
        m_render->setStyle(getDocument()->styleSelector()->styleForElement(this));
        parentNode()->renderer()->addChild(m_render, nextRenderer());
        static_cast<RenderImage*>(m_render)->setAlt(altText());
        static_cast<RenderImage*>(m_render)->setImageUrl(m_imageURL,getDocument()->docLoader());
    }

    NodeBaseImpl::attach();
}

void HTMLImageElementImpl::recalcStyle( StyleChange ch )
{

    HTMLElementImpl::recalcStyle( ch );
    // ### perhaps not the most appropriate place for this.... here so it get's called after
    // a script has executed
    if (m_render) {
        RenderImage* renderImage = static_cast<RenderImage*>( m_render );
        renderImage->setImageUrl(m_imageURL, getDocument()->docLoader());
        renderImage->setAlt(altText());
    }
}

QImage HTMLImageElementImpl::currentImage() const
{
    RenderImage *r = static_cast<RenderImage*>(renderer());

    if(r)
        return r->pixmap().convertToImage();

    return QImage();
}

// -------------------------------------------------------------------------

HTMLMapElementImpl::HTMLMapElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
}

HTMLMapElementImpl::~HTMLMapElementImpl()
{
    if(getDocument() && getDocument()->isHTMLDocument())
        static_cast<HTMLDocumentImpl*>(getDocument())->mapMap.remove(name);
}

NodeImpl::Id HTMLMapElementImpl::id() const
{
    return ID_MAP;
}

bool
HTMLMapElementImpl::mapMouseEvent(int x_, int y_, int width_, int height_,
                                  MouseEvent *ev )
{
    //cout << "map:mapMouseEvent " << endl;
    //cout << x_ << " " << y_ <<" "<< width_ <<" "<< height_ << endl;
    QPtrStack<NodeImpl> nodeStack;

    NodeImpl *current = firstChild();
    while(1)
    {
        if(!current)
        {
            if(nodeStack.isEmpty()) break;
            current = nodeStack.pop();
            current = current->nextSibling();
            continue;
        }
        if(current->id()==ID_AREA)
        {
            //cout << "area found " << endl;
            HTMLAreaElementImpl* area=static_cast<HTMLAreaElementImpl*>(current);
            if (area->mapMouseEvent(x_,y_,width_,height_,ev))
                return true;
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

    return false;
}

void HTMLMapElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch (attr->id())
    {
    case ATTR_ID:
        if (getDocument()->htmlMode() != DocumentImpl::XHtml) break;
        // fall through
    case ATTR_NAME:
    {
        DOMString s = attr->value();
        if(*s.unicode() == '#')
            name = QString(s.unicode()+1, s.length()-1);
        else
            name = s.string();
	// ### make this work for XML documents, e.g. in case of <html:map...>
        if(getDocument()->isHTMLDocument())
            static_cast<HTMLDocumentImpl*>(getDocument())->mapMap[name] = this;
        break;
    }
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLAreaElementImpl::HTMLAreaElementImpl(DocumentPtr *doc)
    : HTMLAnchorElementImpl(doc)
{
    coords=0L;
    nohref = false;
    shape = Unknown;
    lasth = lastw = -1;
}

HTMLAreaElementImpl::~HTMLAreaElementImpl()
{
    delete coords;
}

NodeImpl::Id HTMLAreaElementImpl::id() const
{
    return ID_AREA;
}

void HTMLAreaElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch (attr->id())
    {
    case ATTR_SHAPE:
        if ( strcasecmp( attr->value(), "default" ) == 0 )
            shape = Default;
        else if ( strcasecmp( attr->value(), "circle" ) == 0 )
            shape = Circle;
        else if ( strcasecmp( attr->value(), "poly" ) == 0 )
            shape = Poly;
        else if ( strcasecmp( attr->value(), "rect" ) == 0 )
            shape = Rect;
        break;
    case ATTR_COORDS:
        coords = attr->val()->toLengthList();
        break;
    case ATTR_NOHREF:
        nohref = attr->val() != 0;
        break;
    case ATTR_ALT:
        break;
    case ATTR_ACCESSKEY:
        break;
    default:
        HTMLAnchorElementImpl::parseAttribute(attr);
    }
}

bool
HTMLAreaElementImpl::mapMouseEvent(int x_, int y_, int width_, int height_,
                                   MouseEvent *ev)
{
    //cout << "area:mapMouseEvent " << endl;
    bool inside = false;
    if (width_ != lastw || height_ != lasth)
    {
        region=getRegion(width_, height_);
        lastw=width_; lasth=height_;
    }
    if (region.contains(QPoint(x_,y_)))
    {
        inside = true;
        ev->innerNode = this;
        if(isNoref())
            ev->noHref = true;
        else {
            DOMString href = khtml::parseURL(getAttribute(ATTR_HREF));
            if(m_hasTarget && m_hasHref)
            {
                DOMString s = DOMString("target://") + getAttribute(ATTR_TARGET) + DOMString("/#")
                              + DOMString(href);
                ev->url = s;
            }
            else
                ev->url = href;
        }
    }
    // dynamic HTML...
    // ### if(inside || mouseInside()) mouseEventHandler(ev, inside);

    return inside;
}

QRect HTMLAreaElementImpl::getRect() const
{
    if (parentNode()->renderer()==0)
        return QRect();
    int dx, dy;
    if (!parentNode()->renderer()->absolutePosition(dx, dy))
        return QRect();
    QRegion region = getRegion(lastw,lasth);
    region.translate(dx, dy);
    return region.boundingRect();
}

QRegion HTMLAreaElementImpl::getRegion(int width_, int height_) const
{
    QRegion region;
    if (!coords)
        return region;

    // added broken HTML support (Dirk): some pages omit the SHAPE
    // attribute, so we try to guess by looking at the coords count
    // what the HTML author tried to tell us.

    // a Poly needs at least 3 points (6 coords), so this is correct
    // just ignore poly's with 2 points or less, because
    // QRegion will anyway crash on them.
    if ((shape==Poly || shape==Unknown) && coords->count() > 4)
    {
        //cout << " poly " << endl;
        bool xcoord=true;
        int xx = 0, yy = 0; // shut up egcs...
        int i=0;

        QPtrListIterator<Length> it(*coords);
        QPointArray points(it.count()/2);
        for ( ; it.current(); ++it ) {
            Length* len = it.current();
            if (xcoord)
            {
                xx = len->minWidth(width_);
                xcoord = false;
            } else {
                yy = len->minWidth(height_);
                xcoord = true;
                points.setPoint(i,xx,yy);
                i++;
            }
        }
        region = QRegion(points);
    }
    else if (shape==Circle && coords->count()>=3 || shape==Unknown && coords->count() == 3)
    {
        //cout << " circle " << endl;
        int cx = coords->at(0)->minWidth(width_);
        int cy = coords->at(1)->minWidth(height_);
        int r1 = coords->at(2)->minWidth(width_);
        int r2 = coords->at(2)->minWidth(height_);
        int r  = QMIN(r1, r2);

        region = QRegion(cx-r, cy-r, 2*r, 2*r,QRegion::Ellipse);
    }
    else if (shape==Rect && coords->count()>=4 || shape==Unknown && coords->count() == 4)
    {
        //cout << " rect " << endl;
        int x0 = coords->at(0)->minWidth(width_);
        int y0 = coords->at(1)->minWidth(height_);
        int x1 = coords->at(2)->minWidth(width_);
        int y1 = coords->at(3)->minWidth(height_);
        region = QRegion(x0,y0,x1-x0,y1-y0);
    }
    else /*if (shape==Default || shape == Unknown)*/ {
        //cout << "default/unknown" << endl;
        region = QRegion(0,0,width_,height_);
    }

    return region;
}

