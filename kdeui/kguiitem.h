/* This file is part of the KDE libraries
    Copyright (C) 2001 Holger Freyther (freyher@yahoo.com)
                  based on ideas from Martijn and Simon

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

    Many thanks to Simon tronical Hausmann
*/

#ifndef kguiitem_h
#define kguiitem_h

#include <qstring.h>

#include <kglobal.h>
#include <kicontheme.h>

class QIcon;

/**
 * @short An abstract class for GUI data such as ToolTip and Icon.
 *
 * @author Holger Freyther <freyher@yahoo.com>
 * @see KStdGuiItem
 */
class KDEUI_EXPORT KGuiItem
{
public:
    KGuiItem();

    // ### This should probably be explicit in KDE 4; it's easy to get
    // subtle bugs otherwise - the icon name, tooltip and whatsthis text
    // get changed behind your back if you do 'setButtonFoo( "Bar" );'
    // It gives the wrong impression that you just change the text.
    KGuiItem( const QString &text, 
              const QString &iconName  = QString(),
              const QString &toolTip   = QString(), 
              const QString &whatsThis = QString() );

    KGuiItem( const QString &text, const QIcon &iconSet, 
              const QString &toolTip   = QString(), 
              const QString &whatsThis = QString() );

    KGuiItem( const KGuiItem &rhs );
    KGuiItem &operator=( const KGuiItem &rhs );

    ~KGuiItem();

    QString text() const;
    QString plainText() const;
#ifndef KDE_NO_COMPAT
    QIcon iconSet( K3Icon::Group, int size = 0, KInstance* instance = KGlobal::instance()) const;
    QIcon iconSet() const;
#else
    QIcon iconSet( K3Icon::Group=K3Icon::Small, int size = 0, KInstance* instance = KGlobal::instance()) const;
#endif

    QString iconName() const;
    QString toolTip() const;
    QString whatsThis() const;
    bool isEnabled() const;
    bool hasIcon() const;
#ifndef KDE_NO_COMPAT
    bool hasIconSet() const { return hasIcon(); }
#endif

    void setText( const QString &text );
    void setIcon( const QIcon &iconset );
    void setIconName( const QString &iconName );
    void setToolTip( const QString &tooltip );
    void setWhatsThis( const QString &whatsThis );
    void setEnabled( bool enable );

private:
    class KGuiItemPrivate;
    KGuiItemPrivate *d;
};

/* vim: et sw=4
 */

#endif

