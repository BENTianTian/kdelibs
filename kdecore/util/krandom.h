/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KRANDOM_H
#define KRANDOM_H

#include "kdelibs_export.h"

#include <QtCore/QString>

/**
 * @short Helper class to create random data
 *
 * This namespace provides methods which generate random data.
 */
namespace KRandom {
    /**
     * Generates a uniform random number.
     * @return A truly unpredictable number in the range [0, RAND_MAX)
     */
    KDECORE_EXPORT int random();

    /**
     * Generates a random string.  It operates in the range [A-Za-z0-9]
     * @param length Generate a string of this length.
     * @return the random string
     */
    KDECORE_EXPORT QString randomString(int length);
};


#endif

