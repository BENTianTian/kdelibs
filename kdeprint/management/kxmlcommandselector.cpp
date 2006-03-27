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

#include "kxmlcommandselector.h"
#include "kxmlcommand.h"
#include "kxmlcommanddlg.h"
#include "kdeprintcheck.h"

#include <qcombobox.h>
#include <kpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kseparator.h>
#include <kguiitem.h>
#include <kactivelabel.h>
#include <kdatetable.h>
#include <kdialogbase.h>

KXmlCommandSelector::KXmlCommandSelector(bool canBeNull, QWidget *parent, KDialogBase *dlg)
    : QWidget(parent)
{
	m_cmd = new QComboBox(this);
	connect(m_cmd, SIGNAL(activated(int)), SLOT(slotCommandSelected(int)));
	QPushButton	*m_add = new KPushButton(this);
	QPushButton	*m_edit = new KPushButton(this);
	m_add->setPixmap(SmallIcon("filenew"));
	m_edit->setPixmap(SmallIcon("configure"));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAddCommand()));
	connect(m_edit, SIGNAL(clicked()), SLOT(slotEditCommand()));
	m_add->setToolTip( i18n("New command"));
	m_edit->setToolTip( i18n("Edit command"));
	m_shortinfo = new QLabel(this);
	m_helpbtn = new KPushButton( this );
	m_helpbtn->setIcon( SmallIconSet( "help" ) );
	connect( m_helpbtn, SIGNAL( clicked() ), SLOT( slotHelpCommand() ) );
	m_helpbtn->setToolTip( i18n( "Information" ) );
        m_helpbtn->setEnabled( false );

	m_line = 0;
	m_usefilter = 0;
	QPushButton	*m_browse = 0;

	QVBoxLayout	*l0 = new QVBoxLayout(this);
	l0->setMargin(0);
	l0->setSpacing(10);

	if (canBeNull)
	{
		m_line = new QLineEdit(this);
		m_browse = new KPushButton(KGuiItem(i18n("&Browse..."), "fileopen"), this);
		m_usefilter = new QCheckBox(i18n("Use co&mmand:"), this);
		connect(m_browse, SIGNAL(clicked()), SLOT(slotBrowse()));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_line, SLOT(setDisabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_browse, SLOT(setDisabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_cmd, SLOT(setEnabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_add, SLOT(setEnabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_edit, SLOT(setEnabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_shortinfo, SLOT(setEnabled(bool)));
		connect( m_usefilter, SIGNAL( toggled( bool ) ), SLOT( slotXmlCommandToggled( bool ) ) );
		m_usefilter->setChecked(true);
		m_usefilter->setChecked(false);
		//setFocusProxy(m_line);
		setTabOrder(m_usefilter, m_cmd);
		setTabOrder(m_cmd, m_add);
		setTabOrder(m_add, m_edit);

		QHBoxLayout	*l1 = new QHBoxLayout(0, 0, 10);
		l0->addLayout(l1);
		l1->addWidget(m_line);
		l1->addWidget(m_browse);

		KSeparator	*sep = new KSeparator(Qt::Horizontal, this);
		l0->addWidget(sep);
	}
	else
		setFocusProxy(m_cmd);

	QGridLayout	*l2 = new QGridLayout();
    l2->setMargin(0);
    l2->setSpacing(5);
	int	c(0);
	l0->addLayout(l2);
	if (m_usefilter)
	{
		l2->addWidget(m_usefilter, 0, c++);
	}
	l2->addWidget(m_cmd, 0, c);
	QHBoxLayout *l4 = new QHBoxLayout( 0, 0, 5 );
	l2->addLayout( l4, 1, c );
	l4->addWidget( m_helpbtn, 0 );
	l4->addWidget( m_shortinfo, 1 );
	QHBoxLayout	*l3 = new QHBoxLayout(0, 0, 0);
	l2->addLayout(l3, 0, c+1);
	l3->addWidget(m_add);
	l3->addWidget(m_edit);

	if ( dlg )
		connect( this, SIGNAL( commandValid( bool ) ), dlg, SLOT( enableButtonOK( bool ) ) );

	loadCommands();
}

void KXmlCommandSelector::loadCommands()
{
	QString	thisCmd = (m_cmd->currentIndex() != -1 ? m_cmdlist[m_cmd->currentIndex()] : QString());

	m_cmd->clear();
	m_cmdlist.clear();

	QStringList	list = KXmlCommandManager::self()->commandListWithDescription();
	QStringList	desclist;
	for (QStringList::Iterator it=list.begin(); it!=list.end(); ++it)
	{
		m_cmdlist << (*it);
		++it;
		desclist << (*it);
	}
	m_cmd->insertStringList(desclist);

	int	index = m_cmdlist.indexOf(thisCmd);
	if (index != -1)
		m_cmd->setCurrentItem(index);
	if (m_cmd->currentIndex() != -1 && m_cmd->isEnabled())
		slotCommandSelected(m_cmd->currentIndex());
}

QString KXmlCommandSelector::command() const
{
	QString	cmd;
	if (m_line && !m_usefilter->isChecked())
		cmd = m_line->text();
	else
		cmd = m_cmdlist[m_cmd->currentIndex()];
	return cmd;
}

void KXmlCommandSelector::setCommand(const QString& cmd)
{
	int	index = m_cmdlist.indexOf(cmd);

	if (m_usefilter)
		m_usefilter->setChecked(index != -1);
	if (m_line)
		m_line->setText((index == -1 ? cmd : QString()));
	if (index != -1)
		m_cmd->setCurrentItem(index);
	if (m_cmd->currentIndex() != -1 && m_cmd->isEnabled())
		slotCommandSelected(m_cmd->currentIndex());
}

void KXmlCommandSelector::slotAddCommand()
{
	bool	ok(false);
	QString	cmdId = KInputDialog::getText(i18n("Command Name"), i18n("Enter an identification name for the new command:"), QString(), &ok, this);
	if (ok)
	{
		bool	added(true);

		if (m_cmdlist.indexOf(cmdId) != -1)
		{
			if (KMessageBox::warningContinueCancel(
				this,
				i18n("A command named %1 already exists. Do you want "
				     "to continue and edit the existing one?", cmdId),
				QString(),
				KStdGuiItem::cont()) == KMessageBox::Cancel)
			{
				return;
			}
			else
				added = false;
	}

		KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(cmdId);
		if (KXmlCommandDlg::editCommand(xmlCmd, this))
			KXmlCommandManager::self()->saveCommand(xmlCmd);

		if (added)
			loadCommands();
	}
}

void KXmlCommandSelector::slotEditCommand()
{
	QString	xmlId = m_cmdlist[m_cmd->currentIndex()];
	KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(xmlId);
	if (xmlCmd)
	{
		if (KXmlCommandDlg::editCommand(xmlCmd, this))
		{
			// force to load the driver if not already done
			xmlCmd->driver();
			KXmlCommandManager::self()->saveCommand(xmlCmd);
		}
		m_cmd->changeItem(xmlCmd->description(), m_cmd->currentIndex());
		delete xmlCmd;
		slotCommandSelected(m_cmd->currentIndex());
	}
	else
		KMessageBox::error(this, i18n("Internal error. The XML driver for the command %1 could not be found.", xmlId));
}

void KXmlCommandSelector::slotBrowse()
{
	QString	filename = KFileDialog::getOpenFileName(QString(), QString(), this);
	if (!filename.isEmpty() && m_line)
		m_line->setText(filename);
}

void KXmlCommandSelector::slotCommandSelected(int ID)
{
	KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(m_cmdlist[ID], true);
	if (xmlCmd)
	{
		QString msg;
		if ( xmlCmd->isValid() && KdeprintChecker::check( xmlCmd->requirements() ) )
		{
			msg = QString::fromLocal8Bit("(ID = %1, %2 = ").arg(xmlCmd->name()).arg(i18n("output"));
			if (KXmlCommandManager::self()->checkCommand(xmlCmd->name(), KXmlCommandManager::None, KXmlCommandManager::Basic))
			{
				if (xmlCmd->mimeType() == "all/all")
					msg.append(i18n("undefined"));
				else
					msg.append(xmlCmd->mimeType());
			}
			else
				msg.append(i18n("not allowed"));
			msg.append(")");
			emit commandValid( true );
		}
		else
		{
			msg = "<font color=\"red\">" + i18n( "(Unavailable: requirements not satisfied)" ) + "</font>";
			emit commandValid( false );
		}
		m_shortinfo->setText(msg);
		m_help = xmlCmd->comment();
		m_helpbtn->setEnabled( !m_help.isEmpty() );
	}
	delete xmlCmd;
}

void KXmlCommandSelector::slotXmlCommandToggled( bool on )
{
	if ( on )
		slotCommandSelected( m_cmd->currentIndex() );
	else
	{
		emit commandValid( true );
		m_shortinfo->setText( QString() );
	}
}

void KXmlCommandSelector::slotHelpCommand()
{
	KPopupFrame *pop = new KPopupFrame( m_helpbtn );
	KActiveLabel *lab = new KActiveLabel( m_help, pop );
	lab->resize( lab->sizeHint() );
	pop->setMainWidget( lab );
	pop->exec( m_helpbtn->mapToGlobal( QPoint( m_helpbtn->width(), 0 ) ) );
	pop->close( 0 );
	delete pop;
}

#include "kxmlcommandselector.moc"
