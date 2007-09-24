/* This file is part of the KDE libraries

    Copyright (c) 2007 David Faure <faure@kde.org>

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
#include <qtestevent.h>
#include "klineedit_unittest.h"
#include <kcombobox.h>
#include "klineedit_unittest.moc"
#include <klineedit.h>

QTEST_KDEMAIN(KLineEdit_UnitTest, GUI)

void KLineEdit_UnitTest::testReturnPressed()
{
    KLineEdit w;
    w.setText("Hello world");
    QSignalSpy qReturnPressedSpy(&w, SIGNAL(returnPressed()));
    QSignalSpy kReturnPressedSpy(&w, SIGNAL(returnPressed(QString)));
    QTest::keyClick(&w, Qt::Key_Return);
    QCOMPARE(qReturnPressedSpy.count(), 1);
    QCOMPARE(kReturnPressedSpy.count(), 1);
    QCOMPARE(kReturnPressedSpy[0][0].toString(), QString("Hello world"));
}

void KLineEdit_UnitTest::testComboReturnPressed()
{
    KComboBox w(true);
    QVERIFY(w.lineEdit());
    w.lineEdit()->setText("Hello world");
    QSignalSpy qReturnPressedSpy(w.lineEdit(), SIGNAL(returnPressed()));
    QSignalSpy kReturnPressedSpy(w.lineEdit(), SIGNAL(returnPressed(QString)));
    QTest::keyClick(&w, Qt::Key_Return);
    QCOMPARE(qReturnPressedSpy.count(), 1);
    QCOMPARE(kReturnPressedSpy.count(), 1);
    QCOMPARE(kReturnPressedSpy[0][0].toString(), QString("Hello world"));
}
