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

#ifndef KMDBCREATOR_H
#define KMDBCREATOR_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <qdatetime.h>
#include <k3process.h>

class QWidget;
class QProgressDialog;

class KMDBCreator : public QObject
{
	Q_OBJECT
public:
	KMDBCreator(QObject *parent = 0);
	~KMDBCreator();

	bool checkDriverDB(const QString& dirname, const QDateTime& d);
	bool createDriverDB(const QString& dirname, const QString& filename, QWidget *parent = 0);
	bool status() const	{ return m_status; }

protected Q_SLOTS:
	void slotReceivedStdout(K3Process *p, char *bufm, int len);
	void slotReceivedStderr(K3Process *p, char *bufm, int len);
	void slotProcessExited(K3Process *p);
	void slotCancelled();

Q_SIGNALS:
	void dbCreated();

private:
	K3Process	m_proc;
	QProgressDialog	*m_dlg;
	bool		m_status;
	bool		m_firstflag;
};

#endif
