/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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

#include <qtest_kde.h>
#include "kconfigtest.h"
#include "kconfigtest.moc"

#include <kconfig.h>
#include <kdebug.h>
#include <QtGui/QFont>

KCONFIG_DECLARE_ENUM_QOBJECT(KConfigTest,Testing)
KCONFIG_DECLARE_ENUM_QOBJECT(KConfigTest,Flags)

QTEST_KDEMAIN( KConfigTest, NoGUI );

#define BOOLENTRY1 true
#define BOOLENTRY2 false
#define STRINGENTRY1 "hello"
#define STRINGENTRY2 " hello"
#define STRINGENTRY3 "hello "
#define STRINGENTRY4 " hello "
#define STRINGENTRY5 " "
#define STRINGENTRY6 ""
#define UTF8BITENTRY "Hello äöü"
#define DOUBLEENTRY 123456.78912345
#define POINTENTRY QPoint( 4351, 1235 )
#define SIZEENTRY QSize( 10, 20 )
#define RECTENTRY QRect( 10, 23, 5321, 13 )
#define DATETIMEENTRY QDateTime( QDate( 2002, 06, 23 ), QTime( 12, 55, 40 ) )
#define STRINGLISTENTRY (QStringList( "Hello," ) << " World")
#define INTLISTENTRY1 QList<int>() << 1 << 2 << 3 << 4
#define BYTEARRAYLISTENTRY1 QList<QByteArray>() << "" << "1,2" << "end"
#define COLORENTRY1 QColor("steelblue")
#define COLORENTRY2 QColor(235, 235, 100, 125)
#define COLORENTRY3 QColor(234, 234, 127)
#define FONTENTRY QFont("Times", 16, QFont::Normal)
#define VARIANTLISTENTRY (QVariantList() << true << false << QString("joe") << 10023)
#define VARIANTLISTENTRY2 (QVariantList() << POINTENTRY << SIZEENTRY)

void KConfigTest::initTestCase()
{
  KConfig sc( "kconfigtest" );

  sc.setGroup("AAA");
  sc.writeEntry("stringEntry1", STRINGENTRY1,
                KConfigBase::Persistent|KConfigBase::Global);
  sc.deleteEntry("stringEntry2", KConfigBase::Global);

  sc.setGroup("Hello");
  sc.writeEntry( "boolEntry1", BOOLENTRY1 );
  sc.writeEntry( "boolEntry2", BOOLENTRY2 );

  QByteArray data( UTF8BITENTRY );
  QCOMPARE( data.size(), 12 ); // the source file is in utf8
  sc.writeEntry( "Test", QVariant( data ) ); // passing "data" converts it to char* and KConfigBase calls fromLatin1!
  sc.writeEntry( "Test2", "");
  sc.writeEntry( "stringEntry1", STRINGENTRY1 );
  sc.writeEntry( "stringEntry2", STRINGENTRY2 );
  sc.writeEntry( "stringEntry3", STRINGENTRY3 );
  sc.writeEntry( "stringEntry4", STRINGENTRY4 );
  sc.writeEntry( "stringEntry5", STRINGENTRY5 );
//  sc.writeEntry( "stringEntry6", STRINGENTRY6 );
  sc.writeEntry( "keywith=equalsign", STRINGENTRY1 );
  sc.deleteEntry( "stringEntry5" );
  sc.deleteEntry( "stringEntry6" );
  sc.writeEntry( "byteArrayEntry1", QByteArray( STRINGENTRY1 ),
                 KConfigBase::Global|KConfigBase::Persistent );
  sc.writeEntry( "doubleEntry1", DOUBLEENTRY );

  sc.deleteGroup("deleteMe");

  sc.setGroup("ComplexTypes");
  sc.writeEntry( "rectEntry", RECTENTRY );
  sc.writeEntry( "pointEntry", POINTENTRY );
  sc.writeEntry( "sizeEntry", SIZEENTRY );
  sc.writeEntry( "dateTimeEntry", DATETIMEENTRY );
  sc.writeEntry( "dateEntry", DATETIMEENTRY.date() );
  sc.writeEntry( "colorEntry1", COLORENTRY1 );
  sc.writeEntry( "colorEntry2", COLORENTRY2 );
  sc.writeEntry( "colorEntry3", (QList<int>() << 234 << 234 << 127));
  sc.writeEntry( "colorEntry4",  (QList<int>() << 235 << 235 << 100 << 125));
  sc.writeEntry( "fontEntry", FONTENTRY );

  sc.setGroup( "ListTypes" );
  sc.writeEntry( "listOfIntsEntry1", INTLISTENTRY1 );
  sc.writeEntry( "listOfByteArraysEntry1", BYTEARRAYLISTENTRY1 );
  sc.writeEntry( "stringListEntry", STRINGLISTENTRY );
  sc.writeEntry( "variantListEntry", VARIANTLISTENTRY );

  sc.setGroup( "EnumTypes" );
  sc.writeEntry( "enum-10", KConfigTest::Tens );

#ifndef Q_CC_MSVC
  sc.writeEntry( "enum-100", KConfigTest::Hundreds );
#else
  sc.writeEntry( "enum-100", KConfigTest::Hundreds, KConfigBase::Normal );
#endif

  sc.writeEntry( "flags-bit0", KConfigTest::Flags(KConfigTest::bit0));

#ifndef Q_CC_MSVC
  sc.writeEntry( "flags-bit0-bit1", KConfigTest::Flags(KConfigTest::bit0|KConfigTest::bit1));
#else
  sc.writeEntry( "flags-bit0-bit1", KConfigTest::Flags(KConfigTest::bit0|KConfigTest::bit1), KConfigBase::Normal );
#endif

  sc.sync();

  KConfig sc1("kdebugrc");
  sc1.setGroup("0");
  sc1.writeEntry("AbortFatal", false);
  sc1.writeEntry("WarnOutput", 0);
  sc1.writeEntry("FatalOutput", 0);
  sc1.sync();
}

void KConfigTest::cleanupTestCase()
{
  QDir local = QDir::homePath() + "/.kde-unit-test/share/config";

  foreach(QString file, local.entryList(QDir::Files))
    if(!local.remove(file))
      qWarning("%s: removing failed", qPrintable( file ));

  QCOMPARE((int)local.entryList(QDir::Files).count(), 0);

  local.cdUp();
  local.rmpath("config");
}

// ### TODO: call this, and test the state of things afterwards
void KConfigTest::revertEntries()
{
//  qWarning("Reverting entries");
  KConfig sc( "kconfigtest" );

  sc.setGroup("Hello");
  sc.revertToDefault( "boolEntry1" );
  sc.revertToDefault( "boolEntry2" );

  sc.revertToDefault( "Test" );
  sc.revertToDefault( "Test2" );
  sc.revertToDefault( "stringEntry1" );
  sc.revertToDefault( "stringEntry2" );
  sc.revertToDefault( "stringEntry3" );
  sc.revertToDefault( "stringEntry4" );
  sc.revertToDefault( "stringEntry5" );
  sc.sync();
}

void KConfigTest::testSimple()
{
//  kDebug() << k_funcinfo << endl;

  KConfig sc2( "kconfigtest" );

  KConfigGroup sc3( &sc2, "AAA");
  bool bImmutable = sc3.entryIsImmutable("stringEntry1");

  QVERIFY( !bImmutable );
  //qWarning("sc3.entryIsImmutable() 1: %s", bImmutable ? "true" : "false");

  sc2.setGroup("AAA");
  QVERIFY( sc2.hasKey( "stringEntry1" ) );
  QCOMPARE( sc2.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );
  QCOMPARE( sc2.entryIsImmutable("stringEntry1"), bImmutable );
  QVERIFY( !sc2.hasKey( "stringEntry2" ) );
  QCOMPARE( sc2.readEntry( "stringEntry2", QString("bla") ), QString( "bla" ) );

  QVERIFY( !sc2.hasDefault( "stringEntry1" ) );

  sc2.setGroup("Hello");
  QCOMPARE( sc2.readEntry( "Test", QByteArray() ), QByteArray( UTF8BITENTRY ) );
  QCOMPARE( sc2.readEntry( "Test", QString() ), QString::fromUtf8( UTF8BITENTRY ) );
  QCOMPARE( sc2.readEntry("Test2", QString("Fietsbel")).isEmpty(), true );
  QCOMPARE( sc2.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );
  QCOMPARE( sc2.readEntry( "stringEntry2" ), QString( STRINGENTRY2 ) );
  QCOMPARE( sc2.readEntry( "stringEntry3" ), QString( STRINGENTRY3 ) );
  QCOMPARE( sc2.readEntry( "stringEntry4" ), QString( STRINGENTRY4 ) );
  QVERIFY( !sc2.hasKey( "stringEntry5" ) );
  QCOMPARE( sc2.readEntry( "stringEntry5", QString("test") ), QString( "test" ) );
  QVERIFY( !sc2.hasKey( "stringEntry6" ) );
  QCOMPARE( sc2.readEntry( "stringEntry6", QString("foo") ), QString( "foo" ) );
  QCOMPARE( sc2.readEntry( "boolEntry1", BOOLENTRY1 ), BOOLENTRY1 );
  QCOMPARE( sc2.readEntry( "boolEntry2", false ), BOOLENTRY2 );

#if 0
  QString s;
  s = sc2.readEntry( "keywith=equalsign" );
  fprintf(stderr, "comparing keywith=equalsign %s with %s -> ", STRINGENTRY1, s.toLatin1().constData());
  if (s == STRINGENTRY1)
    fprintf(stderr, "OK\n");
  else {
    fprintf(stderr, "not OK\n");
    exit(-1);
  }
#endif

  QCOMPARE( sc2.readEntry( "byteArrayEntry1", QByteArray() ),
            QByteArray( STRINGENTRY1 ) );
  QCOMPARE( sc2.readEntry( "doubleEntry1", 0.0 ), DOUBLEENTRY );
}

void KConfigTest::testLists()
{
  KConfig sc2( "kconfigtest" );
  sc2.setGroup("ListTypes");

  QCOMPARE( sc2.readEntry( QString("stringListEntry"), QStringList()),
            STRINGLISTENTRY );

  QCOMPARE( sc2.readEntry( "listOfIntsEntry1" ), QString::fromLatin1( "1,2,3,4" ) );
  QList<int> expectedIntList = INTLISTENTRY1;
  QVERIFY( sc2.readEntry( "listOfIntsEntry1", QList<int>() ) == expectedIntList );

  QCOMPARE( QVariant(sc2.readEntry( "variantListEntry", VARIANTLISTENTRY )).toStringList(),
            QVariant(VARIANTLISTENTRY).toStringList() );

  QCOMPARE( sc2.readEntry( "listOfByteArraysEntry1", QList<QByteArray>()), BYTEARRAYLISTENTRY1 );
}

void KConfigTest::testComplex()
{
  KConfig sc2( "kconfigtest" );
  sc2.setGroup("ComplexTypes");

  QCOMPARE( sc2.readEntry( "pointEntry", QPoint() ), POINTENTRY );
  QCOMPARE( sc2.readEntry( "sizeEntry", SIZEENTRY ), SIZEENTRY);
  QCOMPARE( sc2.readEntry( "rectEntry", QRect(1,2,3,4) ), RECTENTRY );
  QCOMPARE( sc2.readEntry( "dateTimeEntry", QDateTime() ).toString(Qt::ISODate),
            DATETIMEENTRY.toString(Qt::ISODate) );
  QCOMPARE( sc2.readEntry( "dateEntry", QDate() ).toString(Qt::ISODate),
            DATETIMEENTRY.date().toString(Qt::ISODate) );
  QCOMPARE( sc2.readEntry( "dateTimeEntry", QDate() ), DATETIMEENTRY.date() );
  QCOMPARE( QVariant(sc2.readEntry( "colorEntry1", Qt::black )).toString(),
            QVariant(COLORENTRY1).toString() );
  QCOMPARE( sc2.readEntry( "colorEntry1", QColor() ), COLORENTRY1 );
  QCOMPARE( sc2.readEntry( "colorEntry2", QColor() ), COLORENTRY2 );
  QCOMPARE( sc2.readEntry( "colorEntry3", QColor() ), COLORENTRY3 );
  QCOMPARE( sc2.readEntry( "colorEntry4", QColor() ), COLORENTRY2 );
  QCOMPARE( sc2.readEntry( "fontEntry", QFont() ), FONTENTRY );
}

void KConfigTest::testEnums()
{
  KConfig sc("kconfigtest");

  sc.setGroup( "EnumTypes" );

  QCOMPARE( sc.readEntry( "enum-10" ), QString("Tens"));
  QVERIFY( sc.readEntry( "enum-100", KConfigTest::Ones) != KConfigTest::Ones);
  QVERIFY( sc.readEntry( "enum-100", KConfigTest::Ones) != KConfigTest::Tens);

  QCOMPARE( sc.readEntry( "flags-bit0" ), QString("bit0"));
  QVERIFY( sc.readEntry( "flags-bit0", KConfigTest::Flags() ) == KConfigTest::bit0 );

  QCOMPARE( sc.readEntry( "flags-bit0-bit1" ), QString("bit1|bit0") );
  QVERIFY( sc.readEntry( "flags-bit0-bit1", KConfigTest::Flags() ) ==
           KConfigTest::bit0|KConfigTest::bit1 );
}

void KConfigTest::testInvalid()
{
  KConfig sc( "kconfigtest" );

  // all of these should print a message to the kdebug.dbg file
  sc.setGroup( "InvalidTypes" );
  sc.writeEntry( "badList", VARIANTLISTENTRY2 );

  QList<int> list;

  // 1 element list
  list << 1;
  sc.writeEntry( QString("badList"), list);
  sc.sync();

  QVERIFY( sc.readEntry( "badList", QColor() ) == QColor() );
  QVERIFY( sc.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc.readEntry( "badList", QSize() ) == QSize() );
  QVERIFY( sc.readEntry( "badList", QDate() ) == QDate() );
  QVERIFY( sc.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 2 element list
  list << 2;
  sc.writeEntry( "badList", list);
  sc.sync();

  QVERIFY( sc.readEntry( "badList", QColor() ) == QColor() );
  QVERIFY( sc.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc.readEntry( "badList", QDate() ) == QDate() );
  QVERIFY( sc.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 3 element list
  list << 303;
  sc.writeEntry( "badList", list);
  sc.sync();

  QVERIFY( sc.readEntry( "badList", QColor() ) == QColor() ); // out of bounds
  QVERIFY( sc.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc.readEntry( "badList", QSize() ) == QSize() );
  QVERIFY( sc.readEntry( "badList", QDate() ) == QDate() ); // out of bounds
  QVERIFY( sc.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 4 element list
  list << 4;
  sc.writeEntry( "badList", list );
  sc.sync();

  QVERIFY( sc.readEntry( "badList", QColor() ) == QColor() ); // out of bounds
  QVERIFY( sc.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc.readEntry( "badList", QSize() ) == QSize() );
  QVERIFY( sc.readEntry( "badList", QDate() ) == QDate() );
  QVERIFY( sc.readEntry( "badList", QDateTime() ) == QDateTime() );

  list[2] = -3;
  sc.writeEntry( "badList", list ); sc.sync();
  QVERIFY( sc.readEntry( "badList", QColor() ) == QColor() ); // out of bounds

  // 5 element list
  list[2] = 3;
  list << 5;
  sc.writeEntry( "badList", list);
  sc.sync();

  QVERIFY( sc.readEntry( "badList", QColor() ) == QColor() );
  QVERIFY( sc.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc.readEntry( "badList", QSize() ) == QSize() );
  QVERIFY( sc.readEntry( "badList", QDate() ) == QDate() );
  QVERIFY( sc.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 6 element list
  list << 6;
  sc.writeEntry( "badList", list);
  sc.sync();

  QVERIFY( sc.readEntry( "badList", QColor() ) == QColor() );
  QVERIFY( sc.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc.readEntry( "badList", QSize() ) == QSize() );
}

void KConfigTest::testDelete()
{
  KConfig sc( "kconfigtest" );
  sc.setGroup("Hello");

  sc.deleteEntry("Test");
  QCOMPARE( sc.readEntry("Test", QString("Fietsbel")), QString("Fietsbel") );

  sc.deleteGroup("ComplexTypes");

  KConfigGroup cg(&sc , "AAA" );
  cg.deleteGroup();
  QVERIFY( sc.entryMap("ComplexTypes").isEmpty() );
  QVERIFY( sc.entryMap("AAA").isEmpty() );
  QVERIFY( !sc.entryMap("Hello").isEmpty() ); //not deleted group
  QVERIFY( sc.entryMap("FooBar").isEmpty() ); //inexistant group
}

void KConfigTest::testKAboutDataOrganizationDomain()
{
    KAboutData data( "app", "program", "version", 0, KAboutData::License_LGPL,
                     0, "hello world", "http://www.koffice.org" );
    QCOMPARE( data.organizationDomain(), QString::fromLatin1( "koffice.org" ) );
}
