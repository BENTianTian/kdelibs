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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <kdebug.h>

#include "halmanager.h"
#include "haldevice.h"

int main()
{
	HalManager manager;

        QStringList devices = manager.allDevices();

	kdDebug() << "=======" << endl;
	kdDebug() << ":" << devices << ":" << endl;
	kdDebug() << ":" << manager.deviceExists("/org/freedesktop/Hal/devices/computer") << ":" << endl;
	kdDebug() << "=======" << endl;

        foreach ( QString udi, devices )
        {
            HalDevice dev( udi );
            QMap<QString, QVariant> properties = dev.allProperties();

            kdDebug() << "udi = '" << udi << "'" << endl;

            QMap<QString, QVariant>::ConstIterator it = properties.begin();
            QMap<QString, QVariant>::ConstIterator end = properties.end();

            for ( ; it!=end; ++it )
            {
                kdDebug() << "  " << it.key() << " = '" << it.value() << endl;
            }
        }

	HalDevice dev( "/org/freedesktop/Hal/devices/computer" );
	kdDebug() << "Computer vendor : " << dev.property( "system.vendor" ) << endl;
	kdDebug() << "Try locking : " << dev.lock("foo") << endl;
	kdDebug() << "Try abusing lock : " << dev.lock("foofoo") << endl;
	kdDebug() << "Try unlocking : " << dev.unlock() << endl;

	return 0;
}
