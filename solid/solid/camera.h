/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_CAMERA_H
#define SOLID_CAMERA_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class CameraPrivate;
    class Device;

    /**
     * This device interface is available on digital camera devices.
     *
     * A digital camera is a device used to transform images into
     * data. Nowaday most digital cameras are multifunctional and
     * able to take photographs, video or sound. On the system side
     * they are a particular type of device holding data, the access
     * method can be different from the typical storage device, hence
     * why it's a separate device interface.
     */
    class SOLID_EXPORT Camera : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(AccessType)
        Q_PROPERTY(AccessType accessMethod READ accessMethod)
        Q_PROPERTY(bool gphotoSupport READ isGphotoSupported)
        Q_DECLARE_PRIVATE(Camera)
        friend class Device;

    public:
        /**
         * This enum type defines the access method that can be used for a camera.
         *
         * - MassStorage : A mass storage camera
         * - Ptp : A camera supporting Picture Transfer Protocol (PTP)
         * - Proprietary : A camera using a proprietary protocol
         */
        enum AccessType { MassStorage, Ptp, Proprietary };


    private:
        /**
         * Creates a new Camera object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Camera(QObject *backendObject);

    public:
        /**
         * Destroys a Camera object.
         */
        virtual ~Camera();


        /**
         * Get the Solid::DeviceInterface::Type of the Camera device interface.
         *
         * @return the Camera device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Camera; }


        /**
         * Retrieves the type of method that should be used to access this
         * device.
         *
         * @return the access method type
         * @see Solid::Camera::AccessType
         */
        AccessType accessMethod() const;

        /**
         * Indicates if the camera is supported by a driver from the libgphoto2 project.
         *
         * @return true if the camera is supported by libgphoto2, false otherwise.
         */
        bool isGphotoSupported() const;
    };
}

#endif
