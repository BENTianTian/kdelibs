/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __kbookmarkmenu_h__
#define __kbookmarkmenu_h__

#include <sys/types.h>

#include <qptrlist.h>
#include <qptrstack.h>
#include <qobject.h>
#include <qlistview.h>

#include <kdialogbase.h>
#include <klocale.h>

#include "kbookmark.h"
#include "kbookmarkmanager.h"

class QString;
class QPopupMenu;
class QPushButton;
class QListView;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KPopupMenu;

namespace KIO { class Job; }

/**
 * This class provides a bookmark menu.  It is typically used in
 * cooperation with KActionMenu but doesn't have to be.
 *
 * If you use this class by itself, then it will use KDE defaults for
 * everything -- the bookmark path, bookmark editor, bookmark launcher..
 * everything.  These defaults reside in the classes
 * @ref KBookmarkOwner (editing bookmarks) and @ref KBookmarkManager
 * (almost everything else).  If you wish to change the defaults in
 * any way, you must reimplement and instantiate those classes
 * <em>before</em> this class is ever called.
 *
 * Using this class is very simple:
 *
 * 1) Create a popup menu (either KActionMenu or KPopupMenu will do)
 * 2) Instantiate a new KBookmarkMenu object using the above popup
 *    menu as a parameter
 * 3) Insert your (now full) popup menu wherever you wish
 *
 * Again, if you wish to modify any defaults, the procedure is:
 *
 * 1a) Reimplement your own KBookmarkOwner
 * 1b) Reimplement and instantiate your own KBookmarkManager
 */
class KBookmarkMenu : public QObject
{
  Q_OBJECT
  friend class KBookmarkMenuNSImporter;
public:
  /**
   * Fills a bookmark menu
   * (one instance of KBookmarkMenu is created for the toplevel menu,
   *  but also one per submenu).
   *
   * @param owner implementation of the KBookmarkOwner interface (callbacks). If this is of type
   *  KBookmarkOwnerListCapable (checked at runtime), then the menu will also allow group opens.
   * @param parentMenu menu to be filled
   * @param collec parent collection for the KActions. 
   *  Only used for other menus than the toplevel one.
   * @param root true for the toplevel menu
   * @param add true to show the "Add Bookmark" and "New Folder" entries
   * @param parentAddress the address of the group containing the items
   *  that we want to show.
   * @see KBookmark::address.
   * Be careful :
   * A _null_ parentAddress denotes a NS-bookmark menu.
   * An _empty_ parentAddress denotes the toplevel bookmark menu
   */
  KBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KPopupMenu * parentMenu,
                 KActionCollection * collec, bool root, bool add = true,
                 const QString & parentAddress = "" );

  ~KBookmarkMenu();

  /**
   * Even if you think you need to use this, you are probably wrong.
   * It fills a bookmark menu starting a given KBookmark.
   * This is public for KBookmarkBar.
   */
  void fillBookmarkMenu();

  /**
   * Call ensureUpToDate() if you need KBookmarkMenu to adjust to its 
   * final size before it is executed.
   **/
  void ensureUpToDate();

public slots: // public for bookmark bar
  void slotBookmarksChanged( const QString & );

protected slots:
  void slotAboutToShow();
  void slotAboutToShowContextMenu( KPopupMenu *, int, QPopupMenu * );
  void slotRMBActionRemove();
  void slotRMBActionOpen();
  //void slotRMBActionEdit();
  void slotBookmarkSelected();
  void slotAddBookmark();
  void slotNewFolder();
  void slotNSBookmarkSelected();
  /**
   * load Netscape's bookmarks
   */
  void slotNSLoad();

protected:

  void refill();
  void addAddBookmark();
  void addEditBookmarks();
  void addNewFolder();

  QString contextMenuItemAddress();

  bool m_bIsRoot:1;
  bool m_bAddBookmark:1;
  bool m_bDirty:1;
  bool m_bNSBookmark:1;
  bool m_bAddShortcuts:1;

  KBookmarkManager * m_pManager;
  KBookmarkOwner *m_pOwner;
  /**
   * The menu in which we plug our actions.
   * Supplied in the constructor.
   */
  KPopupMenu * m_parentMenu;
  /**
   * List of our sub menus
   */
  QPtrList<KBookmarkMenu> m_lstSubMenus;
  KActionCollection * m_actionCollection;
  /**
   * List of our actions.
   */
  QPtrList<KAction> m_actions;
  /**
   * Parent bookmark for this menu.
   */
  QString m_parentAddress;
};


/**
 * A class connected to KNSBookmarkImporter, to fill KActionMenus.
 */
class KBookmarkMenuNSImporter : public QObject
{
  Q_OBJECT
public:
  KBookmarkMenuNSImporter( KBookmarkManager* mgr, KBookmarkMenu * menu, KActionCollection * act ) :
     m_menu(menu), m_actionCollection(act), m_pManager(mgr) {}

  void openNSBookmarks();

protected slots:
  void newBookmark( const QString & text, const QCString & url, const QString & );
  void newFolder( const QString & text, bool, const QString & );
  void newSeparator();
  void endFolder();

protected:
  QPtrStack<KBookmarkMenu> mstack;
  KBookmarkMenu * m_menu;
  KActionCollection * m_actionCollection;
  KBookmarkManager* m_pManager;
};

class BookmarkEditDialog : public KDialogBase
{
  Q_OBJECT

public:
  BookmarkEditDialog( const QString& title, const QString& url, KBookmarkManager *, QWidget * = 0, const char * = 0, const QString& caption = i18n( "Add Bookmark" ) );
  QString finalUrl();
  QString finalTitle();
  QString finalAddress();

protected slots:
  void slotOk();
  void slotCancel();
  void slotInsertFolder();

private:
  QWidget * m_main;
  QListView * m_folderTree;
  KLineEdit * m_url;
  KLineEdit * m_title;
  QPushButton * m_button;
  KBookmarkManager * m_mgr;
};

#endif
