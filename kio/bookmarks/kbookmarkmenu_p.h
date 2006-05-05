//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kbookmarkmenu_p_h__
#define __kbookmarkmenu_p_h__

#include <sys/types.h>

#include <qobject.h>
#include <q3listview.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kaction.h>
#include <kactionmenu.h>

#include "kbookmark.h"
#include "kbookmarkimporter.h"
#include "kbookmarkmanager.h"

class QString;
class QMenu;
class QPushButton;
class Q3ListView;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KBookmarkBar;
class KMenu;

class KImportedBookmarksActionMenu : public KActionMenu {
  Q_OBJECT
  Q_PROPERTY( QString type READ type WRITE setType )
  Q_PROPERTY( QString location READ location WRITE setLocation )
public:
  const QString type() const { return m_type; }
  void setType(const QString &type) { m_type = type; }
  const QString location() const { return m_location; }
  void setLocation(const QString &location) { m_location = location; }
private:
  QString m_type;
  QString m_location;
public:
  KImportedBookmarksActionMenu(
    const KIcon& icon, const QString &text,
    KActionCollection* parent, const char* name)
  : KActionMenu(icon, text, parent, name) {
     ;
  }
};

class KBookmarkActionMenu : public KActionMenu {
  Q_OBJECT
  Q_PROPERTY( QString url READ url WRITE setUrl )
  Q_PROPERTY( QString address READ address WRITE setAddress )
  Q_PROPERTY( bool readOnly READ readOnly WRITE setReadOnly )
public:
  const QString url() const { return m_url; }
  void setUrl(const QString &url) { m_url = url; }
  const QString address() const { return m_address; }
  void setAddress(const QString &address) { m_address = address; }
  bool readOnly() const { return m_readOnly; }
  void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
private:
  QString m_url;
  QString m_address;
  bool m_readOnly;
public:
  KBookmarkActionMenu(
    const KIcon& icon, const QString &text,
    KActionCollection* parent, const char* name)
  : KActionMenu(icon, text, parent, name) {
     ;
  }
};

class KBookmarkAction : public KAction {
  Q_OBJECT
  Q_PROPERTY( QString url READ url WRITE setUrl )
  Q_PROPERTY( QString address READ address WRITE setAddress )
public:
  const QString url() const { return m_url; }
  void setUrl(const QString &url) { m_url = url; }
  const QString address() const { return m_address; }
  void setAddress(const QString &address) { m_address = address; }
private:
  QString m_url;
  QString m_address;
public:
  KBookmarkAction(
    const QString& text, const QString& sIconName, const KShortcut& cut,
    KActionCollection* parent, const char* name)
  : KAction(text, parent, name) {
      setIcon( KIcon( sIconName ) );
      setShortcut( cut );
  }
};

class KBookmarkEditFields {
public:
  typedef enum { FolderFieldsSet, BookmarkFieldsSet } FieldsSet;
  KLineEdit * m_url;
  KLineEdit * m_title;
  KBookmarkEditFields(QWidget *main, QBoxLayout *vbox, FieldsSet isFolder);
  void setName(const QString &str);
  void setLocation(const QString &str);
};

class KBookmarkEditDialog : public KDialogBase
{
  Q_OBJECT

public:
  typedef enum { ModifyMode, InsertionMode } BookmarkEditType;

  KBookmarkEditDialog( const QString& title, const QString& url, KBookmarkManager *, BookmarkEditType editType, const QString& address = QString(),
                       QWidget * = 0, const char * = 0, const QString& caption = i18n( "Add Bookmark" ) );

  QString finalUrl() const;
  QString finalTitle() const;
  QString finalAddress() const;

protected Q_SLOTS:
  void slotOk();
  void slotCancel();
  void slotUser1();
  void slotDoubleClicked(Q3ListViewItem* item);

private:
  QWidget * m_main;
  KBookmarkEditFields * m_fields;
  Q3ListView * m_folderTree;
  QPushButton * m_button;
  KBookmarkManager * m_mgr;
  BookmarkEditType m_editType;
  QString m_address;
};

class KBookmarkFolderTreeItem : public Q3ListViewItem
{
  // make this an accessor
  friend class KBookmarkFolderTree;
public:
  KBookmarkFolderTreeItem( Q3ListView *, const KBookmark & );
  KBookmarkFolderTreeItem( KBookmarkFolderTreeItem *, Q3ListViewItem *, const KBookmarkGroup & );
private:
  KBookmark m_bookmark;
};

class KBookmarkFolderTree
{
public:
  static Q3ListView* createTree( KBookmarkManager *, QWidget * = 0, const char * = 0, const QString& = QString() );
  static void fillTree( Q3ListView*, KBookmarkManager *, const QString& = QString() );
  static QString selectedAddress( Q3ListView* );
  static void setAddress( Q3ListView *, const QString & );
};

class KBookmarkSettings
{
public:
  bool m_advancedaddbookmark;
  bool m_contextmenu;
  bool m_filteredtoolbar;
  static KBookmarkSettings *s_self;
  static void readSettings();
  static KBookmarkSettings *self();
};

/* Right mouse button */
class RMB : public QObject
{
  Q_OBJECT
public:
  RMB(QString parentAddress, QString highlightedAddress,
      KBookmarkManager *pManager, KBookmarkOwner *pOwner, QWidget *parentMenu = 0);

  ~RMB();

  KBookmark atAddress(const QString & address);
  void fillContextMenu( const QString & address);
  void fillContextMenu2( const QString & address);
  QMenu * contextMenu();
  void popup(const QPoint & pos);

public Q_SLOTS:
  void slotEditAt();
  void slotProperties();
  void slotInsert();
  void slotRemove();
  void slotCopyLocation();

private:
  QString m_parentAddress;
  QString m_highlightedAddress;
  KBookmarkManager *m_pManager;
  KBookmarkOwner *m_pOwner;
  QWidget *m_parentMenu;
  QMenu * m_contextMenu;
};

#endif
