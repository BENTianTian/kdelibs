/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <kdeprint@swing.be>
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

#include "posterpreview_p.h"

#include <kdebug.h>
#include <kprocess.h>
#include <kprinter.h>
#include <klocale.h>
#include <kcolorscheme.h>

#include <QtGui/QTextObjectInterface>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QTextDocument>
#include <QtGui/QTextEdit>
#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>

#include <stdio.h>



PosterPreview::PosterPreview( QWidget *parent )
	: QFrame( parent )
{
	m_postersize = m_mediasize = "A4";
	m_cutmargin = 5;
	init();
}

PosterPreview::PosterPreview( const QString& postersize, const QString& mediasize, QWidget *parent )
	: QFrame( parent )
{
	m_postersize = postersize;
	m_mediasize = mediasize;
	m_cutmargin = 5;
	init();
}

PosterPreview::~PosterPreview()
{
	delete m_process;
}

void PosterPreview::init()
{
	m_process = new KProcess;
	connect( m_process, SIGNAL( readyReadStandardError() ), SLOT( slotProcessStderr() ) );
	connect( m_process, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( slotProcessExited() ) );

	m_cols = m_rows = m_pw = m_ph = m_mw = m_mh = 0;
	m_dirty = false;
	setDirty();
	setMouseTracking( true );
}

void PosterPreview::parseBuffer()
{
	int rotate;
	float pw, ph, mw, mh;
	float x1, x2, y1, y2;
	sscanf( m_buffer.toAscii().data(), "%d %d %d %g %g %g %g %g %g %g %g", &m_rows, &m_cols, &rotate, &pw, &ph, &mw, &mh, &x1, &y1, &x2, &y2 );
	m_pw = ( int )( rotate ? ph : pw );
	m_ph = ( int )( rotate ? pw : ph );
	m_mw = ( int )( rotate ? mh : mw );
	m_mh = ( int )( rotate ? mw : mh );
	m_posterbb.setCoords( ( int )x1, ( int )y1, ( int )x2, ( int )y2 );
}

void PosterPreview::setDirty()
{
	if ( !m_dirty )
	{
		m_dirty = true;
		QTimer::singleShot( 1, this, SLOT( updatePoster() ) );
	}
}

void PosterPreview::updatePoster()
{
	// FIXME The poster preview doesn't work since the media page size is not getting set
	// (m_mediasize is empty - it shouldn't be!)

	m_buffer = "";
	delete m_process;
	m_process = new KProcess;
	*m_process << "poster" << "-F" << "-m" + m_mediasize << "-p" + m_postersize
		<< "-c" + QString::number( m_cutmargin ) + '%';
	m_process->setOutputChannelMode( KProcess::OnlyStderrChannel );
	m_process->start();
	if ( !m_process->waitForStarted() )
	{
		m_rows = m_cols = 0;
		m_dirty = false;
		update();
	}
}


void PosterPreview::paintEvent( QPaintEvent * )
{
	QPainter painter( this );

	painter.fillRect( 0, 0, width(), height(), palette().color( QPalette::Background ) );

	if ( isEnabled() )
	{
		if ( m_rows <= 0 || m_cols <= 0 || m_pw <= 0 || m_ph <= 0 )
		{
			QString txt = i18n( "Poster preview not available. Either the <b>poster</b> "
				          "executable is not properly installed, or you don't have "
						  "the required version; available at http://printing.kde.org/downloads/." );

			QTextDocument doc;
			doc.setHtml( m_buffer.isEmpty() ? txt : m_buffer.prepend( "<pre>" ).append( "</pre>" ) );

			doc.setPageSize( size() );
			QAbstractTextDocumentLayout::PaintContext ctx = QAbstractTextDocumentLayout::PaintContext();
			ctx.clip = rect();
			doc.documentLayout()->draw( & painter, ctx );

			m_boundingrect = QRect();
		}
		else
		{
			int totalx = m_cols*m_pw, totaly = m_rows*m_ph;
			float scale = qMin( float( width()-1 )/totalx, float( height()-1 )/totaly );
			painter.translate( 0, height()-1 );
			painter.scale( scale, -scale );
			int x = ( int )( width()/scale-totalx )/2, y = ( int )( height()/scale-totaly )/2;
			painter.translate( x, y );
			m_boundingrect = painter.matrix().mapRect( QRect( 0, 0, totalx, totaly ) );

			x = y = 0;
			int px = m_posterbb.x(), py = m_posterbb.y(), pw = m_posterbb.width(), ph = m_posterbb.height();
			for ( int i=0; i<m_rows; i++, y+=m_ph, x=0 )
			{
				for ( int j=0; j<m_cols; j++, x+=m_pw )
				{
					bool selected = ( m_selectedpages.contains( i*m_cols+j+1 ) );
                                        QBrush backBrush(Qt::white);
                                        QBrush fillBrush(Qt::lightGray);
                                        if ( selected ) {
                                            backBrush = KColorScheme(QPalette::Active, KColorScheme::Selection).background();
                                            fillBrush = QBrush(backBrush.color().darker(160));
                                        }
					painter.fillRect( x+1, y+1, m_pw-2, m_ph-2, backBrush );
					painter.drawRect( x, y, m_pw, m_ph );
					if ( pw > 0 && ph > 0 )
						painter.fillRect( x+m_mw+px, y+m_mh+py, qMin( pw, m_pw-2*m_mw-px ), qMin( ph, m_ph-2*m_mh-py ),
								fillBrush );
					painter.setPen( Qt::DotLine );
					painter.drawRect( x+m_mw, y+m_mh, m_pw-2*m_mw, m_ph-2*m_mh );
					painter.setPen( Qt::SolidLine );

					pw -= m_pw-2*m_mw-px;
					px = 0;
				}

				px = m_posterbb.x();
				ph -= m_ph-2*m_mh-py;
				py = 0;
				pw = m_posterbb.width();
			}
		}
	}
}

void PosterPreview::mouseMoveEvent( QMouseEvent *e )
{
	if ( m_boundingrect.isValid() )
	{
		if ( m_boundingrect.contains( e->pos() ) )
			setCursor( QCursor( Qt::OpenHandCursor ) );
		else
			setCursor( QCursor( Qt::ArrowCursor ) );
	}
}

void PosterPreview::mousePressEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton && m_boundingrect.isValid() )
	{
		if ( m_boundingrect.contains( e->pos() ) )
		{
			int c, r;
			c = ( e->pos().x()-m_boundingrect.x() )/( m_boundingrect.width()/m_cols ) + 1;
			r = m_rows - ( e->pos().y()-m_boundingrect.y() )/( m_boundingrect.height()/m_rows );
			int pagenum = ( r-1 )*m_cols+c;

			if ( !m_selectedpages.contains( pagenum ) ||
					!( e->modifiers() & Qt::ShiftModifier ) )
			{
				if ( !( e->modifiers() & Qt::ShiftModifier ) )
					m_selectedpages.clear();
				m_selectedpages.append( pagenum );
				update();
				emitSelectedPages();
			}
		}
		else if ( m_selectedpages.count() > 0 )
		{
			m_selectedpages.clear();
			update();
			emitSelectedPages();
		}
	}
}

void PosterPreview::slotProcessStderr()
{
	while( m_process->canReadLine() )
		m_buffer.append( QString::fromLocal8Bit( m_process->readLine() ) );
}

void PosterPreview::slotProcessExited()
{
	if ( m_process->exitStatus() == QProcess::NormalExit
            && m_process->exitCode() == 0 )
		parseBuffer();
	else
		m_rows = m_cols = 0;

	m_dirty = false;
	update();
}

void PosterPreview::setPosterSize( int s )
{
	setPosterSize( pageSizeToPageName( KPrinter::PageSize( s ) ) );
}

void PosterPreview::setPosterSize( const QString& s )
{
	if ( m_postersize != s )
	{
		m_selectedpages.clear();
		m_postersize = s;
		setDirty();
		emitSelectedPages();
	}
}

void PosterPreview::setMediaSize( int s )
{
	setMediaSize( pageSizeToPageName( ( KPrinter::PageSize )s ) );
}

void PosterPreview::setMediaSize( const QString& s )
{
	if ( m_mediasize != s )
	{
		m_selectedpages.clear();
		m_mediasize = s;
		setDirty();
		emitSelectedPages();
	}
}

void PosterPreview::setCutMargin( int value )
{
	m_cutmargin = value;
	setDirty();
}

void PosterPreview::setSelectedPages( const QString& s )
{
	QStringList l = s.split( ",", QString::SkipEmptyParts );
	m_selectedpages.clear();
	for ( QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it )
	{
		int p;
		if ( ( p = ( *it ).indexOf( '-' ) ) == -1 )
			m_selectedpages.append( ( *it ).toInt() );
		else
		{
			int p1 = ( *it ).left( p ).toInt(), p2 = ( *it ).mid( p+1 ).toInt();
			for ( int i=p1; i<=p2; i++ )
				m_selectedpages.append( i );
		}
	}
	update();
}

void PosterPreview::emitSelectedPages()
{
	QString s;
	if ( m_selectedpages.count() > 0 )
	{
		for ( QList<int>::ConstIterator it=m_selectedpages.begin(); it!=m_selectedpages.end(); ++it )
			s.append( QString::number( *it ) + ',' );
		s.truncate( s.length()-1 );
	}
	emit selectionChanged( s );
}

#include "posterpreview.moc"
