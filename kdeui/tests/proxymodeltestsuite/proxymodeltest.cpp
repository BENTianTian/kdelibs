/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "proxymodeltest.h"

#include "dynamictreemodel.h"
#include <QItemSelectionModel>
#include "modelspy.h"

ProxyModelTest::ProxyModelTest(QObject *parent)
: QObject(parent),
  m_rootModel(new DynamicTreeModel(this)),
  m_proxyModel(0),
  m_modelSpy(new ModelSpy(this)),
  m_modelCommander(new ModelCommander(m_rootModel, this))
{
}

void ProxyModelTest::doInitTestCase()
{
  m_commandNames = m_modelCommander->commandNames();
  QVERIFY( !m_commandNames.isEmpty() );
}

void ProxyModelTest::doCleanupTestCase()
{
  if (qApp->arguments().isEmpty())
    QVERIFY( m_commandNames.isEmpty() );
}

void ProxyModelTest::init()
{
  m_modelCommander->setDefaultCommands();

  QVERIFY(m_modelSpy->isEmpty());
  bool spyingState = m_modelSpy->isSpying();
  m_modelSpy->stopSpying();
  m_rootModel->clear();
  const char *currentTag = QTest::currentDataTag();

  QVERIFY(currentTag != 0);

  // Get the model into the state it is expected to be in.
  m_modelCommander->executeUntil(currentTag);
  if (spyingState)
    m_modelSpy->startSpying();

  QVERIFY(m_modelSpy->isEmpty());
}

DynamicTreeModel* ProxyModelTest::sourceModel()
{
  return m_rootModel;
}

QVariantList ProxyModelTest::getSignal(SignalType type, IndexFinder parentFinder, int start, int end)
{
  return QVariantList() << type << QVariant::fromValue(parentFinder) << start << end;
}

PersistentIndexChange ProxyModelTest::getChange(IndexFinder parentFinder, int start, int end, int difference, bool toInvalid)
{
  Q_ASSERT(start <= end);
  PersistentIndexChange change;
  change.parentFinder = parentFinder;
  change.startRow = start;
  change.endRow = end;
  change.difference = difference;
  change.toInvalid = toInvalid;
  return change;
}

void ProxyModelTest::handleSignal(QVariantList expected)
{
  QVERIFY(!expected.isEmpty());
  int signalType = expected.takeAt(0).toInt();
  if (NoSignal == signalType)
    return;

  Q_ASSERT(!m_modelSpy->isEmpty());
  QVariantList result = getResultSignal();

  QCOMPARE(result.takeAt(0).toInt(), signalType);
  // Check that the signal we expected to recieve was emitted exactly.
  switch (signalType)
  {
  case RowsAboutToBeInserted:
  case RowsInserted:
  case RowsAboutToBeRemoved:
  case RowsRemoved:
  {
    QVERIFY( expected.size() == 3 );
    IndexFinder parentFinder = qvariant_cast<IndexFinder>(expected.at(0));
    QModelIndex parent = parentFinder.getIndex();

// This is where is usually goes wrong...
#if 0
    kDebug() << qvariant_cast<QModelIndex>(result.at(0)) << parent;
    kDebug() << result.at(1) << expected.at(1);
    kDebug() << result.at(2) << expected.at(2);
#endif

    QCOMPARE(qvariant_cast<QModelIndex>(result.at(0)), parent );
    QCOMPARE(result.at(1), expected.at(1) );
    QCOMPARE(result.at(2), expected.at(2) );
    break;
  }
  case LayoutAboutToBeChanged:
  case LayoutChanged:
  {
    QVERIFY( expected.size() == 0 );
    QVERIFY( result.size() == 0 );
    break;
  }
  case RowsAboutToBeMoved:
  case RowsMoved:
  {
    QVERIFY( expected.size() == 5 );
    IndexFinder scrParentFinder = qvariant_cast<IndexFinder>(expected.at(0));
    QModelIndex srcParent = scrParentFinder.getIndex();
    QCOMPARE(qvariant_cast<QModelIndex>(result.at(0)), srcParent );
    QCOMPARE(result.at(1), expected.at(1) );
    QCOMPARE(result.at(2), expected.at(2) );
    IndexFinder destParentFinder = qvariant_cast<IndexFinder>(expected.at(3));
    QModelIndex destParent = destParentFinder.getIndex();
    QCOMPARE(qvariant_cast<QModelIndex>(result.at(3)), destParent );
    QCOMPARE(result.at(4), expected.at(4) );
    break;
  }
  case DataChanged:
  {
    QVERIFY( expected.size() == 2 );
    IndexFinder topLeftFinder = qvariant_cast<IndexFinder>(expected.at(0));
    QModelIndex topLeft = topLeftFinder.getIndex();
    IndexFinder bottomRightFinder = qvariant_cast<IndexFinder>(expected.at(1));
    QModelIndex bottomRight = bottomRightFinder.getIndex();

    QVERIFY(topLeft.isValid() && bottomRight.isValid());

#if 0
    kDebug() << qvariant_cast<QModelIndex>(result.at(0)) << topLeft;
    kDebug() << qvariant_cast<QModelIndex>(result.at(1)) << bottomRight;
#endif

    QCOMPARE(qvariant_cast<QModelIndex>(result.at(0)), topLeft );
    QCOMPARE(qvariant_cast<QModelIndex>(result.at(1)), bottomRight );
  }

  }
}

QVariantList ProxyModelTest::getResultSignal()
{
  return m_modelSpy->takeAt(0);
}

void ProxyModelTest::testEmptyModel()
{
  QCOMPARE(m_proxyModel->rowCount(), 0);
  QCOMPARE(m_proxyModel->columnCount(), 0);
  QVERIFY(!m_proxyModel->index(0,0).isValid());
  QVERIFY(!m_proxyModel->data(QModelIndex()).isValid());
  QVERIFY(!m_proxyModel->parent(QModelIndex()).isValid());
  QVERIFY(!m_proxyModel->mapToSource(QModelIndex()).isValid());
  QVERIFY(!m_proxyModel->mapFromSource(QModelIndex()).isValid());
}

void ProxyModelTest::doTestMappings(const QModelIndex &parent)
{
  QModelIndex idx;
  QModelIndex srcIdx;
  for (int column = 0; column < m_proxyModel->columnCount(parent); ++column)
  {
    for (int row = 0; row < m_proxyModel->rowCount(parent); ++row)
    {
      idx = m_proxyModel->index(row, column, parent);
      QVERIFY(idx.isValid());
      QVERIFY(idx.row() == row);
      QVERIFY(idx.column() == column);
      QVERIFY(idx.parent() == parent);
      QVERIFY(idx.model() == m_proxyModel);
      srcIdx = m_proxyModel->mapToSource(idx);
      QVERIFY(srcIdx.isValid());
      QVERIFY(srcIdx.model() == m_proxyModel->sourceModel());
      QVERIFY(m_rootModel == m_proxyModel->sourceModel());
      QVERIFY(idx.data() == srcIdx.data());
      QVERIFY(m_proxyModel->mapFromSource(srcIdx) == idx);
      if (m_proxyModel->hasChildren(idx))
        doTestMappings(idx);
    }
  }
}

void ProxyModelTest::testMappings()
{
  doTestMappings(QModelIndex());
}

void ProxyModelTest::verifyModel(const QModelIndex& parent, int start, int end)
{
  Q_UNUSED(start);
  Q_UNUSED(end);

  QVERIFY(parent.model() == m_proxyModel || !parent.isValid());
}

void ProxyModelTest::verifyModel(const QModelIndex& parent, int start, int end, const QModelIndex& destParent, int dest)
{
  Q_UNUSED(start);
  Q_UNUSED(end);
  Q_UNUSED(dest);

  QVERIFY(parent.model() == m_proxyModel || !parent.isValid());
  QVERIFY(destParent.model() == m_proxyModel || !destParent.isValid());
}

void ProxyModelTest::verifyModel(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  QVERIFY(topLeft.model() == m_proxyModel || !topLeft.isValid());
  QVERIFY(bottomRight.model() == m_proxyModel || !bottomRight.isValid());
}

void ProxyModelTest::setProxyModel(QAbstractProxyModel *proxyModel)
{
  if (m_proxyModel)
  {
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(layoutChanged()),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(testMappings()));
    disconnect(m_proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(testMappings()));

    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(m_proxyModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(m_proxyModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int)));
    disconnect(m_proxyModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(m_proxyModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(m_proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(verifyModel(const QModelIndex &, const QModelIndex &)));
  }

  m_proxyModel = proxyModel;
  testEmptyModel();
  m_proxyModel->setSourceModel(m_rootModel);

  connect(m_proxyModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(layoutAboutToBeChanged()),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(layoutChanged()),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
          SLOT(testMappings()));
  connect(m_proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          SLOT(testMappings()));

  connect(m_proxyModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
          SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_proxyModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
          SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_proxyModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
          SLOT(verifyModel(const QModelIndex &, int, int)));
  connect(m_proxyModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
          SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_proxyModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
          SLOT(verifyModel(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_proxyModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          SLOT(verifyModel(const QModelIndex &, const QModelIndex &)));

  testMappings();
  m_modelSpy->setModel(m_proxyModel);
}

QModelIndexList ProxyModelTest::getDescendantIndexes(const QModelIndex &parent)
{
  QModelIndexList list;
  const int column = 0;
  for(int row = 0; row < m_proxyModel->rowCount(parent); ++row)
  {
    QModelIndex idx = m_proxyModel->index(row, column, parent);
    list << idx;
    list << getDescendantIndexes(idx);
  }
  return list;
}

QList< QPersistentModelIndex > ProxyModelTest::toPersistent(QModelIndexList list)
{
  QList<QPersistentModelIndex > persistentList;
  foreach(QModelIndex idx, list)
  {
    persistentList << QPersistentModelIndex(idx);
  }
  return persistentList;
}


QModelIndexList ProxyModelTest::getUnchangedIndexes(const QModelIndex &parent, QList<QItemSelectionRange> ignoredRanges)
{
  QModelIndexList list;
  int rowCount = m_proxyModel->rowCount(parent);
  for (int row = 0; row < rowCount; )
  {
    int column = 0;
    QModelIndex idx = m_proxyModel->index( row, column, parent);
    bool found = false;
    foreach(QItemSelectionRange range, ignoredRanges)
    {
      if (range.topLeft().parent() == parent &&  range.topLeft().row() == idx.row())
      {
        row = range.bottomRight().row() + 1;
        found = true;
        break;
      }
    }
    if (!found)
    {
      for (column = 0; column < m_proxyModel->columnCount(); ++column )
        list << m_proxyModel->index( row, column, parent);
      list << getUnchangedIndexes(idx, ignoredRanges);
      ++row;
    }
  }
  return list;
}

void ProxyModelTest::testData()
{

}

void ProxyModelTest::doInit()
{
  // ProxyModelTest::init is a private slot and needs to remain so.
  // This method allows subclasses to initialize the class properly.
  ProxyModelTest::init();
}

void ProxyModelTest::doTest()
{
  QFETCH( SignalList, signalList );
  QFETCH( PersistentChangeList, changeList );

  QVERIFY(m_modelSpy->isEmpty());
  QList<QPersistentModelIndex> persistentIndexes;

  const int columnCount = m_rootModel->columnCount();
  QMutableListIterator<PersistentIndexChange> it(changeList);

  QString currentTest = QTest::currentDataTag();
  m_commandNames.removeAll(currentTest);

  // The indexes are defined by the test are described with IndexFinder before anything in the model exists.
  // Now that the indexes should exist, resolve them in the change objects.
  QList<QItemSelectionRange> changedRanges;
  while (it.hasNext())
  {
    PersistentIndexChange change = it.next();
    QModelIndex parent = change.parentFinder.getIndex();

    QVERIFY(change.startRow >= 0);
    QVERIFY(change.startRow <= change.endRow);
#if 0
    kDebug() << parent << change.startRow << change.endRow << parent.data() << m_proxyModel->rowCount(parent);
#endif
    Q_ASSERT(change.endRow < m_proxyModel->rowCount(parent));

    QModelIndex topLeft = m_proxyModel->index( change.startRow, 0, parent );
    QModelIndex bottomRight = m_proxyModel->index( change.endRow, columnCount - 1, parent );

    // We store the changed ranges so that we know which ranges should not be changed
    changedRanges << QItemSelectionRange(topLeft, bottomRight);

    // Store the inital state of the indexes in the model which we expect to change.
    for (int row = change.startRow; row <= change.endRow; ++row )
    {
      for (int column = 0; column < columnCount; ++column)
      {
        QModelIndex idx = m_proxyModel->index(row, column, parent);
        QVERIFY(idx.isValid());
        change.indexes << idx;
        change.persistentIndexes << QPersistentModelIndex(idx);
      }

      // Also store the descendants of changed indexes so that we can verify the effect on them
      QModelIndex idx = m_proxyModel->index(row, 0, parent);
      QModelIndexList descs = getDescendantIndexes(idx);
      change.descendantIndexes << descs;
      change.persistentDescendantIndexes << toPersistent(descs);
    }
    it.setValue(change);
  }

  // Any indexes outside of the ranges we expect to be changed are stored
  // so that we can later verify that they remain unchanged.
  QModelIndexList unchangedIndexes = getUnchangedIndexes(QModelIndex(), changedRanges);

  QList<QPersistentModelIndex> unchangedPersistentIndexes = toPersistent(unchangedIndexes);
  // Run the test.

  Q_ASSERT(m_modelSpy->isEmpty());
  m_modelSpy->startSpying();
  m_modelCommander->executeNextCommand();

  if (modelSpy()->isEmpty())
    QVERIFY(signalList.isEmpty());

  // Make sure we didn't get any signals we didn't expect.
  if (signalList.isEmpty())
    QVERIFY(modelSpy()->isEmpty());

  while (!signalList.isEmpty())
  {
    // Process each signal we recieved as a result of running the test.
    QVariantList expected = signalList.takeAt(0);
    handleSignal(expected);
  }

  // Make sure we didn't get any signals we didn't expect.
  QVERIFY(m_modelSpy->isEmpty());

  // Persistent indexes should change by the amount described in change objects.
  foreach (PersistentIndexChange change, changeList)
  {
    for (int i = 0; i < change.indexes.size(); i++)
    {
      QModelIndex idx = change.indexes.at(i);
      QPersistentModelIndex persistentIndex = change.persistentIndexes.at(i);

      // Persistent indexes go to an invalid state if they are removed from the model.
      if (change.toInvalid)
      {
        QVERIFY(!persistentIndex.isValid());
        continue;
      }
#if 0
      kDebug() << idx << idx.data() << change.difference << change.toInvalid << persistentIndex.row();
#endif

      QCOMPARE(idx.row() + change.difference, persistentIndex.row());
      QCOMPARE(idx.column(), persistentIndex.column());
      QCOMPARE(idx.parent(), persistentIndex.parent());
    }

    for (int i = 0; i < change.descendantIndexes.size(); i++)
    {
      QModelIndex idx = change.descendantIndexes.at(i);
      QPersistentModelIndex persistentIndex = change.persistentDescendantIndexes.at(i);

      // The descendant indexes of indexes which were removed should now also be invalid.
      if (change.toInvalid)
      {
        QVERIFY(!persistentIndex.isValid());
        continue;
      }
      // Otherwise they should be unchanged.
      QCOMPARE(idx.row(), persistentIndex.row());
      QCOMPARE(idx.column(), persistentIndex.column());
      QCOMPARE(idx.parent(), persistentIndex.parent());
    }
  }
  // Indexes unaffected by the signals should be unchanged.
  for (int i = 0; i < unchangedIndexes.size(); ++i)
  {
    QModelIndex unchangedIdx = unchangedIndexes.at(i);
    QPersistentModelIndex persistentIndex = unchangedPersistentIndexes.at(i);
    QCOMPARE(unchangedIdx.row(), persistentIndex.row());
    QCOMPARE(unchangedIdx.column(), persistentIndex.column());
    QCOMPARE(unchangedIdx.parent(), persistentIndex.parent());
  }
}

void ProxyModelTest::setCommands(QList<QPair<QString, ModelChangeCommandList> > commands)
{
  m_modelCommander->setCommands(commands);
}
