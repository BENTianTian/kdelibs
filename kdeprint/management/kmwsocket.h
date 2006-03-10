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

#ifndef	KMWSOCKET_H
#define	KMWSOCKET_H

#include "kmwizardpage.h"

class K3ListView;
class Q3ListViewItem;
class QLineEdit;
class NetworkScanner;

class KMWSocket : public KMWizardPage
{
	Q_OBJECT
public:
	KMWSocket(QWidget *parent = 0);
	~KMWSocket();

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);

protected Q_SLOTS:
	void slotPrinterSelected(Q3ListViewItem*);
	void slotScanStarted();
	void slotScanFinished();

private:
	K3ListView	*m_list;
	NetworkScanner *m_scanner;
	QLineEdit	*m_printer, *m_port;
};

#endif
