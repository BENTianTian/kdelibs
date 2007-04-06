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

#ifndef KPFILTERPAGE_H
#define KPFILTERPAGE_H

#include "kprintdialogpage.h"

#include <QtCore/QHash>
#include <QtCore/QStringList>

class QTreeWidget;
class KXmlCommand;
class QToolButton;
class QLabel;

class KPFilterPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPFilterPage(QWidget *parent = 0);
	~KPFilterPage();

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	bool isValid(QString& msg);

protected Q_SLOTS:
	void slotAddClicked();
	void slotRemoveClicked();
	void slotUpClicked();
	void slotDownClicked();
	void slotConfigureClicked();
	void slotItemSelected();

protected:
	KXmlCommand* currentFilter();
	void checkFilterChain();
	void updateInfo();
	QStringList activeList();
    void updateButton();
private:
	QTreeWidget		*m_view;
	QStringList		m_filters;	// <idname,description> pairs
	QHash<QString, KXmlCommand*>	m_activefilters;
	QToolButton		*m_add, *m_remove, *m_up, *m_down, *m_configure;
	bool			m_valid;
	QLabel			*m_info;
};

#endif
