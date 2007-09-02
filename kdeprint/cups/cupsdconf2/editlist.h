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

#ifndef EDITLIST_H
#define EDITLIST_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>

class KListWidget;
class QPushButton;

#ifdef __GNUC__
#warning rename class or remove from global namespace
#endif
class EditList : public QWidget
{
	Q_OBJECT

public:
	EditList(QWidget *parent = 0);

	QString text(int);
	void setText(int, const QString&);
	void insertItem(const QString&);
	void insertItem(const QPixmap&, const QString&);
	void insertItems(const QStringList&);
	QStringList items();
	void clear();

Q_SIGNALS:
	void add();
	void edit(int);
	void defaultList();
	void deleted(int);

protected Q_SLOTS:
	void slotDelete();
	void slotEdit();
	void slotSelected(int);

private:
	KListWidget	*list_;
	QPushButton	*addbtn_, *editbtn_, *delbtn_, *defbtn_;
};

#endif
