/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef __kmimetyperesolver_h
#define __kmimetyperesolver_h

#include <q3scrollview.h>
#include <QtCore/QList>
#include <qtimer.h>
#include <kdebug.h>
#include <QtCore/QEvent>

/**
 * @internal
 * A baseclass for K3MimeTypeResolver, with the interface,
 * K3MimeTypeResolverHelper uses.
 */
class KIO_EXPORT K3MimeTypeResolverBase
{
public:
    virtual ~K3MimeTypeResolverBase() {}
    virtual void slotViewportAdjusted() = 0;
    virtual void slotProcessMimeIcons() = 0;
protected:
    virtual void virtual_hook( int, void* ) {}
};

/**
 * @internal
 * This class is used by K3MimeTypeResolver, because it can't be a QObject
 * itself. So an object of this class is used to handle signals, slots etc.
 * and forwards them to the K3MimeTypeResolver instance.
 */
class KIO_EXPORT K3MimeTypeResolverHelper : public QObject
{
    Q_OBJECT

public:
    K3MimeTypeResolverHelper( K3MimeTypeResolverBase *resolver,
                             Q3ScrollView *view )
        : m_resolver( resolver ),
          m_timer()
    {
        m_timer.setSingleShot( true );
        connect( &m_timer, SIGNAL( timeout() ), SLOT( slotProcessMimeIcons() ));

        connect( view->horizontalScrollBar(), SIGNAL( sliderMoved(int) ),
                 SLOT( slotAdjust() ) );
        connect( view->verticalScrollBar(), SIGNAL( sliderMoved(int) ),
                 SLOT( slotAdjust() ) );

        view->viewport()->installEventFilter( this );
    }

    void start( int delay )
    {
        m_timer.start( delay );
    }

protected:
    virtual bool eventFilter( QObject *o, QEvent *e )
    {
        bool ret = QObject::eventFilter( o, e );

        if ( e->type() == QEvent::Resize )
            m_resolver->slotViewportAdjusted();

        return ret;
    }

private Q_SLOTS:
    void slotProcessMimeIcons()
    {
        m_resolver->slotProcessMimeIcons();
    }

    void slotAdjust()
    {
        m_resolver->slotViewportAdjusted();
    }

private:
    K3MimeTypeResolverBase *m_resolver;
    QTimer m_timer;
};

/**
 * This class implements the "delayed-mimetype-determination" feature,
 * for konqueror's directory views (and KFileDialog's :).
 *
 * It determines the mimetypes of the icons in the background, but giving
 * preferrence to the visible icons.
 *
 * It is implemented as a template, so that it can work with both QPtrListViewItem
 * and QIconViewItem, without requiring hacks such as void * or QPtrDict lookups.
 *
 * Here's what the parent must implement :
 * @li void mimeTypeDeterminationFinished();
 * @li QScrollView * scrollWidget();
 * @li void determineIcon( IconItem * item ), which should call
 * @li KFileItem::determineMimeType on the fileItem, and update the icon, etc.
*/
template<class IconItem, class Parent>
class K3MimeTypeResolver : public K3MimeTypeResolverBase // if only this could be a QObject....
{
public:
  /**
   * Creates a new K3MimeTypeResolver with the given parent.
   * @param parent the parent's resolver
   */
    K3MimeTypeResolver( Parent * parent )
        : m_parent(parent),
          m_helper( new K3MimeTypeResolverHelper(this, parent->scrollWidget())),
          m_delayNonVisibleIcons(10)
    {}

    virtual ~K3MimeTypeResolver() {
        delete m_helper;
    }

    /**
     * Start the mimetype-determination. Call this when the listing is completed.
     * @param delayNonVisibleIcons the delay to use between icons not on screen.
     * Usually 10, but should be set to 0 when the image preview feature is
     * activated, because image preview can only start once we know the mimetypes
     */
    void start( uint delayNonVisibleIcons = 10 )
    {
        m_helper->start( 0 );
        m_delayNonVisibleIcons = delayNonVisibleIcons;
    }

    /**
     * The list of items to process. The view is free to
     * clear it, insert new items into it, remove items, etc.
     * @return the list of items to process
     */
    QList<IconItem *> m_lstPendingMimeIconItems;

    /**
     * "Connected" to the viewportAdjusted signal of the scrollview
     */
    virtual void slotViewportAdjusted();

    /**
     * "Connected" to the timer
     */
    virtual void slotProcessMimeIcons();

private:
    /**
     * Find a visible icon and determine its mimetype.
     * KonqDirPart will call this method repeatedly until it returns 0L
     * (no more visible icon to process).
     * @return the file item that was just processed.
     */
    IconItem * findVisibleIcon();

    Parent * m_parent;
    K3MimeTypeResolverHelper *m_helper;
    uint m_delayNonVisibleIcons;
};

// The main slot
template<class IconItem, class Parent>
inline void K3MimeTypeResolver<IconItem, Parent>::slotProcessMimeIcons()
{
    //kDebug(1203) << "K3MimeTypeResolver::slotProcessMimeIcons() "
    //              << m_lstPendingMimeIconItems.count() << endl;
    IconItem * item = 0L;
    int nextDelay = 0;

    if ( m_lstPendingMimeIconItems.count() > 0 )
    {
        // We only find mimetypes for icons that are visible. When more
        // of our viewport is exposed, we'll get a signal and then get
        // the mimetypes for the newly visible icons. (Rikkus)
        item = findVisibleIcon();
        m_lstPendingMimeIconItems.removeAll(item);
    }

    // No more visible items.
    if (0 == item)
    {
        // Do the unvisible ones, then, but with a bigger delay, if so configured
        if ( m_lstPendingMimeIconItems.count() > 0 )
        {
            item = m_lstPendingMimeIconItems.takeFirst();
            nextDelay = m_delayNonVisibleIcons;
        }
        else
        {
            m_parent->mimeTypeDeterminationFinished();
            return;
        }
    }

    m_parent->determineIcon(item);
    m_helper->start( nextDelay );
}

template<class IconItem, class Parent>
inline void K3MimeTypeResolver<IconItem, Parent>::slotViewportAdjusted()
{
    if (m_lstPendingMimeIconItems.isEmpty()) return;
    IconItem * item = findVisibleIcon();
    if (item)
    {
        m_parent->determineIcon( item );
        m_lstPendingMimeIconItems.removeAll(item);
        m_helper->start( 0 );
    }
}

template<class IconItem, class Parent>
inline IconItem * K3MimeTypeResolver<IconItem, Parent>::findVisibleIcon()
{
    // Find an icon that's visible and whose mimetype we don't know.

    if ( m_lstPendingMimeIconItems.count()<20) // for few items, it's faster to not bother
        return m_lstPendingMimeIconItems.first();

    Q3ScrollView * view = m_parent->scrollWidget();
    QRect visibleContentsRect
        (
            view->viewportToContents(QPoint(0, 0)),
            view->viewportToContents
            (
                QPoint(view->visibleWidth(), view->visibleHeight())
                )
            );

    typename QList<IconItem *>::const_iterator it = m_lstPendingMimeIconItems.begin(),
                                              end = m_lstPendingMimeIconItems.end();
    for ( ; it != end ; ++it ) {
        if (visibleContentsRect.intersects((*it)->rect()))
            return *it;
    }

    return 0L;
}

#endif
