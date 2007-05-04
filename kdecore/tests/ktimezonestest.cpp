/* This file is part of the KDE libraries
    Copyright (c) 2005,2006 David Jarvie <software@astrojar.org.uk>

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

#include <stdio.h>
#include <qtest_kde.h>
#include <QtCore/QDir>
#include <QtCore/QDate>
#include "ksystemtimezone.h"
#include "ktzfiletimezone.h"
#include "ktimezonestest.moc"


QTEST_KDEMAIN_CORE(KTimeZonesTest)

void KTimeZonesTest::initTestCase()
{
    cleanupTestCase();

    mDataDir = QDir::homePath() + "/.kde-unit-test/ktimezonestest";
    QVERIFY(QDir().mkpath(mDataDir));
    QFile f;
    f.setFileName(mDataDir + QLatin1String("/zone.tab"));
    f.open(QIODevice::WriteOnly);
    QTextStream fStream(&f);
    fStream << "EG	+3003+03115	Africa/Cairo\n"
               "FR	+4852+00220	Europe/Paris\n"
               "GB	+512830-0001845	Europe/London	Great Britain\n"
               "US	+340308-1181434	America/Los_Angeles	Pacific Time\n";
    f.close();
    QDir dir(mDataDir);
    QVERIFY(dir.mkdir("Africa"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Cairo"), mDataDir + QLatin1String("/Africa/Cairo"));
    QVERIFY(dir.mkdir("America"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Los_Angeles"), mDataDir + QLatin1String("/America/Los_Angeles"));
    QVERIFY(dir.mkdir("Europe"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/London"), mDataDir + QLatin1String("/Europe/London"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Paris"), mDataDir + QLatin1String("/Europe/Paris"));

    KConfig config("ktimezonedrc");
    KConfigGroup group(&config, "TimeZones");
    group.writeEntry("ZoneinfoDir", mDataDir);
    group.writeEntry("Zonetab", mDataDir + QString::fromLatin1("/zone.tab"));
    group.writeEntry("LocalZone", QString::fromLatin1("Europe/Paris"));
    config.sync();
}

void KTimeZonesTest::cleanupTestCase()
{
    removeDir(QLatin1String("ktimezonestest/Africa"));
    removeDir(QLatin1String("ktimezonestest/America"));
    removeDir(QLatin1String("ktimezonestest/Europe"));
    removeDir(QLatin1String("ktimezonestest"));
    removeDir(QLatin1String("share/config"));
    QDir().rmpath(QDir::homePath() + "/.kde-unit-test/share");
}

void KTimeZonesTest::removeDir(const QString &subdir)
{
    QDir local = QDir::homePath() + QLatin1String("/.kde-unit-test/") + subdir;
    foreach(const QString &file, local.entryList(QDir::Files))
        if(!local.remove(file))
            qWarning("%s: removing failed", qPrintable( file ));
    QCOMPARE((int)local.entryList(QDir::Files).count(), 0);
    local.cdUp();
    QString subd = subdir;
    subd.remove(QRegExp("^.*/"));
    local.rmpath(subd);
}


///////////////////
// KTimeZones tests
///////////////////

void KTimeZonesTest::ktimezones()
{
    KTimeZones timezones;
    KTimeZone *zone1 = new KTimeZone("Zone1");
    KTimeZone *zone2 = new KTimeZone("Zone2");
    QVERIFY(timezones.add(zone1));
    QVERIFY(!timezones.add(zone1));
    QVERIFY(timezones.add(zone2));
    QCOMPARE(timezones.zones().count(), 2);
    const KTimeZone* tz = timezones.zone("Zone1");
    QVERIFY((tz == zone1));
    tz = timezones.zone("Zone99");
    QVERIFY(!tz);
    zone1 = const_cast<KTimeZone*>(timezones.detach(zone1));
    QVERIFY((bool)zone1);
    QCOMPARE(timezones.zones().count(), 1);
    QVERIFY(!timezones.detach(zone1));
    QVERIFY(timezones.add(zone1));
    QVERIFY((bool)timezones.detach("Zone1"));
    QVERIFY(!timezones.detach("Zone1"));
    QVERIFY((bool)timezones.detach("Zone2"));
    zone1 = new KTimeZone("Zone10");
    QVERIFY(timezones.add(zone1));
    QCOMPARE(timezones.zones().count(), 1);
    timezones.clear();
    QCOMPARE(timezones.zones().count(), 0);
}

///////////////////
// KTimeZones: UTC
///////////////////

void KTimeZonesTest::utc()
{
    const KTimeZone* utc = KTimeZones::utc();
    QVERIFY((bool)utc);
    QCOMPARE(utc->name(), QString("UTC"));
    QCOMPARE(utc->offsetAtUtc(QDateTime(QDate(2005,1,1), QTime(), Qt::LocalTime)), 0);
    QCOMPARE(utc->offsetAtUtc(QDateTime(QDate(2005,1,1), QTime(), Qt::UTC)), 0);
    QCOMPARE(utc->offsetAtUtc(QDateTime(QDate(2005,7,1), QTime(), Qt::UTC)), 0);
}

/////////////////////////
// KSystemTimeZones tests
/////////////////////////

void KTimeZonesTest::local()
{
    const KTimeZone *local = KSystemTimeZones::local();
    QVERIFY((bool)local);
    QCOMPARE(local->name(), QString::fromLatin1("Europe/Paris"));
}

void KTimeZonesTest::zone()
{
    const KTimeZone *utc = KSystemTimeZones::zone("UTC");
    QVERIFY(utc != 0);
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles != 0);
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london != 0);
}

void KTimeZonesTest::zoneinfoDir()
{
    QString zoneinfo = KSystemTimeZones::zoneinfoDir();
    QCOMPARE(zoneinfo, mDataDir);
}


    ////////////////////////
    // KSystemTimeZone tests
    ////////////////////////

void KTimeZonesTest::currentOffset()
{
    QString tzfile = ":" + mDataDir + "/Europe/Paris";
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", tzfile.toLatin1().data(), 1);
    ::tzset();

    // Find the current offset of a time zone
    time_t now = time(0);
    tm *tnow = localtime(&now);
#ifndef _WIN32
    int offset = tnow->tm_gmtoff;
#else
    int offset = 0;
#endif
    const KTimeZone *local = KSystemTimeZones::local();
    QVERIFY((bool)local);
    QCOMPARE(local->currentOffset(Qt::UTC), offset);


    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KTimeZonesTest::offsetAtUtc()
{
    // Find some offsets for Europe/London.
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY( losAngeles != 0 );
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london != 0 );
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QCOMPARE(london->offsetAtUtc(winter), 0);
    QCOMPARE(london->offsetAtUtc(summer), 3600);;
    QCOMPARE(losAngeles->offsetAtUtc(winter), -28800);
    QCOMPARE(losAngeles->offsetAtUtc(summer), -25200);;
}

void KTimeZonesTest::offsetAtZoneTime()
{
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime aInvalid(QDate(2005,3,27), QTime(1,30,0), Qt::LocalTime);
    QDateTime aBst(QDate(2005,3,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime bBst(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(1,30,0), Qt::LocalTime);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london != 0 );
    int offset2;
    QCOMPARE(london->offsetAtZoneTime(aGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london->offsetAtZoneTime(aInvalid, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(aBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(bBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(bBstBeforeGmt, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london->offsetAtZoneTime(bGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
}

void KTimeZonesTest::abbreviation()
{
    // Fetch time zone abbreviations
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY( losAngeles != 0 );
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london != 0 );
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QString sResult = london->abbreviation(winter);
    QCOMPARE(london->abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(london->abbreviation(summer), QByteArray("BST"));
    QCOMPARE(losAngeles->abbreviation(winter), QByteArray("PST"));
    QCOMPARE(losAngeles->abbreviation(summer), QByteArray("PDT"));
}

void KTimeZonesTest::timet()
{
    QDateTime t1(QDate(1970,1,2), QTime(1,30,5), Qt::UTC);
    QDateTime t2(QDate(1969,12,30), QTime(22,29,55), Qt::UTC);
    time_t t1t = KTimeZone::toTime_t(t1);
    time_t t2t = KTimeZone::toTime_t(t2);
    QCOMPARE((int)t1t, 86400 + 3600 + 30*60 + 5);
    QCOMPARE((int)t2t, -(86400 + 3600 + 30*60 + 5));
    QCOMPARE(KTimeZone::fromTime_t(t1t), t1);
    QCOMPARE(KTimeZone::fromTime_t(t2t), t2);
}

void KTimeZonesTest::toUtc()
{
    // Convert to UTC.
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY( losAngeles != 0 );
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london != 0 );
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime winterLocal = winter;
    winterLocal.setTimeSpec(Qt::LocalTime);
    QDateTime summerLocal = summer;
    summerLocal.setTimeSpec(Qt::LocalTime);
    QCOMPARE(london->toUtc(winterLocal), winter);
    QCOMPARE(london->toUtc(summerLocal), summer.addSecs(-3600));;
    QCOMPARE(losAngeles->toUtc(winterLocal), winter.addSecs(8*3600));
    QCOMPARE(losAngeles->toUtc(summerLocal), summer.addSecs(7*3600));
}

void KTimeZonesTest::toZoneTime()
{
    // Convert from UTC.
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY( losAngeles != 0 );
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london != 0 );
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime winterLocal = winter;
    winterLocal.setTimeSpec(Qt::LocalTime);
    QDateTime summerLocal = summer;
    summerLocal.setTimeSpec(Qt::LocalTime);
    QCOMPARE(london->toZoneTime(winter), winterLocal);
    QCOMPARE(london->toZoneTime(summer), summerLocal.addSecs(3600));
    QCOMPARE(losAngeles->toZoneTime(winter), winterLocal.addSecs(-8*3600));
    QCOMPARE(losAngeles->toZoneTime(summer), summerLocal.addSecs(-7*3600));

    QDateTime prepre(QDate(2005,10,29), QTime(23,59,59), Qt::UTC);  // before time shift (local time not repeated)
    QDateTime pre(QDate(2005,10,30), QTime(0,0,0), Qt::UTC);  // before time shift (local time repeated afterwards)
    QDateTime before(QDate(2005,10,30), QTime(0,59,59), Qt::UTC);  // before time shift (local time repeated afterwards)
    QDateTime at(QDate(2005,10,30), QTime(1,0,0), Qt::UTC);   // at time shift (second occurrence of local time)
    QDateTime last(QDate(2005,10,30), QTime(1,59,59), Qt::UTC);  // after time shift (second occurrence of local time)
    QDateTime after(QDate(2005,10,30), QTime(2,0,0), Qt::UTC);   // after time shift (local time not repeated)
    bool second;
    QCOMPARE(london->toZoneTime(prepre, &second), QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london->toZoneTime(pre, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london->toZoneTime(before, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london->toZoneTime(at, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(london->toZoneTime(last, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(london->toZoneTime(after, &second), QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime));
    QVERIFY(!second);
}

void KTimeZonesTest::convert()
{
    // Try time zone conversions.
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY( losAngeles != 0 );
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london != 0 );
    QDateTime bstBeforePdt(QDate(2005,3,28), QTime(0,0,0), Qt::LocalTime);
    QDateTime bstAfterPdt(QDate(2005,5,1), QTime(0,0,0), Qt::LocalTime);
    QDateTime gmtBeforePst(QDate(2005,10,30), QTime(4,0,0), Qt::LocalTime);
    QDateTime gmtAfterPst(QDate(2005,12,1), QTime(0,0,0), Qt::LocalTime);
    QDateTime bstBeforePdtResult(QDate(2005,3,27), QTime(15,0,0), Qt::LocalTime);
    QDateTime bstAfterPdtResult(QDate(2005,4,30), QTime(16,0,0), Qt::LocalTime);
    QDateTime gmtBeforePstResult(QDate(2005,10,29), QTime(21,0,0), Qt::LocalTime);
    QDateTime gmtAfterPstResult(QDate(2005,11,30), QTime(16,0,0), Qt::LocalTime);

    QCOMPARE(london->convert(losAngeles, bstBeforePdt), bstBeforePdtResult);
    QCOMPARE(london->convert(losAngeles, bstAfterPdt), bstAfterPdtResult);
    QCOMPARE(london->convert(losAngeles, gmtBeforePst), gmtBeforePstResult);
    QCOMPARE(london->convert(losAngeles, gmtAfterPst), gmtAfterPstResult);
    QCOMPARE(losAngeles->convert(losAngeles, bstBeforePdtResult), bstBeforePdtResult);
}

////////////////////////
// KTzfileTimeZone tests
// Plus KSystemTimeZones::readZone() tests
////////////////////////

void KTimeZonesTest::tzfile()
{
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QString zoneinfo = KSystemTimeZones::zoneinfoDir();
    QVERIFY(!zoneinfo.isEmpty());
    KTzfileTimeZoneSource tzsource(zoneinfo);
    KTimeZone *tzcairo = new KTzfileTimeZone(&tzsource, "Africa/Cairo");
    delete tzcairo;
    tzcairo = new KTzfileTimeZone(&tzsource, "Africa/Cairo");
    QCOMPARE(tzcairo->offsetAtUtc(winter), 7200);
    delete tzcairo;
}

void KTimeZonesTest::tzfileToZoneTime()
{
    // Convert from UTC.
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone *london = new KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY( london != 0 );
    QDateTime prepre(QDate(2005,10,29), QTime(23,59,59), Qt::UTC);  // before time shift (local time not repeated)
    QDateTime pre(QDate(2005,10,30), QTime(0,0,0), Qt::UTC);  // before time shift (local time repeated afterwards)
    QDateTime before(QDate(2005,10,30), QTime(0,59,59), Qt::UTC);  // before time shift (local time repeated afterwards)
    QDateTime at(QDate(2005,10,30), QTime(1,0,0), Qt::UTC);    // at time shift (second occurrence of local time)
    QDateTime last(QDate(2005,10,30), QTime(1,59,59), Qt::UTC);  // after time shift (second occurrence of local time)
    QDateTime after(QDate(2005,10,30), QTime(2,0,0), Qt::UTC);    // after time shift (local time not repeated)
    bool second;
    QCOMPARE(london->toZoneTime(prepre, &second), QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london->toZoneTime(pre, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london->toZoneTime(before, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london->toZoneTime(at, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(london->toZoneTime(last, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(london->toZoneTime(after, &second), QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime));
    QVERIFY(!second);
    delete london;

    KTimeZone *sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY( sysLondon != 0 );
    QCOMPARE(sysLondon->toZoneTime(prepre, &second), QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(sysLondon->toZoneTime(pre, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(sysLondon->toZoneTime(before, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(sysLondon->toZoneTime(at, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(sysLondon->toZoneTime(last, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(sysLondon->toZoneTime(after, &second), QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime));
    QVERIFY(!second);
    delete sysLondon;
}

void KTimeZonesTest::tzfileOffsetAtZoneTime()
{
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime aInvalid(QDate(2005,3,27), QTime(1,30,0), Qt::LocalTime);
    QDateTime aBst(QDate(2005,3,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime bBst(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(1,30,0), Qt::LocalTime);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone *london = new KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY( london != 0 );
    int offset2;
    QCOMPARE(london->offsetAtZoneTime(aGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london->offsetAtZoneTime(aInvalid, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(london->offsetAtZoneTime(aBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(bBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(bBstBeforeGmt, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london->offsetAtZoneTime(bGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    delete london;

    KTimeZone *sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY( sysLondon != 0 );
    QCOMPARE(sysLondon->offsetAtZoneTime(aGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon->offsetAtZoneTime(aInvalid, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(sysLondon->offsetAtZoneTime(aBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(sysLondon->offsetAtZoneTime(bBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(sysLondon->offsetAtZoneTime(bBstBeforeGmt, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon->offsetAtZoneTime(bGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    delete sysLondon;
}

void KTimeZonesTest::tzfileUtcOffsets()
{
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone *london = new KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY( london != 0 );
    QList<int> offsets = london->utcOffsets();
    QCOMPARE(offsets.count(), 3);
    QCOMPARE(offsets[0], 0);    // GMT
    QCOMPARE(offsets[1], 3600); // BST
    QCOMPARE(offsets[2], 7200); // DST
    delete london;

    KTimeZone *sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY( sysLondon != 0 );
    offsets = sysLondon->utcOffsets();
    QCOMPARE(offsets.count(), 3);
    QCOMPARE(offsets[0], 0);    // GMT
    QCOMPARE(offsets[1], 3600); // BST
    QCOMPARE(offsets[2], 7200); // DST
    delete sysLondon;
}

void KTimeZonesTest::tzfileAbbreviation()
{
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone *london = new KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY( london != 0 );
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime standard(QDate(1970,4,30), QTime(12,45,16,25), Qt::UTC);
    QString sResult = london->abbreviation(winter);
    QCOMPARE(london->abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(london->abbreviation(summer), QByteArray("BST"));
    QCOMPARE(london->abbreviation(standard), QByteArray("BST"));
    delete london;

    KTimeZone *sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY( sysLondon != 0 );
    sResult = sysLondon->abbreviation(winter);
    QCOMPARE(sysLondon->abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(sysLondon->abbreviation(summer), QByteArray("BST"));
    QCOMPARE(sysLondon->abbreviation(standard), QByteArray("BST"));
    delete sysLondon;
}
