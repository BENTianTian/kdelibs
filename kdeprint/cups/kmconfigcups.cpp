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

#include "kmconfigcups.h"
#include "kmcupsconfigwidget.h"

#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>

#include <QtGui/QLayout>

KMConfigCups::KMConfigCups(QWidget *parent)
    : KMConfigPage(parent )
{
        setObjectName( "ConfigCups" );
	setPageName(i18n("CUPS Server"));
	setPageHeader(i18n("CUPS Server Settings"));
	setPagePixmap("gear");

	m_widget = new KMCupsConfigWidget(this);

	QVBoxLayout *lay0 = new QVBoxLayout(this);
	lay0->setMargin(0);
	lay0->setSpacing(KDialog::spacingHint());
	lay0->addWidget(m_widget);
	lay0->addStretch(1);
}

void KMConfigCups::loadConfig(KConfig *)
{
	m_widget->load();
}

void KMConfigCups::saveConfig(KConfig *conf)
{
	m_widget->saveConfig(conf);
}
