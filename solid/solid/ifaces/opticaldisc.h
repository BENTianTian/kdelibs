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

#ifndef KDEHW_IFACES_OPTICALDISC_H
#define KDEHW_IFACES_OPTICALDISC_H

#include <kdehw/ifaces/volume.h>

namespace KDEHW
{
namespace Ifaces
{
    class OpticalDisc : virtual public Volume, public Enums::OpticalDisc
    {
    public:
        virtual ~OpticalDisc();

        static Type type() { return Capability::OpticalDisc; }

        virtual ContentTypes availableContent() const = 0;
        virtual DiscType discType() const = 0;
        virtual bool isAppendable() const = 0;
        virtual bool isBlank() const = 0;
        virtual bool isRewritable() const = 0;
        virtual long capacity() const = 0;
    };
}
}

#endif
