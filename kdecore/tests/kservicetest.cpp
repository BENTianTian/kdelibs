/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
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

#include "kservicetest.h"
#include "kservicetest.moc"
#include <ksycoca.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qtest_kde.h>
#include <kprotocolinfo.h>
#include <kservicetypetrader.h>
#include <kservicetype.h>
#include <kservicetypeprofile.h>

#include <QtCore/Q_PID>

void KServiceTest::initTestCase()
{
    QString profilerc = KStandardDirs::locateLocal( "config", "profilerc" );
    if ( !profilerc.isEmpty() )
        QFile::remove( profilerc );

    profilerc = KStandardDirs::locateLocal( "config", "servicetype_profilerc" );
    if ( !profilerc.isEmpty() )
        QFile::remove( profilerc );
}

QTEST_KDEMAIN_CORE( KServiceTest )

void KServiceTest::testByName()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KServiceType::Ptr s0 = KServiceType::serviceType("KParts/ReadOnlyPart");
    QVERIFY( s0 );
    QCOMPARE( s0->name(), QString::fromLatin1("KParts/ReadOnlyPart") );

    KService::Ptr kdeprintd = KService::serviceByDesktopPath("kded/kdeprintd.desktop");
    QCOMPARE( kdeprintd->name(), QString::fromLatin1("KDE Print Daemon"));
}


void KServiceTest::testProperty()
{
    KService::Ptr kdeprintd = KService::serviceByDesktopPath("kded/kdeprintd.desktop");
    QVERIFY(kdeprintd);
    QCOMPARE(kdeprintd->entryPath(), QString("kded/kdeprintd.desktop"));

    QCOMPARE(kdeprintd->property("ServiceTypes").toStringList().join(","), QString("KDEDModule"));
    QCOMPARE(kdeprintd->property("X-KDE-Kded-autoload").toBool(), false);
    QCOMPARE(kdeprintd->property("X-KDE-Kded-load-on-demand").toBool(), true);

    KService::Ptr kjavaappletviewer = KService::serviceByDesktopPath("kjavaappletviewer.desktop");
    QVERIFY(kjavaappletviewer);
    QCOMPARE(kjavaappletviewer->property("X-KDE-BrowserView-PluginsInfo").toString(), QString("kjava/pluginsinfo"));

    // Test property("MimeTypes"), which triggers the KServiceReadProperty code.
    // Didn't find any desktop file in kdelibs that had a stringlist property, so this one needs kdebase/workspace installed.
    KService::Ptr fontthumbnail = KService::serviceByDesktopPath("fontthumbnail.desktop");
    if (fontthumbnail) {
        QVERIFY(fontthumbnail->property("MimeTypes").toStringList().contains("application/x-font-ttf"));
    } else {
        qDebug("Skipping property(\"MimeTypes\") test, fontthumbnail.desktop not found (kdebase/workspace not installed)");
    }
}

void KServiceTest::testAllServiceTypes()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    const KServiceType::List allServiceTypes = KServiceType::allServiceTypes();

    // A bit of checking on the allServiceTypes list itself
    KServiceType::List::ConstIterator stit = allServiceTypes.begin();
    const KServiceType::List::ConstIterator stend = allServiceTypes.end();
    for ( ; stit != stend; ++stit ) {
        const KServiceType::Ptr servtype = (*stit);
        const QString name = servtype->name();
        QVERIFY( !name.isEmpty() );
        QVERIFY( servtype->sycocaType() == KST_KServiceType );
    }
}

void KServiceTest::testAllServices()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );
    const KService::List lst = KService::allServices();
    QVERIFY( !lst.isEmpty() );

    for ( KService::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KService::Ptr service = (*it);
        QVERIFY( service->isType( KST_KService ) );

        const QString name = service->name();
        const QString dep = service->entryPath();
        //qDebug( "%s %s", qPrintable( name ), qPrintable( dep ) );
        QVERIFY( !name.isEmpty() );
        QVERIFY( !dep.isEmpty() );

        KService::Ptr lookedupService = KService::serviceByDesktopPath( dep );
        QVERIFY( lookedupService ); // not null
        QCOMPARE( lookedupService->entryPath(), dep );

        if ( service->isApplication() )
        {
            const QString menuId = service->menuId();
            if ( menuId.isEmpty() )
                qWarning( "%s has an empty menuId!", qPrintable( dep ) );
            QVERIFY( !menuId.isEmpty() );
            lookedupService = KService::serviceByMenuId( menuId );
            QVERIFY( lookedupService ); // not null
            QCOMPARE( lookedupService->menuId(), menuId );

            if (service->exec().isEmpty())
                qWarning( "%s has an empty exec!", qPrintable( dep ) );
            QVERIFY(!service->exec().isEmpty());
        }
    }
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

void KServiceTest::testDBUSStartupType()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );
    KService::Ptr konsole = KService::serviceByDesktopName( "konsole" );
    if ( !konsole )
        QSKIP( "konsole.desktop not found", SkipAll );
    //qDebug() << konsole->entryPath();
    QCOMPARE((int)konsole->DBUSStartupType(), (int)KService::DBUS_Unique);
}

void KServiceTest::testServiceTypeTraderForReadOnlyPart()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying trader for services associated with KParts/ReadOnlyPart
    KService::List offers = KServiceTypeTrader::self()->query("KParts/ReadOnlyPart");
    QVERIFY( offers.count() > 0 );
    //foreach( KService::Ptr service, offers )
    //    qDebug( "%s %s", qPrintable( service->name() ), qPrintable( service->entryPath() ) );

    m_firstOffer = offers[0]->entryPath();

    // Only test for parts provided by kdelibs:
    QVERIFY( offerListHasService( offers, "katepart.desktop" ) );
    QVERIFY( offerListHasService( offers, "kmultipart.desktop" ) );
    QVERIFY( offerListHasService( offers, "khtml.desktop" ) );
    QVERIFY( offerListHasService( offers, "khtmlimage.desktop" ) );
    QVERIFY( offerListHasService( offers, "kjavaappletviewer.desktop" ) );

    // Check ordering according to InitialPreference
    int lastPreference = -1;
    bool lastAllowedAsDefault = true;
    KService::List::const_iterator it = offers.begin();
    for ( ; it != offers.end() ; it++ ) {
        const QString path = (*it)->entryPath();
        const int preference = (*it)->initialPreference(); // ## might be wrong if we use per-servicetype preferences...
        qDebug( "%s has preference %d, allowAsDefault=%d", qPrintable( path ), preference, (*it)->allowAsDefault() );
        if ( lastAllowedAsDefault && !(*it)->allowAsDefault() ) {
            // first "not allowed as default" offer
            lastAllowedAsDefault = false;
            lastPreference = -1; // restart
        }
        if ( lastPreference != -1 )
            QVERIFY( preference <= lastPreference );
        lastPreference = preference;
    }

    // Now look for any KTextEditor/Plugin
    offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin");
    QVERIFY( offerListHasService( offers, "ktexteditor_docwordcompletion.desktop" ) );
    QVERIFY( offerListHasService( offers, "ktexteditor_insertfile.desktop" ) );
}

void KServiceTest::testTraderConstraints()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KService::List offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin", "Library == 'ktexteditor_docwordcompletion'");
    QCOMPARE(offers.count(), 1);
    QVERIFY( offerListHasService( offers, "ktexteditor_docwordcompletion.desktop" ) );
}

void KServiceTest::testHasServiceType1() // with services constructed with a full path (rare)
{
    QString katepartPath = KStandardDirs::locate( "services", "katepart.desktop" );
    QVERIFY( !katepartPath.isEmpty() );
    KService katepart( katepartPath );
    QVERIFY( katepart.hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( katepart.hasServiceType( "KParts/ReadWritePart" ) );

    QString ktexteditor_docwordcompletionPath = KStandardDirs::locate( "services", "ktexteditor_docwordcompletion.desktop" );
    QVERIFY( !ktexteditor_docwordcompletionPath.isEmpty() );
    KService ktexteditor_docwordcompletion( ktexteditor_docwordcompletionPath );
    QVERIFY( ktexteditor_docwordcompletion.hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !ktexteditor_docwordcompletion.hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KServiceTest::testHasServiceType2() // with services coming from ksycoca
{
    KService::Ptr katepart = KService::serviceByDesktopPath( "katepart.desktop" );
    QVERIFY( !katepart.isNull() );
    QVERIFY( katepart->hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( katepart->hasServiceType( "KParts/ReadWritePart" ) );

    KService::Ptr ktexteditor_docwordcompletion = KService::serviceByDesktopPath( "ktexteditor_docwordcompletion.desktop" );
    QVERIFY( !ktexteditor_docwordcompletion.isNull() );
    QVERIFY( ktexteditor_docwordcompletion->hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !ktexteditor_docwordcompletion->hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KServiceTest::testWriteServiceTypeProfile()
{
    const QString serviceType = "KParts/ReadOnlyPart";
    KService::List services, disabledServices;
    services.append(KService::serviceByDesktopPath("khtmlimage.desktop"));
    services.append(KService::serviceByDesktopPath("katepart.desktop"));
    disabledServices.append(KService::serviceByDesktopPath("khtml.desktop"));

    KServiceTypeProfile::writeServiceTypeProfile( serviceType, services, disabledServices );

    // Check that the file got written
    QString profilerc = KStandardDirs::locateLocal( "config", "servicetype_profilerc" );
    QVERIFY(!profilerc.isEmpty());
    QVERIFY(QFile::exists(profilerc));

    KService::List offers = KServiceTypeTrader::self()->query( serviceType );
    QVERIFY( offers.count() > 0 ); // not empty

    //foreach( KService::Ptr service, offers )
    //    qDebug( "%s %s", qPrintable( service->name() ), qPrintable( service->entryPath() ) );

    QVERIFY( offers.count() >= 3 ); // at least 3, even
    QCOMPARE( offers[0]->entryPath(), QString("khtmlimage.desktop") );
    QCOMPARE( offers[1]->entryPath(), QString("katepart.desktop") );
    QVERIFY( offerListHasService( offers, "kmultipart.desktop" ) ); // should still be somewhere in there
    QVERIFY( !offerListHasService( offers, "khtml.desktop" ) ); // it got disabled above
}

void KServiceTest::testDefaultOffers()
{
    // Now that we have a user-profile, let's see if defaultOffers indeed gives us the default ordering.
    const QString serviceType = "KParts/ReadOnlyPart";
    KService::List offers = KServiceTypeTrader::self()->defaultOffers( serviceType );
    QVERIFY( offers.count() > 0 ); // not empty
    QVERIFY( offerListHasService( offers, "khtml.desktop" ) ); // it's here even though it's disabled in the profile
    if ( m_firstOffer.isEmpty() )
        QSKIP( "testServiceTypeTraderForReadOnlyPart not run", SkipAll );
    QCOMPARE( offers[0]->entryPath(), m_firstOffer );
}

void KServiceTest::testDeleteServiceTypeProfile()
{
    const QString serviceType = "KParts/ReadOnlyPart";
    KServiceTypeProfile::deleteServiceTypeProfile( serviceType );

    KService::List offers = KServiceTypeTrader::self()->query( serviceType );
    QVERIFY( offers.count() > 0 ); // not empty
    QVERIFY( offerListHasService( offers, "khtml.desktop" ) ); // it's back

    if ( m_firstOffer.isEmpty() )
        QSKIP( "testServiceTypeTraderForReadOnlyPart not run", SkipAll );
    QCOMPARE( offers[0]->entryPath(), m_firstOffer );
}
