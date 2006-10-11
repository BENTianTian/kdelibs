//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000, 2006 David Faure <faure@kde.org>

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
#ifndef __kbookmarkmanager_h
#define __kbookmarkmanager_h

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qdom.h>
#include <qpair.h>
#include "kbookmark.h"
#include "kbookmarknotifier.h"

class KBookmarkManagerList;
class KBookmarkGroup;
class QDBusMessage;

/**
 * This class implements the reading/writing of bookmarks in XML.
 * The bookmarks file is read and written using the XBEL standard
 * (http://pyxml.sourceforge.net/topics/xbel/)
 *
 * A sample file looks like this :
 * \code
 * <xbel>
 *   <bookmark href="http://developer.kde.org"><title>Developer Web Site</title></bookmark>
 *   <folder folded="no">
 *     <title>Title of this folder</title>
 *     <bookmark icon="kde" href="http://www.kde.org"><title>KDE Web Site</title></bookmark>
 *     <folder toolbar="yes">
 *       <title>My own bookmarks</title>
 *       <bookmark href="http://www.koffice.org"><title>KOffice Web Site</title></bookmark>
 *       <separator/>
 *       <bookmark href="http://www.kdevelop.org"><title>KDevelop Web Site</title></bookmark>
 *     </folder>
 *   </folder>
 * </xbel>
 * \endcode
 */
class KIO_EXPORT KBookmarkManager : public QObject
{
    Q_OBJECT
protected:
    /**
     * Creates a bookmark manager with a path to the bookmarks.  By
     * default, it will use the KDE standard dirs to find and create the
     * correct location.  If you are using your own app-specific
     * bookmarks directory, you must instantiate this class with your
     * own path <em>before</em> KBookmarkManager::managerForFile() is ever
     * called.
     *
     * @param bookmarksFile full path to the bookmarks file,
     * Use ~/.kde/share/apps/konqueror/bookmarks.xml for the konqueror bookmarks
     *
     * @param dbusObjectName a unique name that represents this bookmark collection,
     * usually your kinstance (e.g. kapplication) name. This is "konqueror" for the
     * konqueror bookmarks, "kfile" for KFileDialog bookmarks, etc.
     * The final DBus object path is /KBookmarkManager/dbusObjectName
     * An empty dbusObjectName disables the registration to dbus (used for temporary managers)
     *
     * @param bImportDesktopFiles if true, and if the bookmarksFile
     * doesn't already exist, import bookmarks from desktop files
     */
    KBookmarkManager( const QString & bookmarksFile, const QString& dbusObjectName, bool bImportDesktopFiles = true );


    KBookmarkManager();

public:
    /**
     * Destructor
     */
    ~KBookmarkManager();

    /**
     * Set the update flag. Defaults to true.
     * @param update if true then KBookmarkManager will listen to DBUS update requests.
     */
    void setUpdate( bool update );

    /**
     * Save the bookmarks to an XML file on disk.
     * You should use emitChanged() instead of this function, it saves
     * and notifies everyone that the file has changed.
     * @param toolbarCache iff true save a cache of the toolbar folder, too
     * @return true if saving was successful
     */
    bool save( bool toolbarCache = true ) const;
    //TODO protected

    /**
     * Save the bookmarks to the given XML file on disk.
     * @param filename full path to the desired bookmarks file location
     * @param toolbarCache iff true save a cache of the toolbar folder, too
     * @return true if saving was successful
     */
    bool saveAs( const QString & filename, bool toolbarCache = true ) const;

    /**
     * Update access time stamps for a given url.
     * @param url the viewed url
     * @param emitSignal iff true emit KBookmarkNotifier signal
     * @return true if any metadata was modified (bookmarks file is not saved automatically)
     */
    bool updateAccessMetadata( const QString &url, bool emitSignal = true );

    /*
     * NB. currently *unimplemented*
     *
     * Update favicon url for a given url.
     * @param url the viewed url
     * @param faviconurl the favicion url
     * @emitSignal iff true emit KBookmarkNotifier signal
     */
    void updateFavicon( const QString &url, const QString &faviconurl, bool emitSignal = true );

    /**
     * This will return the path that this manager is using to read
     * the bookmarks.
     * @internal
     * @return the path containing the bookmarks
     */
    QString path() { return m_bookmarksFile; }

    /**
     * This will return the root bookmark.  It is used to iterate
     * through the bookmarks manually.  It is mostly used internally.
     *
     * @return the root (top-level) bookmark
     */
    KBookmarkGroup root() const;

    /**
     * This returns the root of the toolbar menu.
     * In the XML, this is the group with the attribute toolbar=yes
     *
     * @return the toolbar group
     */
    KBookmarkGroup toolbar();

    /**
     * @return the bookmark designated by @p address
     * @param address the address belonging to the bookmark you're looking for
     * @param tolerate when true tries to find the most tolerable bookmark position
     * @see KBookmark::address
     */
    KBookmark findByAddress( const QString & address, bool tolerate = false );
    //TODO tolerate needed?

    /**
     * Saves the bookmark file and notifies everyone.
     * @param group the parent of all changed bookmarks
     */
    void emitChanged( const KBookmarkGroup & group );

    void emitConfigChanged();

    /**
     * Set options with which slotEditBookmarks called keditbookmarks
     * this can be used to change the appearance of the keditbookmarks
     * in order to provide a slightly differing outer shell depending
     * on the bookmarks file / app which calls it.
     * @param caption the --caption string, for instance "Konsole"
     * @param browser iff false display no browser specific
     *            menu items in keditbookmarks :: --nobrowser
     */
    void setEditorOptions( const QString& caption, bool browser );

    /**
     * This static function will return an instance of the
     * KBookmarkManager, responsible for the given @p bookmarksFile.
     * If you do not instantiate this class either
     * natively or in a derived class, then it will return an object
     * with the default behaviors.  If you wish to use different
     * behaviors, you <em>must</em> derive your own class and
     * instantiate it before this method is ever called.
     *
     * @param bookmarksFile full path to the bookmarks file,
     * Use ~/.kde/share/apps/konqueror/bookmarks.xml for the konqueror bookmarks
     *
     * @param dbusObjectName a unique name that represents this bookmark collection,
     * usually your kinstance (e.g. kapplication) name. This is "konqueror" for the
     * konqueror bookmarks, "kfile" for KFileDialog bookmarks, etc.
     * The final DBus object path is /KBookmarkManager/dbusObjectName
     * An empty dbusObjectName disables the registration to dbus (used for temporary managers)
     *
     * @param bImportDesktopFiles if true, and if the bookmarksFile
     * doesn't already exist, import bookmarks from desktop files
     * @return a pointer to an instance of the KBookmarkManager.
     */
    static KBookmarkManager* managerForFile( const QString& bookmarksFile,
                                             const QString& dbusObjectName,
                                             bool bImportDesktopFiles = true );

    static KBookmarkManager* managerForFile( const QString&, bool);
    

    /**
     * only used for KBookmarkBar
     */
    static KBookmarkManager* createTempManager();

    /**
     * Returns a pointer to the user's main (konqueror) bookmark collection.
     */
    static KBookmarkManager* userBookmarksManager();

    /**
     * @internal
     */
    const QDomDocument & internalDocument() const;

    KBookmarkGroup addBookmarkDialog( const QString & _url, const QString & _title,
                                      const QString & _parentBookmarkAddress = QString() );
    //TODO virtual, bool advanced

public Q_SLOTS:
    void slotEditBookmarks();
    void slotEditBookmarksAtAddress( const QString& address );
    //virtual?, default parameter?

    /**
     * Reparse the whole bookmarks file and notify about the change
     */
    void notifyCompleteChange( QString caller );

    /**
     * Emit the changed signal for the group whose address is given
     * @see KBookmark::address()
     * Called by the process that saved the file after
     * a small change (new bookmark or new folder).
     */
    void notifyChanged( QString groupAddress, const QDBusMessage &msg );

    void notifyConfigChanged();

Q_SIGNALS:
    void bookmarkCompleteChange( QString caller );

    void bookmarksChanged( QString groupAddress );

    void bookmarkConfigChanged();

    /**
     * Signals that the group (or any of its children) with the address
     * @p groupAddress (e.g. "/4/5")
     * has been modified by the caller @p caller.
     */
    void changed( const QString & groupAddress, const QString & caller );

protected:
    // consts added to avoid a copy-and-paste of internalDocument
    void parse() const;
    void importDesktopFiles();
    static void convertToXBEL( QDomElement & group ); //TODO used to convert pre KDE 2.1 bookmarks, remove?
    static void convertAttribute( QDomElement elem, const QString & oldName, const QString & newName );

private:
    void init( const QString& dbusPath );

    QString m_bookmarksFile;
    mutable QDomDocument m_doc;
    mutable QDomDocument m_toolbarDoc;
    mutable bool m_docIsLoaded;
    bool m_update;
    static KBookmarkManagerList* s_pSelf;

    QString m_editorCaption;
    bool m_browserEditor;
    class KBookmarkManagerPrivate* d;

    friend class KBookmarkGroup;
};

/**
 * The KBookmarkMenu and KBookmarkBar classes gives the user
 * the ability to either edit bookmarks or add their own.  In the
 * first case, the app may want to open the bookmark in a special way.
 * In the second case, the app <em>must</em> supply the name and the
 * URL for the bookmark.
 *
 * This class gives the app this callback-like ability.
 *
 * If your app does not give the user the ability to add bookmarks and
 * you don't mind using the default bookmark editor to edit your
 * bookmarks, then you don't need to overload this class at all.
 * Rather, just use something like:
 *
 * <CODE>
 * bookmarks = new KBookmarkMenu( mgr, 0, menu, actioncollec  )
 * </CODE>
 *
 * If you wish to use your own editor or allow the user to add
 * bookmarks, you must overload this class.
 */
class KIO_EXPORT KBookmarkOwner
{
public:
    virtual ~KBookmarkOwner() {}

  /**
   * This function is called whenever the user wants to add the
   * current page to the bookmarks list.  The title will become the
   * "name" of the bookmark.  You must overload this function if you
   * wish to give your users the ability to add bookmarks.
   *
   * @return the title of the current page.
   */
  virtual QString currentTitle() const { return QString(); }

  /**
   * This function is called whenever the user wants to add the
   * current page to the bookmarks list.  The URL will become the URL
   * of the bookmark.  You must overload this function if you wish to
   * give your users the ability to add bookmarks.
   *
   * @return the URL of the current page.
   */
  virtual QString currentUrl() const { return QString(); }


  /**
   * This function returns whether the owner supports tabs.
   */
  virtual bool supportsTabs() const { return false; }

  /**
   * Returns a list of title, URL pairs of the open tabs.
   */
  virtual QList<QPair<QString, QString> > currentBookmarkList() const { return QList<QPair<QString, QString> >(); }

  /**
   * Returns true if the bookmark menu/toolbar should show an "Add Bookmark" Entry
   */
  virtual bool addBookmarkEntry() const { return true; }

  /**
   * Returns true if the bookmark menu/toolbar should show an "Edit Bookmarks" Entry
   */
  virtual bool editBookmarkEntry() const { return true; }

  /**
   * Called if a bookmark is selected. You need to override this.
   */
  virtual void openBookmark(KBookmark bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km) = 0;

private:
  class KBookmarkOwnerPrivate;
  KBookmarkOwnerPrivate *d;
};

#endif
