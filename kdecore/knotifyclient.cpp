/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "knotifyclient.h"

#include <qdatastream.h>

#include <dcopclient.h>


KNotifyClient::KNotifyClient(QObject *parent, const QString &message, const QString &text,
                             int present, const QString &sound, const QString &file,
                             DCOPClient* client) : QObject(parent)
{
	if (!client) client=KApplication::kApplication()->dcopClient();
	levent=new Event;
	levent->message=message;
	levent->text=text;
	levent->present=present;
	levent->sound=sound;
	levent->file=file;
	levent->client=client;
}

KNotifyClient::~KNotifyClient()
{
	delete levent;
}

bool KNotifyClient::send()
{
	DCOPClient *client=levent->client;
	if (!client->isAttached())
		client->attach();
	if (!client->isAttached())
		return false;

	QByteArray data;
	QDataStream ds(data, IO_WriteOnly);
	QString appname = kapp->name();
	ds << levent->message << appname << levent->text << levent->sound << levent->file << levent->present;

	return client->send("knotify", "Notify", "notify(QString,QString,QString,QString,QString,int)", data);
}

bool KNotifyClient::event(const QString &message, const QString &text)
{
	KNotifyClient c(0,message, text);
	return c.send();
}

bool KNotifyClient::userEvent(const QString &text, int present,
                              const QString &sound, const QString &file)
{
	KNotifyClient c(0,0, text, present, sound, file);
	return c.send();
}

int KNotifyClient::getPresentation(const QString &eventname)
{
	(void)eventname;
	return None;
}

QString KNotifyClient::getFile(const QString &eventname, int present)
{
	(void)eventname;
	(void)present;
	return "";

}

int KNotifyClient::getDefaultPresentation(const QString &eventname)
{
	(void)eventname;
	return None;
}

QString KNotifyClient::getDefaultFile(const QString &eventname, int present)
{
	(void)eventname;
	(void)present;
	return "";
}

#include "knotifyclient.moc"

