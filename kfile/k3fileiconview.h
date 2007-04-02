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

#ifndef K3FILEICONVIEW_H
#define K3FILEICONVIEW_H

class KFileItem;
class QWidget;
class QLabel;

#include <k3iconview.h>
#include <kiconloader.h>
#include <kfileview.h>
#include <k3mimetyperesolver.h>
#include <kfile.h>

/**
 * An item for the iconview, that has a reference to its corresponding
 * KFileItem.
 */
class KFILE_EXPORT K3FileIconViewItem : public K3IconViewItem
{
public:
    K3FileIconViewItem( Q3IconView *parent, const QString &text,
		       const QPixmap &pixmap,
		       KFileItem *fi )
	: K3IconViewItem( parent, text, pixmap ), inf( fi ) {}
    K3FileIconViewItem( Q3IconView *parent, KFileItem *fi )
	: K3IconViewItem( parent ), inf( fi ) {}

    virtual ~K3FileIconViewItem();

    /**
     * @returns the corresponding KFileItem
     */
    KFileItem *fileInfo() const {
	return inf;
    }

private:
    KFileItem *inf;

private:
    class K3FileIconViewItemPrivate;
    K3FileIconViewItemPrivate* d;

};

class KJob;
namespace KIO {
    class Job;
}

/**
 * An icon-view capable of showing KFileItem's. Used in the filedialog
 * for example. Most of the documentation is in KFileView class.
 *
 * @see KDirOperator
 * @see KCombiView
 * @see KFileDetailView
 */
class KFILE_EXPORT K3FileIconView : public K3IconView, public KFileView
{
    Q_OBJECT

public:
    K3FileIconView(QWidget *parent, const char *name);
    virtual ~K3FileIconView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();

    virtual void updateView( bool );
    virtual void updateView(const KFileItem*);
    virtual void removeItem(const KFileItem*);

    virtual void listingCompleted();

    virtual void insertItem( KFileItem *i );
    virtual void setSelectionMode( KFile::SelectionMode sm );

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

    /**
     * Sets the size of the icons to show. Defaults to K3Icon::SizeSmall.
     */
    void setIconSize( int size );

    /**
     * Sets the size of the previews. Defaults to K3Icon::SizeLarge.
     */
    void setPreviewSize( int size );

    /**
     * Disables the "Maximum file size" configuration option for previews
     *
     * Set this before calling showPreviews()
     **/
    void setIgnoreMaximumSize(bool ignoreSize=true);

    /**
     * @returns the current size used for icons.
     */
    int iconSize() const { return myIconSize; }

    void ensureItemVisible( const KFileItem * );

    virtual void setSorting(QDir::SortFlags sort);

    virtual void readConfig( KConfigGroup *configGroup );
    virtual void writeConfig( KConfigGroup *configGroup );

    // for K3MimeTypeResolver
    void mimeTypeDeterminationFinished();
    void determineIcon( K3FileIconViewItem *item );
    Q3ScrollView *scrollWidget() const { return (Q3ScrollView*) this; }
    void setAcceptDrops(bool b)
    {
      K3IconView::setAcceptDrops(b);
      viewport()->setAcceptDrops(b);
    }

public Q_SLOTS:
    /**
     * Starts loading previews for all files shown and shows them. Switches
     * into 'large rows' mode, if that isn't the current mode yet.
     *
     * @sa setIgnoreMaximumSize
     */
    void showPreviews();

    void zoomIn();

    void zoomOut();

    /**
     * Reimplemented for performance reasons.
     */
    virtual void arrangeItemsInGrid( bool updated = true );

protected:
    /**
     * Reimplemented to not let QIconView eat return-key events
     */
    virtual void keyPressEvent( QKeyEvent * );

    // DND support
    virtual Q3DragObject *dragObject();
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
    virtual void contentsDropEvent( QDropEvent *ev );

    virtual bool acceptDrag(QDropEvent* e ) const;

private Q_SLOTS:
    void selected( Q3IconViewItem *item );
    void slotActivate( Q3IconViewItem * );
    void highlighted( Q3IconViewItem *item );
    void showToolTip( Q3IconViewItem *item );
    void slotActivateMenu( Q3IconViewItem *, const QPoint& );
    void slotSelectionChanged();

    void slotSmallColumns();
    void slotLargeRows();
    void slotPreviewsToggled( bool );

    void slotPreviewResult( KJob * );
    void gotPreview( const KFileItem *item, const QPixmap& pix );
    void slotAutoOpen();

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

private:
    K3MimeTypeResolver<K3FileIconViewItem,K3FileIconView> *m_resolver;

    int th;
    int myIconSize;

    virtual void insertItem(Q3IconViewItem *a, Q3IconViewItem *b) { K3IconView::insertItem(a, b); }
    virtual void setSelectionMode(Q3IconView::SelectionMode m) { K3IconView::setSelectionMode(m); }
    virtual void setSelected(Q3IconViewItem *i, bool a, bool b) { K3IconView::setSelected(i, a, b); }

    bool canPreview( const KFileItem * ) const;
    void stopPreview();

    void updateIcons();

    inline K3FileIconViewItem * viewItem( const KFileItem *item ) const {
        if ( item )
            return (K3FileIconViewItem *) item->extraData( this );
        return 0L;
    }

    void initItem(K3FileIconViewItem *item, const KFileItem *i,
                  bool updateTextAndPixmap );

private:
    class K3FileIconViewPrivate;
    K3FileIconViewPrivate* const d;
};

#endif // KFILESIMPLEVIEW_H
