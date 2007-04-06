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

#include "plugincombobox.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <klocale.h>

PluginComboBox::PluginComboBox(QWidget *parent)
    :QWidget(parent)
{
        QString whatsThisCurrentPrintsystem = i18n(" <qt><b>Print Subsystem Selection</b>"
						" <p>This combo box shows (and lets you select)"
						" a print subsystem to be used by KDEPrint. (This print"
						" subsystem must, of course, be installed inside your"
						" Operating System.) KDEPrint usually auto-detects the"
                                                " correct print subsystem by itself upon first startup."
						" Most Linux distributions have \"CUPS\", the <em>Common"
						" UNIX Printing System</em>."
                                                " </qt>" );

	m_combo = new QComboBox(this);
  m_combo->setObjectName("PluginCombo");
        m_combo->setWhatsThis(whatsThisCurrentPrintsystem);
	QLabel	*m_label = new QLabel(i18n("Print s&ystem currently used:"), this);
        m_label->setWhatsThis(whatsThisCurrentPrintsystem);
	m_label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	m_label->setBuddy(m_combo);
	m_plugininfo = new QLabel(i18n("Plugin information"), this);
	QGridLayout	*l0 = new QGridLayout(this);
  l0->setMargin(0);
  l0->setSpacing(5);
	l0->setColumnStretch(0, 1);
	l0->addWidget(m_label, 0, 0);
	l0->addWidget(m_combo, 0, 1);
	l0->addWidget(m_plugininfo, 1, 1);

	QList<KMFactory::PluginInfo>	list = KMFactory::self()->pluginList();
	QString			currentPlugin = KMFactory::self()->printSystem();
	for (QList<KMFactory::PluginInfo>::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		m_combo->addItem((*it).comment);
		if ((*it).name == currentPlugin)
			m_combo->setCurrentIndex(m_combo->count()-1);
		m_pluginlist.append((*it).name);
	}

	connect(m_combo, SIGNAL(activated(int)), SLOT(slotActivated(int)));
	configChanged();
}

void PluginComboBox::slotActivated(int index)
{
	QString	plugin = m_pluginlist[index];
	if (!plugin.isEmpty())
	{
		// the factory will notify all registered objects of the change
		KMFactory::self()->reload(plugin, true);
	}
}

void PluginComboBox::reload()
{
	QString	syst = KMFactory::self()->printSystem();
	int	index(-1);
	if ((index=m_pluginlist.indexOf(syst)) != -1)
		m_combo->setCurrentIndex(index);
	configChanged();
}

void PluginComboBox::configChanged()
{
        QString whatsThisCurrentConnection = i18n(" <qt><b>Current Connection</b>"
						" <p>This line shows which CUPS server your PC is"
						" currently connected to for printing and retrieving"
						" printer info. To switch to a different CUPS server,"
						" click \"System Options\", then select \"Cups server\""
						" and fill in the required info."
                                                " </qt>" );

	m_plugininfo->setText(KMManager::self()->stateInformation());
        m_plugininfo->setWhatsThis(whatsThisCurrentConnection);

}

#include "plugincombobox.moc"
