/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kaction.h"
#include "kaction_p.h"
#include "kglobalaccel_p.h"

#include <QtGui/QApplication>

#include <kdebug.h>

#include "kguiitem.h"
#include "kicon.h"

//---------------------------------------------------------------------
// KActionPrivate
//---------------------------------------------------------------------

void KActionPrivate::setActiveGlobalShortcutNoEnable(const KShortcut &cut)
{
    globalShortcut = cut;
}

void KActionPrivate::init(KAction *q_ptr)
{
  q = q_ptr;
  globalShortcutEnabled = false;
  firstTimeSetGlobalShortcut = true;

  QObject::connect(q, SIGNAL(triggered(bool)), q, SLOT(slotTriggered()));

  q->setProperty("isShortcutConfigurable", true);
}

//---------------------------------------------------------------------
// KAction
//---------------------------------------------------------------------

KAction::KAction(QObject *parent)
  : QWidgetAction(parent), d(new KActionPrivate)
{
  d->init(this);
}

KAction::KAction(const QString &text, QObject *parent)
  : QWidgetAction(parent), d(new KActionPrivate)
{
  d->init(this);
  setText(text);
}

KAction::KAction(const KIcon &icon, const QString &text, QObject *parent)
  : QWidgetAction(parent), d(new KActionPrivate)
{
  d->init(this);
  setIcon(icon);
  setText(text);
}

KAction::~KAction()
{
    // When deleting actions in the configuration module, we don't want to
    // deactivate the global shortcut
    if (property("isConfigurationAction").toBool() == false) {
        if (d->globalShortcutEnabled) {
            // - remove the action from KGlobalAccel
            // - mark the action as inactive in the KDED module
            d->globalShortcutEnabled = false;
            KGlobalAccel::self()->d->setInactive(this);
        }
    } else {
        // we leak memory in KGlobalAccel for this action's data set.
        // HOWEVER this is a limited leak because if/when this action is
        // created again there will be no additional memory consumption.
    }

    KGestureMap::self()->removeGesture(d->shapeGesture, this);
    KGestureMap::self()->removeGesture(d->rockerGesture, this);
    delete d;
}

bool KAction::isShortcutConfigurable() const
{
    return property("isShortcutConfigurable").toBool();
}

void KAction::setShortcutConfigurable( bool b )
{
    setProperty("isShortcutConfigurable", b);
}

KShortcut KAction::shortcut(ShortcutTypes type) const
{
  Q_ASSERT(type);

  if (type == DefaultShortcut) {
      QKeySequence primary = property("defaultPrimaryShortcut").value<QKeySequence>();
      QKeySequence secondary = property("defaultAlternateShortcut").value<QKeySequence>();
      return KShortcut(primary, secondary);
  }

  QKeySequence primary = shortcuts().value(0);
  QKeySequence secondary = shortcuts().value(1);
  return KShortcut(primary, secondary);
}

void KAction::setShortcut( const KShortcut & shortcut, ShortcutTypes type )
{
  Q_ASSERT(type);

  if (type & DefaultShortcut) {
      setProperty("defaultPrimaryShortcut", shortcut.primary());
      setProperty("defaultAlternateShortcut", shortcut.alternate());
  }

  if (type & ActiveShortcut) {
      QAction::setShortcuts(shortcut);
  }
}

void KAction::setShortcut( const QKeySequence & keySeq, ShortcutTypes type )
{
  Q_ASSERT(type);

  if (type & DefaultShortcut)
      setProperty("defaultPrimaryShortcut", keySeq);

  if (type & ActiveShortcut) {
      QAction::setShortcut(keySeq);
  }
}

void KAction::setShortcuts(const QList<QKeySequence>& shortcuts, ShortcutTypes type)
{
  setShortcut(KShortcut(shortcuts), type);
}

void KActionPrivate::slotTriggered()
{
#ifdef KDE3_SUPPORT
  emit q->activated();
#endif
  emit q->triggered(QApplication::mouseButtons(), QApplication::keyboardModifiers());
}

const KShortcut & KAction::globalShortcut(ShortcutTypes type) const
{
  Q_ASSERT(type);

  if (type == DefaultShortcut)
    return d->defaultGlobalShortcut;

  return d->globalShortcut;
}

void KAction::setGlobalShortcut( const KShortcut & shortcut, ShortcutTypes type,
                                 GlobalShortcutLoading load )
{
  Q_ASSERT(type);
  bool changed = false;
  if (!d->globalShortcutEnabled) {
    //return;
    changed = true;
    enableGlobalShortcut();   //backwards compatibility
  }

  if ((type & DefaultShortcut) && d->defaultGlobalShortcut != shortcut) {
    d->defaultGlobalShortcut = shortcut;
    changed = true;
  }

  if ((type & ActiveShortcut) && d->globalShortcut != shortcut) {
    d->globalShortcut = shortcut;
    changed = true;
  }

  //We want to have updateGlobalShortcuts called on a new action in any case so that
  //it will be registered properly. In the case of the first setShortcut() call getting an
  //empty shortcut parameter this would not happen...
  if (changed || d->firstTimeSetGlobalShortcut) {
    KGlobalAccel::self()->d->updateGlobalShortcut(this, type | load);
    d->firstTimeSetGlobalShortcut = false;
  }
}

bool KAction::globalShortcutAllowed() const
{
  return d->globalShortcutEnabled;
}

bool KAction::isGlobalShortcutEnabled() const
{
  return d->globalShortcutEnabled;
}

void KAction::setGlobalShortcutAllowed( bool allowed, GlobalShortcutLoading load )
{
  if (d->globalShortcutEnabled == allowed)
    return;
  
  d->globalShortcutEnabled = allowed;
  if (allowed) {
    KGlobalAccel::self()->d->doRegister(this);
  } else {
    KGlobalAccel::self()->d->setInactive(this);
  }
}

bool KAction::enableGlobalShortcut()
{
    //If the object name was nonempty before and globalShortcutEnabled() == true -
    //well, you shouldn't have changed the objectName() anyway so we don't check that.
    if (objectName().isEmpty()) {
        kWarning() << "Attempt to set global shortcut for action without objectName(). See enableGlobalShortcut()";
        return false;
    }
    if (d->globalShortcutEnabled) {
        return true;
    }
    d->globalShortcutEnabled = true;
    KGlobalAccel::self()->d->doRegister(this);
    return true;
}

void KAction::disableGlobalShortcut()
{
    d->globalShortcut = KShortcut();
    d->defaultGlobalShortcut = KShortcut();
    if (d->globalShortcutEnabled) {
        d->globalShortcutEnabled = false;
        KGlobalAccel::self()->d->setInactive(this);
    }
}

KShapeGesture KAction::shapeGesture( ShortcutTypes type ) const
{
  Q_ASSERT(type);
  if ( type & DefaultShortcut )
    return d->defaultShapeGesture;

  return d->shapeGesture;
}

KRockerGesture KAction::rockerGesture( ShortcutTypes type ) const
{
  Q_ASSERT(type);
  if ( type & DefaultShortcut )
    return d->defaultRockerGesture;

  return d->rockerGesture;
}

void KAction::setShapeGesture( const KShapeGesture& gest,  ShortcutTypes type )
{
  Q_ASSERT(type);

  if( type & DefaultShortcut )
    d->defaultShapeGesture = gest;

  if ( type & ActiveShortcut ) {
    if ( KGestureMap::self()->findAction( gest ) ) {
      kDebug(283) << "New mouse gesture already in use, won't change gesture.";
      return;
    }
    KGestureMap::self()->removeGesture( d->shapeGesture, this );
    KGestureMap::self()->addGesture( gest, this );
    d->shapeGesture = gest;
  }
}

void KAction::setRockerGesture( const KRockerGesture& gest,  ShortcutTypes type )
{
  Q_ASSERT(type);

  if( type & DefaultShortcut )
    d->defaultRockerGesture = gest;

  if ( type & ActiveShortcut ) {
    if ( KGestureMap::self()->findAction( gest ) ) {
      kDebug(283) << "New mouse gesture already in use, won't change gesture.";
      return;
    }
    KGestureMap::self()->removeGesture( d->rockerGesture, this );
    KGestureMap::self()->addGesture( gest, this );
    d->rockerGesture = gest;
  }
}

/* vim: et sw=2 ts=2
 */

#include "kaction.moc"
