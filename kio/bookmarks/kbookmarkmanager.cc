// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#include "kbookmarkmanager.h"
#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkimporter.h"
#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <qregexp.h>
#include <kmessagebox.h>
#include <qprocess.h>
#include <klocale.h>
#include <QtGui/QApplication>
#include <kconfiggroup.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <QtDBus/QtDBus>
#include <QtCore/QTextStream>
#include "kbookmarkmanageradaptor_p.h"

class KBookmarkManagerList : public QList<KBookmarkManager *>
{
public:
    ~KBookmarkManagerList() {
        qDeleteAll( begin() , end() ); // auto-delete functionality
    }
};

K_GLOBAL_STATIC(KBookmarkManagerList, s_pSelf)

class KBookmarkMap : private KBookmarkGroupTraverser {
public:
    KBookmarkMap( KBookmarkManager * );
    void update();
    QList<KBookmark> find( const QString &url ) const
    { return m_bk_map[url]; }
private:
    virtual void visit(const KBookmark &);
    virtual void visitEnter(const KBookmarkGroup &) { ; }
    virtual void visitLeave(const KBookmarkGroup &) { ; }
private:
    typedef QList<KBookmark> KBookmarkList;
    QMap<QString, KBookmarkList> m_bk_map;
    KBookmarkManager *m_manager;
};

static KBookmarkMap *s_bk_map = 0;

KBookmarkMap::KBookmarkMap( KBookmarkManager *manager ) {
    m_manager = manager;
}

void KBookmarkMap::update()
{
    m_bk_map.clear();
    KBookmarkGroup root = m_manager->root();
    traverse(root);
}

void KBookmarkMap::visit(const KBookmark &bk)
{
    if (!bk.isSeparator()) {
        // add bookmark to url map
        m_bk_map[bk.internalElement().attribute("href")].append(bk);
    }
}

// #########################
// KBookmarkManager::Private
class KBookmarkManager::Private
{
public:
    Private(bool bDocIsloaded, const QString &dbusObjectName = QString())
      : m_doc("xbel")
      , m_dbusObjectName(dbusObjectName)
      , m_docIsLoaded(bDocIsloaded)

    {}

    mutable QDomDocument m_doc;
    mutable QDomDocument m_toolbarDoc;
    QString m_bookmarksFile;
    QString m_dbusObjectName;
    mutable bool m_docIsLoaded;
    bool m_update;

    bool m_browserEditor;
    QString m_editorCaption;

};

// ################
// KBookmarkManager
KBookmarkManager* KBookmarkManager::managerForFile( const QString& bookmarksFile, const QString& dbusObjectName )
{
    for ( KBookmarkManagerList::ConstIterator bmit = s_pSelf->constBegin(), bmend = s_pSelf->constEnd();
          bmit != bmend; ++bmit )
        if ( (*bmit)->path() == bookmarksFile )
            return *bmit;

    KBookmarkManager* mgr = new KBookmarkManager( bookmarksFile, dbusObjectName );
    s_pSelf->append( mgr );
    return mgr;
}

// principally used for filtered toolbars
KBookmarkManager* KBookmarkManager::createTempManager()
{
    KBookmarkManager* mgr = new KBookmarkManager();
    s_pSelf->append( mgr );
    return mgr;
}

#define PI_DATA "version=\"1.0\" encoding=\"UTF-8\""

KBookmarkManager::KBookmarkManager( const QString & bookmarksFile, const QString & dbusObjectName )
 : d(new Private(false, dbusObjectName))
{
    init( "/KBookmarkManager/"+dbusObjectName );

    d->m_update = true;

    Q_ASSERT( !bookmarksFile.isEmpty() );
    d->m_bookmarksFile = bookmarksFile;

    if ( !QFile::exists(d->m_bookmarksFile) )
    {
        QDomElement topLevel = d->m_doc.createElement("xbel");
        d->m_doc.appendChild( topLevel );
        d->m_doc.insertBefore( d->m_doc.createProcessingInstruction( "xml", PI_DATA), topLevel );
        d->m_docIsLoaded = true;
    }
}

KBookmarkManager::KBookmarkManager( )
    : d(new Private(true))
{
    init( "/KBookmarkManager/generated" );
    d->m_update = false; // TODO - make it read/write

    QDomElement topLevel = d->m_doc.createElement("xbel");
    d->m_doc.appendChild( topLevel );
    d->m_doc.insertBefore( d->m_doc.createProcessingInstruction( "xml", PI_DATA), topLevel );
}

void KBookmarkManager::init( const QString& dbusPath )
{
    // A KBookmarkManager without a dbus name is a temporary one, like those used by importers;
    // no need to register them to dbus
    if ( dbusPath != "/KBookmarkManager/" )
    {
        new KBookmarkManagerAdaptor(this);
        QDBusConnection::sessionBus().registerObject( dbusPath, this );

        QDBusConnection::sessionBus().connect(QString(), dbusPath, "org.kde.KIO.KBookmarkManager",
                                    "bookmarksChanged", this, SLOT(notifyChanged(QString,QDBusMessage)));
        QDBusConnection::sessionBus().connect(QString(), dbusPath, "org.kde.KIO.KBookmarkManager",
                                    "bookmarkConfigChanged", this, SLOT(notifyConfigChanged()));
    }
}

KBookmarkManager::~KBookmarkManager()
{
    if(!s_pSelf.isDestroyed())
        s_pSelf->removeAll( this );
    delete d;
}

void KBookmarkManager::setUpdate( bool update )
{
    d->m_update = update;
}

const QDomDocument &KBookmarkManager::internalDocument() const
{
    if(!d->m_docIsLoaded)
    {
        parse();
        d->m_toolbarDoc.clear();
    }
    return d->m_doc;
}


void KBookmarkManager::parse() const
{
    d->m_docIsLoaded = true;
    //kDebug(7043) << "KBookmarkManager::parse " << d->m_bookmarksFile << endl;
    QFile file( d->m_bookmarksFile );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        kWarning() << "Can't open " << d->m_bookmarksFile << endl;
        return;
    }
    d->m_doc = QDomDocument("xbel");
    d->m_doc.setContent( &file );

    QDomElement docElem = d->m_doc.documentElement();
    if ( docElem.isNull() )
        kWarning() << "KBookmarkManager::parse : can't parse " << d->m_bookmarksFile << endl;
    else
    {
        QString mainTag = docElem.tagName();
        if ( mainTag == "BOOKMARKS" )
        {
            kWarning() << "Old style bookmarks found. Calling convertToXBEL." << endl;
            docElem.setTagName("xbel");
            if ( docElem.hasAttribute( "HIDE_NSBK" ) ) // non standard either, but we need it
            {
                docElem.setAttribute( "hide_nsbk", docElem.attribute( "HIDE_NSBK" ) == "1" ? "yes" : "no" );
                docElem.removeAttribute( "HIDE_NSBK" );
            }

            convertToXBEL( docElem );
            save();
        }
        else if ( mainTag != "xbel" )
            kWarning() << "KBookmarkManager::parse : unknown main tag " << mainTag << endl;

        QDomNode n = d->m_doc.documentElement().previousSibling();
        if ( n.isProcessingInstruction() )
        {
            QDomProcessingInstruction pi = n.toProcessingInstruction();
            pi.parentNode().removeChild(pi);
        }

        QDomProcessingInstruction pi;
        pi = d->m_doc.createProcessingInstruction( "xml", PI_DATA );
        d->m_doc.insertBefore( pi, docElem );
    }

    file.close();
    if ( !s_bk_map )
        s_bk_map = new KBookmarkMap( const_cast<KBookmarkManager*>( this ) );
    s_bk_map->update();
}

void KBookmarkManager::convertToXBEL( QDomElement & group )
{
    QDomNode n = group.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if ( !e.isNull() )
            if ( e.tagName() == "TEXT" )
            {
                e.setTagName("title");
            }
            else if ( e.tagName() == "SEPARATOR" )
            {
                e.setTagName("separator"); // so close...
            }
            else if ( e.tagName() == "GROUP" )
            {
                e.setTagName("folder");
                convertAttribute(e, "ICON","icon"); // non standard, but we need it
                if ( e.hasAttribute( "TOOLBAR" ) ) // non standard either, but we need it
                {
                    e.setAttribute( "toolbar", e.attribute( "TOOLBAR" ) == "1" ? "yes" : "no" );
                    e.removeAttribute( "TOOLBAR" );
                }

                convertAttribute(e, "NETSCAPEINFO","netscapeinfo"); // idem
                bool open = (e.attribute("OPEN") == "1");
                e.removeAttribute("OPEN");
                e.setAttribute("folded", open ? "no" : "yes");
                convertToXBEL( e );
            }
            else
                if ( e.tagName() == "BOOKMARK" )
                {
                    e.setTagName("bookmark"); // so much difference :-)
                    convertAttribute(e, "ICON","icon"); // non standard, but we need it
                    convertAttribute(e, "NETSCAPEINFO","netscapeinfo"); // idem
                    convertAttribute(e, "URL","href");
                    QString text = e.text();
                    while ( !e.firstChild().isNull() ) // clean up the old contained text
                        e.removeChild(e.firstChild());
                    QDomElement titleElem = e.ownerDocument().createElement("title");
                    e.appendChild( titleElem ); // should be the only child anyway
                    titleElem.appendChild( e.ownerDocument().createTextNode( text ) );
                }
                else
                    kWarning(7043) << "Unknown tag " << e.tagName() << endl;
        n = n.nextSibling();
    }
}

void KBookmarkManager::convertAttribute( QDomElement elem, const QString & oldName, const QString & newName )
{
    if ( elem.hasAttribute( oldName ) )
    {
        elem.setAttribute( newName, elem.attribute( oldName ) );
        elem.removeAttribute( oldName );
    }
}

bool KBookmarkManager::save( bool toolbarCache ) const
{
    return saveAs( d->m_bookmarksFile, toolbarCache );
}

bool KBookmarkManager::saveAs( const QString & filename, bool toolbarCache ) const
{
    kDebug(7043) << "KBookmarkManager::save " << filename << endl;

    // Save the bookmark toolbar folder for quick loading
    // but only when it will actually make things quicker
    const QString cacheFilename = filename + QLatin1String(".tbcache");
    if(toolbarCache && !root().isToolbarGroup())
    {
        KSaveFile cacheFile( cacheFilename );
        if ( cacheFile.open() )
        {
            QString str;
            QTextStream stream(&str, QIODevice::WriteOnly);
            stream << root().findToolbar();
            const QByteArray cstr = str.toUtf8();
            cacheFile.write( cstr.data(), cstr.length() );
            cacheFile.finalize();
        }
    }
    else // remove any (now) stale cache
    {
        QFile::remove( cacheFilename );
    }

    KSaveFile file( filename );
    if ( file.open() )
    {
        file.simpleBackupFile( file.fileName(), QString(), ".bak" );
        QTextStream stream(&file);
        stream << internalDocument().toString();
        stream.flush();
        if ( file.finalize() )
            return true;
    }

    static int hadSaveError = false;
    file.abort();
    if ( !hadSaveError ) {
        QString error = i18n("Unable to save bookmarks in %1. Reported error was: %2. "
                             "This error message will only be shown once. The cause "
                             "of the error needs to be fixed as quickly as possible, "
                             "which is most likely a full hard drive.",
                         filename, file.errorString());
        if (qApp->type() != QApplication::Tty)
            KMessageBox::error( 0L, error );
        else
            kError() << error << endl;
    }
    hadSaveError = true;
    return false;
}

QString KBookmarkManager::path() const
{
    return d->m_bookmarksFile;
}

KBookmarkGroup KBookmarkManager::root() const
{
    return KBookmarkGroup(internalDocument().documentElement());
}

KBookmarkGroup KBookmarkManager::toolbar()
{
    kDebug(7043) << "KBookmarkManager::toolbar begin" << endl;
    // Only try to read from a toolbar cache if the full document isn't loaded
    if(!d->m_docIsLoaded)
    {
        kDebug(7043) << "KBookmarkManager::toolbar trying cache" << endl;
        const QString cacheFilename = d->m_bookmarksFile + QLatin1String(".tbcache");
        QFileInfo bmInfo(d->m_bookmarksFile);
        QFileInfo cacheInfo(cacheFilename);
        if (d->m_toolbarDoc.isNull() &&
            QFile::exists(cacheFilename) &&
            bmInfo.lastModified() < cacheInfo.lastModified())
        {
            kDebug(7043) << "KBookmarkManager::toolbar reading file" << endl;
            QFile file( cacheFilename );

            if ( file.open( QIODevice::ReadOnly ) )
            {
                d->m_toolbarDoc = QDomDocument("cache");
                d->m_toolbarDoc.setContent( &file );
                kDebug(7043) << "KBookmarkManager::toolbar opened" << endl;
            }
        }
        if (!d->m_toolbarDoc.isNull())
        {
            kDebug(7043) << "KBookmarkManager::toolbar returning element" << endl;
            QDomElement elem = d->m_toolbarDoc.firstChild().toElement();
            return KBookmarkGroup(elem);
        }
    }

    // Fallback to the normal way if there is no cache or if the bookmark file
    // is already loaded
    QDomElement elem = root().findToolbar();
    if (elem.isNull())
        return root(); // Root is the bookmark toolbar if none has been set.
    else
        return KBookmarkGroup(root().findToolbar());
}

KBookmark KBookmarkManager::findByAddress( const QString & address, bool tolerant )
{
    //kDebug(7043) << "KBookmarkManager::findByAddress " << address << endl;
    KBookmark result = root();
    // The address is something like /5/10/2+
    const QStringList addresses = address.split(QRegExp("[/+]"),QString::SkipEmptyParts);
    // kWarning() << addresses.join(",") << endl;
    for ( QStringList::const_iterator it = addresses.begin() ; it != addresses.end() ; )
    {
       bool append = ((*it) == "+");
       uint number = (*it).toUInt();
       Q_ASSERT(result.isGroup());
       KBookmarkGroup group = result.toGroup();
       KBookmark bk = group.first(), lbk = bk; // last non-null bookmark
       for ( uint i = 0 ; ( (i<number) || append ) && !bk.isNull() ; ++i ) {
           lbk = bk;
           bk = group.next(bk);
         //kWarning() << i << endl;
       }
       it++;
       int shouldBeGroup = !bk.isGroup() && (it != addresses.end());
       if ( tolerant && ( bk.isNull() || shouldBeGroup ) ) {
          if (!lbk.isNull()) result = lbk;
          //kWarning() << "break" << endl;
          break;
       }
       //kWarning() << "found section" << endl;
       result = bk;
    }
    if (result.isNull()) {
       kWarning() << "KBookmarkManager::findByAddress: couldn't find item " << address << endl;
       Q_ASSERT(!tolerant);
    }
    //kWarning() << "found " << result.address() << endl;
    return result;
 }

static QString pickUnusedTitle( const KBookmarkGroup &parentBookmark,
                                const QString &title, const QString &url
) {
    // If this title is already used, we'll try to find something unused.
    KBookmark ch = parentBookmark.first();
    int count = 1;
    QString uniqueTitle = title;
    do
    {
        while ( !ch.isNull() )
        {
            if ( uniqueTitle == ch.text() )
            {
                // Title already used !
                if ( url != ch.url().url() )
                {
                    uniqueTitle = title + QString(" (%1)").arg(++count);
                    // New title -> restart search from the beginning
                    ch = parentBookmark.first();
                    break;
                }
                else
                {
                    // this exact URL already exists
                    return QString();
                }
            }
            ch = parentBookmark.next( ch );
        }
    } while ( !ch.isNull() );

    return uniqueTitle;
}

KBookmarkGroup KBookmarkManager::addBookmarkDialog(
                     const QString & _url, const QString & _title,
                     const QString & _parentBookmarkAddress //TODO parametr to override m_advancedaddbookmark
) {
    QString url = _url;
    QString title = _title;
    QString parentBookmarkAddress = _parentBookmarkAddress;

    if ( url.isEmpty() )
    {
        KMessageBox::error( 0L, i18n("Cannot add bookmark with empty URL."));
        return KBookmarkGroup();
    }

    if ( title.isEmpty() )
        title = url;

    if ( KBookmarkSettings::self()->m_advancedaddbookmark)
    {
        KBookmarkEditDialog dlg( title, url, this, KBookmarkEditDialog::InsertionMode, parentBookmarkAddress );
        if ( dlg.exec() != KDialog::Accepted )
            return KBookmarkGroup();
        title = dlg.finalTitle();
        url = dlg.finalUrl();
        parentBookmarkAddress = dlg.finalAddress();
    }

    KBookmarkGroup parentBookmark;
    parentBookmark = findByAddress( parentBookmarkAddress ).toGroup();
    Q_ASSERT( !parentBookmark.isNull() );

    QString uniqueTitle = pickUnusedTitle( parentBookmark, title, url ); //TODO pickUnusedTitle is used only here. Why?
    if ( !uniqueTitle.isNull() )
        parentBookmark.addBookmark( this, uniqueTitle, KUrl( url ));

    return parentBookmark;
}


void KBookmarkManager::emitChanged( const KBookmarkGroup & group )
{
    save();

    // Tell the other processes too
    // kDebug(7043) << "KBookmarkManager::emitChanged : broadcasting change " << group.address() << endl;

    emit bookmarksChanged(group.address());

    // We do get our own broadcast, so no need for this anymore
    //emit changed( group );
}

void KBookmarkManager::emitConfigChanged()
{
    emit bookmarkConfigChanged();
}

void KBookmarkManager::notifyCompleteChange( const QString &caller ) // DBUS call
{
    if (!d->m_update)
        return;

    kDebug(7043) << "KBookmarkManager::notifyCompleteChange" << endl;
    // The bk editor tells us we should reload everything
    // Reparse
    parse();
    // Tell our GUI
    // (emit where group is "" to directly mark the root menu as dirty)
    emit changed( "", caller );
}

void KBookmarkManager::notifyConfigChanged() // DBUS call
{
    kDebug() << "reloaded bookmark config!" << endl;
    KBookmarkSettings::self()->readSettings();
    parse(); // reload, and thusly recreate the menus
}

void KBookmarkManager::notifyChanged( const QString &groupAddress, const QDBusMessage &msg ) // DBUS call
{
    kDebug() << "KBookmarkManager::notifyChanged ( "<<groupAddress<<")"<<endl;
    if (!d->m_update)
        return;

    // Reparse (the whole file, no other choice)
    // if someone else notified us
    if (msg.service() != QDBusConnection::sessionBus().baseService())
       parse();

    //kDebug(7043) << "KBookmarkManager::notifyChanged " << groupAddress << endl;
    //KBookmarkGroup group = findByAddress( groupAddress ).toGroup();
    //Q_ASSERT(!group.isNull());
    emit changed( groupAddress, QString() );
}

void KBookmarkManager::setEditorOptions( const QString& caption, bool browser )
{
    d->m_editorCaption = caption;
    d->m_browserEditor = browser;
}

void KBookmarkManager::slotEditBookmarks()
{
    QStringList args;
    if ( !d->m_editorCaption.isEmpty() )
       args << QLatin1String("--customcaption") << d->m_editorCaption;
    if ( !d->m_browserEditor )
       args << QLatin1String("--nobrowser");
    if( !d->m_dbusObjectName.isEmpty() )
      args << QLatin1String("--dbusObjectName") << d->m_dbusObjectName;
    args << d->m_bookmarksFile;
    QProcess::startDetached("keditbookmarks", args);
}

void KBookmarkManager::slotEditBookmarksAtAddress( const QString& address )
{
    QStringList args;
    //TODO shouldn't we pass --customcaption and --nobrowser too?
    args << QLatin1String("--address") << address
         << d->m_bookmarksFile;
    QProcess::startDetached("keditbookmarks", args);
}

///////
bool KBookmarkManager::updateAccessMetadata( const QString & url, bool emitSignal )
{
    if (!s_bk_map) {
        s_bk_map = new KBookmarkMap(this);
        s_bk_map->update();
    }

    QList<KBookmark> list = s_bk_map->find(url);
    if ( list.count() == 0 )
        return false;

    for ( QList<KBookmark>::iterator it = list.begin();
          it != list.end(); ++it )
        (*it).updateAccessMetadata();

    if (emitSignal)
        emit KBookmarkNotifier::updatedAccessMetadata( path(), url );

    return true;
}

void KBookmarkManager::updateFavicon( const QString &url, const QString &faviconurl, bool emitSignal )
{
    Q_UNUSED(faviconurl);

    if (!s_bk_map) {
        s_bk_map = new KBookmarkMap(this);
        s_bk_map->update();
    }

    QList<KBookmark> list = s_bk_map->find(url);
    for ( QList<KBookmark>::iterator it = list.begin();
          it != list.end(); ++it )
    {
        // TODO - update favicon data based on faviconurl
        //        but only when the previously used icon
        //        isn't a manually set one.
    }

    if (emitSignal)
    {
        // TODO
        // emit notifier().updatedFavicon( path(), url, faviconurl );
    }
}

KBookmarkManager* KBookmarkManager::userBookmarksManager()
{
   QString bookmarksFile = KStandardDirs::locateLocal("data", QLatin1String("konqueror/bookmarks.xml"));
   return KBookmarkManager::managerForFile( bookmarksFile, "konqueror" );
}

KBookmarkSettings* KBookmarkSettings::s_self = 0;

void KBookmarkSettings::readSettings()
{
   KConfig config("kbookmarkrc", KConfig::NoGlobals);
   KConfigGroup cg(&config, "Bookmarks");

   // add bookmark dialog usage - no reparse
   s_self->m_advancedaddbookmark = cg.readEntry("AdvancedAddBookmarkDialog", false);

   // this one alters the menu, therefore it needs a reparse
   s_self->m_contextmenu = cg.readEntry("ContextMenuActions", true);
}

KBookmarkSettings *KBookmarkSettings::self()
{
   if (!s_self)
   {
      s_self = new KBookmarkSettings;
      readSettings();
   }
   return s_self;
}

#include "kbookmarkmanager.moc"
