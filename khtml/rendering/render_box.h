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
#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include "render_object.h"
#include "loader.h"

namespace khtml {


inline int MAX(int a, int b)
{
    return a > b ? a : b;
}

inline int MIN(int a, int b)
{
    return a < b ? a : b;
}

class RenderBox : public RenderObject
{


// combines ElemImpl & PosElImpl (all rendering objects are positioned)
// should contain all border and padding handling

public:
    RenderBox();
    virtual ~RenderBox();

    virtual const char *renderName() const { return "RenderBox"; }

    virtual void setStyle(RenderStyle *style);
    
    virtual void print(QPainter *p, int _x, int _y, int _w, int _h,
		       int _tx, int _ty);

    virtual void updateSize();
    virtual void updateHeight();
    virtual void close();

    virtual QSize contentSize() const;
    virtual QSize contentOffset() const;
    virtual QSize paddingSize() const;
    virtual QSize size() const;

    virtual short minWidth() const { return m_minWidth; }
    virtual short maxWidth() const { return m_maxWidth; }
    virtual void setMinWidth(short w) { m_minWidth = w; m_layouted = false; }
    virtual void  setMaxWidth(short w) { m_maxWidth = w; m_layouted = false; }

    virtual short contentWidth() const;
    virtual int contentHeight() const;

    virtual void absolutePosition(int &xPos, int &yPos);

    virtual void setPos( int xPos, int yPos ) { m_x = xPos, m_y = yPos; }
    virtual void setXPos( int xPos ) { m_x = xPos; }
    virtual void setYPos( int yPos ) { m_y = yPos; }

    virtual int xPos() const { return m_x; }
    virtual int yPos() const { return m_y; }
    virtual short width() const;
    virtual int height() const;

    virtual void setSize( int width, int height ) { m_width = width; m_height = height; }
    virtual void setWidth( int width ) { m_width = width; }
    virtual void setHeight( int height ) { m_height = height; }

    // for table cells
    virtual short baselineOffset() const;

    // from BidiObject
    virtual short verticalPositionHint() const;
    virtual int bidiHeight() const;

    // from BiDiObject
    virtual void position(int x, int y, int from, int len, int width, bool reverse);
    virtual unsigned int width( int, int) const { return width(); }

    void repaint();

    virtual void repaintRectangle(int x, int y, int w, int h);
    virtual void repaintObject(RenderObject *o, int x, int y);

    virtual void setPixmap(const QPixmap &);

    virtual short containingBlockWidth() const;

protected:
    virtual void printBoxDecorations(QPainter *p,int _x, int _y,
				       int _w, int _h, int _tx, int _ty);
    void outlineBox(QPainter *p, int _tx, int _ty, const char *color = "red");

    virtual int cellTopExtra() { return 0; }
    virtual int cellBottomExtra() { return 0; }

    /*
     * the minimum width the element needs, to be able to render
     * it's content without clipping
     */
    short m_minWidth;
    /* The maximum width the element can fill horizontally
     * ( = the width of the element with line breaking disabled)
     */
    short m_maxWidth;

    short m_x;
    int m_y;

    // the actual width of the contents + borders + padding
    short m_width;
    // the actual height of the contents + borders + padding
    int m_height;
};


}; //namespace

#endif
