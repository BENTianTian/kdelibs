/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "messagewindow.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/QTimer>
#include <QtGui/QPixmap>

#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kvbox.h>

QHash<QWidget*, MessageWindow*> MessageWindow::m_windows;

MessageWindow::MessageWindow( const QString& txt, int delay, QWidget *parent )
	: QWidget( parent, Qt::Dialog )
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowModality(Qt::ApplicationModal);

	KHBox *box = new KHBox( this );
	box->setFrameStyle( QFrame::Panel|QFrame::Raised );
	box->setLineWidth( 1 );
	box->setSpacing( 10 );
	box->setMargin( 5 );
	QLabel *pix = new QLabel( box );
	pix->setPixmap( DesktopIcon( "kdeprint-printer" ) );
	m_text = new QLabel( txt, box );

	QHBoxLayout *l0 = new QHBoxLayout( this );
  l0->setMargin( 0 );
  l0->setSpacing( 0 );
	l0->addWidget( box );

	m_windows[parent] = this;

	if ( delay == 0 )
		slotTimer();
	else
		QTimer::singleShot( delay, this, SLOT( slotTimer() ) );
}

MessageWindow::~MessageWindow()
{
	m_windows.remove( parentWidget() );
}

void MessageWindow::slotTimer()
{
	QSize psz = parentWidget()->size(), sz = sizeHint();
	move( parentWidget()->mapToGlobal( QPoint( (psz.width()-sz.width())/2, (psz.height()-sz.height())/2 ) ) );
	if ( !isVisible() )
	{
		show();
		kapp->processEvents();
	}
}

QString MessageWindow::text() const
{
	return m_text->text();
}

void MessageWindow::setText( const QString& txt )
{
	m_text->setText( txt );
}

void MessageWindow::add( QWidget *parent, const QString& txt, int delay )
{
	if ( !parent )
		kWarning( 500 ) << "Cannot add a message window to a null parent" << endl;
	else
	{
		if ( m_windows.contains( parent ) )
		{
			MessageWindow *w = m_windows[parent];
			w->setText( txt );
		}
		else {
                    MessageWindow *mw = new MessageWindow( txt, delay, parent );
                    mw->setObjectName( "MessageWindow" );
                }
	}
}

void MessageWindow::remove( QWidget *parent )
{
	if ( parent && m_windows.contains( parent ) )
		delete m_windows[parent];
}

void MessageWindow::change( QWidget *parent, const QString& txt )
{
	if ( parent )
	{
		if ( m_windows.contains( parent ) )
		{
			MessageWindow *w = m_windows[ parent ];
			w->setText( txt );
		}
		else
			kWarning( 500 ) << "MessageWindow::change, no message window found" << endl;
	}
}

void MessageWindow::removeAll()
{
	QList<MessageWindow*> all = m_windows.values();
	foreach ( MessageWindow * w, all )
		delete w;
}

#include "messagewindow.moc"
