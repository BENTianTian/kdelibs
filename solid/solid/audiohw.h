/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_AUDIOHW_H
#define SOLID_AUDIOHW_H

#include <kdelibs_export.h>

#include <solid/capability.h>
#include <solid/ifaces/enums.h>

namespace Solid
{
    /**
     * This capability is available on interfaces exposed by sound cards.
     */
    class SOLID_EXPORT AudioHw : public Capability, public Ifaces::Enums::AudioHw
    {
        Q_OBJECT
    public:
        /**
         * Creates a new AudioHw object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        AudioHw( QObject *backendObject );

        /**
         * Destroys an AudioHw object.
         */
        virtual ~AudioHw();


        /**
         * Get the Solid::Capability::Type of the AudioHw capability.
         *
         * @return the AudioHw capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::AudioHw; }



        /**
         * Retrieves the audio driver that should be used to access the device.
         *
         * @return the driver needed to access the device
         * @see Solid::Ifaces::Enums::AudioDriver
         */
        AudioDriver driver();

        /**
         * Retrieves a driver specific string allowing to access the device.
         *
         * For example for Alsa devices it is of the form "hw:0,0"
         * while for OSS it is "/dev/foo".
         *
         * @return the driver specific string to handle this device
         */
        QString driverHandler();



        /**
         * Retrieves the name of this audio interface.
         *
         * @return the name of the audio interface if available, QString() otherwise
         */
        QString name();

        /**
         * Retrieves the type of this audio interface.
         *
         * @return the type of this audio interface
         * @see Solid::Ifaces::Enums::AudioHwTypes
         */
        AudioHwTypes type();
    };
}

#endif
