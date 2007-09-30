/* This file is part of the KDE libraries
   Copyright (C) 2005-2007 Olivier Goffart <ogoffart at kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "knotifyconfigactionswidget.h"
#include "knotifyconfigelement.h"

#include <kstandarddirs.h>
#include <kiconloader.h>

#include <phonon/mediaobject.h>

KNotifyConfigActionsWidget::KNotifyConfigActionsWidget( QWidget * parent )
	: QWidget(parent)
{
	m_ui.setupUi(this);
	m_ui.Sound_play->setIcon(KIcon("media-playback-start"));
	connect(m_ui.Execute_check,SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	connect(m_ui.Sound_check,SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	connect(m_ui.Popup_check,SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	connect(m_ui.Logfile_check,SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	connect(m_ui.Execute_check,SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	connect(m_ui.Taskbar_check,SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	connect(m_ui.Sound_play,SIGNAL(clicked()), this, SLOT(slotPlay()));
}

void KNotifyConfigActionsWidget::setConfigElement( KNotifyConfigElement * config )
{
	bool blocked = blockSignals(true); //to block the changed() signal
	QString prstring=config->readEntry( "Action" );
	QStringList actions=prstring.split ("|");

	m_ui.Sound_check->setChecked( actions.contains("Sound") );
	m_ui.Popup_check->setChecked( actions.contains("Popup") );
	m_ui.Logfile_check->setChecked( actions.contains("Logfile") );
	m_ui.Execute_check->setChecked( actions.contains("Execute") );
	m_ui.Taskbar_check->setChecked( actions.contains("Taskbar") );

	m_ui.Sound_select->setUrl( KUrl( config->readEntry( "Sound" , true ) ) );
	m_ui.Logfile_select->setUrl( KUrl( config->readEntry( "Logfile" , true ) ) );
	m_ui.Execute_select->setUrl( KUrl( config->readEntry( "Execute"  ) ) );
	blockSignals(blocked);
}

void KNotifyConfigActionsWidget::save( KNotifyConfigElement * config )
{
	QStringList actions;
	if(m_ui.Sound_check->isChecked())
		actions << "Sound";
	if(m_ui.Popup_check->isChecked())
		actions << "Popup";
	if(m_ui.Logfile_check->isChecked())
		actions << "Logfile";
	if(m_ui.Execute_check->isChecked())
		actions << "Execute";
	if(m_ui.Taskbar_check->isChecked())
		actions << "Taskbar";

	config->writeEntry( "Action" , actions.join("|") );

	config->writeEntry( "Sound" , m_ui.Sound_select->url().url() );
	config->writeEntry( "Logfile" , m_ui.Logfile_select->url().url() );
	config->writeEntry( "Execute" , m_ui.Execute_select->url().url() );
}

void KNotifyConfigActionsWidget::slotPlay(  )
{
	KUrl soundURL = m_ui.Sound_select->url();
	if ( soundURL.isRelative() )
	{
		QString soundString = soundURL.toLocalFile();
		// we need a way to get the application name in order to ba able to do this :
		/*QString search = QString("%1/sounds/%2").arg(config->appname).arg(soundFile);
		search = KGlobal::mainComponent().dirs()->findResource("data", search);
		if ( search.isEmpty() )*/
		soundURL = KUrl::fromPath( KStandardDirs::locate( "sound", soundString ) );
	}
	Phonon::MediaObject* media = Phonon::createPlayer( Phonon::NotificationCategory, soundURL );
	media->play();
	connect(media, SIGNAL(finished()), media, SLOT(deleteLater()));
}

#include "knotifyconfigactionswidget.moc"
