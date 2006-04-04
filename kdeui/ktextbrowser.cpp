/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
 *  Copyright (C) 2006 Urs Wolfer <uwolfer @ fwo.ch>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#include <QTextBrowser>

#include <kcursor.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <ktextbrowser.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <QAction>
#include <QMenu>
#include <QKeyEvent>

class KTextBrowser::KTextBrowserPrivate
{
public:
    KTextBrowserPrivate()
        : mNotifyClick( false )
    { }
    ~KTextBrowserPrivate() { }

    bool mNotifyClick;
};

KTextBrowser::KTextBrowser( QWidget *parent, bool notifyClick )
  : QTextBrowser( parent ), d( new KTextBrowserPrivate )
{
  d->mNotifyClick = notifyClick;
}

KTextBrowser::~KTextBrowser( void )
{
}


void KTextBrowser::setNotifyClick( bool notifyClick )
{
  d->mNotifyClick = notifyClick;
}


bool KTextBrowser::isNotifyClick() const
{
  return d->mNotifyClick;
}


void KTextBrowser::setSource( const QUrl& name )
{
  QString strName = name.toString();
  if( strName.isNull() )
  {
    return;
  }

  if( strName.indexOf('@') > -1 )
  {
    if( !d->mNotifyClick )
    {
      KToolInvocation::invokeMailer( KUrl( strName ) );
    }
    else
    {
      emit mailClick( QString(), strName );
    }
  }
  else
  {
    if( !d->mNotifyClick )
    {
      KToolInvocation::invokeBrowser( strName );
    }
    else
    {
      emit urlClick( strName );
    }
  }
}


void KTextBrowser::keyPressEvent( QKeyEvent *e )
{
  if( e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
  else if( e->key() == Qt::Key_F1 )
  {
    e->ignore();
  }
  else
  {
    QTextBrowser::keyPressEvent( e );
  }
}

void KTextBrowser::mouseMoveEvent( QMouseEvent* e)
{
  // do this first so we get the right type of cursor
  QTextBrowser::mouseMoveEvent(e);

  if ( viewport()->cursor().shape() == Qt::PointingHandCursor )
    viewport()->setCursor( KCursor::handCursor() );
}

void KTextBrowser::wheelEvent( QWheelEvent *e )
{
    if ( KGlobalSettings::wheelMouseZooms() )
        QTextBrowser::wheelEvent( e );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        QAbstractScrollArea::wheelEvent( e );
}

void KTextBrowser::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *popup = createStandardContextMenu();
    QList<QAction *> lstAction = popup->actions ();

    if ( !lstAction.isEmpty() )
    {
        enum { UndoAct, RedoAct, CutAct, CopyAct, PasteAct, ClearAct, SelectAllAct, NCountActs };
        if ( isReadOnly() )
            lstAction[CopyAct]->setIcon( SmallIconSet("editcopy") );
        else
        {
            lstAction[UndoAct]->setIcon( SmallIconSet("undo") );
            lstAction[RedoAct]->setIcon( SmallIconSet("redo") );
            lstAction[CutAct]->setIcon( SmallIconSet("editcut") );
            lstAction[CopyAct]->setIcon( SmallIconSet("editcopy") );
            lstAction[PasteAct]->setIcon( SmallIconSet("editpaste") );
            lstAction[ClearAct]->setIcon( SmallIconSet("editclear") );
        }
    }

    popup->exec(e->globalPos());
    delete popup;
}

void KTextBrowser::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktextbrowser.moc"
