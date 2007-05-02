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

#include "kmconfiggeneral.h"

#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

#include <kpushbutton.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <krun.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <klineedit.h>
#include <kguiitem.h>
#include <kdialog.h>
#include <kmimetype.h>

KMConfigGeneral::KMConfigGeneral(QWidget *parent)
    : KMConfigPage(parent)
{
    setObjectName( "ConfigTimer" );
	setPageName(i18n("General"));
	setPageHeader(i18n("General Settings"));
	setPagePixmap("document-print");

	QGroupBox	*m_timerbox = new QGroupBox(i18n("Refresh Interval"), this);
	m_timerbox->setLayout( new QVBoxLayout );
	m_timer = new KIntNumInput(m_timerbox/*,"Timer"*/);
	m_timer->setRange(0,30);
    m_timer->setSuffix( i18n( " sec" ) );
	m_timer->setSpecialValueText(i18n("Disabled"));
	m_timer->setWhatsThis(i18n("This time setting controls the refresh rate of various "
			              "<b>KDE Print</b> components like the print manager "
				      "and the job viewer."));

	QGroupBox	*m_testpagebox = new QGroupBox(i18n("Test Page"), this);
	m_testpagebox->setLayout( new QVBoxLayout );
	m_defaulttestpage = new QCheckBox(i18n("&Specify personal test page"), m_testpagebox);
	m_defaulttestpage->setObjectName(QLatin1String("TestPageCheck"));
	m_testpage = new KUrlRequester(m_testpagebox);
	m_testpage->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
	m_preview = new KPushButton(KGuiItem(i18n("Preview..."), "file-find"), m_testpagebox);
	connect(m_defaulttestpage,SIGNAL(toggled(bool)),m_testpage,SLOT(setEnabled(bool)));
	connect(m_defaulttestpage,SIGNAL(toggled(bool)),this,SLOT(setEnabledPreviewButton(bool)));
	connect(m_preview,SIGNAL(clicked()),SLOT(slotTestPagePreview()));
        connect(m_testpage->lineEdit(),SIGNAL(textChanged ( const QString & )),this,SLOT(testPageChanged(const QString & )));
	m_testpage->setDisabled(true);
	m_preview->setDisabled(true);
	m_defaulttestpage->setCursor(QCursor(Qt::PointingHandCursor));

	QGroupBox	*m_statusbox = new QGroupBox(i18n("Miscellaneous"), this);
	m_statusbox->setLayout( new QVBoxLayout );
	m_statusmsg = new QCheckBox(i18n("Sho&w printing status message box"), m_statusbox);
	m_uselast = new QCheckBox(i18n("De&faults to the last printer used in the application"), m_statusbox);

	//layout
	QVBoxLayout	*lay0 = new QVBoxLayout(this);
	lay0->setMargin(0);
	lay0->setSpacing(KDialog::spacingHint());
	lay0->addWidget(m_timerbox);
	lay0->addWidget(m_testpagebox);
	lay0->addWidget(m_statusbox);
	lay0->addStretch(1);
	QVBoxLayout	*lay1 = new QVBoxLayout();
  lay1->setSpacing(KDialog::spacingHint());
  m_timerbox->layout()->addItem(lay1);
	lay1->addWidget(m_timer);
	QVBoxLayout	*lay2 = new QVBoxLayout();
  lay2->setSpacing(KDialog::spacingHint());
  m_testpagebox->layout()->addItem(lay2);
	QHBoxLayout	*lay3 = new QHBoxLayout();
  lay3->setMargin(0);
  lay3->setSpacing(0);
	lay2->addWidget(m_defaulttestpage);
	lay2->addWidget(m_testpage);
	lay2->addLayout(lay3);
	lay3->addStretch(1);
	lay3->addWidget(m_preview);
	QVBoxLayout	*lay4 = new QVBoxLayout();
  lay4->setSpacing(KDialog::spacingHint());
  m_statusbox->layout()->addItem(lay4);
	lay4->addWidget(m_statusmsg);
	lay4->addWidget(m_uselast);
	m_preview->setEnabled( !m_testpage->url().isEmpty());
}

void KMConfigGeneral::testPageChanged(const QString &test )
{
    m_preview->setEnabled( !test.isEmpty());
}

void KMConfigGeneral::setEnabledPreviewButton(bool b)
{
    m_preview->setEnabled(!m_testpage->url().isEmpty() && b);
}

void KMConfigGeneral::loadConfig(KConfig *_conf)
{
	KConfigGroup conf( _conf, "General");
	m_timer->setValue(conf.readEntry("TimerDelay", 5));
	QString	tpage = conf.readPathEntry("TestPage");
	if (!tpage.isEmpty())
	{
		m_defaulttestpage->setChecked(true);
		m_testpage->setUrl(KUrl(tpage));
	}
	m_statusmsg->setChecked(conf.readEntry("ShowStatusMsg", true));
	m_uselast->setChecked(conf.readEntry("UseLast", true));
}

void KMConfigGeneral::saveConfig(KConfig *_conf)
{
	KConfigGroup conf( _conf, "General");
	conf.writeEntry("TimerDelay",m_timer->value());
	conf.writePathEntry("TestPage",(m_defaulttestpage->isChecked() ? m_testpage->url().url() : QString()));
	if (m_defaulttestpage->isChecked() &&
		KMimeType::findByFileContent(m_testpage->url().path())->name() != "application/postscript")
		KMessageBox::sorry(this, i18n("The selected test page is not a PostScript file. You may not "
		                              "be able to test your printer anymore."));
	conf.writeEntry("ShowStatusMsg", m_statusmsg->isChecked());
	conf.writeEntry("UseLast", m_uselast->isChecked());
}

void KMConfigGeneral::slotTestPagePreview()
{
	KUrl tpage = m_testpage->url();
	if (tpage.isEmpty())
		KMessageBox::error(this, i18n("Empty file name."));
	else
		(void)new KRun( tpage, this );
}

#include "kmconfiggeneral.moc"
