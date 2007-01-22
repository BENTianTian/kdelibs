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

#include "predicate.h"

#include <solid/device.h>
#include <solid/capability.h>
#include <QStringList>

namespace Solid
{
    class Predicate::Private
    {
    public:
        enum OperatorType { AtomType, AndType, OrType, IsType };

        Private() : isValid( false ), type( AtomType ),
                    operand1( 0 ), operand2( 0 ) {}

        bool isValid;
        OperatorType type;

        Capability::Type capability;
        QString property;
        QVariant value;

        Predicate *operand1;
        Predicate *operand2;
    };
}


Solid::Predicate::Predicate()
    : d( new Private() )
{
}

Solid::Predicate::Predicate( const Predicate &other )
    : d( new Private() )
{
    *this = other;
}

Solid::Predicate::Predicate( const Capability::Type &capability,
                             const QString &property, const QVariant &value )
    : d( new Private() )
{
    d->isValid = true;
    d->capability = capability;
    d->property = property;
    d->value = value;
}

Solid::Predicate::Predicate( const QString &capability,
                             const QString &property, const QVariant &value )
    : d( new Private() )
{
    QMap<QString, Capability::Type> map;
    map["Processor"] = Capability::Processor;
    map["Block"] = Capability::Block;
    map["Storage"] = Capability::Storage;
    map["Cdrom"] = Capability::Cdrom;
    map["Volume"] = Capability::Volume;
    map["OpticalDisc"] = Capability::OpticalDisc;
    map["Camera"] = Capability::Camera;
    map["PortableMediaPlayer"] = Capability::PortableMediaPlayer;
    map["NetworkHw"] = Capability::NetworkHw;
    map["AcAdapter"] = Capability::AcAdapter;
    map["Battery"] = Capability::Battery;
    map["Button"] = Capability::Button;
    map["Display"] = Capability::Display;
    map["AudioHw"] = Capability::AudioHw;

    if ( map.contains( capability ) )
    {
        d->isValid = true;
        d->capability = map[capability];
        d->property = property;
        d->value = value;
    }
}

Solid::Predicate::Predicate( const Capability::Type &capability )
    : d( new Private() )
{
    d->isValid = true;
    d->type = Private::IsType;
    d->capability = capability;
}

Solid::Predicate::Predicate( const QString &capability )
    : d( new Private() )
{
    QMap<QString, Capability::Type> map;
    map["Processor"] = Capability::Processor;
    map["Block"] = Capability::Block;
    map["Storage"] = Capability::Storage;
    map["Cdrom"] = Capability::Cdrom;
    map["Volume"] = Capability::Volume;
    map["OpticalDisc"] = Capability::OpticalDisc;
    map["Camera"] = Capability::Camera;
    map["PortableMediaPlayer"] = Capability::PortableMediaPlayer;
    map["NetworkHw"] = Capability::NetworkHw;
    map["AcAdapter"] = Capability::AcAdapter;
    map["Battery"] = Capability::Battery;
    map["Button"] = Capability::Button;
    map["Display"] = Capability::Display;
    map["AudioHw"] = Capability::AudioHw;

    if ( map.contains( capability ) )
    {
        d->isValid = true;
        d->type = Private::IsType;
        d->capability = map[capability];
    }
}

Solid::Predicate::~Predicate()
{
    if (d->type!=Private::AtomType && d->type!=Private::IsType) {
        delete d->operand1;
        delete d->operand2;
    }

    delete d;
}

Solid::Predicate &Solid::Predicate::operator=( const Predicate &other )
{
    d->isValid = other.d->isValid;
    d->type = other.d->type;

    if ( d->type!=Private::AtomType && d->type!=Private::IsType )
    {
        d->operand1 = new Predicate( *( other.d->operand1 ) );
        d->operand2 = new Predicate( *( other.d->operand2 ) );
    }
    else
    {
        d->capability = other.d->capability;
        d->property = other.d->property;
        d->value = other.d->value;
    }

    return *this;
}

Solid::Predicate Solid::Predicate::operator&( const Predicate &other )
{
    Predicate result;

    result.d->isValid = true;
    result.d->type = Private::AndType;
    result.d->operand1 = new Predicate( *this );
    result.d->operand2 = new Predicate( other );

    return result;
}

Solid::Predicate Solid::Predicate::operator|( const Predicate &other )
{
    Predicate result;

    result.d->isValid = true;
    result.d->type = Private::OrType;
    result.d->operand1 = new Predicate( *this );
    result.d->operand2 = new Predicate( other );

    return result;
}

bool Solid::Predicate::isValid() const
{
    return d->isValid;
}

bool Solid::Predicate::matches( const Device &device ) const
{
    if ( !d->isValid ) return false;

    switch( d->type )
    {
    case Private::OrType:
        return d->operand1->matches( device )
            || d->operand2->matches( device );
    case Private::AndType:
        return d->operand1->matches( device )
            && d->operand2->matches( device );
    case Private::AtomType:
    {
        const Capability *iface = device.asCapability( d->capability );

        if ( iface!=0 )
        {
            QVariant value = iface->property( d->property.toLatin1() );
            return ( value == d->value );
        }
        break;
    }
    case Private::IsType:
        return device.queryCapability(d->capability);
    }

    return false;
}

QSet<Solid::Capability::Type> Solid::Predicate::usedCapabilities() const
{
    QSet<Capability::Type> res;

    if (d->isValid) {

        switch( d->type )
        {
        case Private::OrType:
        case Private::AndType:
            res+= d->operand1->usedCapabilities();
            res+= d->operand2->usedCapabilities();
            break;
        case Private::AtomType:
        case Private::IsType:
            res << d->capability;
            break;
        }

    }

    return res;
}


QString Solid::Predicate::toString() const
{
    if ( !d->isValid ) return "False";

    if ( d->type!=Private::AtomType && d->type!=Private::IsType )
    {
        QString op = " AND ";
        if ( d->type==Private::OrType ) op = " OR ";

        return "[ "+d->operand1->toString()+op+d->operand2->toString()+" ]";
    }
    else
    {
        QString capability = "Unknown";

        switch( d->capability )
        {
        case Capability::Processor:
            capability = "Processor";
            break;
        case Capability::Block:
            capability = "Block";
            break;
        case Capability::Storage:
            capability = "Storage";
            break;
        case Capability::Cdrom:
            capability = "Cdrom";
            break;
        case Capability::Volume:
            capability = "Volume";
            break;
        case Capability::OpticalDisc:
            capability = "OpticalDisc";
            break;
        case Capability::Camera:
            capability = "Camera";
            break;
        case Capability::PortableMediaPlayer:
            capability = "PortableMediaPlayer";
            break;
        case Capability::NetworkHw:
            capability = "NetworkHw";
            break;
        case Capability::AcAdapter:
            capability = "AcAdapter";
            break;
        case Capability::Battery:
            capability = "Battery";
            break;
        case Capability::Button:
            capability = "Button";
            break;
        case Capability::Display:
            capability = "Display";
            break;
        case Capability::AudioHw:
            capability = "AudioHw";
            break;
        case Capability::Unknown:
            break;
        }

        if (d->type==Private::IsType) {
            return "IS "+capability;
        }

        QString value;

        switch ( d->value.type() )
        {
        case QVariant::StringList:
        {
            value = "{";

            QStringList list = d->value.toStringList();

            QStringList::ConstIterator it = list.begin();
            QStringList::ConstIterator end = list.end();

            for ( ; it!=end; ++it )
            {
                value+= '\''+*it+'\'';

                if ( it+1!=end )
                {
                    value+= ", ";
                }
            }

            value+= '}';
            break;
        }
        case QVariant::Bool:
            value = ( d->value.toBool()?"true":"false" );
            break;
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            value = d->value.toString();
            break;
        default:
            value = '\''+d->value.toString()+'\'';
            break;
        }

        return capability+'.'+d->property+" == "+value;
    }
}




