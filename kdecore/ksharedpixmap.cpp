/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE libraries.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 *
 * Shared pixmap client for KDE.
 *
 * 5 Dec 99 Geert Jansen:
 *
 *	MAJOR change: KSharedPixmap is client-only and asynchronous now. It
 *	uses the new selection mechanism to transfer the pixmap handle. This
 *	has the advantage that shared pixmaps can be deleted and/or changed
 *	now, and that the very ugly XSetCloseDownMode() is not necessary
 *	anymore.
 *	The server is implemented in KPixmapServer, see the file
 *	kdebase/kdesktop/pixmapserver.cpp.
 *
 * 1 Oct 99 Geert Jansen:
 *
 *	Initial implementation.
 */

#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qwindowdefs.h>
#include <qwidget.h>

#include <kapp.h>
#include <krootprop.h>
#include <ksharedpixmap.h>

#include <X11/Xlib.h>


/**
 * KSharedPixmap
 */

KSharedPixmap::KSharedPixmap()
    : QWidget(0L, "shpixmap comm window")
{
    init();
}


KSharedPixmap::~KSharedPixmap()
{
}


void KSharedPixmap::init()
{
    pixmap = XInternAtom(qt_xdisplay(), "PIXMAP", false);
    target = XInternAtom(qt_xdisplay(), "target prop", false);
    m_Selection = None;
}


bool KSharedPixmap::isAvailable(QString name)
{
    QString str = QString("KDESHPIXMAP:%1").arg(name);
    Atom sel = XInternAtom(qt_xdisplay(), str.latin1(), true);
    if (sel == None)
	return false;
    return XGetSelectionOwner(qt_xdisplay(), sel) != None;
}


bool KSharedPixmap::loadFromShared(QString name, QRect rect)
{
    if (m_Selection != None)
	// already active
	return false;

    m_Rect = rect;
    QPixmap::resize(0, 0); // invalidate

    QString str = QString("KDESHPIXMAP:%1").arg(name);
    m_Selection = XInternAtom(qt_xdisplay(), str.latin1(), true);
    if (m_Selection == None)
	return false;
    if (XGetSelectionOwner(qt_xdisplay(), m_Selection) == None) {
	m_Selection = None;
	return false;
    }

    XConvertSelection(qt_xdisplay(), m_Selection, pixmap, target,
	    winId(), CurrentTime);
    return true;
}


bool KSharedPixmap::x11Event(XEvent *event)
{
    if (event->type != SelectionNotify)
	return false;
	
    XSelectionEvent *ev = &event->xselection;
    if (ev->selection != m_Selection)
	return false;

    if ((ev->target != pixmap) || (ev->property == None)) {
	m_Selection = None;
	emit done(false);
	return true;
    }

    // Read pixmap handle from ev->property

    int dummy, format;
    unsigned long nitems, ldummy;
    Drawable *pixmap_id;
    Atom type;

    XGetWindowProperty(qt_xdisplay(), winId(), ev->property, 0, 1, false,
	    pixmap, &type, &format, &nitems, &ldummy,
	    (unsigned char **) &pixmap_id);

    if (nitems != 1) {
	emit done(false);
	return true;
    }

    Window root;
    unsigned int width, height, udummy;
    XGetGeometry(qt_xdisplay(), *pixmap_id, &root, &dummy, &dummy, &width,
	    &height, &udummy, &udummy);

    if (m_Rect.isEmpty()) {
	QPixmap::resize(width, height);
	XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
		0, 0, width, height, 0, 0);
	XDeleteProperty(qt_xdisplay(), winId(), ev->property);
	m_Selection = None;
	emit done(true);
	return true;
    }

    // Do some more processing here: Generate a tile that can be used as a
    // background tile for the rectangle "rect".
	
    unsigned w = m_Rect.width(), h = m_Rect.height();
    unsigned tw = QMIN(width, w), th = QMIN(height, h);
    unsigned xa = m_Rect.x() % width, ya = m_Rect.y() % height;
    unsigned t1w = QMIN(width-xa,tw), t1h = QMIN(height-ya,th);

    QPixmap::resize(tw, th);

    XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
	    xa, ya, t1w, t1h, 0, 0);
    XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
	    0, ya, tw-t1w, t1h, t1w, 0);
    XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
	    xa, 0, t1w, th-t1h, 0, t1h);
    XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
	    0, 0, tw-t1w, th-t1h, t1w, t1h);

    m_Selection = None;
    XDeleteProperty(qt_xdisplay(), winId(), ev->property);
    emit done(true);
    return true;
}


#include "ksharedpixmap.moc"
