/*
 *  Copyright (C) 2005 David Faure   <faure@kde.org>
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
 */

#include <config.h>
#include <kdefakes.h>
#include "kmimetypetest.h"
#include "kmimetypetest.moc"
#include <kmimetype.h>
#include <ksycoca.h>
#include <kglobal.h>
#include <kuser.h>
#include <kstandarddirs.h>
#include <ktempdir.h>

#include <qtest_kde.h>
#include <kprotocolinfo.h>
#include <kmimetypetrader.h>
#include <kservicetypetrader.h>
#include <QtCore/Q_PID>
#include <kmimetypefactory.h>
#include <ktemporaryfile.h>
#include <kdesktopfile.h>

void KMimeTypeTest::initTestCase()
{
    // Create fake text/plain part with a higher initial preference than katepart.
    const QString fakePart = KStandardDirs::locateLocal("services", "faketextpart.desktop");
    bool mustUpdateKSycoca = false;
    const bool mustCreate = !QFile::exists(fakePart);
    if (mustCreate) {
        mustUpdateKSycoca = true;
        KDesktopFile file(fakePart);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "FakePart");
        group.writeEntry("Type", "Service");
        group.writeEntry("X-KDE-Library", "faketextpart");
        group.writeEntry("ServiceTypes", "KParts/ReadOnlyPart");
        group.writeEntry("MimeType", "text/plain");
        group.writeEntry("InitialPreference",100);
    }

    // Create fake text/plain ktexteditor plugin.
    const QString fakePlugin = KStandardDirs::locateLocal("services", "faketextplugin.desktop");
    const bool mustCreatePlugin = !QFile::exists(fakePlugin);
    if (mustCreatePlugin) {
        mustUpdateKSycoca = true;
        KDesktopFile file(fakePlugin);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "FakePlugin");
        group.writeEntry("Type", "Service");
        group.writeEntry("X-KDE-Library", "faketextplugin");
        group.writeEntry("ServiceTypes", "KTextEditor/Plugin");
        group.writeEntry("MimeType", "text/plain");
    }

    if ( mustUpdateKSycoca ) {
        // Update ksycoca in ~/.kde-unit-test after creating the above
        QProcess::execute( KGlobal::dirs()->findExe(KBUILDSYCOCA_EXENAME), QStringList() << "--noincremental" );
    }
}

static void checkIcon( const KUrl& url, const QString& expectedIcon )
{
    QString icon = KMimeType::iconNameForUrl( url );
    QCOMPARE( icon, expectedIcon );
}

QTEST_KDEMAIN_CORE( KMimeTypeTest )

void KMimeTypeTest::testByName()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KMimeType::Ptr s0 = KMimeType::mimeType("application/x-zerosize");
    QVERIFY( s0 );
    QCOMPARE( s0->name(), QString::fromLatin1("application/x-zerosize") );
    //qDebug("Comment is %s", qPrintable(s0->comment()) );

    KMimeType::Ptr s0Again = KMimeType::mimeType("application/x-zerosize");
    QCOMPARE(s0Again->name(), s0->name());
    QVERIFY(s0Again != s0);

    KMimeType::Ptr s1 = KMimeType::mimeType("text/plain");
    QVERIFY( s1 );
    QCOMPARE( s1->name(), QString::fromLatin1("text/plain") );
    //qDebug("Comment is %s", qPrintable(s1->comment()) );

    KMimeType::Ptr krita = KMimeType::mimeType("application/x-krita");
    QVERIFY( krita );
}

void KMimeTypeTest::testIcons()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    checkIcon( KUrl( "file:/tmp/" ), "inode-directory" );

    if ( !KUser().isSuperUser() ) // Can't test this one if running as root
    {
        QString emptyString; // gcc-3.3 workaround
        KTempDir tmp( emptyString, 0 );
        tmp.setAutoRemove( true );
        KUrl url( tmp.name() );
        checkIcon( url, "inode-directory" ); // was folder_locked, but we don't have that anymore - TODO
#ifndef Q_WS_WIN
        chmod( QFile::encodeName( tmp.name() ), 0500 ); // so we can 'rm -rf' it
#endif
    }
}

void KMimeTypeTest::testFindByPath()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KMimeType::Ptr mime;

    QString exePath = KStandardDirs::findExe( "kioexec" );
    if ( exePath.isEmpty() )
        QSKIP( "kioexec not found", SkipAll );

    mime = KMimeType::findByPath( exePath );
    QVERIFY( mime );

#ifdef Q_WS_WIN
    QCOMPARE( mime->name(), QString::fromLatin1( "application/x-ms-dos-executable" ) );
#else
    QCOMPARE( mime->name(), QString::fromLatin1( "application/x-executable" ) );
#endif

    mime = KMimeType::findByPath("textfile.txt");
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1( "text/plain" ) );

    mime = KMimeType::findByPath("textfile.TxT"); // case-insensitive search
    QCOMPARE( mime->name(), QString::fromLatin1( "text/plain" ) );

    mime = KMimeType::findByPath("textfile.C"); // case-sensitive uppercase match
    QCOMPARE( mime->name(), QString::fromLatin1( "text/x-c++src" ) );
    mime = KMimeType::findByPath("textfile.c"); // for comparison
    QCOMPARE( mime->name(), QString::fromLatin1( "text/x-csrc" ) );

    mime = KMimeType::findByPath("foo.desktop");
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1( "application/x-desktop" ) );
    mime = KMimeType::findByPath("foo.kdelnk");
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1( "application/x-desktop" ) );

    // Test a real PDF file.
    // If we find x-matlab because it starts with '%' then we are not ordering by priority.
    KTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString tempFileName = tempFile.fileName();
    tempFile.write("%PDF-");
    tempFile.close();
    mime = KMimeType::findByPath( tempFileName );
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1( "application/pdf" ) );

    // Can't use KIconLoader since this is a "without GUI" test.
    QString fh = KStandardDirs::locate( "icon", "oxygen/22x22/places/folder.png" );
    QVERIFY( !fh.isEmpty() ); // if the file doesn't exist, please fix the above to point to an existing icon
    mime = KMimeType::findByPath( fh );
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1( "image/png" ) );

    // Calling findByPath on a directory
    mime = KMimeType::findByPath("/");
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("inode/directory") );

    // Test a file that doesn't exist
    mime = KMimeType::findByPath("/IDontExist");
    QVERIFY( mime );
    QCOMPARE(mime->name(), QString::fromLatin1("application/octet-stream"));

    // Test a file that doesn't exist but that has a known extension
    mime = KMimeType::findByPath("/IDontExist.txt");
    QVERIFY( mime );
    QCOMPARE(mime->name(), QString::fromLatin1("text/plain"));
}

void KMimeTypeTest::testFindByUrl()
{
    // Tests with local files are already done in testFindByPath,
    // here we test for remote urls only.
    KMimeType::Ptr mime;
    QVERIFY( KProtocolInfo::isKnownProtocol(KUrl("http:/")) );
    QVERIFY( KProtocolInfo::isKnownProtocol(KUrl("file:/")) );
    mime = KMimeType::findByUrl( KUrl("http://foo/bar.png") );
    QVERIFY( mime );

    QCOMPARE( mime->name(), QString::fromLatin1( "application/octet-stream" ) ); // HTTP can't know before downloading

    if ( !KProtocolInfo::isKnownProtocol(KUrl("man:/")) )
        QSKIP( "man protocol not installed", SkipSingle );

    mime = KMimeType::findByUrl( KUrl("man:/ls") );
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/html") );

    mime = KMimeType::findByUrl( KUrl("man:/ls/") );
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/html") );
}

void KMimeTypeTest::testFindByNameAndContent()
{
    KMimeType::Ptr mime;

    QByteArray textData = "Hello world";
    // textfile -> text/plain. No extension -> mimetype is found from the contents.
    mime = KMimeType::findByNameAndContent("textfile", textData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/plain") );

    // textfile.foo -> text/plain. Unknown extension -> mimetype is found from the contents.
    mime = KMimeType::findByNameAndContent("textfile.foo", textData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/plain") );

    // textfile.doc -> text/plain. We don't trust the .doc extension, because of this case.
    mime = KMimeType::findByNameAndContent("textfile.doc", textData);
    QVERIFY( mime );
    // Well, Thomas Leonard (freedesktop.org) doesn't agree that this matters,
    // so currently the xdg mime database has application/msword:*.doc
    //QCOMPARE( mime->name(), QString::fromLatin1("text/plain") );
    QCOMPARE( mime->name(), QString::fromLatin1("application/msword") );

    // mswordfile.doc -> application/msword. Found by contents, because of the above case.
    // Note that it's application/msword, not application/vnd.ms-word, since it's the former that is registered to IANA.
    QByteArray mswordData = "\320\317\021\340\241\261";
    mime = KMimeType::findByNameAndContent("mswordfile.doc", mswordData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/msword") );

    // excelfile.xls -> application/vnd.ms-excel. Found by extension.
    mime = KMimeType::findByNameAndContent("excelfile.xls", mswordData /*same magic*/);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/vnd.ms-excel") );

    // textfile.xls -> application/vnd.ms-excel. Found by extension. User shouldn't rename a text file to .xls ;)
    mime = KMimeType::findByNameAndContent("textfile.xls", textData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/vnd.ms-excel") );

#if 0   // needs shared-mime-info >= 0.20
    QByteArray tnefData = "\x78\x9f\x3e\x22";
    mime = KMimeType::findByNameAndContent("tneffile", mswordData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/vnd.ms-tnef") );
#endif

    QByteArray pdfData = "%PDF-";
    mime = KMimeType::findByNameAndContent("foo", pdfData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/pdf") );

    // High-priority rule (80)
    QByteArray phpData = "<?php";
    mime = KMimeType::findByNameAndContent("foo", phpData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/x-php") );
}

void KMimeTypeTest::testFindByContent()
{
    KMimeType::Ptr mime;

    QByteArray textData = "Hello world";
    mime = KMimeType::findByContent(textData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/plain") );

#if 0 // https://bugs.freedesktop.org/show_bug.cgi?id=11259
    QByteArray htmlData = "<script>foo</script>";
    mime = KMimeType::findByContent(htmlData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/html") );
#endif

    QByteArray pdfData = "%PDF-";
    mime = KMimeType::findByContent(pdfData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/pdf") );

    // Calling findByContent on a directory
    mime = KMimeType::findByFileContent("/");
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("inode/directory") );
}

void KMimeTypeTest::testAllMimeTypes()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    const KMimeType::List lst = KMimeType::allMimeTypes();
    QVERIFY( !lst.isEmpty() );

    for ( KMimeType::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KMimeType::Ptr mime = (*it);
        const QString name = mime->name();
        //qDebug( "%s", qPrintable( name ) );
        QVERIFY( !name.isEmpty() );
        QCOMPARE( name.count( '/' ), 1 );
        QVERIFY( mime->isType( KST_KMimeType ) );

        const KMimeType::Ptr lookedupMime = KMimeType::mimeType( name );
        QVERIFY( lookedupMime ); // not null
        QCOMPARE( lookedupMime->name(), name );
    }
}

void KMimeTypeTest::testAlias()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    const KMimeType::Ptr canonical = KMimeType::mimeType( "application/xml" );
    QVERIFY( canonical );
    KMimeType::Ptr alias = KMimeType::mimeType( "text/xml" );
    QVERIFY( !alias );
    alias = KMimeType::mimeType( "text/xml", KMimeType::ResolveAliases );
    QVERIFY( alias );
    QCOMPARE( alias->name(), QString("application/xml") );

    QVERIFY(alias->is("application/xml"));
    QVERIFY(canonical->is("text/xml"));
}

void KMimeTypeTest::testMimeTypeParent()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // All file-like mimetypes inherit from octet-stream
    const KMimeType::Ptr msword = KMimeType::mimeType("application/msword");
    QVERIFY(msword);
    QCOMPARE(msword->parentMimeType(), QString("application/octet-stream"));
    QVERIFY(msword->is("application/octet-stream"));

    const KMimeType::Ptr directory = KMimeType::mimeType("inode/directory");
    QVERIFY(directory);
    QCOMPARE(directory->parentMimeType(), QString());
    QVERIFY(!directory->is("application/octet-stream"));

    // Check that text/x-patch knows that it inherits from text/plain (it says so explicitely)
    const KMimeType::Ptr plain = KMimeType::mimeType( "text/plain" );
    const KMimeType::Ptr derived = KMimeType::mimeType( "text/x-patch" );
    QVERIFY( derived );
    QCOMPARE( derived->parentMimeType(), plain->name() );
    QVERIFY( derived->is("text/plain") );
    QVERIFY( derived->is("application/octet-stream") );

    // Check that text/mrml knows that it inherits from text/plain (implicitly)
    const KMimeType::Ptr mrml = KMimeType::mimeType("text/mrml");
    if (!mrml)
        QSKIP("kdelibs not installed", SkipAll);
    QVERIFY(mrml->is("text/plain"));
    QVERIFY(mrml->is("application/octet-stream"));
}

// Helper method for all the trader tests
static bool offerListHasService( const KService::List& offers,
                                 const QString& entryPath )
{
    bool found = false;
    KService::List::const_iterator it = offers.begin();
    for ( ; it != offers.end() ; it++ )
    {
        if ( (*it)->entryPath() == entryPath ) {
            if( found ) { // should be there only once
                qWarning( "ERROR: %s was found twice in the list", qPrintable( entryPath ) );
                return false; // make test fail
            }
            found = true;
        }
    }
    return found;
}

void KMimeTypeTest::testMimeTypeTraderForTextPlain()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying mimetype trader for services associated with text/plain
    KService::List offers = KMimeTypeTrader::self()->query("text/plain", "KParts/ReadOnlyPart");
    QVERIFY( offerListHasService( offers, "katepart.desktop" ) );
    QVERIFY( offerListHasService( offers, "faketextpart.desktop" ) );

    offers = KMimeTypeTrader::self()->query("text/plain", "KTextEditor/Plugin");
    QVERIFY( offers.count() > 0 );

    // We should have at least the fake text plugin that we created for this.
    // (The actual plugins from kdelibs don't mention text/plain anymore)
    QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );

    // We shouldn't have non-plugins
    QVERIFY( !offerListHasService( offers, "katepart.desktop" ) );

}

void KMimeTypeTest::testMimeTypeTraderForDerivedMimeType()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying mimetype trader for services associated with text/x-patch, which inherits from text/plain
    KService::List offers = KMimeTypeTrader::self()->query("text/x-patch", "KParts/ReadOnlyPart");
    QVERIFY( offerListHasService( offers, "katepart.desktop" ) );
    QVERIFY( offerListHasService( offers, "faketextpart.desktop" ) );
    QVERIFY( (*offers.begin())->entryPath() != "faketextpart.desktop" ); // in the list, but not preferred

    offers = KMimeTypeTrader::self()->query("text/x-patch", "KTextEditor/Plugin");
    QVERIFY( offers.count() > 0 );

    // We should have at least the fake text plugin that we created for this.
    // (The actual plugins from kdelibs don't mention text/plain anymore)
    QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );

    offers = KMimeTypeTrader::self()->query("text/x-patch", "Application");
    QVERIFY( !offerListHasService( offers, "katepart.desktop" ) );
}


void KMimeTypeTest::testMimeTypeTraderForAlias()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    const KService::List referenceOffers = KMimeTypeTrader::self()->query("application/xml", "KParts/ReadOnlyPart");
    QVERIFY( offerListHasService( referenceOffers, "katepart.desktop" ) );

    // Querying mimetype trader for services associated with text/xml, which is an alias for application/xml
    const KService::List offers = KMimeTypeTrader::self()->query("text/xml", "KParts/ReadOnlyPart");
    QVERIFY( offerListHasService( offers, "katepart.desktop" ) );

    QCOMPARE(offers.count(), referenceOffers.count());
}

void KMimeTypeTest::testHasServiceType1() // with services constructed with a full path (rare)
{
    QString katepartPath = KStandardDirs::locate( "services", "katepart.desktop" );
    QVERIFY( !katepartPath.isEmpty() );
    KService katepart( katepartPath );
    QVERIFY( katepart.hasMimeType( KMimeType::mimeType( "text/plain" ).data() ) );
    //QVERIFY( katepart.hasMimeType( KMimeType::mimeType( "text/x-patch" ).data() ) ); // inherited mimetype; fails
    QVERIFY( !katepart.hasMimeType( KMimeType::mimeType( "image/png" ).data() ) );
    QVERIFY( katepart.hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( katepart.hasServiceType( "KParts/ReadWritePart" ) );
    QVERIFY( !katepart.hasServiceType( "KTextEditor/Plugin" ) );

    QString ktexteditor_insertfilePath = KStandardDirs::locate( "services", "ktexteditor_insertfile.desktop" );
    QVERIFY( !ktexteditor_insertfilePath.isEmpty() );
    KService ktexteditor_insertfile( ktexteditor_insertfilePath );
    QVERIFY( ktexteditor_insertfile.hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !ktexteditor_insertfile.hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KMimeTypeTest::testHasServiceType2() // with services coming from ksycoca
{
    KService::Ptr katepart = KService::serviceByDesktopPath( "katepart.desktop" );
    QVERIFY( !katepart.isNull() );
    QVERIFY( katepart->hasMimeType( KMimeType::mimeType( "text/plain" ).data() ) );
    QVERIFY( katepart->hasMimeType( KMimeType::mimeType( "text/x-patch" ).data() ) ); // due to inheritance
    QVERIFY( !katepart->hasMimeType( KMimeType::mimeType( "image/png" ).data() ) );
    QVERIFY( katepart->hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( katepart->hasServiceType( "KParts/ReadWritePart" ) );
    QVERIFY( !katepart->hasServiceType( "KTextEditor/Plugin" ) );

    KService::Ptr ktexteditor_insertfile = KService::serviceByDesktopPath( "ktexteditor_insertfile.desktop" );
    QVERIFY( !ktexteditor_insertfile.isNull() );
    QVERIFY( ktexteditor_insertfile->hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !ktexteditor_insertfile->hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KMimeTypeTest::testPatterns()
{
    // mimetype with a single pattern
    KMimeType::Ptr mime_pdf = KMimeType::mimeType( "application/pdf" );
    QVERIFY( mime_pdf );
    QCOMPARE( mime_pdf->patterns().join(","), QString("*.pdf") );
    // mimetype with more than one pattern
    KMimeType::Ptr mime_kpresenter = KMimeType::mimeType( "application/x-kpresenter" );
    QVERIFY( mime_kpresenter );
    QCOMPARE( mime_kpresenter->patterns().join(","), QString("*.kpr,*.kpt") );
    // mimetype with a no patterns
    KMimeType::Ptr mime_pkcs7 = KMimeType::mimeType( "application/pkcs7-mime" );
    QVERIFY( mime_pkcs7 );
    QCOMPARE( mime_pkcs7->patterns().join(","), QString() );
}

void KMimeTypeTest::testExtractKnownExtension()
{
    const QString pdf = KMimeType::extractKnownExtension("foo.pdf");
    QCOMPARE(pdf, QString("pdf"));
    const QString kpt = KMimeType::extractKnownExtension("kpresenter.foo.kpt");
    QCOMPARE(kpt, QString("kpt"));
    const QString tarbz2 = KMimeType::extractKnownExtension("foo.tar.bz2");
    QCOMPARE(tarbz2, QString("tar.bz2"));
}

void KMimeTypeTest::testParseMagicFile_data()
{
    //kDebug() ;
    // This magic data is fake; just a way to test various features.
    static const char s_magicData[] = "MIME-Magic\0\n"
                                      "[50:application/x-desktop]\n"
                                      ">0=\0\017[Desktop Entry]+11\n"
                                      ">2=\0\002[A&\xff\001\n" // '[' and then any non-even value
                                      "1>4=\0\001]\n"
                                      "[40:application/vnd.ms-tnef]\n"
                                      ">0=\0\004\x78\x9f\x3e\x22\n" // tnef magic, in "ready to use" form
                                      // https://bugs.freedesktop.org/show_bug.cgi?id=435
                                      // <match value="0x1234" type="host16" offset="0"/>
                                      "[30:text/x-test-mime-host16]\n"
                                      ">0=\0\002\x12\x34~2\n"
                                      // <match value="0x1278" type="big16" offset="0"/>
                                      "[30:text/x-test-mime-big16]\n"
                                      ">0=\0\002\x12\x78\n"
                                      // <match value="0x5678" type="little16" offset="0"/>
                                      "[30:text/x-test-mime-little16]\n"
                                      ">0=\0\002\x78\x55\n"
                                      // <match value="0x12345678" type="host32" offset="0"/>
                                      "[30:text/x-test-mime-host32]\n"
                                      ">0=\0\004\x12\x34\x56\x78~4\n"
                                      "[50:application/vnd.ms-powerpoint]\n"
                                      // fixed ppt magic, see https://bugs.freedesktop.org/show_bug.cgi?id=435
                                      ">0=\0\004\xd0\xcf\x11\xe0\n";

    // Do this first, to avoid "no test data available" in case of a mistake here
    QTest::addColumn<QString>("testData");
    QTest::addColumn<QString>("expected");

    QTest::newRow("First rule, no offset") << "[Desktop Entry]" << "application/x-desktop";
    QTest::newRow("First rule, with offset") << "# Comment\n[Desktop Entry]" << "application/x-desktop";
    QTest::newRow("Missing char") << "# Comment\n[Desktop Entry" << QString();
    QTest::newRow("Second rule, two-level match") << "AB[C]" << "application/x-desktop";
    QTest::newRow("Second rule, failure at first level (mask)") << "AB[B]" << QString();
    QTest::newRow("Second rule, failure at second level") << "AB[CN" << QString();
    QTest::newRow("Tnef magic, should pass") << "\x78\x9f\x3e\x22" << "application/vnd.ms-tnef";
    QTest::newRow("Tnef magic, should fail") << "\x22\x3e\x9f\x78" << QString();
    QTest::newRow("Powerpoint rule, for endianness check, should pass") << "\xd0\xcf\x11\xe0" << "application/vnd.ms-powerpoint";
    QTest::newRow("Powerpoint rule, no swapping, should fail") << "\x11\xe0\xd0\xcf" << QString();
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    QTest::newRow("Test mime, host16, wrong endianness") << "\x12\x34" << QString();
    QTest::newRow("Test mime, host16, little endian, ok") << "\x34\x12" << "text/x-test-mime-host16";
    QTest::newRow("Test mime, host32, wrong endianness") << "\x12\x34\x56\x78" << QString();
    QTest::newRow("Test mime, host32, little endian, ok") << "\x78\x56\x34\x12" << "text/x-test-mime-host32";
#else
    QTest::newRow("Test mime, host16, big endian, ok") << "\x12\x34" << "text/x-test-mime-host16";
    QTest::newRow("Test mime, host16, wrong endianness") << "\x34\x12" << QString();
    QTest::newRow("Test mime, host32, big endian, ok") << "\x12\x34\x56\x78" << "text/x-test-mime-host32";
    QTest::newRow("Test mime, host32, wrong endianness") << "\x78\x56\x34\x12" << QString();
#endif
    QTest::newRow("Test mime, little16, little endian, ok") << "\x78\x55" << "text/x-test-mime-little16";
    QTest::newRow("Test mime, little16, wrong endianness") << "\x55\x78" << QString();
    QTest::newRow("Test mime, big16, little endian, ok") << "\x12\x78" << "text/x-test-mime-big16";
    QTest::newRow("Test mime, big16, wrong endianness") << "\x78\x12" << QString();

    QByteArray magicData;
    magicData.resize(sizeof(s_magicData) - 1 /*trailing nul*/);
    memcpy(magicData.data(), s_magicData, magicData.size());

    QBuffer magicBuffer(&magicData);
    magicBuffer.open(QIODevice::ReadOnly);
    m_rules = KMimeTypeFactory::self()->parseMagicFile(&magicBuffer, "magicData");
    QCOMPARE(m_rules.count(), 7);

    const KMimeMagicRule rule = m_rules[0];
    QCOMPARE(rule.mimetype(), QString("application/x-desktop"));
    QCOMPARE(rule.priority(), 50);
    const QList<KMimeMagicMatch>& matches = rule.matches();
    QCOMPARE(matches.count(), 2);
    const KMimeMagicMatch& match0 = matches[0];
    QCOMPARE((int)match0.m_rangeStart, 0);
    QCOMPARE((int)match0.m_rangeLength, 11);
    QCOMPARE(match0.m_data, QByteArray("[Desktop Entry]"));
    QCOMPARE(match0.m_subMatches.count(), 0);
    const KMimeMagicMatch& match1 = matches[1];
    QCOMPARE((int)match1.m_rangeStart, 2);
    QCOMPARE((int)match1.m_rangeLength, 1);
    QCOMPARE(match1.m_data, QByteArray("[A"));
    QCOMPARE(match1.m_data.size(), match1.m_mask.size());
    QCOMPARE(match1.m_subMatches.count(), 1);
    const KMimeMagicMatch& submatch1 = match1.m_subMatches[0];
    QCOMPARE((int)submatch1.m_rangeStart, 4);
    QCOMPARE((int)submatch1.m_rangeLength, 1);
    QCOMPARE(submatch1.m_data, QByteArray("]"));
    QCOMPARE(submatch1.m_subMatches.count(), 0);
}

void KMimeTypeTest::testParseMagicFile()
{
    QFETCH(QString, testData);
    //kDebug() << QTest::currentDataTag();
    QFETCH(QString, expected);
    QBuffer testBuffer;
    testBuffer.setData(testData.toLatin1());
    QVERIFY(testBuffer.open(QIODevice::ReadOnly));
    QString found;
    for ( QList<KMimeMagicRule>::const_iterator it = m_rules.begin(), end = m_rules.end();
          it != end; ++it ) {
        const KMimeMagicRule& rule = *it;
        if (rule.match(&testBuffer, QByteArray())) {
            found = rule.mimetype();
            break;
        }
    }
    QCOMPARE(found, expected);
    testBuffer.close();
}


// TODO tests that involve writing a profilerc and checking that the trader is obeying it
