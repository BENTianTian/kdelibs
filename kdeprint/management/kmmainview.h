/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KMMAINVIEW_H
#define KMMAINVIEW_H

#include <qwidget.h>
#include <kdeprint/kpreloadobject.h>
#include <kmainwindow.h>

class KMManager;
class KMPrinterView;
class KMPrinter;
class KMPages;
class KActionCollection;
class QPopupMenu;
class QTimer;
class QSplitter;
class KToolBar;
class KAction;
class PluginComboBox;

class KMMainView : public KMainWindow, public KPReloadObject
{
	Q_OBJECT
public:
	KMMainView(QWidget *parent = 0, const char *name = 0, KActionCollection *coll = 0);
	~KMMainView();

	void setOrientation(int);
	int orientation() const;
	void setViewType(int);
	int viewType() const;
	void enableToolbar(bool on = true);
	KAction* action(const char *name);
	void showPrinterInfos(bool on);
	bool printerInfosShown() const;

public slots:
	void slotTimer();
	void slotShowPrinterInfos(bool);
	void slotEnable();
	void slotDisable();
	void slotRemove();
	void slotConfigure();
	void slotAdd();
	void slotHardDefault();
	void slotSoftDefault();
	void slotTest();
	void slotServerRestart();
	void slotServerConfigure();
	void slotManagerConfigure();
	void slotAddSpecial();
	void slotRefresh();

protected slots:
	void slotPrinterSelected(KMPrinter*);
	void slotRightButtonClicked(KMPrinter*, const QPoint&);
	void slotToggleToolBar(bool);
	void slotChangeView(int);
	void slotChangeDirection(int);

protected:
	void initActions();
	void showErrorMsg(const QString& msg, bool usemgr = true);
	void restoreSettings();
	void saveSettings();
	void loadParameters();
	void reload();
	void aboutToReload();
	void loadPluginActions();
	void removePluginActions();

private:
	KMPrinterView	*m_printerview;
	KMPages		*m_printerpages;
	QPopupMenu	*m_pop;
	KActionCollection	*m_actions;
	KMPrinter	*m_current;
	QSplitter	*m_splitter;
	KToolBar	*m_toolbar;
	PluginComboBox	*m_plugin;
	int			m_pactionsindex;
};

#endif
