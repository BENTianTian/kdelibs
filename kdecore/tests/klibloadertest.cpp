/* This file is part of the KDE libraries
    Copyright (c) 2005-2006 David Faure <faure@kde.org>

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

#include <qtest_kde.h>
#include "klibloadertest.h"
#include "klibloadertest.moc"

QTEST_KDEMAIN_CORE( KLibLoaderTest )

#include <klibloader.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <kdebug.h>

void KLibLoaderTest::initTestCase()
{
    const QString libdir = QDir::currentPath() + "/../../lib";
    KGlobal::dirs()->addResourceDir( "module", libdir );
    //qDebug( "initTestCase: added %s to 'module' resource", qPrintable(libdir) );
}

void KLibLoaderTest::testNonWorking()
{
    int error = 0;
    QObject* obj = KLibLoader::createInstance<QObject>( "idontexist", this, QStringList(), &error );
    QCOMPARE( obj, (QObject*)0 );
    QCOMPARE( error, (int)KLibLoader::ErrNoLibrary );
    QString errorString = KLibLoader::errorString( error );
    kDebug() << errorString << endl;
    QVERIFY( !errorString.isEmpty() );
}

// We need a module to dlopen, which uses a standard factory (e.g. not an ioslave)
static const char* s_module = "libklibloadertestmodule";

void KLibLoaderTest::testFindLibrary()
{
    const QString library = KLibLoader::findLibrary( s_module );
    QVERIFY( !library.isEmpty() );
    const QString libraryPath = QFileInfo( library ).canonicalFilePath();
#ifdef Q_OS_WIN
    const QString expectedPath = QFileInfo( QDir::currentPath() + "/../../lib/" + s_module + ".dll" ).canonicalFilePath();
#else
    const QString expectedPath = QFileInfo( QDir::currentPath() + "/../../lib/" + s_module + ".so" ).canonicalFilePath();
#endif
    QCOMPARE( library, expectedPath );
}

void KLibLoaderTest::testWorking()
{
    int error = 0;
    QObject* obj = KLibLoader::createInstance<QObject>( s_module, 0, QStringList(), &error );
    if ( error )
        kWarning() << "error=" << error << " lastErrorMessage=" << KLibLoader::self()->lastErrorMessage() << endl;
    QVERIFY( obj != 0 );
}

void KLibLoaderTest::testWorking4()
{
    int error = 0;
    QObject* obj = KLibLoader::createInstance<QObject>( "libklibloadertestmodule4", 0, QStringList(), &error );
    if ( error )
        kWarning() << "error=" << error << " lastErrorMessage=" << KLibLoader::self()->lastErrorMessage() << endl;
    QVERIFY( obj != 0 );
}

void KLibLoaderTest::testWrongClass()
{
    int error = 0;

    KLibLoaderTest* obj = KLibLoader::createInstance<KLibLoaderTest>( s_module, 0, QStringList(), &error );
    QCOMPARE( obj, (KLibLoaderTest*)0 );
    QCOMPARE( error, (int)KLibLoader::ErrNoComponent );
    QString errorString = KLibLoader::errorString( error );
    kDebug() << errorString << endl;
    QVERIFY( !errorString.isEmpty() );
}
