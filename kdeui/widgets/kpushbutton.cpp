/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "kpushbutton.h"

#include <qdrag.h>
#include <qevent.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qtimer.h>

#include "config.h"

#include <kguiitem.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kglobal.h>
#include <qmenu.h>

class KPushButton::KPushButtonPrivate
{
public:
    KGuiItem item;
    KStdGuiItem::StdItem itemType;
    QPointer<QMenu> delayedMenu;
    QTimer * delayedMenuTimer;
};

bool KPushButton::s_useIcons = false;

KPushButton::KPushButton( QWidget *parent ) : QPushButton( parent ),
      m_dragEnabled( false )
{
    init( KGuiItem( "" ) );
}

KPushButton::KPushButton( const QString &text, QWidget *parent ) : QPushButton( parent ),
      m_dragEnabled( false )
{
    init( KGuiItem( text ) );
}

KPushButton::KPushButton( const QIcon &icon, const QString &text,
                          QWidget *parent )
    : QPushButton( text, parent ),
      m_dragEnabled( false )
{
    init( KGuiItem( text, icon ) );
}

KPushButton::KPushButton( const KGuiItem &item, QWidget *parent )
    : QPushButton( parent ),
      m_dragEnabled( false )
{
    init( item );
}

KPushButton::~KPushButton()
{
    if( d )
    {
        delete d;
        d = 0L;
    }
}

void KPushButton::init( const KGuiItem &item )
{
    d = new KPushButtonPrivate;
    d->item = item;
    d->itemType = (KStdGuiItem::StdItem) 0;
    d->delayedMenuTimer=0;

    connect(this,SIGNAL(pressed()),this, SLOT(slotPressedInternal()));
    connect(this,SIGNAL(clicked()),this, SLOT(slotClickedInternal()));
    // call QPushButton's implementation since we don't need to
    // set the GUI items text or check the state of the icon set
    QPushButton::setText( d->item.text() );

    static bool initialized = false;
    if ( !initialized ) {
        readSettings();
        initialized = true;
    }

    setIcon( d->item.iconSet() );

    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    setToolTip( item.toolTip() );

    setWhatsThis(item.whatsThis());

    connect( KGlobalSettings::self(), SIGNAL( settingsChanged(int) ),
             SLOT( slotSettingsChanged(int) ) );
}

void KPushButton::readSettings()
{
    s_useIcons = KGlobalSettings::showIconsOnPushButtons();
}

void KPushButton::setGuiItem( const KGuiItem& item )
{
    d->item = item;

    // call QPushButton's implementation since we don't need to
    // set the GUI items text or check the state of the icon set
    QPushButton::setText( d->item.text() );
    setIcon( d->item.iconSet() );
    setToolTip( d->item.toolTip() );
    setWhatsThis( d->item.whatsThis() );
}

void KPushButton::setGuiItem( KStdGuiItem::StdItem item )
{
	setGuiItem( KStdGuiItem::guiItem(item) );
	d->itemType = item;
}

KStdGuiItem::StdItem KPushButton::guiItem() const
{
	return d->itemType;
}

void KPushButton::setText( const QString &text )
{
    QPushButton::setText(text);

    // we need to re-evaluate the icon set when the text
    // is removed, or when it is supplied
    if (text.isEmpty() != d->item.text().isEmpty())
        setIcon(d->item.iconSet());

    d->item.setText(text);
}

void KPushButton::setIcon( const QIcon &iconSet )
{
    d->item.setIcon(iconSet);

    if ( s_useIcons || text().isEmpty() )
        QPushButton::setIcon( iconSet );
    else
        QPushButton::setIcon( QIcon() );
}

void KPushButton::slotSettingsChanged( int /* category */ )
{
    readSettings();
    setIcon( d->item.iconSet() );
}

void KPushButton::setDragEnabled( bool enable )
{
    m_dragEnabled = enable;
}

void KPushButton::mousePressEvent( QMouseEvent *e )
{
    if ( m_dragEnabled )
	startPos = e->pos();
    QPushButton::mousePressEvent( e );
}

void KPushButton::mouseMoveEvent( QMouseEvent *e )
{
    if ( !m_dragEnabled )
    {
        QPushButton::mouseMoveEvent( e );
        return;
    }

    if ( (e->buttons() & Qt::LeftButton) &&
         (e->pos() - startPos).manhattanLength() >
         KGlobalSettings::dndEventDelay() )
    {
        startDrag();
        setDown( false );
    }
}

QDrag * KPushButton::dragObject()
{
    return 0;
}

void KPushButton::startDrag()
{
    QDrag *d = dragObject();
    if ( d )
	d->start();
}

void KPushButton::setDelayedMenu(QMenu *delayedMenu)
{
    d->delayedMenu=delayedMenu;
}

QMenu* KPushButton::delayedMenu()
{
    return d->delayedMenu;
}


void KPushButton::slotPressedInternal()
{
    if (!d->delayedMenu.isNull()) {
	if (d->delayedMenuTimer==0) {
		d->delayedMenuTimer=new QTimer(this);
		d->delayedMenuTimer->setSingleShot(true);
		connect(d->delayedMenuTimer,SIGNAL(timeout()),this,SLOT(slotDelayedMenuTimeout()));
	}
	int delay=style()->styleHint(QStyle::SH_ToolButton_PopupDelay, 0, this);
	d->delayedMenuTimer->start((delay<=0) ? 150:delay);
    }
}

void KPushButton::slotClickedInternal()
{
    if (d->delayedMenuTimer)
	d->delayedMenuTimer->stop();
}

void KPushButton::slotDelayedMenuTimeout() {
    d->delayedMenuTimer->stop();
    if (!d->delayedMenu.isNull()) {
    	setMenu(d->delayedMenu);
    	showMenu();
    	setMenu(0);
    }
}

#include "kpushbutton.moc"
