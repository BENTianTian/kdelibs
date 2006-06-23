// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>

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

#ifndef KFILEDETAILVIEW_H
#define KFILEDETAILVIEW_H

class KFileItem;
class QWidget;
class QKeyEvent;

#include <k3listview.h>
#include <kmimetyperesolver.h>

#include "kfileview.h"

/**
 * An item for the listiew, that has a reference to its corresponding
 * KFileItem.
 */
class KIO_EXPORT KFileListViewItem : public K3ListViewItem
{
public:
    KFileListViewItem( Q3ListView *parent, const QString &text,
		       const QPixmap &icon, KFileItem *fi )
	: K3ListViewItem( parent, text ), inf( fi ) {
        setPixmap( 0, icon );
        setText( 0, text );
    }

    KFileListViewItem( Q3ListView *parent, KFileItem *fi )
        : K3ListViewItem( parent ), inf( fi ) {
        init();
    }

    KFileListViewItem( Q3ListView *parent, const QString &text,
		       const QPixmap &icon, KFileItem *fi,
		       Q3ListViewItem *after)
	: K3ListViewItem( parent, after ), inf( fi ) {
        setPixmap( 0, icon );
        setText( 0, text );
    }
    ~KFileListViewItem() {
        inf->removeExtraData( listView() );
    }

    /**
     * @returns the corresponding KFileItem
     */
    KFileItem *fileInfo() const {
	return inf;
    }

    virtual QString key( int /*column*/, bool /*ascending*/ ) const {
        return m_key;
    }

    void setKey( const QString& key ) { m_key = key; }

    QRect rect() const
    {
        QRect r = listView()->itemRect(this);
        return QRect( listView()->viewportToContents( r.topLeft() ),
                      QSize( r.width(), r.height() ) );
    }

    void init();

private:
    KFileItem *inf;
    QString m_key;

private:
    class KFileListViewItemPrivate;
    KFileListViewItemPrivate* d;

};

/**
 * A list-view capable of showing KFileItem'. Used in the filedialog
 * for example. Most of the documentation is in KFileView class.
 *
 * @see KDirOperator
 * @see KCombiView
 * @see KFileIconView
 */
class KIO_EXPORT KFileDetailView : public K3ListView, public KFileView
{
    Q_OBJECT

public:
    KFileDetailView(QWidget *parent);
    virtual ~KFileDetailView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();

    virtual void setSelectionMode( KFile::SelectionMode sm );

    virtual void updateView( bool );
    virtual void updateView(const KFileItem*);
    virtual void removeItem( const KFileItem *);
    virtual void listingCompleted();

    virtual void setSelected(const KFileItem *, bool);
    virtual bool isSelected(const KFileItem *i) const;
    virtual void clearSelection();
    virtual void selectAll();
    virtual void invertSelection();

    virtual void setCurrentItem( const KFileItem * );
    virtual KFileItem * currentFileItem() const;
    virtual KFileItem * firstFileItem() const;
    virtual KFileItem * nextItem( const KFileItem * ) const;
    virtual KFileItem * prevItem( const KFileItem * ) const;

    virtual void insertItem( KFileItem *i );

    // implemented to get noticed about sorting changes (for sortingIndicator)
    virtual void setSorting( QDir::SortFlags );

    void ensureItemVisible( const KFileItem * );

    // for KMimeTypeResolver
    void mimeTypeDeterminationFinished();
    void determineIcon( KFileListViewItem *item );
    Q3ScrollView *scrollWidget() const { return (Q3ScrollView*) this; }

    virtual void readConfig( KConfig *, const QString& group = QString() );
    virtual void writeConfig( KConfig *, const QString& group = QString());

Q_SIGNALS:
    /**
     * The user dropped something.
     * @p fileItem points to the item dropped on or can be 0 if the
     * user dropped on empty space.
     */
    void dropped(QDropEvent *event, KFileItem *fileItem);
    /**
     * The user dropped the URLs @p urls.
     * @p url points to the item dropped on or can be empty if the
     * user dropped on empty space.
     */
    void dropped(QDropEvent *event, const KUrl::List &urls, const KUrl &url);

protected:
    virtual void keyPressEvent( QKeyEvent * );

    // DND support
    virtual Q3DragObject *dragObject();
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
    virtual void contentsDropEvent( QDropEvent *ev );
    virtual bool acceptDrag(QDropEvent* e ) const;

    int m_sortingCol;

protected Q_SLOTS:
    void slotSelectionChanged();

private Q_SLOTS:
    void slotSortingChanged( int );
    void selected( Q3ListViewItem *item );
    void slotActivate( Q3ListViewItem *item );
    void highlighted( Q3ListViewItem *item );
    void slotActivateMenu ( Q3ListViewItem *item, const QPoint& pos );
    void slotAutoOpen();

private:
    virtual void insertItem(Q3ListViewItem *i) { K3ListView::insertItem(i); }
    virtual void setSorting(int i, bool b) { K3ListView::setSorting(i, b); }
    virtual void setSelected(Q3ListViewItem *i, bool b) { K3ListView::setSelected(i, b); }

    inline KFileListViewItem * viewItem( const KFileItem *item ) const {
        if ( item )
            return (KFileListViewItem *) item->extraData( this );
        return 0L;
    }

    void setSortingKey( KFileListViewItem *item, const KFileItem *i );


    bool m_blockSortingSignal;
    KMimeTypeResolver<KFileListViewItem,KFileDetailView> *m_resolver;

private:
    class KFileDetailViewPrivate;
    KFileDetailViewPrivate* const d;
};

#endif // KFILEDETAILVIEW_H
