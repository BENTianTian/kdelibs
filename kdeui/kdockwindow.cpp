/* This file is part of the KDE libraries

    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "kdockwindow.h"
#include "kpopmenu.h"
#include "kapp.h"
#include "klocale.h"
#include <kwin.h>

KDockWindow::KDockWindow( QWidget* parent, const char* name )
    : QLabel( parent, name, WType_TopLevel )
{
    KWin::setDockWindow( winId(), parent?parent->topLevelWidget()->winId():0 );
    hasQuit = 0;
    menu = new KPopupMenu( this );
    menu->setTitle( kapp->caption() );
}

KDockWindow::~KDockWindow()
{

}


void KDockWindow::showEvent( QShowEvent * )
{
    if ( !hasQuit ) {
	menu->insertSeparator();
	if ( parentWidget() ) {
	    minimizeRestoreId = menu->insertItem(i18n("Minimize"), this, SLOT( toggleMinimizeRestore() ) );
	    menu->insertItem(i18n("Quit"), parentWidget(), SLOT(close() ) );
	}
	else {
	    minimizeRestoreId = -1;
	    menu->insertItem(i18n("Quit"), qApp, SLOT(closeAllWindows() ) );
	}
	hasQuit = 1;
    }
}

KPopupMenu* KDockWindow::contextMenu()
{
    return menu;
}


void KDockWindow::mousePressEvent( QMouseEvent * e)
{
    switch ( e->button() ) {
    case LeftButton:
	if ( parentWidget() ){
	    parentWidget()->show();
	    KWin::setActiveWindow( parentWidget()->winId() );
	}
	break;
    case MidButton:
	// fall through
    case RightButton:
	if ( parentWidget() ) {
	    if ( parentWidget()->isVisible() )
		menu->changeItem( minimizeRestoreId, i18n("Minimize") );
	    else
		menu->changeItem( minimizeRestoreId, i18n("Restore") );
	}
	contextMenuAboutToShow( menu );
	menu->popup( e->globalPos() );
	break;
    default:
	// nothing
	break;
    }
}



void KDockWindow::mouseReleaseEvent( QMouseEvent * )
{
}


void KDockWindow::contextMenuAboutToShow( KPopupMenu* )
{
}


void KDockWindow::toggleMinimizeRestore()
{
    if ( !parentWidget() )
	return;
    if ( !parentWidget()->isVisible() ) {
	parentWidget()->show();
	KWin::setActiveWindow( parentWidget()->winId() );
    } else {
	parentWidget()->hide();
    }
}
