/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

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

#ifndef KDEHW_DEVICEMANAGER_H
#define KDEHW_DEVICEMANAGER_H

#include <QObject>
#include <QList>

#include <kdelibs_export.h>

#include <kstaticdeleter.h>
#include <kdehw/predicate.h>

namespace KDEHW
{
    namespace Ifaces
    {
        class DeviceManager;
    }
    class Device;
    typedef QList<Device> DeviceList;

    /**
     * This class allow to query the underlying system to obtain information
     * about the hardware available.
     *
     * It's the unique entry point for hardware discovery. Applications should use
     * it to find devices, or to be notified about hardware changes.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    class KDE_EXPORT DeviceManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Retrieves the unique instance of this class.
         *
         * @return unique instance of the class
         */
        static DeviceManager &self();

        /**
         * Retrieves the unique instance of this class and forces the registration of
         * the given backend.
         * The DeviceManager will use this backend. The parameter will be ignored if an
         * instance of DeviceManager already exists.
         *
         * Use this method at your own risks. It's primarily available to easier tests
         * writing. If you need to test the DeviceManager, use a call to this method, and
         * then use self() as usual.
         *
         * @param backend the in the application
         * @return unique instance of the class
         * @see self()
         */
        static DeviceManager &selfForceBackend( Ifaces::DeviceManager *backend );

        /**
         * Returns a text describing the error that occured while loading
         * the backend.
         *
         * @return the error description, or QString() if the backend loaded successfully
         */
        const QString &errorText() const;


        /**
         * Retrieves all the devices available in the underlying system.
         *
         * @return the list of the devices available
         */
        DeviceList allDevices();

        /**
         * Tests if a device exists in the underlying system given its
         * Universal Device Identifier (UDI).
         *
         * @param udi the identifier of the device to check
         * @return true if a device has the given udi in the system, false otherwise
         */
        bool deviceExists( const QString &udi );

        /**
         * Retrieves a device of the system given it's UDI.
         *
         * @param udi the identifier of the device to find
         * @return a device that has the given UDI in the system if possible, an
         * invalid device otherwise
         */
        Device findDevice( const QString &udi );

        /**
         * Retrieves a list of devices of the system given a set of constraints.
         * FIXME: Currently document this
         *
         * @param parentUdi
         * @param capability
         * @param predicate
         * @return the list of devices corresponding to the given constraints
         */
        DeviceList findDevicesFromQuery( const QString &parentUdi,
                                         const Capability::Type &capability = Capability::Unknown,
                                         const Predicate &predicate = Predicate() );

        /**
         * Convenience function see above.
         */
        DeviceList findDevicesFromQuery( const QString &parentUdi,
                                         const Capability::Type &capability,
                                         const QString &predicate );

        /**
         * Retrieves a reference to the loaded backend.
         *
         * @return a pointer to the backend, or 0 if no backend is loaded
         */
        const Ifaces::DeviceManager *backend() const;

    signals:
        /**
         * This signal is emitted when a new device appear in the underlying system.
         *
         * @param udi the new device UDI
         */
        void deviceAdded( const QString &udi );

        /**
         * This signal is emitted when a device disappear from the underlying system.
         *
         * @param udi the old device UDI
         */
        void deviceRemoved( const QString &udi );

        /**
         * This signal is emitted when a new capability is detected in a device.
         *
         * @param udi the UDI of the device getting a new capability
         * @param capability the capability type
         */
        void newCapability( const QString &udi, int capability );

    private:
        DeviceManager();
        DeviceManager( Ifaces::DeviceManager *backend );
        ~DeviceManager();

    private slots:
        void slotDeviceAdded( const QString &udi );
        void slotDeviceRemoved( const QString &udi );
        void slotNewCapability( const QString &udi, int capability );
        void slotDestroyed( QObject *object );

    private:
        static DeviceManager *s_self;
        class Private;
        Private *d;

        friend void ::KStaticDeleter<DeviceManager>::destructObject();
    };
}

#endif
