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

#include "kmconfigjobs.h"

#include <QtGui/QGroupBox>
#include <QtGui/QBoxLayout>
#include <QtGui/QLayout>

#include <knuminput.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>

KMConfigJobs::KMConfigJobs(QWidget *parent)
        : KMConfigPage(parent)
{
    setPageName(i18n("Jobs"));
    setPageHeader(i18n("Print Job Settings"));
    setPagePixmap("exec");

    QGroupBox *box = new QGroupBox(i18n("Jobs Shown"), this);
    box->setLayout(new QVBoxLayout);

    m_limit = new KIntNumInput(box);
    m_limit->setRange(0, 9999, 1, true);
    m_limit->setSpecialValueText(i18n("Unlimited"));
    m_limit->setLabel(i18n("Maximum number of jobs shown:"));

    QVBoxLayout *l0 = new QVBoxLayout(this);
    l0->setMargin(0);
    l0->setSpacing(KDialog::spacingHint());
    l0->addWidget(box, 0);
    l0->addStretch(1);
    QVBoxLayout *l1 = new QVBoxLayout();
    l1->setSpacing(KDialog::spacingHint());
    box->layout()->addItem(l1);
    l1->addWidget(m_limit);
}

void KMConfigJobs::loadConfig(KConfig *conf)
{
    KConfigGroup cg = conf->group("Jobs");
    m_limit->setValue(cg.readEntry("Limit", 0));
}

void KMConfigJobs::saveConfig(KConfig *conf)
{
    KConfigGroup cg = conf->group("Jobs");
    cg.writeEntry("Limit", m_limit->value());
}

#include "kmconfigjobs.moc"
