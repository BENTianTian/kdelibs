/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright (C) 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

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

#ifndef PORTABLEMEDIAPLAYER_H
#define PORTABLEMEDIAPLAYER_H

#include <QtCore/QStringList>

#include "solid/ifaces/portablemediaplayer.h"
#include "solid/backends/hal/haldeviceinterface.h"

class HalDevice;

class PortableMediaPlayer : public DeviceInterface, virtual public Solid::Ifaces::PortableMediaPlayer
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::PortableMediaPlayer)

public:
    PortableMediaPlayer(HalDevice *device);
    virtual ~PortableMediaPlayer();

    virtual QStringList supportedProtocols() const;
    virtual QStringList supportedDrivers(QString protocol = QString()) const;
};

#endif
