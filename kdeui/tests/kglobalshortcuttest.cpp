/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#include "kglobalshortcuttest.h"
#include <qtest_kde.h>
#include <kaction.h>
#include <kglobalaccel.h>

#include <QtDBus/QDBusConnectionInterface>
#include <QtCore/QtDebug>


/* These tests could be better. They don't include actually triggering actions,
   and we just choose very improbable shortcuts to avoid conflicts with real
   applications' shortcuts. */

//we need a KComponentData and a GUI so that the implementation can grab keys
QTEST_KDEMAIN( KGlobalShortcutTest, GUI )

void KGlobalShortcutTest::initTestCase()
{
    m_actionA = new KAction("Action A", this);
    m_actionB = new KAction("Action B", this);
    //purge previous config file entries of the actions by setting them to "empty and default empty"
    //(this is the only safe situation to remove entries in normal operation)
    m_actionA->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut | KAction::DefaultShortcut,
                                 KAction::NoAutoloading);
    m_actionB->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut | KAction::DefaultShortcut,
                                 KAction::NoAutoloading);
    QVERIFY(m_actionA->globalShortcut().isEmpty());
    QVERIFY(m_actionB->globalShortcut().isEmpty());
}


void KGlobalShortcutTest::testSetShortcut()
{
    //possible modifiers are SHIFT META CTRL ALT
    KShortcut cutA(Qt::SHIFT + Qt::META + Qt::CTRL + Qt::ALT + Qt::Key_F28, Qt::Key_F29);
    m_actionA->setGlobalShortcut(cutA);
    QCOMPARE(m_actionA->globalShortcut(), cutA);

    m_actionB->setGlobalShortcut(cutA);
    QVERIFY(m_actionB->globalShortcut().isEmpty());
}


void KGlobalShortcutTest::testFindActionByKey()
{
    QStringList actionNames = KGlobalAccel::self()->findActionNameSystemwide(Qt::Key_F29);
    QCOMPARE(actionNames.count(), 2);
    QCOMPARE(actionNames.at(0), QString("qttest"));
    QCOMPARE(actionNames.at(1), QString("Action A"));
    actionNames = KGlobalAccel::self()->findActionNameSystemwide(Qt::SHIFT + Qt::META + Qt::CTRL
                                                                 + Qt::ALT + Qt::Key_F28);
    QCOMPARE(actionNames.count(), 2);
    QCOMPARE(actionNames.at(1),
             QString("Action A"));
}


void KGlobalShortcutTest::testChangeShortcut()
{
    KShortcut cutB(m_actionA->globalShortcut().primary(), QKeySequence(Qt::META + Qt::Key_F29));
    m_actionB->setGlobalShortcut(cutB, KAction::ActiveShortcut | KAction::DefaultShortcut,
                                 KAction::NoAutoloading);
    QVERIFY(m_actionB->globalShortcut().primary().isEmpty());
    QCOMPARE(m_actionB->globalShortcut().alternate(), QKeySequence(Qt::META + Qt::Key_F29));

    cutB.setPrimary(Qt::META + Qt::ALT + Qt::Key_F30);
    m_actionB->setGlobalShortcut(cutB, KAction::ActiveShortcut | KAction::DefaultShortcut,
                                 KAction::NoAutoloading);
    QCOMPARE(m_actionB->globalShortcut(), cutB);
}


void KGlobalShortcutTest::testStealShortcut()
{
    QVERIFY(!m_actionB->globalShortcut().primary().isEmpty());
    KGlobalAccel::stealShortcutSystemwide(Qt::META + Qt::ALT + Qt::Key_F30);
    //let DBus do its thing in case it happens asynchronously
    QTest::qWait(200);
    QVERIFY(m_actionB->globalShortcut().primary().isEmpty());
}


void KGlobalShortcutTest::testSaveRestore()
{
    //It /would be nice/ to test persistent storage. That is not so easy...
    KShortcut cutA = m_actionA->globalShortcut();
    delete m_actionA;
    m_actionA = new KAction("Action A", this);
    QVERIFY(m_actionA->globalShortcut().isEmpty());

    m_actionA->setGlobalShortcutAllowed(true);
    QCOMPARE(m_actionA->globalShortcut(), cutA);

    delete m_actionA;
    m_actionA = new KAction("Action A", this);
    m_actionA->setGlobalShortcut(KShortcut(QKeySequence(), cutA.primary()));
    QCOMPARE(m_actionA->globalShortcut(), cutA);
}


void KGlobalShortcutTest::cleanupTestCase()
{
    //purge config file entries, see initTestCase()
    m_actionA->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut | KAction::DefaultShortcut,
                                 KAction::NoAutoloading);
    m_actionB->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut | KAction::DefaultShortcut,
                                 KAction::NoAutoloading);
}

#include "kglobalshortcuttest.moc"
