/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>

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

#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <QMouseEvent>

#include "ktabbar.h"
#include "ktabwidget.h"

KTabBar::KTabBar( QWidget *parent )
    : QTabBar( parent )
    , mReorderStartTab( -1 )
    , mReorderPreviousTab( -1 )
    //, mHoverCloseButtonTab( 0 )
    , mDragSwitchTab( -1 )
    //, mHoverCloseButton( 0 )
    //, mHoverCloseButtonEnabled( false )
    //, mHoverCloseButtonDelayed( true )
    , mTabReorderingEnabled( false )
    , mTabCloseActivatePrevious( false )
{
    setAcceptDrops( true );
    setMouseTracking( true );

    //mEnableCloseButtonTimer = new QTimer( this );
    //connect( mEnableCloseButtonTimer, SIGNAL( timeout() ), SLOT( enableCloseButton() ) );

    mActivateDragSwitchTabTimer = new QTimer( this );
    mActivateDragSwitchTabTimer->setSingleShot(true);
    connect( mActivateDragSwitchTabTimer, SIGNAL( timeout() ), SLOT( activateDragSwitchTab() ) );

    //connect(this, SIGNAL(layoutChanged()), SLOT(onLayoutChange()));
}

KTabBar::~KTabBar()
{
    //For the future
    //delete d;
}

void KTabBar::mouseDoubleClickEvent( QMouseEvent *e )
{
    if( e->button() != Qt::LeftButton )
        return;

    int tab = selectTab( e->pos() );
    if( tab != -1 ) {
        emit( mouseDoubleClick( tab ) );
        return;
    }
    QTabBar::mouseDoubleClickEvent( e );
}

void KTabBar::mousePressEvent( QMouseEvent *e )
{
    if( e->button() == Qt::LeftButton ) {
        //mEnableCloseButtonTimer->stop();
        mDragStart = e->pos();
    }
    else if( e->button() == Qt::RightButton ) {
        int tab = selectTab( e->pos() );
        if( tab != -1 ) {
            emit( contextMenu( tab, mapToGlobal( e->pos() ) ) );
            return;
        }
    }
    QTabBar::mousePressEvent( e );
}

void KTabBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( e->buttons() == Qt::LeftButton ) {
        int tab = selectTab( e->pos() );
        if ( mDragSwitchTab && tab != mDragSwitchTab ) {
          mActivateDragSwitchTabTimer->stop();
          mDragSwitchTab = 0;
        }

        int delay = KGlobalSettings::dndEventDelay();
        QPoint newPos = e->pos();
        if( newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
            newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay )
         {
            if( tab != -1 ) {
                emit( initiateDrag( tab ) );
                return;
           }
       }
    }
    else if ( e->buttons() == Qt::MidButton ) {
        if (mReorderStartTab==-1) {
            int delay = KGlobalSettings::dndEventDelay();
            QPoint newPos = e->pos();
            if( newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
                newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay )
            {
                int tab = selectTab( e->pos() );
                if( tab != -1 && mTabReorderingEnabled ) {
                    mReorderStartTab = tab;
                    grabMouse( Qt::SizeAllCursor );
                    return;
                }
            }
        }
        else {
            int tab = selectTab( e->pos() );
            if( tab != -1 ) {
                int reorderStopTab = tab;
                if ( mReorderStartTab!=reorderStopTab && mReorderPreviousTab!=reorderStopTab ) {
                    emit( moveTab( mReorderStartTab, reorderStopTab ) );
                    mReorderPreviousTab=mReorderStartTab;
                    mReorderStartTab=reorderStopTab;
                    return;
                }
            }
        }
    }

    /*if ( mHoverCloseButtonEnabled && mReorderStartTab==-1) {
        QTab *t = selectTab( e->pos() );
        if( t && t->iconSet() && t->isEnabled() ) {
            QPixmap pixmap = t->iconSet()->pixmap( QIcon::Small, QIcon::Normal );
            QRect rect( 0, 0, pixmap.width() + 4, pixmap.height() +4);

            int xoff = 0, yoff = 0;
            // The additional offsets were found by try and error, TODO: find the rational behind them
            if ( t == tab( currentTab() ) ) {
                xoff = style().pixelMetric( QStyle::PM_TabBarTabShiftHorizontal, this ) + 3;
                yoff = style().pixelMetric( QStyle::PM_TabBarTabShiftVertical, this ) - 4;
            }
            else {
                xoff = 7;
                yoff = 0;
            }
            rect.moveLeft( t->rect().left() + 2 + xoff );
            rect.moveTop( t->rect().center().y()-pixmap.height()/2 + yoff );
            if ( rect.contains( e->pos() ) ) {
                if ( mHoverCloseButton ) {
                    if ( mHoverCloseButtonTab == t )
                        return;
                    mEnableCloseButtonTimer->stop();
                    delete mHoverCloseButton;
                }

                mHoverCloseButton = new QPushButton( this );
                mHoverCloseButton->setIcon( KGlobal::iconLoader()->loadIconSet("fileclose", K3Icon::Toolbar, K3Icon::SizeSmall) );
                mHoverCloseButton->setGeometry( rect );
                QToolTip::add(mHoverCloseButton,i18n("Close this tab"));
                mHoverCloseButton->setFlat(true);
                mHoverCloseButton->show();
                if ( mHoverCloseButtonDelayed ) {
                  mHoverCloseButton->setEnabled(false);
                  mEnableCloseButtonTimer->start( QApplication::doubleClickInterval(), true );
                }
                mHoverCloseButtonTab = t;
                connect( mHoverCloseButton, SIGNAL( clicked() ), SLOT( closeButtonClicked() ) );
                return;
            }
        }
        if ( mHoverCloseButton ) {
            mEnableCloseButtonTimer->stop();
            delete mHoverCloseButton;
            mHoverCloseButton = 0;
        }
    }*/

    QTabBar::mouseMoveEvent( e );
}

/*void KTabBar::enableCloseButton()
{
    mHoverCloseButton->setEnabled(true);
}*/

void KTabBar::activateDragSwitchTab()
{
    int tab = selectTab( mapFromGlobal( QCursor::pos() ) );
    if ( tab != -1 && mDragSwitchTab == tab )
        setCurrentIndex( mDragSwitchTab );
    mDragSwitchTab = 0;
}

void KTabBar::mouseReleaseEvent( QMouseEvent *e )
{
    if( e->button() == Qt::MidButton ) {
        if ( mReorderStartTab==-1 ) {
            int tab = selectTab( e->pos() );
            if( tab != -1 ) {
                emit( mouseMiddleClick( tab ) );
                return;
            }
        }
        else {
            releaseMouse();
            setCursor( Qt::ArrowCursor );
            mReorderStartTab=-1;
            mReorderPreviousTab=-1;
        }
    }
    QTabBar::mouseReleaseEvent( e );
}

void KTabBar::dragEnterEvent( QDragEnterEvent *e )
{
    e->setAccepted( true );
    QTabBar::dragEnterEvent( e );
}

void KTabBar::dragMoveEvent( QDragMoveEvent *e )
{
    int tab = selectTab( e->pos() );
    if( tab != -1 ) {
        bool accept = false;
        // The receivers of the testCanDecode() signal has to adjust
        // 'accept' accordingly.
        emit testCanDecode( e, accept);
        if ( accept && tab != currentIndex() ) {
          mDragSwitchTab = tab;
          mActivateDragSwitchTabTimer->start( QApplication::doubleClickInterval()*2);
        }
        e->setAccepted( accept );
        return;
    }
    e->setAccepted( false );
    QTabBar::dragMoveEvent( e );
}

void KTabBar::dropEvent( QDropEvent *e )
{
    int tab = selectTab( e->pos() );
    if( tab != -1 ) {
        mActivateDragSwitchTabTimer->stop();
        mDragSwitchTab = 0;
        emit( receivedDropEvent( tab , e ) );
        return;
    }
    QTabBar::dropEvent( e );
}

#ifndef QT_NO_WHEELEVENT
void KTabBar::wheelEvent( QWheelEvent *e )
{
    if ( e->orientation() == Qt::Horizontal )
        return;

    emit( wheelDelta( e->delta() ) );
}
#endif

/*int KTabBar::insertTab( QTab *t, int index )
{
    int res = QTabBar::insertTab( t, index );

    if ( mTabCloseActivatePrevious && count() > 2 ) {
        Q3PtrList<QTab> *tablist = tabList();
        tablist->insert( count()-2, tablist->take( tablist->findRef( t ) ) );
    }

    return res;
}
*/

bool KTabBar::isTabReorderingEnabled() const
{
    return mTabReorderingEnabled;
}

void KTabBar::setTabReorderingEnabled( bool on )
{
    mTabReorderingEnabled = on;
}

bool KTabBar::tabCloseActivatePrevious() const
{
    return mTabCloseActivatePrevious;
}

void KTabBar::setTabCloseActivatePrevious( bool on )
{
    mTabCloseActivatePrevious = on;
}

/*void KTabBar::closeButtonClicked()
{
    emit closeRequest( indexOf( mHoverCloseButtonTab->identifier() ) );
}*/

void KTabBar::setHoverCloseButton( bool /*button*/ )
{
    // KDE4 porting: disabled
    return;
    
    /*mHoverCloseButtonEnabled = button;
    if ( !button )
        tabLayoutChange();*/
}

bool KTabBar::hoverCloseButton() const
{
    // KDE4 porting: disabled
    return false;
    
    //return mHoverCloseButtonEnabled;
}

void KTabBar::setHoverCloseButtonDelayed( bool /*delayed*/ )
{
    // KDE4 porting: disabled
    return;
    
    //mHoverCloseButtonDelayed = delayed;
}

bool KTabBar::hoverCloseButtonDelayed() const
{
    // KDE4 porting: disabled
    return false;
    
    //return mHoverCloseButtonDelayed;
}

void KTabBar::tabLayoutChange()
{
    //mEnableCloseButtonTimer->stop();
    //delete mHoverCloseButton;
    //mHoverCloseButton = 0;
    //mHoverCloseButtonTab = 0;
    mActivateDragSwitchTabTimer->stop();
    mDragSwitchTab = 0;
}

int KTabBar::selectTab( const QPoint & pos ) const
{
    for (int i = 0; i < count(); ++i)
        if (tabRect(i).contains(pos))
            return i;
    return -1;
}

#include "ktabbar.moc"
