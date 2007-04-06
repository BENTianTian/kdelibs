/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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
 **/

#ifndef KMDRIVERDB_H
#define KMDRIVERDB_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QString>

#include "kmdbentry.h"

class KMDBCreator;

class KMDriverDB : public QObject
{
	Q_OBJECT
public:
	static KMDriverDB* self();

	KMDriverDB(QObject *parent = 0);
	~KMDriverDB();

	void init(QWidget *parent = 0);
	KMDBEntryList* findEntry(const QString& manu, const QString& model);
	KMDBEntryList* findPnpEntry(const QString& manu, const QString& model);
	QHash<QString, KMDBEntryList*>* findModels(const QString& manu);
	const QHash<QString, QHash<QString, KMDBEntryList*>* >& manufacturers() const	{ return m_entries; }

protected:
	void loadDbFile();
	void insertEntry(KMDBEntry *entry);
	QString dbFile();

protected Q_SLOTS:
	void slotDbCreated();

Q_SIGNALS:
	void dbLoaded(bool reloaded);
	void error(const QString&);

private:
  void clear();

	KMDBCreator			*m_creator;
	QHash<QString, QHash<QString, KMDBEntryList*>* >	m_entries;
	QHash<QString, QHash<QString, KMDBEntryList*>* >	m_pnpentries;

	static KMDriverDB	*m_self;
};

#endif
