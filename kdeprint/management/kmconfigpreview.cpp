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

#include "kmconfigpreview.h"

#include <qcheckbox.h>
#include <QtGui/QLayout>
#include <QGroupBox>
#include <QtGui/QLabel>

#include <klocale.h>
#include <kurlrequester.h>
#include <kconfig.h>
#include <kdialog.h>

KMConfigPreview::KMConfigPreview(QWidget *parent)
    : KMConfigPage(parent)
{
	setPageName(i18n("Preview"));
	setPageHeader(i18n("Preview Settings"));
	setPagePixmap("filefind");

	QGroupBox *box = new QGroupBox(i18n("Preview Program"), this);
	box->setLayout( new QVBoxLayout );

	m_useext = new QCheckBox(i18n("&Use external preview program"), box);
	m_program = new KUrlRequester(box);
	QLabel	*lab = new QLabel(box);
	lab->setText(i18n("You can use an external preview program (PS viewer) instead of the "
					  "KDE built-in preview system. Note that if the KDE default PS viewer "
					  "(KGhostView) cannot be found, KDE tries automatically to find another "
					  "external PostScript viewer"));
	lab->setTextFormat(Qt::RichText);
	lab->setWordWrap(true);

	QVBoxLayout	*l0 = new QVBoxLayout(this);
	l0->setMargin(0);
	l0->setSpacing(KDialog::spacingHint());
	l0->addWidget(box);
	l0->addStretch(1);
	QVBoxLayout	*l1 = new QVBoxLayout();
  l1->setSpacing(KDialog::spacingHint());
  box->layout()->addItem(l1);
	l1->addWidget(lab);
	l1->addWidget(m_useext);
	l1->addWidget(m_program);

	connect(m_useext, SIGNAL(toggled(bool)), m_program, SLOT(setEnabled(bool)));
	m_program->setEnabled(false);
}

void KMConfigPreview::loadConfig(KConfig *conf)
{
	KConfigGroup cg( conf, "General");
	m_useext->setChecked(cg.readEntry("ExternalPreview", false));
	m_program->setUrl(KUrl(cg.readPathEntry("PreviewCommand", "gv")));
}

void KMConfigPreview::saveConfig(KConfig *conf)
{
	KConfigGroup cg( conf, "General");
	cg.writeEntry("ExternalPreview", m_useext->isChecked());
	cg.writePathEntry("PreviewCommand", m_program->url().url());
}
