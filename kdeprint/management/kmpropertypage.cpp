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

#include "kmpropertypage.h"
#include "kmpropwidget.h"
#include "kmpropcontainer.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmuimanager.h"

#include "kmpropgeneral.h"

#include <kiconloader.h>

KMPropertyPage::KMPropertyPage(QWidget *parent)
        : CJanusWidget(parent)
{
    initialize();
}

KMPropertyPage::~KMPropertyPage()
{
}

void KMPropertyPage::setPrinter(KMPrinter *p)
{
    QListIterator<KMPropWidget*> it(m_widgets);
    while (it.hasNext())
        it.next()->setPrinterBase(p);
}

void KMPropertyPage::addPropPage(KMPropWidget *w)
{
    if (w) {
        m_widgets.append(w);
        KMPropContainer *ctn = new KMPropContainer(this);
        ctn->setObjectName("Container");
        ctn->setWidget(w);
        connect(ctn, SIGNAL(enable(bool)), SLOT(slotEnable(bool)));

        QPixmap icon = KIconLoader::global()->loadIcon(
                           w->pixmap(),
                           K3Icon::NoGroup,
                           K3Icon::SizeMedium
                       );
        addPage(ctn, w->title(), w->header(), icon);
    }
}

void KMPropertyPage::slotEnable(bool on)
{
    QWidget *w = (QWidget*)(sender());
    if (on)
        enablePage(w);
    else
        disablePage(w);
}

void KMPropertyPage::initialize()
{
    // add General page
    KMPropGeneral *mp = new KMPropGeneral(this);
    mp->setObjectName("General");
    addPropPage(mp);
    // add plugin specific pages
    KMFactory::self()->uiManager()->setupPropertyPages(this);
}

void KMPropertyPage::reload()
{
    clearPages();
    m_widgets.clear();
    initialize();
    setPrinter(0);
}

#include "kmpropertypage.moc"
