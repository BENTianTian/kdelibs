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

#include "kmwippselect.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "cupsinfos.h"
#include "ipprequest.h"

#include <klistwidget.h>
#include <QtGui/QLayout>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

KMWIppSelect::KMWIppSelect(QWidget *parent)
    : KMWizardPage(parent)
{
	m_ID = KMWizard::IPPSelect;
	m_title = i18n("Remote IPP Printer Selection");
	m_nextpage = KMWizard::Driver;

	m_list = new KListWidget(this);

	QVBoxLayout	*lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(0);
	lay->addWidget(m_list);
}

bool KMWIppSelect::isValid(QString& msg)
{
	if (m_list->currentRow() == -1)
	{
		msg = i18n("You must select a printer.");
		return false;
	}
	return true;
}

void KMWIppSelect::initPrinter(KMPrinter *p)
{
	// storage variables
	QString	host, login, password;
	int	port;

	// save config
	host = CupsInfos::self()->host();
	login = CupsInfos::self()->login();
	password = CupsInfos::self()->password();
	port = CupsInfos::self()->port();

	m_list->clear();

	// retrieve printer list
	KUrl	url = p->device();
	CupsInfos::self()->setHost(url.host());
	CupsInfos::self()->setLogin(url.user());
	CupsInfos::self()->setPassword(url.pass());
	CupsInfos::self()->setPort(url.port());
	IppRequest	req;
	QString		uri;
	req.setOperation(CUPS_GET_PRINTERS);
	uri = QString::fromLatin1("ipp://%1:%2/printers/").arg(url.host()).arg(url.port());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",QLatin1String("printer-name"));
	if (req.doRequest("/printers/"))
	{
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
			if (attr->name && strcmp(attr->name,"printer-name") == 0) {
				QListWidgetItem* item = new QListWidgetItem();
                item->setIcon(SmallIcon("kdeprint_printer"));
                item->setText(QLatin1String(attr->values[0].string.text)); 
                m_list->addItem(item);
            }
			attr = attr->next;
		}
		m_list->model()->sort(0);
	}

	// restore config
	CupsInfos::self()->setHost(host);
	CupsInfos::self()->setLogin(login);
	CupsInfos::self()->setPassword(password);
	CupsInfos::self()->setPort(port);
}

void KMWIppSelect::updatePrinter(KMPrinter *p)
{
	KUrl	url = p->device();
	QString	path = m_list->currentItem()->text();
	path.prepend("/printers/");
	url.setPath(path);
	p->setDevice(url.url());
kDebug(500) << url.url() << endl;
}
