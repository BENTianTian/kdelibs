/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>

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
#ifndef _KGLOBAL_H
#define _KGLOBAL_H

class KStandardDirs;
class KConfig;
class KLocale;
class KIconLoader;
class KCharsets;
class QFont;
class KInstance;

/**
 * Access the KDE global objects.
 *
 * @author Sirtaj Singh Kang (taj@kde.org)
 * @version $Id$
 */
class KGlobal
{
public:

    /**
     * Retrieve the global instance.  There is always at least
     * one instance of a component in one application (in most
     * cases the application itself).
     */
    static KInstance            *instance();

    /**
     *  Retrieve the application standard dirs object.
     */
    static KStandardDirs	*dirs();

    /**
     *  Retrieve the general config object.
     */
    static KConfig		*config();

    /**
     *  Retrieve an iconloader object.
     */
    static KIconLoader	        *iconLoader();

    static KLocale             *locale();
    static KCharsets	        *charsets();

    static QFont                generalFont();
    static QFont                fixedFont();
    static QFont                toolBarFont();
    static QFont                menuFont();

    static  KInstance           *_instance;
    static  KLocale             *_locale;
    static  KCharsets	        *_charsets;

    static  QFont               *_generalFont;
    static  QFont               *_fixedFont;
    static  QFont               *_toolBarFont;
    static  QFont               *_menuFont;
};

/**
 * @libdoc The KDE Core Functionality Library
 *
 * All KDE programs use this library to provide basic functionality such
 * as the configuration system, IPC, internationalization and locale
 * support, site-independent access to the filesystem and a large number
 * of other (but no less important) things.
 *
 * All KDE applications should link to the kdecore library. Also, using a
 * @ref KApplication derived class instead of @ref QApplication is almost
 * mandatory if you expect your application to behave nicely within the
 * KDE environment.
 */
#endif // _KGLOBAL_H
