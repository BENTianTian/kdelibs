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

#ifndef KMPRINTERVIEW_H
#define KMPRINTERVIEW_H

#include <QStackedWidget>
#include <QList>

class KMIconView;
class KMListView;
class KMPrinter;

class KMPrinterView : public QStackedWidget
{
	Q_OBJECT
public:
	enum ViewType { Icons = 0, List, Tree };

	explicit KMPrinterView(QWidget *parent = 0, const char *name = 0);
	~KMPrinterView();

	void setPrinterList(QList<KMPrinter*> *list);
	void setPrinter( KMPrinter* );
	void setViewType(ViewType t);
	ViewType viewType() const 	{ return m_type; }

	QSize minimumSizeHint() const;

Q_SIGNALS:
	void printerSelected(const QString&);
	void rightButtonClicked(const QString&, const QPoint&);

protected Q_SLOTS:
	void slotPrinterSelected(const QString&);

private:
	KMIconView	*m_iconview;
	KMListView	*m_listview;
	ViewType	m_type;
	QString		m_current;
	bool m_listset;
};

#endif
