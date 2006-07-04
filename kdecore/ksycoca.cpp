/*  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#include <config.h>

#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocafactory.h"
#include "ktoolinvocation.h"
#include "kglobal.h"
#include "kdebug.h"
#include "kstandarddirs.h"

#include <qdatastream.h>
#include <qcoreapplication.h>
#include <qfile.h>
#include <qbuffer.h>
#include <QtDBus/QtDBus>

#include <stdlib.h>
#include <fcntl.h>

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#ifdef Q_OS_SOLARIS
extern "C" extern int madvise(caddr_t, size_t, int);
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

// The following limitations are in place:
// Maximum length of a single string: 8192 bytes
// Maximum length of a string list: 1024 strings
// Maximum number of entries: 8192
//
// The purpose of these limitations is to limit the impact
// of database corruption.

class KSycoca::Private
{
public:
    Private() {
        database = 0;
        readError = false;
        updateSig = 0;
#ifdef Q_WS_WIN
        autoRebuild = false;
#else
        autoRebuild = true;
#endif
    }
    QFile *database;
    QStringList changeList;
    QString language;
    bool readError;
    bool autoRebuild;
    quint32 updateSig;
    QStringList allResourceDirs;
};

int KSycoca::version()
{
   return KSYCOCA_VERSION;
}

// Read-only constructor
KSycoca::KSycoca()
  : m_lstFactories(0), m_str(0), bNoDatabase(false),
    m_sycoca_size(0), m_sycoca_mmap(0), m_timeStamp(0), d(new Private)
{
   QDBus::sessionBus().connect(QString(), QString(), "org.kde.KSycoca", "notifyDatabaseChanged",
                               this, SLOT(notifyDatabaseChanged(QStringList)));
   // We register with D-Bus _before_ we try to open the database.
   // This way we can be relatively sure that the KDE framework is
   // up and running (kdeinit, dcopserver, klauncher, kded) and
   // that the database is up to date.

   //   -> huh? -thiago
   //   This is because dcopserver was autostarted (via kdeinit) when trying to register to dcop. - David
   //   But the "launching kdeinit" case below takes care of it.
   openDatabase();
   _self = this;
}

bool KSycoca::openDatabase( bool openDummyIfNotFound )
{
   bool result = true;

   m_sycoca_mmap = 0;
   m_str = 0;
   QString path;
   QByteArray ksycoca_env = getenv("KDESYCOCA");
   if (ksycoca_env.isEmpty())
      path = KGlobal::dirs()->saveLocation("cache") + KSYCOCA_FILENAME;
   else
      path = QFile::decodeName(ksycoca_env);

   kDebug(7011) << "Trying to open ksycoca from " << path << endl;
   QFile *database = new QFile(path);
   bool bOpen = database->open( QIODevice::ReadOnly );
   if (!bOpen)
   {
     path = KStandardDirs::locate("services", KSYCOCA_FILENAME);
     if (!path.isEmpty())
     {
       kDebug(7011) << "Trying to open global ksycoca from " << path << endl;
       delete database;
       database = new QFile(path);
       bOpen = database->open( QIODevice::ReadOnly );
     }
   }

   if (bOpen)
   {
     fcntl(database->handle(), F_SETFD, FD_CLOEXEC);
     m_sycoca_size = database->size();
#ifdef HAVE_MMAP
     m_sycoca_mmap = (const char *) mmap(0, m_sycoca_size,
                                PROT_READ, MAP_SHARED,
                                database->handle(), 0);
     /* POSIX mandates only MAP_FAILED, but we are paranoid so check for
        null pointer too.  */
     if (m_sycoca_mmap == (const char*) MAP_FAILED || m_sycoca_mmap == 0)
     {
        kDebug(7011) << "mmap failed. (length = " << m_sycoca_size << ")" << endl;
#endif
        m_str = new QDataStream(database);
        m_str->setVersion(QDataStream::Qt_3_1);
#ifdef HAVE_MMAP
     }
     else
     {
#ifdef HAVE_MADVISE
        (void) madvise((char*)m_sycoca_mmap, m_sycoca_size, MADV_WILLNEED);
#endif
        QByteArray b = QByteArray::fromRawData(m_sycoca_mmap, m_sycoca_size);
        QBuffer *buffer = new QBuffer( new QByteArray(b) );
        buffer->open(QIODevice::ReadOnly);
        m_str = new QDataStream( buffer);
        m_str->setVersion(QDataStream::Qt_3_1);
        //### FIXME: cleanup the array?
     }
#endif
     bNoDatabase = false;
   }
   else
   {
     kDebug(7011) << "Could not open ksycoca" << endl;

     // No database file
     delete database;
     database = 0;

     bNoDatabase = true;
     if (openDummyIfNotFound)
     {
        // We open a dummy database instead.
        //kDebug(7011) << "No database, opening a dummy one." << endl;
        QBuffer *buffer = new QBuffer( new QByteArray );
        buffer->open(QIODevice::ReadWrite);
        m_str = new QDataStream( buffer);
        m_str->setVersion(QDataStream::Qt_3_1);
        *m_str << qint32(KSYCOCA_VERSION);
        *m_str << qint32(0);
     }
     else
     {
        result = false;
     }
   }
   m_lstFactories = new KSycocaFactoryList;
   d->database = database;
   return result;
}

// Read-write constructor - only for KBuildSycoca
KSycoca::KSycoca( bool /* dummy */ )
  : m_lstFactories(0), m_str(0), bNoDatabase(false),
    m_sycoca_size(0), m_sycoca_mmap(0), d(new Private)
{
   QDBus::sessionBus().registerObject("/ksycoca_building", this, QDBusConnection::ExportSlots);
   m_lstFactories = new KSycocaFactoryList;
   _self = this;
}

static void delete_ksycoca_self() {
  delete KSycoca::_self;
}

KSycoca * KSycoca::self()
{
    if (!_self) {
        qAddPostRoutine(delete_ksycoca_self);
        _self = new KSycoca;
    }
  return _self;
}

KSycoca::~KSycoca()
{
   closeDatabase();
   delete d;
   _self = 0L;
}

bool KSycoca::isAvailable()
{
   if ( self()->bNoDatabase )
       return false;
   return self()->checkVersion(false);
}

void KSycoca::closeDatabase()
{
   QIODevice *device = 0;
   if (m_str)
      device = m_str->device();
#ifdef HAVE_MMAP
   if (device && m_sycoca_mmap)
   {
      QBuffer *buf = static_cast<QBuffer*>(device);
      buf->buffer().clear();
      // Solaris has munmap(char*, size_t) and everything else should
      // be happy with a char* for munmap(void*, size_t)
      munmap(const_cast<char*>(m_sycoca_mmap), m_sycoca_size);
      m_sycoca_mmap = 0;
   }
#endif

   delete m_str;
   m_str = 0;
   delete device;
   if (d->database != device)
      delete d->database;
   device = 0;
   d->database = 0;
   // It is very important to delete all factories here
   // since they cache information about the database file
   if ( m_lstFactories )
       qDeleteAll( *m_lstFactories );
   delete m_lstFactories;
   m_lstFactories = 0;
}

void KSycoca::addFactory( KSycocaFactory *factory )
{
   Q_ASSERT(m_lstFactories != 0);
   m_lstFactories->append(factory);
}

bool KSycoca::isChanged(const char *type)
{
    return self()->d->changeList.contains(type);
}

void KSycoca::notifyDatabaseChanged(const QStringList &changeList)
{
    d->changeList = changeList;
    //kDebug(7011) << "got a notifyDatabaseChanged signal !" << endl;
    // kded tells us the database file changed
    // Close the database and forget all about what we knew
    // The next call to any public method will recreate
    // everything that's needed.
    closeDatabase();

    // Now notify applications
    emit databaseChanged();
}

QDataStream * KSycoca::findEntry(int offset, KSycocaType &type)
{
   if ( !m_str )
      openDatabase();
   //kDebug(7011) << QString("KSycoca::_findEntry(offset=%1)").arg(offset,8,16) << endl;
   m_str->device()->seek(offset);
   qint32 aType;
   *m_str >> aType;
   type = KSycocaType(aType);
   //kDebug(7011) << QString("KSycoca::found type %1").arg(aType) << endl;
   return m_str;
}

bool KSycoca::checkVersion(bool abortOnError)
{
   if ( !m_str )
   {
      if( !openDatabase(false /* don't open dummy db if not found */) )
        return false; // No database found

      // We should never get here... if a database was found then m_str shouldn't be 0L.
      Q_ASSERT(m_str);
   }
   m_str->device()->seek(0);
   qint32 aVersion;
   *m_str >> aVersion;
   if ( aVersion < KSYCOCA_VERSION )
   {
      kWarning(7011) << "Found version " << aVersion << ", expecting version " << KSYCOCA_VERSION << " or higher." << endl;
      if (!abortOnError) return false;
      kFatal(7011) << "Outdated database ! Stop kded and restart it !" << endl;
   }
   return true;
}

QDataStream * KSycoca::findFactory(KSycocaFactoryId id)
{
   // The constructor found no database, but we want one
   if (bNoDatabase)
   {
      closeDatabase(); // close the dummy one
      // Check if new database already available
      if ( !openDatabase(false /* no dummy one*/) )
      {
         static bool triedLaunchingKdeinit = false;
         if (!triedLaunchingKdeinit) // try only once
         {
           triedLaunchingKdeinit = true;
           kDebug(7011) << "findFactory: we have no database.... launching kdeinit" << endl;
           KToolInvocation::klauncher(); // this calls startKdeinit
           // Ok, the new database should be here now, open it.
         }
         if (!openDatabase(false))
            return 0L; // Still no database - uh oh
      }
   }
   // rewind and check
   if (!checkVersion(false))
   {
     kWarning(7011) << "Outdated database found" << endl;
     return 0L;
   }
   qint32 aId;
   qint32 aOffset;
   while(true)
   {
      *m_str >> aId;
      //kDebug(7011) << QString("KSycoca::findFactory : found factory %1").arg(aId) << endl;
      if (aId == 0)
      {
         kError(7011) << "Error, KSycocaFactory (id = " << int(id) << ") not found!" << endl;
         break;
      }
      *m_str >> aOffset;
      if (aId == id)
      {
         //kDebug(7011) << QString("KSycoca::findFactory(%1) offset %2").arg((int)id).arg(aOffset) << endl;
         m_str->device()->seek(aOffset);
         return m_str;
      }
   }
   return 0;
}

QString KSycoca::kfsstnd_prefixes()
{
   if (bNoDatabase) return "";
   if (!checkVersion(false)) return "";
   qint32 aId;
   qint32 aOffset;
   // skip factories offsets
   while(true)
   {
      *m_str >> aId;
      if ( aId )
        *m_str >> aOffset;
      else
        break; // just read 0
   }
   // We now point to the header
   QString prefixes;
   KSycocaEntry::read(*m_str, prefixes);
   *m_str >> m_timeStamp;
   KSycocaEntry::read(*m_str, d->language);
   *m_str >> d->updateSig;
   KSycocaEntry::read(*m_str, d->allResourceDirs);
   return prefixes;
}

quint32 KSycoca::timeStamp()
{
   if (!m_timeStamp)
      (void) kfsstnd_prefixes();
   return m_timeStamp;
}

quint32 KSycoca::updateSignature()
{
   if (!m_timeStamp)
      (void) kfsstnd_prefixes();
   return d->updateSig;
}

QString KSycoca::language()
{
   if (d->language.isEmpty())
      (void) kfsstnd_prefixes();
   return d->language;
}

QStringList KSycoca::allResourceDirs()
{
   if (!m_timeStamp)
      (void) kfsstnd_prefixes();
   return d->allResourceDirs;
}

QString KSycoca::determineRelativePath( const QString & _fullpath, const char *_resource )
{
  QString sRelativeFilePath;
  QStringList dirs = KGlobal::dirs()->resourceDirs( _resource );
  QStringList::ConstIterator dirsit = dirs.begin();
  for ( ; dirsit != dirs.end() && sRelativeFilePath.isEmpty(); ++dirsit ) {
    // might need canonicalPath() ...
    if ( _fullpath.indexOf( *dirsit ) == 0 ) // path is dirs + relativePath
      sRelativeFilePath = _fullpath.mid( (*dirsit).length() ); // skip appsdirs
  }
  if ( sRelativeFilePath.isEmpty() )
    kFatal(7011) << QString("Couldn't find %1 in any %2 dir !!!").arg( _fullpath ).arg( _resource) << endl;
  //else
    // debug code
    //kDebug(7011) << sRelativeFilePath << endl;
  return sRelativeFilePath;
}

KSycoca * KSycoca::_self = 0L;

void KSycoca::flagError()
{
   qWarning("ERROR: KSycoca database corruption!");
   if (_self)
   {
      if (_self->d->readError)
         return;
      _self->d->readError = true;
      if (_self->d->autoRebuild)
         if(system("kbuildsycoca") < 0) // Rebuild the damned thing.
	   qWarning("ERROR: Running KSycoca failed.");
   }
}

void KSycoca::disableAutoRebuild()
{
   d->autoRebuild = false;
}

bool KSycoca::readError()
{
   bool b = false;
   if (_self)
   {
      b = _self->d->readError;
      _self->d->readError = false;
   }
   return b;
}

void KSycocaEntry::read( QDataStream &s, QString &str )
{
  quint32 bytes;
  s >> bytes;                          // read size of string
  if ( bytes > 8192 ) {                // null string or too big
      if (bytes != 0xffffffff)
         KSycoca::flagError();
      str.clear();
  }
  else if ( bytes > 0 ) {              // not empty
      int bt = bytes/2;
      str.resize( bt );
      QChar* ch = (QChar *) str.unicode();
      char t[8192];
      char *b = t;
      s.readRawData( b, bytes );
      while ( bt-- ) {
          *ch++ = (ushort) (((ushort)b[0])<<8) | (uchar)b[1];
	  b += 2;
      }
  } else {
      str = "";
  }
}

KSycocaEntry::~KSycocaEntry()
{
}

void KSycocaEntry::read( QDataStream &s, QStringList &list )
{
  list.clear();
  quint32 count;
  s >> count;                          // read size of list
  if (count >= 1024)
  {
     KSycoca::flagError();
     return;
  }
  for(quint32 i = 0; i < count; i++)
  {
     QString str;
     read(s, str);
     list.append( str );
     if (s.atEnd())
     {
        KSycoca::flagError();
        return;
     }
  }
}

void KSycocaEntry::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ksycoca.moc"
