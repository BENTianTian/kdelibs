/* This file is part of the KDE project
   Copyright (C) 2007 David Faure <faure@kde.org>

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

#include "kdirlistertest.h"
#include "kdirlistertest.moc"
#include <kdirlister.h>
#include <qtest_kde.h>

QTEST_KDEMAIN( KDirListerTest, GUI )

#include <kdebug.h>
#include "kiotesthelper.h"
#include <kdirwatch.h>

void KDirListerTest::initTestCase()
{
    s_referenceTimeStamp = QDateTime::currentDateTime().addSecs( -120 ); // 2 minutes ago

    // Create test data:
    /*
     * PATH/toplevelfile_1
     * PATH/toplevelfile_2
     * PATH/toplevelfile_3
     * PATH/subdir
     * PATH/subdir/testfile
     * PATH/subdir/subsubdir
     * PATH/subdir/subsubdir/testfile
     */
    const QString path = m_tempDir.name();
    createTestFile(path+"toplevelfile_1");
    createTestFile(path+"toplevelfile_2");
    createTestFile(path+"toplevelfile_3");
    createTestDirectory(path+"subdir");
    createTestDirectory(path+"subdir/subsubdir");

    // Hmmpf.
    qRegisterMetaType<KUrl>();
}

void KDirListerTest::testOpenUrl()
{
    m_items.clear();
    const QString path = m_tempDir.name();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    // The call to openUrl itself, emits started
    m_dirLister.openUrl(KUrl(path), false, false /*no reload*/);

    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 0);
    QCOMPARE(spyCompletedKUrl.count(), 0);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), 0);

    // then wait for completed
    qDebug("waiting for completed");
    connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 1);
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), 4);
    disconnect(&m_dirLister, 0, this, 0);
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testOpenUrlFromCache()
{
    m_items.clear();
    const QString path = m_tempDir.name();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    // The call to openUrl itself, emits started, the items, and completed, since it's all in the cache
    // ### Maybe we should get rid of this behavior difference...
    m_dirLister.openUrl(KUrl(path), false, false /*no reload*/); // TODO turn those bools into an enum

    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 1);
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), 4);

    // Get into the case where another dirlister is holding the items
    {
        m_items.clear();
        KDirLister secondDirLister;
        QSignalSpy spyStarted(&secondDirLister, SIGNAL(started(KUrl)));
        QSignalSpy spyClear(&secondDirLister, SIGNAL(clear()));
        QSignalSpy spyClearKUrl(&secondDirLister, SIGNAL(clear(KUrl)));
        QSignalSpy spyCompleted(&secondDirLister, SIGNAL(completed()));
        QSignalSpy spyCompletedKUrl(&secondDirLister, SIGNAL(completed(KUrl)));
        QSignalSpy spyCanceled(&secondDirLister, SIGNAL(canceled()));
        QSignalSpy spyCanceledKUrl(&secondDirLister, SIGNAL(canceled(KUrl)));
        connect(&secondDirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
        secondDirLister.openUrl(KUrl(path), false, false /*no reload*/); // TODO turn those bools into an enum
        QCOMPARE(spyStarted.count(), 1);
        QCOMPARE(spyCompleted.count(), 1);
        QCOMPARE(spyCompletedKUrl.count(), 1);
        QCOMPARE(spyCanceled.count(), 0);
        QCOMPARE(spyCanceledKUrl.count(), 0);
        QCOMPARE(spyClear.count(), 1);
        QCOMPARE(spyClearKUrl.count(), 0);
        QCOMPARE(m_items.count(), 4);
    }

    disconnect(&m_dirLister, 0, this, 0);
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testNewItems()
{
    QCOMPARE(m_items.count(), 4);
    const QString path = m_tempDir.name();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    QTest::qWait(1000); // We need a 1s timestamp difference on the dir, otherwise FAM won't notice anything.

    kDebug() << "Creating new file";
    QFile file(path+"toplevelfile_new");
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("foo");
    file.close();

    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (m_items.count() == 4) {
        QVERIFY(++numTries < 10);
        QTest::qWait(200);
    }
    //kDebug() << "numTries=" << numTries;
    QCOMPARE(m_items.count(), 5);

    QCOMPARE(spyStarted.count(), 1); // Updates call started
    QCOMPARE(spyCompleted.count(), 1); // and completed
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 0);
    QCOMPARE(spyClearKUrl.count(), 0);
    disconnect(&m_dirLister, 0, this, 0);
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testRefreshItems()
{
    m_refreshedItems.clear();
    const QString path = m_tempDir.name();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(refreshItems(KFileItemList)), this, SLOT(slotRefreshItems(KFileItemList)));

    QFile file(path+"toplevelfile_2");
    QVERIFY(file.open(QIODevice::Append));
    file.write("foo");
    file.close();

    // KDirWatch doesn't make this work when using FAM :(
    //KDirWatch::self()->setDirty(path+"toplevelfile_2"); // hack
    KDirWatch::self()->setDirty(path); // with only the file, we get into the new fast path that doesn't even emit started...

    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (m_refreshedItems.isEmpty()) {
        QVERIFY(++numTries < 10);
        QTest::qWait(200);
    }

    QCOMPARE(spyStarted.count(), 1); // Updates (to a directory) call started...
    QCOMPARE(spyCompleted.count(), 1); // and completed
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 0);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(m_refreshedItems.count(), 1);
    KFileItem* newItem = m_refreshedItems.first();
    QCOMPARE(newItem->size(), KIO::filesize_t(11 /*Hello world*/ + 3 /*foo*/));
    disconnect(&m_dirLister, 0, this, 0);
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testDeleteItem()
{
    const QString path = m_tempDir.name();
    qRegisterMetaType<KFileItem*>("KFileItem*");
    QSignalSpy spyDeleteItem(&m_dirLister, SIGNAL(deleteItem(KFileItem*)));
    connect(&m_dirLister, SIGNAL(deleteItem(KFileItem*)), this, SLOT(exitLoop()));

    //kDebug() << "Removing " << path+"toplevelfile_1";
    QFile::remove(path+"toplevelfile_1");
    // the remove() doesn't always trigger kdirwatch in stat mode, if this all happens in the same second
    KDirWatch::self()->setDirty(path);
    if (spyDeleteItem.count() == 0) {
        qDebug("waiting for deleteItem");
        enterLoop();
    }

    // OK now kdirlister told us the file was deleted, let's try a re-listing
    m_items.clear();
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    m_dirLister.openUrl(KUrl(path), false, false /*no reload*/);
    QCOMPARE(m_items.count(), 4);

    disconnect(&m_dirLister, 0, this, 0);
}

void KDirListerTest::testOpenAndStop()
{
    m_items.clear();
    const QString path = "/"; // better not use a directory that we already listed!
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    m_dirLister.openUrl(KUrl(path), false, false /*no reload*/);
    m_dirLister.stop(); // we should also test stop(KUrl(path))...

    QCOMPARE(spyStarted.count(), 1); // The call to openUrl itself, emits started
    QCOMPARE(spyCompleted.count(), 0); // we had time to stop before the job even started
    QCOMPARE(spyCompletedKUrl.count(), 0);
    QCOMPARE(spyCanceled.count(), 1);
    QCOMPARE(spyCanceledKUrl.count(), 1);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), 0); // we had time to stop before the job even started
    disconnect(&m_dirLister, 0, this, 0);
}

void KDirListerTest::enterLoop()
{
    qDebug("enterLoop");
    m_eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void KDirListerTest::exitLoop()
{
    qDebug("exitLoop");
    m_eventLoop.quit();
}

void KDirListerTest::slotNewItems(const KFileItemList& lst)
{
    m_items += lst;
}

void KDirListerTest::slotRefreshItems(const KFileItemList& lst)
{
    m_refreshedItems += lst;
}
