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
#ifndef HTML_IMAGEIMPL_H
#define HTML_IMAGEIMPL_H

#include "dtd.h"
#include "html_elementimpl.h"
#include "misc/loader.h"
#include "html_documentimpl.h"

#include <qmap.h>
#include <qpixmap.h>

namespace DOM {

class DOMString;

class HTMLImageElementImpl
    : public HTMLElementImpl
{
public:
    HTMLImageElementImpl(DocumentImpl *doc);

    ~HTMLImageElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return IMGStartTag; }
    virtual tagStatus endTag() { return IMGEndTag; }

    virtual void parseAttribute(khtml::Attribute *);

    virtual bool mouseEvent( int _x, int _y, int button, MouseEventType type,
                             int _tx, int _ty, DOMString &url,
                             NodeImpl *&innerNode, long &offset );

    virtual void attach(KHTMLView *w);
    virtual void applyChanges(bool top=true);

protected:
    bool ismap;

    /**
     * The URL of this image.
     */
    DOMString imageURL;

    // text to display as long as the image isn't available
    DOMString alt;

    DOMString usemap;
};


//------------------------------------------------------------------



class HTMLAreaElementImpl : public HTMLElementImpl
{
public:

    enum Shape { Default, Poly, Rect, Circle, Unknown };

    HTMLAreaElementImpl(DocumentImpl *doc);

    ~HTMLAreaElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual DOMString areaHref() const {
    	return href;
    }

    virtual tagStatus startTag() { return AREAStartTag; }
    virtual tagStatus endTag() { return AREAEndTag; }

    long tabIndex() const;
    void setTabIndex( long );

    void parseAttribute(khtml::Attribute *attr);

    bool isDefault() { return shape==Default; }

    bool mapMouseEvent(int x_, int y_, int width_, int height_,
    	int button_, MouseEventType type_, DOMString& url_);

protected:

    QRegion getRegion(int width_, int height);

    Shape shape;
    QList<khtml::Length>* coords;
    DOMStringImpl *href;
    DOMStringImpl *target;

    QRegion region;

    int lastw, lasth;
};


// -------------------------------------------------------------------------

class HTMLMapElementImpl : public HTMLElementImpl
{
public:
    HTMLMapElementImpl(DocumentImpl *doc);

    ~HTMLMapElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual DOMString getName() const {
    	return name;
    }

    virtual void parseAttribute(khtml::Attribute *attr);

    virtual tagStatus startTag() { return MAPStartTag; }
    virtual tagStatus endTag() { return MAPEndTag; }

    bool mapMouseEvent(int x_, int y_, int width_, int height_,
    	int button_, MouseEventType type_, DOMString& url_);

    static HTMLMapElementImpl* getMap(const DOMString& url_);

private:

    QString name;
    // ### FIxme: this doesn't work, if we have multiple views at the same time.
    // the map has to be somehow attached to the document.
    static QMap<QString,HTMLMapElementImpl*> mapMap;
};


}; //namespace

#endif
