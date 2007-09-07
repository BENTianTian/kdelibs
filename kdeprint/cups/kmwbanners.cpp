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

#include "kmwbanners.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/QMap>
#include <klocale.h>

static QStringList defaultBanners()
{
    QStringList bans;
    QList<KMPrinter*> list = KMFactory::self()->manager()->printerList(false);
    QListIterator<KMPrinter*> it(list);
    KMPrinter *printer = 0;
    while (it.hasNext()) {
        printer = it.next();
        if (printer->isPrinter())
            break;
    }

    if (printer && KMFactory::self()->manager()->completePrinter(printer)) {
        QString s = list.first()->option("kde-banners-supported");
        bans = s.split(',', QString::SkipEmptyParts);
    }
    if (bans.count() == 0)
        bans.append("none");
    return bans;
}

static struct {
    const char *banner;
    const char *name;
} bannermap[] = {
    { "none", I18N_NOOP("No Banner") },
    { "classified", I18N_NOOP("Classified") },
    { "confidential", I18N_NOOP("Confidential") },
    { "secret", I18N_NOOP("Secret") },
    { "standard", I18N_NOOP("Standard") },
    { "topsecret", I18N_NOOP("Top Secret") },
    { "unclassified", I18N_NOOP("Unclassified") },
    { 0, 0 }
};

QString mapBanner(const QString& ban)
{
    static QMap<QString, QString> map;
    if (map.size() == 0)
        for (int i = 0; bannermap[ i ].banner; i++)
            map[ bannermap[ i ].banner ] = bannermap[ i ].name;
    QMap<QString, QString>::ConstIterator it = map.find(ban);
    if (it == map.end())
        return ban;
    else
        return it.value();
}

//**************************************************************************************************************

KMWBanners::KMWBanners(QWidget *parent)
        : KMWizardPage(parent)
{
    m_ID = KMWizard::Banners;
    m_title = i18n("Banner Selection");
    m_nextpage = KMWizard::Custom + 3;

    m_start = new QComboBox(this);
    m_end = new QComboBox(this);

    QLabel *l1 = new QLabel(i18n("&Starting banner:"), this);
    QLabel *l2 = new QLabel(i18n("&Ending banner:"), this);

    l1->setBuddy(m_start);
    l2->setBuddy(m_end);

    QLabel *l0 = new QLabel(this);
    l0->setWordWrap(true);
    l0->setText(i18n("<p>Select the default banners associated with this printer. These "
                     "banners will be inserted before and/or after each print job sent "
                     "to the printer. If you don't want to use banners, select <b>No Banner</b>.</p>"));

    QGridLayout *lay = new QGridLayout(this);
    lay->setMargin(0);
    lay->setSpacing(10);
    lay->setColumnStretch(1, 1);
    lay->addItem(new QSpacerItem(0, 20), 1, 0);
    lay->setRowStretch(4, 1);
    lay->addWidget(l0, 0, 0, 1, 2);
    lay->addWidget(l1, 2, 0);
    lay->addWidget(l2, 3, 0);
    lay->addWidget(m_start, 2, 1);
    lay->addWidget(m_end, 3, 1);
}

void KMWBanners::initPrinter(KMPrinter *p)
{
    if (p) {
        if (m_start->count() == 0) {
            m_bans = p->option("kde-banners-supported").split(',', QString::SkipEmptyParts);
            if (m_bans.count() == 0)
                m_bans = defaultBanners();
            if (!m_bans.contains("none"))
                m_bans.prepend("none");
            for (QStringList::Iterator it = m_bans.begin(); it != m_bans.end(); ++it) {
                m_start->addItem(i18n(mapBanner(*it).toUtf8()));
                m_end->addItem(i18n(mapBanner(*it).toUtf8()));
            }
        }
        QStringList l = p->option("kde-banners").split(',', QString::SkipEmptyParts);
        while (l.count() < 2)
            l.append("none");
        m_start->setCurrentIndex(m_bans.indexOf(l[0]));
        m_end->setCurrentIndex(m_bans.indexOf(l[1]));
    }
}

void KMWBanners::updatePrinter(KMPrinter *p)
{
    if (m_start->count() > 0) {
        p->setOption("kde-banners", m_bans[m_start->currentIndex()] + ',' + m_bans[m_end->currentIndex()]);
    }
}
