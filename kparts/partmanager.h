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
#ifndef __kpartmanager_h__
#define __kpartmanager_h__

#include <qobject.h>
#include <qwidget.h>
#include <qlist.h>

class KInstance;

namespace KParts
{

class Part;

class PartManagerPrivate;

/**
 * The part manager is an object which knows about a collection of parts
 * (even nested ones) and handles activation/deactivation.
 *
 * Applications that want to embed parts without merging GUIs
 * only use a @ref KPartManager. Those who want to merge GUIs use a
 * @ref KPartsMainWindow for example, in addition to a part manager.
 *
 * Parts know about the part manager to add nested parts to it.
 * See also @ref KParts::Part::manager and @ref KParts::Part::setManager .
 */
class PartManager : public QObject
{
  Q_OBJECT
public:
  // the default policy of a PartManager is Direct!
  enum SelectionPolicy { Direct, TriState };

  /**
   * Create a part manager.
   *
   * @param parent The toplevel widget (window / dialog).
   */
  PartManager( QWidget * parent, const char * name = 0L );
  virtual ~PartManager();

  /**
   * Set the selection policy of the partmanager.
   */
  void setSelectionPolicy( SelectionPolicy policy );
  /**
   * Retrieve the current selection policy.
   */
  SelectionPolicy selectionPolicy() const;

  /**
   * Specify whether the partmanager should handle/allow nested parts or not.
   * This is a property the shell has to set/specify. Per default we assume that the
   * shell cannot handle nested parts. However in case of a KOffice shell for example
   * we allow nested parts.
   * A Part is nested (a child part) if its parent object inherits KParts::Part.
   * If a child part is activated and nested parts are not allowed/handled, then the top parent
   * part in the tree is activated.
   */
  void setAllowNestedParts( bool allow );
  bool allowNestedParts() const;

  /**
   * @internal
   */
  virtual bool eventFilter( QObject *obj, QEvent *ev );

  /**
   * Add a Part to the manager.
   *
   * Sets it to the active part automatically if @p setActive is true (default ).
   */
  virtual void addPart( Part *part, bool setActive = true );

  /**
   * Remove a part from the manager (this does not delete the object) .
   *
   * Sets the active part to 0 if @p part is the @ref activePart() .
   */
  virtual void removePart( Part *part );

  /**
   * Set the active part.
   *
   * The active part receives activation events.
   *
   * @p widget can be used to specify which widget was responsible for the activation.
   * This is important if you have multiple views for a document/part , like in KOffice .
   */
  virtual void setActivePart( Part *part, QWidget *widget = 0L );

  /**
   * Retrieve the active part.
   **/
  virtual Part *activePart() const;

  /**
   * Retrieve the active widget of the current active part (see @ref activePart ).
   */
  virtual QWidget *activeWidget() const;

  /**
   * Set the selected part.
   *
   * The selected part receives selection events.
   *
   * @p widget can be used to specify which widget was responsible for the selection.
   * This is important if you have multiple views for a document/part , like in KOffice .
   */
  virtual void setSelectedPart( Part *part, QWidget *widget = 0L );

  /**
   * Retrieve the current selected part.
   */
  virtual Part *selectedPart() const;

  /**
   * Retrieve the selected widget of the current selected part (see @ref selectedPart ).
   */
  virtual QWidget *selectedWidget() const;

  /**
   * Retrieve the list of parts being managed by the partmanager.
   */
  const QList<Part> *parts() const;

signals:
  /**
   * Emitted when a new part has been added.
   * @see addPart()
   **/
  void partAdded( KParts::Part *part );
  /**
   * Emitted when a part has been removed.
   * @see removePart()
   **/
  void partRemoved( KParts::Part *part );
  /**
   * Emitted when the active part has changed.
   * @see setActivePart()
   **/
  void activePartChanged( KParts::Part *newPart );

protected:
  /**
   * Changes the active instance when the active part changes.
   * The active instance is used by KBugReport and KAboutDialog.
   * Override if you really need to - usually you don't need to.
   */
  virtual void setActiveInstance( KInstance * instance );

protected slots:
  /**
   * Removes a part when it is destroyed.
   **/
  void slotObjectDestroyed();

  /**
   * @internal
   */
  void slotWidgetDestroyed();

private:
  Part * findPartFromWidget( QWidget * widget, const QPoint &pos );

  PartManagerPrivate *d;
};

};

#endif

