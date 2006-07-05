/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>

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

#include "qtest_kde.h"

#include "jobtest.h"

#include <config.h>

#include <kurl.h>
#include <kapplication.h>
#include <kde_file.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <qfileinfo.h>
#include <qeventloop.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qhash.h>
#include <qvariant.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <utime.h>
#include <kprotocolinfo.h>

QTEST_KDEMAIN( JobTest, GUI )

// The code comes partly from kdebase/kioslave/trash/testtrash.cpp

QString JobTest::homeTmpDir() const
{
    return QFile::decodeName( getenv( "KDEHOME" ) ) + "/jobtest/";
}

QString JobTest::otherTmpDir() const
{
    // This one needs to be on another partition
    return "/tmp/jobtest/";
}

KUrl JobTest::systemTmpDir() const
{
    return KUrl( "system:/home/.kde-unit-test/jobtest-system/" );
}

QString JobTest::realSystemPath() const
{
    return QFile::decodeName( getenv( "KDEHOME" ) ) + "/jobtest-system/";
}

void JobTest::initTestCase()
{
    qDebug( "initTestCase" );
    // Start with a clean base dir
    cleanupTestCase();
    QDir dir; // TT: why not a static method?
    if ( !QFile::exists( homeTmpDir() ) ) {
        bool ok = dir.mkdir( homeTmpDir() );
        if ( !ok )
            kFatal() << "Couldn't create " << homeTmpDir() << endl;
    }
    if ( !QFile::exists( otherTmpDir() ) ) {
        bool ok = dir.mkdir( otherTmpDir() );
        if ( !ok )
            kFatal() << "Couldn't create " << otherTmpDir() << endl;
    }
    if ( KProtocolInfo::isKnownProtocol( "system" ) ) {
        if ( !QFile::exists( realSystemPath() ) ) {
            bool ok = dir.mkdir( realSystemPath() );
            if ( !ok )
                kFatal() << "Couldn't create " << realSystemPath() << endl;
        }
    }
}

void JobTest::cleanupTestCase()
{
    KIO::NetAccess::del( KUrl::fromPath( homeTmpDir() ), 0 );
    KIO::NetAccess::del( KUrl::fromPath( otherTmpDir() ), 0 );
    if ( KProtocolInfo::isKnownProtocol( "system" ) ) {
        KIO::NetAccess::del( systemTmpDir(), 0 );
    }
}

static void setTimeStamp( const QString& path )
{
#ifdef Q_OS_UNIX
    // Put timestamp in the past so that we can check that the
    // copy actually preserves it.
    struct timeval tp;
    gettimeofday( &tp, 0 );
    struct utimbuf utbuf;
    utbuf.actime = tp.tv_sec - 30; // 30 seconds ago
    utbuf.modtime = tp.tv_sec - 60; // 60 second ago
    utime( QFile::encodeName( path ), &utbuf );
    qDebug( "Time changed for %s", qPrintable( path ) );
#endif
}

static void createTestFile( const QString& path )
{
    QFile f( path );
    if ( !f.open( QIODevice::WriteOnly ) )
        kFatal() << "Can't create " << path << endl;
    f.write( QByteArray( "Hello world" ) );
    f.close();
    setTimeStamp( path );
}

static void createTestSymlink( const QString& path )
{
    // Create symlink if it doesn't exist yet
    KDE_struct_stat buf;
    if ( KDE_lstat( QFile::encodeName( path ), &buf ) != 0 ) {
        bool ok = symlink( "/IDontExist", QFile::encodeName( path ) ) == 0; // broken symlink
        if ( !ok )
            kFatal() << "couldn't create symlink: " << strerror( errno ) << endl;
        QVERIFY( KDE_lstat( QFile::encodeName( path ), &buf ) == 0 );
        QVERIFY( S_ISLNK( buf.st_mode ) );
    } else {
        QVERIFY( S_ISLNK( buf.st_mode ) );
    }
    qDebug( "symlink %s created", qPrintable( path ) );
    QVERIFY( QFileInfo( path ).isSymLink() );
}

static void createTestDirectory( const QString& path )
{
    QDir dir;
    bool ok = dir.mkdir( path );
    if ( !ok && !dir.exists() )
        kFatal() << "couldn't create " << path << endl;
    createTestFile( path + "/testfile" );
    createTestSymlink( path + "/testlink" );
    QVERIFY( QFileInfo( path + "/testlink" ).isSymLink() );
    setTimeStamp( path );
}

void JobTest::enterLoop()
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(exitLoop()),
            &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void JobTest::get()
{
    kDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    createTestFile( filePath );
    KUrl u; u.setPath( filePath );
    m_result = -1;
    KIO::StoredTransferJob* job = KIO::storedGet( u );
    job->setUiDelegate( 0 );
    connect( job, SIGNAL( result( KJob* ) ),
            this, SLOT( slotGetResult( KJob* ) ) );
    enterLoop();
    QVERIFY( m_result == 0 ); // no error
    QVERIFY( m_data.size() == 11 );
    QVERIFY( QByteArray( m_data ) == "Hello world" );
}

void JobTest::slotGetResult( KJob* job )
{
    m_result = job->error();
    m_data = static_cast<KIO::StoredTransferJob *>(job)->data();
    emit exitLoop();
}

////

void JobTest::copyLocalFile( const QString& src, const QString& dest )
{
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    // copy the file with file_copy
    bool ok = KIO::NetAccess::file_copy( u, d );
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // still there

    {
        // check that the timestamp is the same (#24443)
        // Note: this only works because of copy() in kio_file.
        // The datapump solution ignores mtime, the app has to call FileCopyJob::setModificationTime()
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        QCOMPARE( srcInfo.lastModified(), destInfo.lastModified() );
    }

    // cleanup and retry with KIO::copy()
    QFile::remove( dest );
    ok = KIO::NetAccess::dircopy( u, d, 0 );
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // still there
    {
        // check that the timestamp is the same (#24443)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        QCOMPARE( srcInfo.lastModified(), destInfo.lastModified() );
    }
}

void JobTest::copyLocalDirectory( const QString& src, const QString& _dest, int flags )
{
    QVERIFY( QFileInfo( src ).isDir() );
    QVERIFY( QFileInfo( src + "/testfile" ).isFile() );
    KUrl u;
    u.setPath( src );
    QString dest( _dest );
    KUrl d;
    d.setPath( dest );
    if ( flags & AlreadyExists )
        QVERIFY( QFile::exists( dest ) );
    else
        QVERIFY( !QFile::exists( dest ) );

    bool ok = KIO::NetAccess::dircopy( u, d, 0 );
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFileInfo( dest ).isDir() );
    QVERIFY( QFileInfo( dest + "/testfile" ).isFile() );
    QVERIFY( QFile::exists( src ) ); // still there

    if ( flags & AlreadyExists ) {
        dest += '/' + u.fileName();
        //kDebug() << "Expecting dest=" << dest << endl;
    }

    {
        // check that the timestamp is the same (#24443)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        QCOMPARE( srcInfo.lastModified(), destInfo.lastModified() );
    }
}

void JobTest::copyFileToSamePartition()
{
    kDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = homeTmpDir() + "fileFromHome_copied";
    createTestFile( filePath );
    copyLocalFile( filePath, dest );
}

void JobTest::copyDirectoryToSamePartition()
{
    kDebug() << k_funcinfo << endl;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_copied";
    createTestDirectory( src );
    copyLocalDirectory( src, dest );
}

void JobTest::copyDirectoryToExistingDirectory()
{
    kDebug() << k_funcinfo << endl;
    // just the same as copyDirectoryToSamePartition, but it means that
    // this time dest exists.
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_copied";
    createTestDirectory( src );
    copyLocalDirectory( src, dest, AlreadyExists );
}

void JobTest::copyFileToOtherPartition()
{
    kDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = otherTmpDir() + "fileFromHome_copied";
    createTestFile( filePath );
    copyLocalFile( filePath, dest );
}

void JobTest::copyDirectoryToOtherPartition()
{
    kDebug() << k_funcinfo << endl;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = otherTmpDir() + "dirFromHome_copied";
    // src is already created by copyDirectoryToSamePartition()
    // so this is just in case someone calls this method only
    if ( !QFile::exists( src ) )
        createTestDirectory( src );
    copyLocalDirectory( src, dest );
}

void JobTest::moveLocalFile( const QString& src, const QString& dest )
{
    QVERIFY( QFile::exists( src ) );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    // move the file with file_move
    bool ok = KIO::NetAccess::file_move( u, d );
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( !QFile::exists( src ) ); // not there anymore

    // move it back with KIO::move()
    ok = KIO::NetAccess::move( d, u, 0 );
    QVERIFY( ok );
    QVERIFY( !QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // it's back
}

static void moveLocalSymlink( const QString& src, const QString& dest )
{
    KDE_struct_stat buf;
    QVERIFY ( KDE_lstat( QFile::encodeName( src ), &buf ) == 0 );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    // move the symlink with move, NOT with file_move
    bool ok = KIO::NetAccess::move( u, d );
    if ( !ok )
        kWarning() << KIO::NetAccess::lastError() << endl;
    QVERIFY( ok );
    QVERIFY ( KDE_lstat( QFile::encodeName( dest ), &buf ) == 0 );
    QVERIFY( !QFile::exists( src ) ); // not there anymore

    // move it back with KIO::move()
    ok = KIO::NetAccess::move( d, u, 0 );
    QVERIFY( ok );
    QVERIFY ( KDE_lstat( QFile::encodeName( dest ), &buf ) != 0 ); // doesn't exist anymore
    QVERIFY ( KDE_lstat( QFile::encodeName( src ), &buf ) == 0 ); // it's back
}

void JobTest::moveLocalDirectory( const QString& src, const QString& dest )
{
    kDebug() << k_funcinfo << src << " " << dest << endl;
    QVERIFY( QFile::exists( src ) );
    QVERIFY( QFileInfo( src ).isDir() );
    QVERIFY( QFileInfo( src + "/testfile" ).isFile() );
    QVERIFY( QFileInfo( src + "/testlink" ).isSymLink() );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    bool ok = KIO::NetAccess::move( u, d, 0 );
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFileInfo( dest ).isDir() );
    QVERIFY( QFileInfo( dest + "/testfile" ).isFile() );
    QVERIFY( !QFile::exists( src ) ); // not there anymore

    QVERIFY( QFileInfo( dest + "/testlink" ).isSymLink() );
}

void JobTest::moveFileToSamePartition()
{
    kDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = homeTmpDir() + "fileFromHome_moved";
    createTestFile( filePath );
    moveLocalFile( filePath, dest );
}

void JobTest::moveDirectoryToSamePartition()
{
    kDebug() << k_funcinfo << endl;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_moved";
    createTestDirectory( src );
    moveLocalDirectory( src, dest );
}

void JobTest::moveFileToOtherPartition()
{
    kDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = otherTmpDir() + "fileFromHome_moved";
    createTestFile( filePath );
    moveLocalFile( filePath, dest );
}

void JobTest::moveSymlinkToOtherPartition()
{
    kDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "testlink";
    const QString dest = otherTmpDir() + "testlink_moved";
    createTestSymlink( filePath );
    moveLocalSymlink( filePath, dest );
}

void JobTest::moveDirectoryToOtherPartition()
{
    kDebug() << k_funcinfo << endl;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = otherTmpDir() + "dirFromHome_moved";
    createTestDirectory( src );
    moveLocalDirectory( src, dest );
}

void JobTest::moveFileNoPermissions()
{
    kDebug() << k_funcinfo << endl;
    const QString src = "/etc/passwd";
    const QString dest = homeTmpDir() + "passwd";
    QVERIFY( QFile::exists( src ) );
    QVERIFY( QFileInfo( src ).isFile() );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    KIO::CopyJob* job = KIO::move( u, d, 0 );
    job->setUiDelegate( 0 ); // no skip dialog, thanks
    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun( job, 0, 0, 0, &metaData );
    QVERIFY( !ok );
    QVERIFY( KIO::NetAccess::lastError() == KIO::ERR_ACCESS_DENIED );
    // OK this is fishy. Just like mv(1), KIO's behavior depends on whether
    // a direct rename(2) was used, or a full copy+del. In the first case
    // there is no destination file created, but in the second case the
    // destination file remains.
    // In fact we assume /home is a separate partition, in this test, so:
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) );
}

void JobTest::moveDirectoryNoPermissions()
{
    kDebug() << k_funcinfo << endl;
#if 1
    QString src = "/etc/rc.d";
    if ( !QFile::exists( src ) )
        src= "/etc";
#else
    QString src = "/etc";
#endif
    const QString dest = homeTmpDir() + "mdnp";
    QVERIFY( QFile::exists( src ) );
    QVERIFY( QFileInfo( src ).isDir() );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    KIO::CopyJob* job = KIO::move( u, d, 0 );
    job->setUiDelegate( 0 ); // no skip dialog, thanks
    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun( job, 0, 0, 0, &metaData );
    QVERIFY( !ok );
    qDebug( "%d", KIO::NetAccess::lastError() );
    QVERIFY( KIO::NetAccess::lastError() == KIO::ERR_ACCESS_DENIED );
    QVERIFY( QFile::exists( dest ) ); // see moveFileNoPermissions
    QVERIFY( QFile::exists( src ) );
}

void JobTest::listRecursive()
{
    const QString src = homeTmpDir();
    // Add a symlink to a dir, to make sure we don't recurse into those
    bool symlinkOk = symlink( "dirFromHome", QFile::encodeName( src + "/dirFromHome_link" ) ) == 0;
    QVERIFY( symlinkOk );

    KUrl u;
    u.setPath( src );
    KIO::ListJob* job = KIO::listRecursive( u );
    job->setUiDelegate( 0 );
    connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
             SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );
    m_names.sort();
    qDebug( "%s", qPrintable( m_names.join( "," ) ) );
    QCOMPARE( m_names.join( "," ).toLatin1(), QByteArray( ".,..,"
            "dirFromHome,dirFromHome/testfile,dirFromHome/testlink,dirFromHome_copied,"
            "dirFromHome_copied/dirFromHome,dirFromHome_copied/dirFromHome/testfile,dirFromHome_copied/dirFromHome/testlink,"
            "dirFromHome_copied/testfile,dirFromHome_copied/testlink,dirFromHome_link,"
            "fileFromHome,fileFromHome_copied" ) );
}

void JobTest::slotEntries( KIO::Job*, const KIO::UDSEntryList& lst )
{
    for( KIO::UDSEntryList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
        QString displayName = (*it).stringValue( KIO::UDS_NAME );
        //KUrl url = (*it).stringValue( KIO::UDS_URL );
        m_names.append( displayName );
    }
}

class OldUDSAtom
{
public:
  QString m_str;
  long long m_long;
  unsigned int m_uds;
};
typedef QList<OldUDSAtom> OldUDSEntry; // well it was a QValueList :)

static void fillOldUDSEntry( OldUDSEntry& entry, time_t now_time_t, const QString& nameStr )
{
    OldUDSAtom atom;
    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = nameStr;
    entry.append( atom );
    atom.m_uds = KIO::UDS_SIZE;
    atom.m_long = 123456ULL;
    entry.append( atom );
    atom.m_uds = KIO::UDS_MODIFICATION_TIME;
    atom.m_long = now_time_t;
    entry.append( atom );
    atom.m_uds = KIO::UDS_ACCESS_TIME;
    atom.m_long = now_time_t;
    entry.append( atom );
    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFREG;
    entry.append( atom );
    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = 0644;
    entry.append( atom );
    atom.m_uds = KIO::UDS_USER;
    atom.m_str = nameStr;
    entry.append( atom );
    atom.m_uds = KIO::UDS_GROUP;
    atom.m_str = nameStr;
    entry.append( atom );
}

// QHash or QMap? doesn't seem to make much difference.
typedef QHash<uint, QVariant> UDSEntryHV;

static void fillUDSEntryHV( UDSEntryHV& entry, const QDateTime& now, const QString& nameStr )
{
    entry.reserve( 8 );
    entry.insert( KIO::UDS_NAME, nameStr );
    // we might need a method to make sure people use unsigned long long
    entry.insert( KIO::UDS_SIZE, 123456ULL );
    entry.insert( KIO::UDS_MODIFICATION_TIME, now );
    entry.insert( KIO::UDS_ACCESS_TIME, now );
    entry.insert( KIO::UDS_FILE_TYPE, S_IFREG );
    entry.insert( KIO::UDS_ACCESS, 0644 );
    entry.insert( KIO::UDS_USER, nameStr );
    entry.insert( KIO::UDS_GROUP, nameStr );
}

// Which one is used depends on UDS_STRING vs UDS_LONG
struct UDSAtom4 // can't be a union due to qstring...
{
  UDSAtom4() {} // for QHash or QMap
  UDSAtom4( const QString& s ) : m_str( s ) {}
  UDSAtom4( long long l ) : m_long( l ) {}

  QString m_str;
  long long m_long;
};

// Another possibility, to save on QVariant costs
typedef QHash<uint, UDSAtom4> UDSEntryHS; // hash+struct

static void fillQHashStructEntry( UDSEntryHS& entry, time_t now_time_t, const QString& nameStr )
{
    entry.reserve( 8 );
    entry.insert( KIO::UDS_NAME, nameStr );
    entry.insert( KIO::UDS_SIZE, 123456ULL );
    entry.insert( KIO::UDS_MODIFICATION_TIME, now_time_t );
    entry.insert( KIO::UDS_ACCESS_TIME, now_time_t );
    entry.insert( KIO::UDS_FILE_TYPE, S_IFREG );
    entry.insert( KIO::UDS_ACCESS, 0644 );
    entry.insert( KIO::UDS_USER, nameStr );
    entry.insert( KIO::UDS_GROUP, nameStr );
}

// Let's see if QMap makes any difference
typedef QMap<uint, UDSAtom4> UDSEntryMS; // map+struct

static void fillQMapStructEntry( UDSEntryMS& entry, time_t now_time_t, const QString& nameStr )
{
    entry.insert( KIO::UDS_NAME, nameStr );
    entry.insert( KIO::UDS_SIZE, 123456ULL );
    entry.insert( KIO::UDS_MODIFICATION_TIME, now_time_t );
    entry.insert( KIO::UDS_ACCESS_TIME, now_time_t );
    entry.insert( KIO::UDS_FILE_TYPE, S_IFREG );
    entry.insert( KIO::UDS_ACCESS, 0644 );
    entry.insert( KIO::UDS_USER, nameStr );
    entry.insert( KIO::UDS_GROUP, nameStr );
}


void JobTest::newApiPerformance()
{
    const QDateTime now = QDateTime::currentDateTime();
    const time_t now_time_t = now.toTime_t();
    // use 30000 for callgrind, at least 100 times that for timing-based
    // use /10 times that in svn, so that jobtest doesn't last forever
    const int iterations = 30000 /* * 100 */ / 10;
    const int lookupIterations = 5 * iterations;
    const QString nameStr = QString::fromLatin1( "name" );

    /*
      This is to compare the old list-of-lists API vs a QMap/QHash-based API
      in terms of performance.

      The number of atoms and their type map to what kio_file would put in
      for any normal file.

      The lookups are done for two atoms that are present, and for one that is not.

    */

    /// Old API
    {
        qDebug( "Timing old api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            OldUDSEntry entry;
            fillOldUDSEntry( entry, now_time_t, nameStr );
        }
        qDebug("Old API: slave code: %ld", time(0) - start);

        OldUDSEntry entry;
        fillOldUDSEntry( entry, now_time_t, nameStr );
        QCOMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        QString displayName;
        KIO::filesize_t size;
        KUrl url;

        for (int i = 0; i < lookupIterations; ++i) {
            OldUDSEntry::ConstIterator it2 = entry.begin();
            for( ; it2 != entry.end(); it2++ ) {
                switch ((*it2).m_uds) {
                case KIO::UDS_NAME:
                    displayName = (*it2).m_str;
                    break;
                case KIO::UDS_URL:
                    url = (*it2).m_str;
                    break;
                case KIO::UDS_SIZE:
                    size = (*it2).m_long;
                    break;
                }
            }
        }

        qDebug("Old API: app code: %ld", time(0) - start);

        QCOMPARE( size, 123456ULL );
        QCOMPARE( displayName, QString::fromLatin1( "name" ) );
        QVERIFY( url.isEmpty() );
    }

    ///////// TEST CODE FOR FUTURE KIO API


    ////

    {
        qDebug( "Timing new QHash+QVariant api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            UDSEntryHV entry;
            fillUDSEntryHV( entry, now, nameStr );
        }

        qDebug("QHash+QVariant API: slave code: %ld", time(0) - start);

        UDSEntryHV entry;
        fillUDSEntryHV( entry, now, nameStr );
        QCOMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        // Normally the code would look like this, but let's change it to time it like the old api
        /*
        QString displayName = entry.value( KIO::UDS_NAME ).toString();
        KUrl url = entry.value( KIO::UDS_URL ).toString();
        KIO::filesize_t size = entry.value( KIO::UDS_SIZE ).toULongLong();
        */

        QString displayName;
        KIO::filesize_t size;
        KUrl url;

        for (int i = 0; i < lookupIterations; ++i) {

            // For a field that we assume to always be there
            displayName = entry.value( KIO::UDS_NAME ).toString();

            // For a field that might not be there
            UDSEntryHV::const_iterator it = entry.find( KIO::UDS_URL );
            const UDSEntryHV::const_iterator end = entry.end();
            if ( it != end )
                 url = it.value().toString();

            it = entry.find( KIO::UDS_SIZE );
            if ( it != end )
                size = it.value().toULongLong();
        }

        qDebug("QHash+QVariant API: app code: %ld", time(0) - start);

        QCOMPARE( size, 123456ULL );
        QCOMPARE( displayName, QString::fromLatin1( "name" ) );
        QVERIFY( url.isEmpty() );
    }

    // ########### THE CHOSEN SOLUTION:
    {
        qDebug( "Timing new QHash+struct api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            UDSEntryHS entry;
            fillQHashStructEntry( entry, now_time_t, nameStr );
        }

        qDebug("QHash+struct API: slave code: %ld", time(0) - start);

        UDSEntryHS entry;
        fillQHashStructEntry( entry, now_time_t, nameStr );
        QCOMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        QString displayName;
        KIO::filesize_t size;
        KUrl url;

        for (int i = 0; i < lookupIterations; ++i) {

            // For a field that we assume to always be there
            displayName = entry.value( KIO::UDS_NAME ).m_str;

            // For a field that might not be there
            UDSEntryHS::const_iterator it = entry.find( KIO::UDS_URL );
            const UDSEntryHS::const_iterator end = entry.end();
            if ( it != end )
                 url = it.value().m_str;

            it = entry.find( KIO::UDS_SIZE );
            if ( it != end )
                size = it.value().m_long;
        }

        qDebug("QHash+struct API: app code: %ld", time(0) - start);

        QCOMPARE( size, 123456ULL );
        QCOMPARE( displayName, QString::fromLatin1( "name" ) );
        QVERIFY( url.isEmpty() );
    }

    {
        qDebug( "Timing new QMap+struct api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            UDSEntryMS entry;
            fillQMapStructEntry( entry, now_time_t, nameStr );
        }

        qDebug("QMap+struct API: slave code: %ld", time(0) - start);

        UDSEntryMS entry;
        fillQMapStructEntry( entry, now_time_t, nameStr );
        QCOMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        QString displayName;
        KIO::filesize_t size;
        KUrl url;

        for (int i = 0; i < lookupIterations; ++i) {

            // For a field that we assume to always be there
            displayName = entry.value( KIO::UDS_NAME ).m_str;

            // For a field that might not be there
            UDSEntryMS::const_iterator it = entry.find( KIO::UDS_URL );
            const UDSEntryMS::const_iterator end = entry.end();
            if ( it != end )
                 url = it.value().m_str;

            it = entry.find( KIO::UDS_SIZE );
            if ( it != end )
                size = it.value().m_long;
        }

        qDebug("QMap+struct API: app code: %ld", time(0) - start);

        QCOMPARE( size, 123456ULL );
        QCOMPARE( displayName, QString::fromLatin1( "name" ) );
        QVERIFY( url.isEmpty() );
    }
}

void JobTest::calculateRemainingSeconds()
{
    unsigned int seconds = KIO::calculateRemainingSeconds( 2 * 86400 - 60, 0, 1 );
    QCOMPARE( seconds, static_cast<unsigned int>( 2 * 86400 - 60 ) );
    QString text = KIO::convertSeconds( seconds );
    QCOMPARE( text, i18n( "1 day 23:59:00" ) );

    seconds = KIO::calculateRemainingSeconds( 520, 20, 10 );
    QCOMPARE( seconds, static_cast<unsigned int>( 50 ) );
    text = KIO::convertSeconds( seconds );
    QCOMPARE( text, i18n( "00:00:50" ) );
}

void JobTest::copyFileToSystem()
{
    if ( !KProtocolInfo::isKnownProtocol( "system" ) ) {
        kDebug() << k_funcinfo << "no kio_system, skipping test" << endl;
        return;
    }

    // First test with support for UDS_LOCAL_PATH
    copyFileToSystem( true );

    QString dest = realSystemPath() + "fileFromHome_copied";
    QFile::remove( dest );

    // Then disable support for UDS_LOCAL_PATH, i.e. test what would
    // happen for ftp, smb, http etc.
    copyFileToSystem( false );
}

void JobTest::copyFileToSystem( bool resolve_local_urls )
{
    kDebug() << k_funcinfo << resolve_local_urls << endl;
    extern KIO_EXPORT bool kio_resolve_local_urls;
    kio_resolve_local_urls = resolve_local_urls;

    const QString src = homeTmpDir() + "fileFromHome";
    createTestFile( src );
    KUrl u;
    u.setPath( src );
    KUrl d = systemTmpDir();
    d.addPath( "fileFromHome_copied" );

    kDebug() << "copying " << u << " to " << d << endl;

    // copy the file with file_copy
    bool ok = KIO::NetAccess::file_copy( u, d );
    QVERIFY( ok );

    QString dest = realSystemPath() + "fileFromHome_copied";

    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // still there

    {
        // do NOT check that the timestamp is the same.
        // It can't work with file_copy when it uses the datapump,
        // unless we use setModificationTime in the app code.
    }

    // cleanup and retry with KIO::copy()
    QFile::remove( dest );
    ok = KIO::NetAccess::dircopy( u, d, 0 );
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // still there
    {
        // check that the timestamp is the same (#79937)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        QCOMPARE( srcInfo.lastModified(), destInfo.lastModified() );
    }

    // restore normal behavior
    kio_resolve_local_urls = true;
}

#include "jobtest.moc"
