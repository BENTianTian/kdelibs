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

#ifndef PROXY_MODEL_TEST_H
#define PROXY_MODEL_TEST_H

#include <QtTest>
#include <QtCore>
#include <qtest_kde.h>
#include <qtestevent.h>
#include <QItemSelectionRange>
#include <QAbstractProxyModel>

#include "dynamictreemodel.h"
#include "indexfinder.h"
#include "modelcommander.h"
#include "modelspy.h"
#include "persistentchangelist.h"

typedef QList<ModelChangeCommand*> CommandList;

Q_DECLARE_METATYPE( CommandList )

typedef QList<QVariantList> SignalList;

Q_DECLARE_METATYPE( SignalList )

class ProxyModelTest : public QObject
{
  Q_OBJECT
public:
  ProxyModelTest(QObject *parent = 0);

  void setProxyModel(QAbstractProxyModel *proxyModel);
  DynamicTreeModel* sourceModel();

  ModelSpy* modelSpy() const { return m_modelSpy; }

protected:
  virtual void doInitTestCase();
  virtual void doInit();
  virtual void testData();
  virtual void doCleanupTestCase();

  void testEmptyModel();
  void doTestMappings(const QModelIndex &parent);

protected slots:
  void testMappings();
  void verifyModel(const QModelIndex &parent, int start, int end);
  void verifyModel(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int dest);
  void verifyModel(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private slots:
  void init();

  void testProxyModel_data() { testData(); }
  void testProxyModel() { doTest(); }

protected:
  QModelIndexList getUnchangedIndexes(const QModelIndex &parent, QList< QItemSelectionRange > ignoredRanges);

  QModelIndexList getDescendantIndexes(const QModelIndex &index);
  QList< QPersistentModelIndex > toPersistent(QModelIndexList list);

  PersistentIndexChange getChange(IndexFinder sourceFinder, int start, int end, int difference, bool toInvalid = false);
  QVariantList getSignal(SignalType type, IndexFinder parentFinder, int start, int end);

  void doTest();
  void handleSignal(QVariantList expected);
  QVariantList getResultSignal();
  int getChange(bool sameParent, int start, int end, int currentPosition, int destinationStart);

  void setCommands(QList<QPair<QString, ModelChangeCommandList> >  commands);

private:
  DynamicTreeModel *m_rootModel;
  QAbstractProxyModel *m_proxyModel;
  ModelSpy *m_modelSpy;
  ModelCommander *m_modelCommander;
  QStringList m_commandNames;

};

#endif
