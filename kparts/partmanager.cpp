/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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

#include <kparts/event.h>
#include <kparts/partmanager.h>
#include <kparts/part.h>
#include <kdebug.h>

#include <qapplication.h>

using namespace KParts;

template class QList<Part>;

namespace KParts {

class PartManagerPrivate
{
public:
  PartManagerPrivate()
  {
    m_activeWidget = 0;
    m_activePart = 0;
    m_selectedPart = 0;
    m_selectedWidget = 0;
    m_bAllowNestedParts = false;
  }
  ~PartManagerPrivate()
  {
  }


  Part * m_activePart;
  QWidget *m_activeWidget;

  QList<Part> m_parts;

  PartManager::SelectionPolicy m_policy;

  Part *m_selectedPart;
  QWidget *m_selectedWidget;

  bool m_bAllowNestedParts;
};

};

PartManager::PartManager( QWidget * parent, const char * name )
 : QObject( parent, name )
{
  d = new PartManagerPrivate;

  qApp->installEventFilter( this );

  d->m_policy = Direct;
}

PartManager::~PartManager()
{
  // core dumps ... setActivePart( 0L );
  qApp->removeEventFilter( this );
  delete d;
}

void PartManager::setSelectionPolicy( SelectionPolicy policy )
{
  d->m_policy = policy;
}

PartManager::SelectionPolicy PartManager::selectionPolicy() const
{
  return d->m_policy;
}

void PartManager::setAllowNestedParts( bool allow )
{
  d->m_bAllowNestedParts = allow;
}

bool PartManager::allowNestedParts() const
{
  return d->m_bAllowNestedParts;
}

bool PartManager::eventFilter( QObject *obj, QEvent *ev )
{

  if ( ev->type() != QEvent::MouseButtonPress &&
       ev->type() != QEvent::MouseButtonDblClick &&
       ev->type() != QEvent::FocusIn )
    return false;

  if ( !obj->isWidgetType() )
    return false;

  QWidget *w = (QWidget *)obj;

  if ( ( w->testWFlags( WStyle_Dialog ) && w->isModal() ) ||
       w->testWFlags( WType_Popup ) || w->testWFlags( WStyle_Tool ) )
    return false;

  Part * part;
  while ( w )
  {
    QPoint pos;

    if ( ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonDblClick )
      pos = ((QMouseEvent *)ev)->globalPos();

    if ( w->topLevelWidget() != ((QWidget *)parent())->topLevelWidget() )
      return false;

    part = findPartFromWidget( w, pos );
    if ( part ) // We found a part whose widget is w
    {
      if ( d->m_policy == PartManager::TriState )
      {
        if ( ev->type() == QEvent::MouseButtonDblClick )
	{
	  if ( part == d->m_activePart && w == d->m_activeWidget )
	    return false;
	
	  setActivePart( part, w );
	  return true;
	}
	
	if ( ( d->m_selectedWidget != w || d->m_selectedPart != part ) &&
	     ( d->m_activeWidget != w || d->m_activePart != part ) )
	{
	  if ( part->isSelectable() )
  	    setSelectedPart( part, w );
	  else
	    setActivePart( part, w );
	  return true;
	}
	else if ( d->m_selectedWidget == w && d->m_selectedPart == part )
	{
	  setActivePart( part, w );
	  return true;
	}
	else if ( d->m_activeWidget == w && d->m_activePart == part )
	{
  	  setSelectedPart( 0L );
	  return false;
	}
	
	return false;
      }
      else if ( part != d->m_activePart )
      {
        kdDebug(1000) << QString("Part %1 made active because %2 got event").arg(part->name()).arg(w->className()) << endl;

        setActivePart( part, w );
      }

      // I suppose we don't return here in case of child parts, right ?
      // But it means we'll emit the event for each intermediate parent ? (David)
      // Perhaps we should store the new part and emit at the end ?

      // I think we should return here (Simon)
      return false; // Ok, let's return. We'll test child parts later on. (David)
    }

    w = w->parentWidget();

    if ( w && ( ( w->testWFlags( WStyle_Dialog ) && w->isModal() ) ||
                w->testWFlags( WType_Popup ) || w->testWFlags( WStyle_Tool ) ) )
    {
      kdDebug(1000) << QString("No part made active although %1/%2 got event - loop aborted").arg(obj->name()).arg(obj->className()) << endl;
      return false;
    }

  }

  kdDebug(1000) << QString("No part made active although %1/%2 got event").arg(obj->name()).arg(obj->className()) << endl;
  return false;
}

Part * PartManager::findPartFromWidget( QWidget * widget, const QPoint &pos )
{
  QListIterator<Part> it ( d->m_parts );
  for ( ; it.current() ; ++it )
  {
    Part *part = it.current()->hitTest( widget, pos );
    if ( part )
      return part;
  }
  return 0L;
}

void PartManager::addPart( Part *part, bool setActive )
{
  connect( part, SIGNAL( destroyed() ), this, SLOT( slotObjectDestroyed() ) );

  d->m_parts.append( part );

  part->setManager( this );

  if ( setActive )
  {
    setActivePart( part );
    if ( part->widget() )
      part->widget()->setFocus();
  }

  // Prevent focus problems
  if ( part->widget() &&
      ( part->widget()->focusPolicy() == QWidget::NoFocus ||
        part->widget()->focusPolicy() == QWidget::TabFocus ) )
  {
    kdWarning(1000) << QString("Part %1 must have at least a ClickFocus policy. Prepare for trouble !").arg(part->name()) << endl;
  }

  if ( part->widget() )
    part->widget()->show();
  emit partAdded( part );
}

void PartManager::removePart( Part *part )
{
  if ( d->m_parts.findRef( part ) == -1 )
  {
    kdFatal(1000) << QString("Can't remove part %1, not in KPartManager's list.").arg(part->name()) << endl;
    return;
  }
  disconnect( part, SIGNAL( destroyed() ), this, SLOT( slotObjectDestroyed() ) );

  kdDebug(1000) << QString("Part %1 removed").arg(part->name()) << endl;
  d->m_parts.removeRef( part );

  if ( part == d->m_activePart )
    setActivePart( 0 );

  emit partRemoved( part );
}

void PartManager::setActivePart( Part *part, QWidget *widget )
{
  //check whether nested parts are disallowed and activate the top parent part then, by traversing the
  //tree recursively (Simon)
  if ( part && !d->m_bAllowNestedParts && part->parent() && part->parent()->inherits( "KParts::Part" ) )
    setActivePart( (KParts::Part *)part->parent() );

  KParts::Part *oldActivePart = d->m_activePart;
  QWidget *oldActiveWidget = d->m_activeWidget;

  setSelectedPart( 0L );

  d->m_activePart = part;
  d->m_activeWidget = widget;

  if ( oldActivePart )
  {
    KParts::Part *savedActivePart = part;
    QWidget *savedActiveWidget = widget;

    PartActivateEvent ev( false, oldActivePart, oldActiveWidget );
    QApplication::sendEvent( oldActivePart, &ev );
    if ( oldActiveWidget )
    {
      disconnect( oldActiveWidget, SIGNAL( destroyed() ),
		  this, SLOT( slotWidgetDestroyed() ) );
      QApplication::sendEvent( oldActiveWidget, &ev );
    }

    d->m_activePart = savedActivePart;
    d->m_activeWidget = savedActiveWidget;
  }

  if ( d->m_activePart )
  {
    if ( !widget )
      d->m_activeWidget = part->widget();

    PartActivateEvent ev( true, d->m_activePart, d->m_activeWidget );
    QApplication::sendEvent( d->m_activePart, &ev );
    if ( d->m_activeWidget )
    {
      connect( d->m_activeWidget, SIGNAL( destroyed() ),
	       this, SLOT( slotWidgetDestroyed() ) );
      QApplication::sendEvent( d->m_activeWidget, &ev );
    }
  }

  emit activePartChanged( d->m_activePart );
}

Part *PartManager::activePart() const
{
  return d->m_activePart;
}

QWidget *PartManager::activeWidget() const
{
  return  d->m_activeWidget;
}

void PartManager::setSelectedPart( Part *part, QWidget *widget )
{
  if ( part == d->m_selectedPart && widget == d->m_selectedWidget )
    return;

  Part *oldPart = d->m_selectedPart;
  QWidget *oldWidget = d->m_selectedWidget;

  d->m_selectedPart = part;
  d->m_selectedWidget = widget;

  if ( part && !widget )
    d->m_selectedWidget = part->widget();

  if ( oldPart )
  {
    PartSelectEvent ev( false, oldPart, oldWidget );
    QApplication::sendEvent( oldPart, &ev );
    QApplication::sendEvent( oldWidget, &ev );
  }

  if ( d->m_selectedPart )
  {
    PartSelectEvent ev( true, d->m_selectedPart, d->m_selectedWidget );
    QApplication::sendEvent( d->m_selectedPart, &ev );
    QApplication::sendEvent( d->m_selectedWidget, &ev );
  }
}

Part *PartManager::selectedPart() const
{
  return d->m_selectedPart;
}

QWidget *PartManager::selectedWidget() const
{
  return d->m_selectedWidget;
}

void PartManager::slotObjectDestroyed()
{
  kdDebug(1000) << "KPartManager::slotObjectDestroyed()" << endl;
  removePart( (Part *)sender() );
}

void PartManager::slotWidgetDestroyed()
{
  kdDebug(1000) << "KPartsManager::slotWidgetDestroyed()" << endl;
  if ( (QWidget *)sender() == d->m_activeWidget )
    setActivePart( 0L ); //do not remove the part because if the part's widget dies, then the
                         //part will delete itself anyway (which ends up in a slotObjectDestroyed() call
}

const QList<Part> *PartManager::parts() const
{
  return &d->m_parts;
}

#include "partmanager.moc"
