/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_NETWORK_H
#define SOLID_NETWORK_H

#include <solid/ifaces/enums.h>

#include <QObject>
#include <QStringList>

namespace Solid
{
    class Network : public QObject, public Ifaces::Enums::Network
    {
        Q_OBJECT
    public:
        Network( QObject * );
        virtual ~Network();
        // TODO ask Thiago whether to use QHostAddress or KIPAddress for these
        QStringList ipV4Addresses();
        QStringList ipV6Addresses();

        QString subnetMask();
        QString broadcastAddress();
        // wtf does NM use this for?
        QString route();

        QString primaryDNS();
        QString secondaryDNS();
        QString uni();
        void setActivated( bool active );
        bool isActive() const;
    Q_SIGNALS:
        void ipDetailsChanged();

    protected:
        QObject *backendObject();
        const QObject *backendObject() const;

    private:
        class Private;
        Private * d;

    //HACK: to make NetworkList polymorphic (containing both wired and wireless networks, I used Network * here - Will.
    };
    typedef QList<Network*> NetworkList;


} //Solid

#endif

