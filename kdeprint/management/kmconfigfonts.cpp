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

#include "kmconfigfonts.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <kpushbutton.h>
#include <QtGui/QLayout>
#include <q3header.h>
#include <QtGui/QLabel>
#include <qcheckbox.h>
#include <qsettings.h>

#include <klocale.h>
#include <kconfig.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <kfile.h>
#include <k3listview.h>
#include <kdialog.h>

KMConfigFonts::KMConfigFonts(QWidget *parent)
    : KMConfigPage(parent)
{
	setPageName(i18n("Fonts"));
	setPageHeader(i18n("Font Settings"));
	setPagePixmap("fonts");

	QGroupBox	*box = new QGroupBox(i18n("Fonts Embedding"), this);
	box->setLayout( new QVBoxLayout );
	QGroupBox	*box2 = new QGroupBox(i18n("Fonts Path"), this);
	box2->setLayout( new QVBoxLayout );

	m_embedfonts = new QCheckBox(i18n("&Embed fonts in PostScript data when printing"), box);
	m_fontpath = new K3ListView(box2);
	m_fontpath->addColumn("");
	m_fontpath->header()->setStretchEnabled(true, 0);
	m_fontpath->header()->hide();
	m_fontpath->setSorting(-1);
	m_addpath = new KUrlRequester(box2);
	m_addpath->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
	m_up = new KPushButton(KGuiItem(i18n("&Up"), "go-up"), box2);
	m_down = new KPushButton(KGuiItem(i18n("&Down"), "go-down"), box2);
	m_add = new KPushButton(KGuiItem(i18n("&Add"), "list-add"), box2);
	m_remove = new KPushButton(KGuiItem(i18n("&Remove"), "edit-delete"), box2);
	QLabel	*lab0 = new QLabel(i18n("Additional director&y:"), box2);
	lab0->setBuddy(m_addpath);

	QVBoxLayout	*l0 = new QVBoxLayout();
  l0->setSpacing(KDialog::spacingHint());
  box->layout()->addItem(l0);
	l0->addWidget(m_embedfonts);
	QVBoxLayout	*l1 = new QVBoxLayout();
  l1->setSpacing(KDialog::spacingHint());
  box2->layout()->addItem(l1);
	l1->addWidget(m_fontpath);
	QHBoxLayout	*l2 = new QHBoxLayout();
  l2->setSpacing(KDialog::spacingHint());
	l1->addLayout(l2);
	l2->addWidget(m_up);
	l2->addWidget(m_down);
	l2->addWidget(m_remove);
	l1->addSpacing(10);
	l1->addWidget(lab0);
	l1->addWidget(m_addpath);
	QHBoxLayout	*l3 = new QHBoxLayout();
  l3->setSpacing(KDialog::spacingHint());
	l1->addLayout(l3);
	l3->addStretch(1);
	l3->addWidget(m_add);
	QVBoxLayout	*l4 = new QVBoxLayout(this);
	l4->setMargin(0);
	l4->setSpacing(KDialog::spacingHint());
	l4->addWidget(box);
	l4->addWidget(box2);

	m_embedfonts->setWhatsThis(			i18n("These options will automatically put fonts in the PostScript file "
                             "which are not present on the printer. Font embedding usually produces better print results "
			     "(closer to what you see on the screen), but larger print data as well."));
	m_fontpath->setWhatsThis(			i18n("When using font embedding you can select additional directories where "
			     "KDE should search for embeddable font files. By default, the X server "
			     "font path is used, so adding those directories is not needed. The default "
			     "search path should be sufficient in most cases."));

	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemove()));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAdd()));
	connect(m_up, SIGNAL(clicked()), SLOT(slotUp()));
	connect(m_down, SIGNAL(clicked()), SLOT(slotDown()));
	connect(m_fontpath, SIGNAL(selectionChanged()), SLOT(slotSelected()));
	connect(m_addpath, SIGNAL(textChanged(const QString&)), SLOT(slotTextChanged(const QString&)));
	m_add->setEnabled(false);
	m_remove->setEnabled(false);
	m_up->setEnabled(false);
	m_down->setEnabled(false);
}

void KMConfigFonts::loadConfig(KConfig *)
{
	QSettings	settings;
	m_embedfonts->setChecked(settings.value("/qt/embedFonts", true).toBool());
	QStringList	paths = settings.value("/qt/fontPath", ':').toStringList();
	Q3ListViewItem	*item(0);
	for (QStringList::ConstIterator it=paths.begin(); it!=paths.end(); ++it)
		item = new Q3ListViewItem(m_fontpath, item, *it);
}

void KMConfigFonts::saveConfig(KConfig *)
{
	QSettings	settings;
	settings.setValue("/qt/embedFonts", m_embedfonts->isChecked());
	QStringList	l;
	Q3ListViewItem	*item = m_fontpath->firstChild();
	while (item)
	{
		l << item->text(0);
		item = item->nextSibling();
	}
	settings.setValue("/qt/fontPath", l);
}

void KMConfigFonts::slotSelected()
{
	Q3ListViewItem	*item = m_fontpath->selectedItem();
	m_remove->setEnabled(item);
	m_up->setEnabled(item && item->itemAbove());
	m_down->setEnabled(item && item->itemBelow());
}

void KMConfigFonts::slotAdd()
{
	if (m_addpath->url().isEmpty())
		return;
	Q3ListViewItem	*lastItem(m_fontpath->firstChild());
	while (lastItem && lastItem->nextSibling())
		lastItem = lastItem->nextSibling();
	Q3ListViewItem	*item = new Q3ListViewItem(m_fontpath, lastItem, m_addpath->url().path());
	m_fontpath->setSelected(item, true);
}

void KMConfigFonts::slotRemove()
{
	delete m_fontpath->selectedItem();
	if (m_fontpath->currentItem())
		m_fontpath->setSelected(m_fontpath->currentItem(), true);
	slotSelected();
}

void KMConfigFonts::slotUp()
{
	Q3ListViewItem	*citem = m_fontpath->selectedItem(), *nitem = 0;
	if (!citem || !citem->itemAbove())
		return;
	nitem = new Q3ListViewItem(m_fontpath, citem->itemAbove()->itemAbove(), citem->text(0));
	delete citem;
	m_fontpath->setSelected(nitem, true);
}

void KMConfigFonts::slotDown()
{
	Q3ListViewItem	*citem = m_fontpath->selectedItem(), *nitem = 0;
	if (!citem || !citem->itemBelow())
		return;
	nitem = new Q3ListViewItem(m_fontpath, citem->itemBelow(), citem->text(0));
	delete citem;
	m_fontpath->setSelected(nitem, true);
}

void KMConfigFonts::slotTextChanged(const QString& t)
{
	m_add->setEnabled(!t.isEmpty());
}

#include "kmconfigfonts.moc"
