/* This file is part of the KDE libraries
   Copyright (c) 1999 Daniel Duley <mosfet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <kstyle.h>
#include <qapplication.h>
#include <qdrawutil.h>

#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <qmenudata.h>

// lame default implementations of KStyle drawing methods. These are meant
// to be overridden in new styles.

void KStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool horizontal,
                            QBrush *fill)
{
    int stipple_height;
    
    if(horizontal){
        qDrawShadePanel(p, x, y, w, h, g , false, 1, fill);
        p->setPen(g.light());
        stipple_height = 3;
        while(stipple_height < h-4) {
            p->drawPoint(x+1, y+stipple_height+1);
            p->drawPoint(x+4, y+stipple_height);
        stipple_height+=3;
        }
        p->setPen(g.dark());
        stipple_height = 4;
        while(stipple_height < h-4) {
            p->drawPoint(x+2, y+stipple_height+1);
            p->drawPoint(x+5, y+stipple_height);
            stipple_height+=3;
        }
    }
    else{
        qDrawShadePanel(p, x, y, w, h, g , false, 1, fill);
        p->setPen(g.light());
        stipple_height = 3;
        while(stipple_height < w-4) {
            p->drawPoint(x+stipple_height+1, y+1);
            p->drawPoint(x+stipple_height, y+4);
            stipple_height+=3;
        }
        p->setPen(g.dark());
        stipple_height = 4;
        while(stipple_height < w-4) {
            p->drawPoint(x+stipple_height+1, y+2);
            p->drawPoint(x+stipple_height, y+5);
            stipple_height+=3;
        }
    }
}

void KStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool floating)
{
    if(!floating)
        qDrawShadePanel(p, x, y, w, h, g , false, 1);
}

void KStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h, const
                                QColorGroup &g, bool sunken, bool raised,
                                bool enabled, bool popup,
                                KToolButtonType icontext, const QString& btext,
                                const QPixmap *pixmap,
                                QFont *font, QWidget *)
{
    if ( sunken )
        qDrawWinButton(p, x, y, w, h, g, true );
    else if ( raised )
        qDrawWinButton(p, x, y, w, h, g, false );

    int dx, dy;

    if (icontext == Icon){ // icon only
        if (pixmap){
            dx = ( w - pixmap->width() ) / 2;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken )
            {
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
    }
    else if (icontext == IconTextRight){ // icon and text (if any)
        if (pixmap){
            dx = 1;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            if (pixmap)
                dx= pixmap->width();
            else
                dx= 1;
            dy = 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == Text){ // only text, even if there is a icon
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            dx= 1;
            dy= 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == IconTextBottom){
        if (pixmap){
            dx = (w - pixmap->width()) / 2;
            dy = 1;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignBottom|AlignHCenter;
            if (!enabled)
                p->setPen(g.dark());
            dy= pixmap->height();
            dx = 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x, y, w, h-4, tf, btext);
        }
    }
    //#warning What about Icontext=3

    if (popup){
        if (enabled)
            qDrawArrow (p, DownArrow, WindowsStyle, false, w-5, h-5, 0, 0,
                        g, true);
        else
            qDrawArrow (p, DownArrow, WindowsStyle, false, w-5, h-5,
                        0, 0, g, false);
    }
}

void KStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, QBrush *fill)
{
    QBrush brush = fill ? *fill : g.brush(QColorGroup::Background);
    qDrawWinPanel(p, x, y, w, h, g, false, fill ? fill : &brush);
}

void KStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, bool active, QMenuItem *mi,
                           QBrush *)
{
    QColor btext = g.buttonText();
    if(active)
        qDrawShadePanel(p, x, y, w, h, g, false, 1);
    
    QApplication::style().drawItem(p, x, y, w, h,
                                   AlignCenter|ShowPrefix|DontClip|SingleLine,
                                   g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                   -1, &btext );;
}

void KStyle::drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &, QBrush *fill)
{
    p->fillRect(x, y, w, h, *fill);
}

void KStyle::getKProgressBackground(const QColorGroup &g, QBrush &bg)
{
    bg = g.brush(QColorGroup::Base);
}

void KStyle::drawOPToolBar(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, QBrush *fill)
{
    QBrush brush = fill ? *fill : g.brush(QColorGroup::Background);
    qDrawWinPanel(p, x, y, w, h, g, false, fill ? fill : &brush);
}

#include "kstyle.moc"

