/* This file is part of the KDE project
   Copyright (C) 2006 David Faure <faure@kde.org>

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

// This file can only be included once in a given binary

#include <kde_file.h>
#ifdef Q_OS_UNIX
#include <utime.h>
#endif
#include <errno.h>

QDateTime s_referenceTimeStamp;

static void setTimeStamp( const QString& path, const QDateTime& mtime )
{
#ifdef Q_OS_UNIX
    // Put timestamp in the past so that we can check that the listing is correct
    struct utimbuf utbuf;
    utbuf.actime = mtime.toTime_t();
    utbuf.modtime = utbuf.actime;
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
    setTimeStamp( path, s_referenceTimeStamp );
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
    //qDebug( "symlink %s created", qPrintable( path ) );
    QVERIFY( QFileInfo( path ).isSymLink() );
}

static void createTestDirectory( const QString& path )
{
    QDir dir;
    bool ok = dir.mkdir( path );
    if ( !ok && !dir.exists() )
        kFatal() << "couldn't create " << path << endl;
    createTestFile( path + "/testfile" );
#ifndef Q_WS_WIN
    createTestSymlink( path + "/testlink" );
    QVERIFY( QFileInfo( path + "/testlink" ).isSymLink() );
#endif
    setTimeStamp( path, s_referenceTimeStamp );
}
