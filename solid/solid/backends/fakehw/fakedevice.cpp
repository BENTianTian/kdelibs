/*  This file is part of the KDE project
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

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
#include "backends/fakehw/fakedevice.h"

#include "backends/fakehw/fakedeviceinterface.h"
#include "backends/fakehw/fakegenericinterface.h"
#include "backends/fakehw/fakeprocessor.h"
#include "backends/fakehw/fakeblock.h"
#include "backends/fakehw/fakestorage.h"
#include "backends/fakehw/fakecdrom.h"
#include "backends/fakehw/fakevolume.h"
#include "backends/fakehw/fakeopticaldisc.h"
#include "backends/fakehw/fakecamera.h"
#include "backends/fakehw/fakeportablemediaplayer.h"
#include "backends/fakehw/fakenetworkinterface.h"
#include "backends/fakehw/fakeacadapter.h"
#include "backends/fakehw/fakebattery.h"
#include "backends/fakehw/fakebutton.h"
#include "backends/fakehw/fakeaudiointerface.h"
#include "backends/fakehw/fakedvbinterface.h"

#include <QtCore/QStringList>
#include <QtDBus/QDBusConnection>

#include <solid/genericinterface.h>

class FakeDevice::Private
{
public:
    QString udi;
    QMap<QString, QVariant> propertyMap;
    QStringList interfaceList;
    bool locked;
    QString lockReason;
    bool broken;
};

FakeDevice::FakeDevice(const QString &udi, const QMap<QString, QVariant> &propertyMap)
    : Solid::Ifaces::Device(), d(new Private)
{
    d->udi = udi;
    d->propertyMap = propertyMap;
    d->interfaceList = d->propertyMap["interfaces"].toString().simplified().split(',');
    d->interfaceList << "GenericInterface";
    d->locked = false;
    d->broken = false;

    QDBusConnection::sessionBus().registerObject(udi, this, QDBusConnection::ExportNonScriptableSlots);

    // Force instantiation of all the device interfaces
    // this way they'll get exported on the bus
    // that means they'll be created twice, but that won't be
    // a problem for unit testing.
    foreach (const QString &interface, d->interfaceList)
    {
        Solid::DeviceInterface::Type type = Solid::DeviceInterface::stringToType(interface);
        createDeviceInterface(type);
    }
}

FakeDevice::~FakeDevice()
{
    delete d;
}

QString FakeDevice::udi() const
{
    return d->udi;
}

QString FakeDevice::parentUdi() const
{
    return d->propertyMap["parent"].toString();
}

QString FakeDevice::vendor() const
{
    return d->propertyMap["vendor"].toString();
}

QString FakeDevice::product() const
{
    return d->propertyMap["name"].toString();
}

QVariant FakeDevice::property(const QString &key) const
{
    return d->propertyMap[key];
}

QMap<QString, QVariant> FakeDevice::allProperties() const
{
    return d->propertyMap;
}

bool FakeDevice::propertyExists(const QString &key) const
{
    return d->propertyMap.contains(key);
}

bool FakeDevice::setProperty(const QString &key, const QVariant &value)
{
    if (d->broken) return false;

    Solid::GenericInterface::PropertyChange change_type = Solid::GenericInterface::PropertyModified;

    if (!d->propertyMap.contains(key))
    {
        change_type = Solid::GenericInterface::PropertyAdded;
    }

    d->propertyMap[key] = value;

    QMap<QString,int> change;
    change[key] = change_type;

    emit propertyChanged(change);

    return true;
}

bool FakeDevice::removeProperty(const QString &key)
{
    if (d->broken || !d->propertyMap.contains(key)) return false;

    d->propertyMap.remove(key);

    QMap<QString,int> change;
    change[key] = Solid::GenericInterface::PropertyRemoved;

    emit propertyChanged(change);

    return true;
}

void FakeDevice::setBroken(bool broken)
{
    d->broken = broken;
}

bool FakeDevice::isBroken()
{
    return d->broken;
}

bool FakeDevice::lock(const QString &reason)
{
    if (d->broken || d->locked) return false;

    d->locked = true;
    d->lockReason = reason;

    return true;
}

bool FakeDevice::unlock()
{
    if (d->broken || !d->locked) return false;

    d->locked = false;
    d->lockReason = QString();

    return true;
}

bool FakeDevice::isLocked() const
{
    return d->locked;
}

QString FakeDevice::lockReason() const
{
    return d->lockReason;
}

void FakeDevice::raiseCondition(const QString &condition, const QString &reason)
{
    emit conditionRaised(condition, reason);
}

bool FakeDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    return d->interfaceList.contains(Solid::DeviceInterface::typeToString(type));
}

QObject *FakeDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    // Do not try to cast with a unsupported device interface.
    if (!queryDeviceInterface(type))
        return 0;

    FakeDeviceInterface *iface = 0;

    switch(type)
    {
    case Solid::DeviceInterface::GenericInterface:
        iface = new FakeGenericInterface(this);
        break;
    case Solid::DeviceInterface::Processor:
        iface = new FakeProcessor(this);
        break;
    case Solid::DeviceInterface::Block:
        iface = new FakeBlock(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new FakeStorage(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new FakeCdrom(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new FakeVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new FakeOpticalDisc(this);
        break;
    case Solid::DeviceInterface::Camera:
        iface = new FakeCamera(this);
        break;
    case Solid::DeviceInterface::PortableMediaPlayer:
        iface = new FakePortableMediaPlayer(this);
        break;
    case Solid::DeviceInterface::NetworkInterface:
        iface = new FakeNetworkInterface(this);
        break;
    case Solid::DeviceInterface::AcAdapter:
        iface = new FakeAcAdapter(this);
        break;
    case Solid::DeviceInterface::Battery:
        iface = new FakeBattery(this);
        break;
    case Solid::DeviceInterface::Button:
        iface = new FakeButton(this);
        break;
    case Solid::DeviceInterface::AudioInterface:
        iface = new FakeAudioInterface(this);
        break;
    case Solid::DeviceInterface::DvbInterface:
        iface = new FakeDvbInterface(this);
        break;
    case Solid::DeviceInterface::Unknown:
        break;
    }

    if(iface)
    {
        QDBusConnection::sessionBus().registerObject(d->udi+'/'+Solid::DeviceInterface::typeToString(type), iface,
                                                      QDBusConnection::ExportNonScriptableSlots);
    }

    return iface;
}

#include "backends/fakehw/fakedevice.moc"
