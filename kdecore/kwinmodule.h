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
/*
 * kwinmodule.h. Part of the KDE project.
 */

#ifndef KWINMODULE_H
#define KWINMODULE_H

#include <qobject.h>
#include <qvaluelist.h>

class KWinModulePrivate;

/**

   THIS API ISN'T FINISHED YET: IT'S STILL VERY COMPATIBLE WITH THE OLD KWMMODULEAPPLICATION.
   BE AWARE THAT IT MIGHT CHANGE A BIT BEFORE KDE-2.0 FREEZE

 *   The class KWinModule provides information about the window manager required by
 * windowmanager modules. It mainly informs a module about all
 * currently managed windows and changes to them (via Qt
 * signals). There are no methods to manipulate windows. These are
 * defined in the class KWM (see kwm.h).
 * @short Base class for KDE Window Manager modules.
 * @author Matthias Ettrich (ettrich@kde.org)
 * @version $Id$
 */
class KWinModule : public QObject
{
    Q_OBJECT

public:

    /**
       Creates a win module object and connects to the window
       manager. If @param dockModule is TRUE, the module will also
       serve as docking module.
     */
    KWinModule( QObject* parent = 0, bool dockModule = FALSE );

    /**
       Destructor. Internal cleanup, nothing fancy.
    */
    ~KWinModule();

  /**
   * A list of all toplevel windows currently managed by the
   * windowmanger in the order of creation. Please do not rely on
   * indexes of this list: Whenever you enter Qt's eventloop in your
   * application it may happen, that entries are removed or added.
   * your module should perhaps work on a copy of this list and verify a
   * window with hasWindow() before any operations.
   *
   * Iterateration over this list can be done easily with
   QValueList<WId>::ConstIterator it;
   for ( it = module->windows().begin(); it != modules->windows().end(); ++it ) {
         ... do something here,  (*it) is the current WId.
   }
   *
   */
    const QValueList<WId>& windows() const;

  /**
   * A list of all toplevel windows currently managed by the
   * windowmanger in the current stacking order (from lower to
   * higher). May be useful for pagers.
   */
    const QValueList<WId>& windowsSorted() const;

    /**
   * Is <WId> still managed at present?
   */
    bool hasWId(WId) const;


    /**
    * The dock windows. Only valid if you are succesfully connected as
    * docking module
    */
    const QValueList<WId>& dockWindows() const;


    /*
      A cached shortcut for KWM::currentDesktop()
     */
    int currentDesktop() const;

signals:

    /**
   * Switch to another virtual desktop
   */
    void desktopChange(int);

    /**
   * Add a window
   */
    void windowAdd(WId);

    /**
   * Remove a window
   */
    void windowRemove(WId);

    /**
   * A window has been changed (size, title, etc.)
   */
    void windowChange(WId);

    /**
   * Hint that <Window> is active (= has focus) now.
   */
    void windowActivate(WId);

    /**
    * The specified desktop got a new name
    */
    void desktopNameChange(int, QString);

    /**
    * The number of desktops changed
    */
    void desktopNumberChange(int);

    /**
   * Add a dock window
   */
    void dockWindowAdd(WId);

    /**
   * Remove a dock window
   */
    void dockWindowRemove(WId);

    /**
   * The workspace area has changed
   */
    void workspaceAreaChanged();
    
    /**
     * The stacking order of the window changed. The new order
     * can be obtained with windowsSorted()
     */
    void stackingOrderChanged();


private:
    KWinModulePrivate* d;

    friend KWinModulePrivate;
};

#endif
