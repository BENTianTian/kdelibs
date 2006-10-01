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

#ifndef CJANUSWIDGET_H
#define CJANUSWIDGET_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qlist.h>

class QStackedWidget;
class QLabel;
class Q3ListBoxItem;

class CJanusWidget : public QWidget
{
	Q_OBJECT
public:
	explicit CJanusWidget(QWidget *parent);
	~CJanusWidget();

	void addPage(QWidget *w, const QString& text, const QString& header, const QPixmap& pix);
	void enablePage(QWidget *w);
	void disablePage(QWidget *w);
	void clearPages();

protected Q_SLOTS:
	void slotSelected(Q3ListBoxItem*);

public:
	class CPage;
	class CListBox;
	class CListBoxItem;

private:
	CPage* findPage(QWidget *w);
	CPage* findPage(Q3ListBoxItem *i);
	Q3ListBoxItem* findPrevItem(CPage*);

private:
	QList<CPage*>		m_pages;
	CListBox		*m_iconlist;
	QLabel			*m_header;
	QStackedWidget		*m_stack;
	QWidget			*m_empty;
};

#endif
