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

#ifndef KMICONVIEW_H
#define KMICONVIEW_H

#include <k3iconview.h>
#include <QList>

#include "kmobject.h"

class KMPrinter;

class KMIconViewItem : public Q3IconViewItem, public KMObject
{
public:
	KMIconViewItem(Q3IconView *parent, KMPrinter *p);
	void updatePrinter(KMPrinter *printer = 0, int mode = Qt::DockBottom);
	bool isClass() const	{ return m_isclass; }

protected:
	virtual void paintItem(QPainter*, const QColorGroup&);

private:
	int		m_mode;
	QString		m_pixmap;
	char		m_state;
	bool		m_isclass;
};

class KMIconView : public K3IconView
{
	Q_OBJECT
public:
	enum ViewMode { Big, Small };

	KMIconView(QWidget *parent = 0, const char *name = 0);
	~KMIconView();

	void setPrinterList(QList<KMPrinter*> *list);
	void setPrinter(const QString&);
	void setPrinter(KMPrinter*);
	void setViewMode(ViewMode);

Q_SIGNALS:
	void rightButtonClicked(const QString&, const QPoint&);
	void printerSelected(const QString&);

protected Q_SLOTS:
	void slotRightButtonClicked(Q3IconViewItem*, const QPoint&);
	void slotSelectionChanged();

private:
	KMIconViewItem* findItem(KMPrinter *p);

private:
	QList<KMIconViewItem*>	m_items;
	ViewMode		m_mode;
};

#endif
