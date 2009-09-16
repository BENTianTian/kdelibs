/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "convertertest.h"
#include <../unitcategory.h>

using namespace KUnitConversion;

void ConverterTest::initTestCase()
{
}

void ConverterTest::testCategory()
{
    QCOMPARE(c.categoryForUnit("km")->id(), (int)LengthCategory);
    QCOMPARE(c.category(i18n("Length"))->id(), (int)LengthCategory);
    QCOMPARE(c.category(LengthCategory)->name(), i18n("Length"));
    QVERIFY(c.categories().size() > 0);
}

void ConverterTest::testUnits()
{
    QCOMPARE(c.unit(i18n("km"))->symbol(), i18n("km"));
    QCOMPARE(c.unit(Kilogram)->symbol(), i18n("kg"));
}

void ConverterTest::testConvert()
{
    Value v = c.convert(Value(3.14, Kilometer), "m");
    QCOMPARE(v.number(), 3140.0);
    v = c.convert(v, "cm");
    QCOMPARE(v.number(), 314000.0);
    v = c.convert(v, c.category(LengthCategory)->defaultUnit());
    QCOMPARE(v.number(), 3140.0);
}

void ConverterTest::testInvalid()
{
    QCOMPARE(c.categoryForUnit("does not exist")->id(), (int)InvalidCategory);
    QCOMPARE(c.unit("does not exist")->symbol(), QString(""));
    QCOMPARE(c.category("does not exist")->name(), i18n("Invalid"));
}

QTEST_KDEMAIN(ConverterTest, NoGUI)

