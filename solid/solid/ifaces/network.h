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

#ifndef SOLID_IFACES_NETWORK_H
#define SOLID_IFACES_NETWORK_H

#include <kdelibs_export.h>

#include <solid/ifaces/enums.h>

#include <QObject>
#include <QStringList>

namespace Solid
{
namespace Ifaces
{
    /**
     * Represents a generic IP network which we may be connected to
     * TODO what about QNetworkAddressEntry
     */
    class KDE_EXPORT Network : public Enums::Network
    {
    public:
        virtual ~Network();
        // TODO Decide if QList<KNetwork::KSocketAddress> is preferred here
        virtual QStringList ipV4Addresses() = 0;
        virtual QStringList ipV6Addresses() = 0;

        virtual QString subnetMask() = 0;
        virtual QString broadcastAddress() = 0;
        // wtf does NM use this for?
        virtual QString route() = 0;

        virtual QString primaryDNS() = 0;
        virtual QString secondaryDNS() = 0;
        /**
         * Activate or deactivate this network
         * TODO add status change signals?
         */
        virtual void setActivated( bool ) = 0;
        /**
         * The Unique Network Identifier for this network
         */
        virtual QString uni() = 0;

    protected:
    //Q_SIGNALS:
        virtual void ipDetailsChanged() = 0;
    };
} //Ifaces
} //Solid

Q_DECLARE_INTERFACE( Solid::Ifaces::Network, "org.kde.Solid.Ifaces.Network/0.1" )

#endif
