/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KNEWSTUFF2_DXS_DXS_H
#define KNEWSTUFF2_DXS_DXS_H

#include <knewstuff2/knewstuff_export.h>
#include <QtCore/QObject>

#include <kurl.h>

class QDomNode;

namespace KNS
{

class Soap;
class Entry;
class Category;

class KNEWSTUFF_EXPORT Dxs : public QObject
{
Q_OBJECT
public:
	Dxs();
	~Dxs();
	void setEndpoint(KUrl endpoint);

	void call_info();
	void call_categories();
	void call_entries(QString category, QString feed);
	void call_comments(int id);
	void call_changes(int id);
	void call_history(int id);
	void call_removal(int id);
	void call_subscription(int id, bool subscribe);
	void call_comment(int id, QString comment);
	void call_rating(int id, int rating);

signals:
	void signalInfo(QString provider, QString server, QString version);
	void signalCategories(QList<KNS::Category*> categories);
	void signalEntries(QList<KNS::Entry*> entries);
	void signalComments(QStringList comments);
	void signalChanges(QStringList comments);
	void signalHistory(QStringList entries);
	void signalRemoval(bool success);
	void signalSubscription(bool success);
	void signalComment(bool success);
	void signalRating(bool success);
	void signalFault();
	void signalError();

private slots:
	void slotResult(QDomNode node);
	void slotError();

private:
	Soap *m_soap;
	KUrl m_endpoint;
};

}

#endif
