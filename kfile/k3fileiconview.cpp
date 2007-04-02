// -*- c++ -*-
/* This file is part of the KDE libraries
   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
                 2000,2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.	If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "k3fileiconview.h"

#include <qapplication.h>
#include <qevent.h>
#include <qfontmetrics.h>
#include <qnamespace.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <kaction.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kiconeffect.h>
#include <kglobalsettings.h>
#include <kio/previewjob.h>
#include <ktoggleaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kconfiggroup.h>

#include "config-kfile.h"

#define DEFAULT_PREVIEW_SIZE 60
#define DEFAULT_SHOW_PREVIEWS false
#define DEFAULT_VIEW_MODE "KFileIconView_SmallColumns"

K3FileIconViewItem::~K3FileIconViewItem()
{
    fileInfo()->removeExtraData( iconView() );
}

class K3FileIconView::K3FileIconViewPrivate
{
public:
    K3FileIconViewPrivate( K3FileIconView *parent ) {
        previewIconSize = 60;
        job = 0;
        dropItem = 0;

        noArrangement = false;
        ignoreMaximumSize = false;
        smallColumns = new KAction( i18n("Small Icons"), parent );
        parent->actionCollection()->addAction( "small columns", smallColumns );
        connect( smallColumns, SIGNAL( triggered( bool ) ), parent, SLOT( slotSmallColumns() ) );

        largeRows = new KAction( i18n("Large Icons"), parent );
        parent->actionCollection()->addAction( "large rows", largeRows );
        connect( largeRows, SIGNAL( triggered( bool ) ), parent, SLOT( slotLargeRows() ) );


        QActionGroup* sizeGroup = new QActionGroup(parent);
        smallColumns->setActionGroup(sizeGroup);
        largeRows->setActionGroup(sizeGroup);

        previews = new KToggleAction( i18n("Thumbnail Previews"), parent );
        parent->actionCollection()->addAction( "show previews", previews );
        zoomIn = parent->actionCollection()->addAction( KStandardAction::ZoomIn, "zoomIn", parent, SLOT( zoomIn() ) );
        zoomOut = parent->actionCollection()->addAction( KStandardAction::ZoomOut, "zoomOut", parent, SLOT( zoomOut() ) );

        connect( previews, SIGNAL( toggled( bool )),
                 parent, SLOT( slotPreviewsToggled( bool )));

        connect( &previewTimer, SIGNAL( timeout() ),
                 parent, SLOT( showPreviews() ));
        connect( &autoOpenTimer, SIGNAL( timeout() ),
                 parent, SLOT( slotAutoOpen() ));
    }

    ~K3FileIconViewPrivate() {
        if ( job )
            job->kill();
    }

    QAction *smallColumns, *largeRows;
    QAction *zoomIn, *zoomOut;
    KToggleAction *previews;
    KIO::PreviewJob *job;
    K3FileIconViewItem *dropItem;
    QTimer previewTimer;
    QTimer autoOpenTimer;
    QStringList previewMimeTypes;
    int previewIconSize;
    bool noArrangement :1;
    bool ignoreMaximumSize :1;
};

K3FileIconView::K3FileIconView(QWidget *parent, const char *name)
    : K3IconView(parent, name), KFileView(),d(new K3FileIconViewPrivate( this ))
{

    setViewName( i18n("Icon View") );
    setItemTextPos( Q3IconView::Right );
    setResizeMode( Adjust );
    setMaxItemWidth( 300 );
    setWordWrapIconText( false );
    setArrangement( TopToBottom );
    setAutoArrange( true );
    setItemsMovable( false );
    setMode( K3IconView::Select );
    K3IconView::setSorting( true );
    // as long as QIconView only shows tooltips when the cursor is over the
    // icon (and not the text), we have to create our own tooltips
    setShowToolTips( false );
    slotSmallColumns();
    d->smallColumns->setChecked( true );

    connect( this, SIGNAL( returnPressed(Q3IconViewItem *) ),
	     SLOT( slotActivate( Q3IconViewItem *) ) );

    // we want single click _and_ double click (as convenience)
    connect( this, SIGNAL( clicked(Q3IconViewItem *, const QPoint&) ),
	     SLOT( selected( Q3IconViewItem *) ) );
    connect( this, SIGNAL( doubleClicked(Q3IconViewItem *, const QPoint&) ),
	     SLOT( slotActivate( Q3IconViewItem *) ) );

    connect( this, SIGNAL( onItem( Q3IconViewItem * ) ),
	     SLOT( showToolTip( Q3IconViewItem * ) ) );
    connect( this, SIGNAL( contextMenuRequested(Q3IconViewItem*,const QPoint&)),
	     SLOT( slotActivateMenu( Q3IconViewItem*, const QPoint& ) ) );

    KFile::SelectionMode sm = KFileView::selectionMode();
    switch ( sm ) {
    case KFile::Multi:
	Q3IconView::setSelectionMode( Q3IconView::Multi );
	break;
    case KFile::Extended:
	Q3IconView::setSelectionMode( Q3IconView::Extended );
	break;
    case KFile::NoSelection:
	Q3IconView::setSelectionMode( Q3IconView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	Q3IconView::setSelectionMode( Q3IconView::Single );
	break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, SIGNAL( selectionChanged() ),
		 SLOT( slotSelectionChanged() ));
    else
	connect( this, SIGNAL( selectionChanged( Q3IconViewItem * )),
		 SLOT( highlighted( Q3IconViewItem * )));

    // for mimetype resolving
    m_resolver = new K3MimeTypeResolver<K3FileIconViewItem,K3FileIconView>(this);
}

K3FileIconView::~K3FileIconView()
{
    delete m_resolver;
    delete d;
}

void K3FileIconView::readConfig( KConfigGroup *configGroup)
{
    QString small = QLatin1String("KFileIconView_SmallColumns");
    d->previewIconSize = configGroup->readEntry( "KFileIconView_Preview Size", DEFAULT_PREVIEW_SIZE );
    d->previews->setChecked( configGroup->readEntry( "KFileIconView_ShowPreviews", DEFAULT_SHOW_PREVIEWS ) );

    if ( configGroup->readEntry("KFileIconView_ViewMode", DEFAULT_VIEW_MODE ) == small ) {
	d->smallColumns->setChecked( true );
	slotSmallColumns();
    }
    else {
	d->largeRows->setChecked( true );
	slotLargeRows();
    }

    if ( d->previews->isChecked() )
        showPreviews();
}

void K3FileIconView::writeConfig( KConfigGroup *configGroup)
{
    QString viewMode =  d->smallColumns->isChecked() ?
        QLatin1String("SmallColumns") :
        QLatin1String("LargeRows");
    if(!configGroup->hasDefault( "KFileIconView_ViewMode" ) && viewMode == DEFAULT_VIEW_MODE )
        configGroup->revertToDefault( "KFileIconView_ViewMode" );
    else
        configGroup->writeEntry( "KFileIconView_ViewMode", viewMode );

    int previewsIconSize = d->previewIconSize;
    if(!configGroup->hasDefault( "KFileIconView_Preview Size" ) && previewsIconSize == DEFAULT_PREVIEW_SIZE )
        configGroup->revertToDefault( "KFileIconView_Preview Size" );
    else
        configGroup->writeEntry( "KFileIconView_Preview Size", previewsIconSize );

    bool showPreviews = d->previews->isChecked();
    if(!configGroup->hasDefault( "KFileIconView_ShowPreviews" ) && showPreviews == DEFAULT_SHOW_PREVIEWS )
        configGroup->revertToDefault( "KFileIconView_ShowPreviews" );
    else
        configGroup->writeEntry( "KFileIconView_ShowPreviews", showPreviews );
}

void K3FileIconView::showToolTip( Q3IconViewItem *item )
{
    if ( !item )
	return;

    int w = maxItemWidth() - ( itemTextPos() == Bottom ? 0 :
			       item->pixmapRect().width() ) - 4;
    if ( fontMetrics().width( item->text() ) >= w ) {
	QToolTip::showText(QCursor::pos() + QPoint( 14, 14 ), QString::fromLatin1(" %1 ").arg(item->text()), this);
    } else {
	QToolTip::showText(QCursor::pos() + QPoint( 14, 14 ), "", this);
    }
}

void K3FileIconView::slotActivateMenu( Q3IconViewItem* item, const QPoint& pos )
{
    if ( !item ) {
	sig->activateMenu( 0, pos );
	return;
    }
    K3FileIconViewItem *i = (K3FileIconViewItem*) item;
    sig->activateMenu( i->fileInfo(), pos );
}

void K3FileIconView::keyPressEvent( QKeyEvent *e )
{
    K3IconView::keyPressEvent( e );

    // ignore Ctrl-Return so that the dialog can catch it.
    if ( (e->modifiers() & Qt::ControlModifier) &&
         (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) )
        e->ignore();
}

void K3FileIconView::setSelected( const KFileItem *info, bool enable )
{
    K3FileIconViewItem *item = viewItem( info );
    if ( item )
        K3IconView::setSelected( item, enable, true );
}

void K3FileIconView::selectAll()
{
    if (KFileView::selectionMode() == KFile::NoSelection ||
        KFileView::selectionMode() == KFile::Single)
	return;

    K3IconView::selectAll( true );
}

void K3FileIconView::clearSelection()
{
    K3IconView::clearSelection();
}

void K3FileIconView::invertSelection()
{
    K3IconView::invertSelection();
}

void K3FileIconView::clearView()
{
    m_resolver->m_lstPendingMimeIconItems.clear();

    K3IconView::clear();
    stopPreview();
}

void K3FileIconView::insertItem( KFileItem *i )
{
    KFileView::insertItem( i );

    Q3IconView* qview = static_cast<Q3IconView*>( this );
    // Since creating and initializing an item leads to a repaint,
    // we disable updates on the IconView for a while.
    qview->setUpdatesEnabled( false );
    K3FileIconViewItem *item = new K3FileIconViewItem( qview, i );
    initItem( item, i, true );
    qview->setUpdatesEnabled( true );

    if ( !i->isMimeTypeKnown() )
        m_resolver->m_lstPendingMimeIconItems.append( item );

    i->setExtraData( this, item );
}

void K3FileIconView::slotActivate( Q3IconViewItem *item )
{
    if ( !item )
	return;
    const KFileItem *fi = ( (K3FileIconViewItem*)item )->fileInfo();
    if ( fi )
	sig->activate( fi );
}

void K3FileIconView::selected( Q3IconViewItem *item )
{
    if ( !item || (QApplication::keyboardModifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) != 0 )
	return;

    if ( KGlobalSettings::singleClick() ) {
	const KFileItem *fi = ( (K3FileIconViewItem*)item )->fileInfo();
	if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
	    sig->activate( fi );
    }
}

void K3FileIconView::setCurrentItem( const KFileItem *item )
{
    K3FileIconViewItem *it = viewItem( item );
    if ( it )
        K3IconView::setCurrentItem( it );
}

KFileItem * K3FileIconView::currentFileItem() const
{
    K3FileIconViewItem *current = static_cast<K3FileIconViewItem*>( currentItem() );
    if ( current )
        return current->fileInfo();

    return 0L;
}

void K3FileIconView::highlighted( Q3IconViewItem *item )
{
    if ( !item )
	return;
    const KFileItem *fi = ( (K3FileIconViewItem*)item )->fileInfo();
    if ( fi )
	sig->highlightFile( fi );
}

void K3FileIconView::setSelectionMode( KFile::SelectionMode sm )
{
    disconnect( SIGNAL( selectionChanged() ), this );
    disconnect( SIGNAL( selectionChanged( Q3IconViewItem * )), this );

    KFileView::setSelectionMode( sm );
    switch ( KFileView::selectionMode() ) {
    case KFile::Multi:
	Q3IconView::setSelectionMode( Q3IconView::Multi );
	break;
    case KFile::Extended:
	Q3IconView::setSelectionMode( Q3IconView::Extended );
	break;
    case KFile::NoSelection:
	Q3IconView::setSelectionMode( Q3IconView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	Q3IconView::setSelectionMode( Q3IconView::Single );
	break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, SIGNAL( selectionChanged() ),
		 SLOT( slotSelectionChanged() ));
    else
	connect( this, SIGNAL( selectionChanged( Q3IconViewItem * )),
		 SLOT( highlighted( Q3IconViewItem * )));
}

bool K3FileIconView::isSelected( const KFileItem *i ) const
{
    K3FileIconViewItem *item = viewItem( i );
    return (item && item->isSelected());
}

void K3FileIconView::updateView( bool b )
{
    if ( !b )
        return; // eh?

    K3FileIconViewItem *item = static_cast<K3FileIconViewItem*>(Q3IconView::firstItem());
    if ( item ) {
        do {
            if ( d->previews->isChecked() ) {
                if ( canPreview( item->fileInfo() ) )
                    item->setPixmapSize( QSize( d->previewIconSize, d->previewIconSize ) );
            }
            else {
                // unset pixmap size (used for previews)
                if ( !item->pixmapSize().isNull() )
                    item->setPixmapSize( QSize( 0, 0 ) );
            }
            // recalculate item parameters but avoid an in-place repaint
            item->setPixmap( (item->fileInfo())->pixmap( myIconSize ), true, false );
            item = static_cast<K3FileIconViewItem *>(item->nextItem());
        } while ( item != 0L );
    }
}

void K3FileIconView::updateView( const KFileItem *i )
{
    K3FileIconViewItem *item = viewItem( i );
    if ( item )
        initItem( item, i, true );
}

void K3FileIconView::removeItem( const KFileItem *i )
{
    if ( !i )
	return;

    if ( d->job )
        d->job->removeItem( i );

    K3FileIconViewItem *item = viewItem( i );
    m_resolver->m_lstPendingMimeIconItems.removeAll( item );
    delete item;

    KFileView::removeItem( i );
}

void K3FileIconView::setIconSize( int size )
{
    myIconSize = size;
    updateIcons();
}

void K3FileIconView::setPreviewSize( int size )
{
    if ( size < 30 )
        size = 30; // minimum

    d->previewIconSize = size;
    if ( d->previews->isChecked() )
        showPreviews();
}

void K3FileIconView::setIgnoreMaximumSize(bool ignoreSize)
{
    d->ignoreMaximumSize = ignoreSize;
}

void K3FileIconView::updateIcons()
{
    updateView( true );
    arrangeItemsInGrid();
}

void K3FileIconView::ensureItemVisible( const KFileItem *i )
{
    K3FileIconViewItem *item = viewItem( i );
    if ( item )
	K3IconView::ensureItemVisible( item );
}

void K3FileIconView::slotSelectionChanged()
{
    sig->highlightFile( 0L );
}

void K3FileIconView::slotSmallColumns()
{
    //kDebug(250) << "slotSmallColumns " << kBacktrace() << endl;

    // setItemTextPos(), setArrangement(), setWordWrapIconText() and
    // setIconSize() all call arrangeItemsInGrid() :( Prevent this.
    d->noArrangement = true; // stop arrangeItemsInGrid()!

    // Make sure to uncheck previews if selected
    if ( d->previews->isChecked() )
    {
        stopPreview();
        d->previews->setChecked( false );
    }
    setGridX( -1 );
    setMaxItemWidth( 300 );
    setItemTextPos( Right );
    setArrangement( TopToBottom );
    setWordWrapIconText( false );
    setSpacing( 0 );

    d->noArrangement = false; // now we can arrange
    setIconSize( K3Icon::SizeSmall );
}

void K3FileIconView::slotLargeRows()
{
    // kDebug(250) << "slotLargeRows " << kBacktrace() << endl;
    // setItemTextPos(), setArrangement(), setWordWrapIconText() and
    // setIconSize() all call arrangeItemsInGrid() :( Prevent this.
    d->noArrangement = true; // stop arrangeItemsInGrid()!

    setGridX( KIconLoader::global()->currentSize( K3Icon::Desktop ) + 50 );
    setItemTextPos( Bottom );
    setArrangement( LeftToRight );
    setWordWrapIconText( true );
    setSpacing( 5 ); // default in QIconView

    d->noArrangement = false; // now we can arrange
    setIconSize( K3Icon::SizeMedium );
}

void K3FileIconView::stopPreview()
{
    if ( d->job ) {
        d->job->kill();
        d->job = 0L;
    }
}

void K3FileIconView::slotPreviewsToggled( bool on )
{
    if ( on )
        showPreviews();
    else {
        stopPreview();
        slotLargeRows();
    }
}

void K3FileIconView::showPreviews()
{
    if ( d->previewMimeTypes.isEmpty() )
        d->previewMimeTypes = KIO::PreviewJob::supportedMimeTypes();

    stopPreview();
    d->previews->setChecked( true );

    if ( !d->largeRows->isChecked() ) {
        d->largeRows->setChecked( true );
        slotLargeRows(); // also sets the icon size and updates the grid
    }
    else {
        updateIcons();
    }

    d->job = KIO::filePreview(*items(), d->previewIconSize,d->previewIconSize);
    d->job->setIgnoreMaximumSize(d->ignoreMaximumSize);

    connect( d->job, SIGNAL( result( KJob * )),
             this, SLOT( slotPreviewResult( KJob * )));
    connect( d->job, SIGNAL( gotPreview( const KFileItem*, const QPixmap& )),
             SLOT( gotPreview( const KFileItem*, const QPixmap& ) ));
//     connect( d->job, SIGNAL( failed( const KFileItem* )),
//              this, SLOT( slotFailed( const KFileItem* ) ));
}

void K3FileIconView::slotPreviewResult( KJob *job )
{
    if ( job == d->job )
        d->job = 0L;
}

void K3FileIconView::gotPreview( const KFileItem *item, const QPixmap& pix )
{
    K3FileIconViewItem *it = viewItem( item );
    if ( it )
        if( item->overlays() & K3Icon::HiddenOverlay )
        {
            QPixmap p( pix );

            KIconEffect::semiTransparent( p );
            it->setPixmap( p );
        }
        else
            it->setPixmap( pix );
}

bool K3FileIconView::canPreview( const KFileItem *item ) const
{
    QStringList::Iterator it = d->previewMimeTypes.begin();
    QRegExp r;
    r.setPatternSyntax( QRegExp::Wildcard );

    for ( ; it != d->previewMimeTypes.end(); ++it ) {
        QString type = *it;
        // the "mimetype" can be "image/*"
        if ( type.at( type.length() - 1 ) == '*' ) {
            r.setPattern( type );
            if ( r.indexIn( item->mimetype() ) != -1 )
                return true;
        }
        else
            if ( item->mimetype() == type )
                return true;
    }

    return false;
}

KFileItem * K3FileIconView::firstFileItem() const
{
    K3FileIconViewItem *item = static_cast<K3FileIconViewItem*>( firstItem() );
    if ( item )
        return item->fileInfo();
    return 0L;
}

KFileItem * K3FileIconView::nextItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        K3FileIconViewItem *item = viewItem( fileItem );
        if ( item && item->nextItem() )
            return ((K3FileIconViewItem*) item->nextItem())->fileInfo();
    }
    return 0L;
}

KFileItem * K3FileIconView::prevItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        K3FileIconViewItem *item = viewItem( fileItem );
        if ( item && item->prevItem() )
            return ((K3FileIconViewItem*) item->prevItem())->fileInfo();
    }
    return 0L;
}

void K3FileIconView::setSorting( QDir::SortFlags spec )
{
    KFileView::setSorting( spec );

    const KFileItemList itemList = *items();
    KFileItemList::const_iterator kit = itemList.begin();
    const KFileItemList::const_iterator kend = itemList.end();
    for ( ; kit != kend; ++kit ) {
        KFileItem *item = *kit;
        QString key;
        if ( spec & QDir::Time ) {
            // warning, time_t is often signed -> cast it
            key = sortingKey( (unsigned long)item->time( KIO::UDS_MODIFICATION_TIME ), item->isDir(), spec );
        }
        else if ( spec & QDir::Size ) {
            key = sortingKey( item->size(), item->isDir(), spec );
        }
        else { // Name or Unsorted
            key = sortingKey( item->text(), item->isDir(), spec );
        }
        viewItem( item )->setKey( key );
    }
    K3IconView::setSorting( true, !isReversed() );
    sort( !isReversed() );
}

//
// mimetype determination on demand
//
void K3FileIconView::mimeTypeDeterminationFinished()
{
    // anything to do?
}

void K3FileIconView::determineIcon( K3FileIconViewItem *item )
{
    (void) item->fileInfo()->determineMimeType();
    updateView( item->fileInfo() );
}

void K3FileIconView::listingCompleted()
{
    arrangeItemsInGrid();

    // QIconView doesn't set the current item automatically, so we have to do
    // that. We don't want to emit selectionChanged() tho.
    if ( !currentItem() ) {
        bool block = signalsBlocked();
        blockSignals( true );
        Q3IconViewItem *item = viewItem( firstFileItem() );
        K3IconView::setCurrentItem( item );
        K3IconView::setSelected( item, false );
        blockSignals( block );
    }

    m_resolver->start( d->previews->isChecked() ? 0 : 10 );
}

/////////////////////////////////////////////////////////////////

void K3FileIconView::initItem( K3FileIconViewItem *item, const KFileItem *i,
                              bool updateTextAndPixmap )
{
    if ( d->previews->isChecked() && canPreview( i ) )
        item->setPixmapSize( QSize( d->previewIconSize, d->previewIconSize ) );

    if ( updateTextAndPixmap )
    {
        // this causes a repaint of the item, which we want to avoid during
        // directory listing, when all items are created. We want to paint all
        // items at once, not every single item in that case.
        item->setText( i->text() , false, false );
        item->setPixmap( i->pixmap( myIconSize ) );
    }

    // see also setSorting()
    QDir::SortFlags spec = KFileView::sorting();

    if ( spec & QDir::Time )
        // warning, time_t is often signed -> cast it
        item->setKey( sortingKey( (unsigned long) i->time( KIO::UDS_MODIFICATION_TIME ),
                                  i->isDir(), spec ));
    else if ( spec & QDir::Size )
        item->setKey( sortingKey( i->size(), i->isDir(), spec ));

    else // Name or Unsorted
        item->setKey( sortingKey( i->text(), i->isDir(), spec ));

    //qDebug("** key for: %s: %s", i->text().toLatin1().constData(), item->key().toLatin1().constData());

    if ( d->previews->isChecked() ) {
        d->previewTimer.setSingleShot( true );
        d->previewTimer.start( 10 );
    }
}

void K3FileIconView::arrangeItemsInGrid( bool update )
{
    if ( d->noArrangement )
        return;

    K3IconView::arrangeItemsInGrid( update );
}

void K3FileIconView::zoomIn()
{
    setPreviewSize( d->previewIconSize + 30 );
}

void K3FileIconView::zoomOut()
{
    setPreviewSize( d->previewIconSize - 30 );
}

// Qt4 porting: once we use QListWidget, this becomes a reimplementation of
// virtual QMimeData *mimeData(const QList<QListWidgetItem*> items) const;
// or better: something at the model level, instead?
Q3DragObject *K3FileIconView::dragObject()
{
    // create a list of the URL:s that we want to drag
    const KUrl::List urls = KFileView::selectedItems()->urlList();
    QPixmap pixmap;
    if( urls.count() > 1 )
        pixmap = DesktopIcon( "kmultiple", iconSize() );
    if( pixmap.isNull() )
        pixmap = currentFileItem()->pixmap( iconSize() );

    QPoint hotspot;
    hotspot.setX( pixmap.width() / 2 );
    hotspot.setY( pixmap.height() / 2 );

#if 0 // TODO there is no more kurldrag, this should use urls.populateMimeData( mimeData ) instead
    Q3DragObject* myDragObject = new KUrlDrag( urls, widget() );
    myDragObject->setPixmap( pixmap, hotspot );
    return myDragObject;
#endif
    return 0;
}

void K3FileIconView::slotAutoOpen()
{
    d->autoOpenTimer.stop();
    if( !d->dropItem )
        return;

    KFileItem *fileItem = d->dropItem->fileInfo();
    if (!fileItem)
        return;

    if( fileItem->isFile() )
        return;

    if ( fileItem->isDir() || fileItem->isLink())
        sig->activate( fileItem );
}

bool K3FileIconView::acceptDrag(QDropEvent* e) const
{
    return KUrl::List::canDecode( e->mimeData() ) &&
       (e->source()!=const_cast<K3FileIconView*>(this)) &&
       ( e->dropAction() == Qt::CopyAction
      || e->dropAction() == Qt::MoveAction
      || e->dropAction() == Qt::LinkAction );
}

void K3FileIconView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptProposedAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    K3FileIconViewItem *item = dynamic_cast<K3FileIconViewItem*>(findItem( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       d->dropItem = item;
       d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void K3FileIconView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptProposedAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    K3FileIconViewItem *item = dynamic_cast<K3FileIconViewItem*>(findItem( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       if (d->dropItem != item)
       {
           d->dropItem = item;
           d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
       }
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void K3FileIconView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();
}

void K3FileIconView::contentsDropEvent( QDropEvent *e )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();

    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptProposedAction();     // Yes

    K3FileIconViewItem *item = dynamic_cast<K3FileIconViewItem*>(findItem( contentsToViewport( e->pos() ) ));
    KFileItem * fileItem = 0;
    if (item)
        fileItem = item->fileInfo();

    emit dropped(e, fileItem);

    KUrl::List urls = KUrl::List::fromMimeData( e->mimeData() );
    if ( !urls.isEmpty() )
    {
        emit dropped( e, urls, fileItem ? fileItem->url() : KUrl() );
        sig->dropURLs( fileItem, e, urls );
    }
}

#include "k3fileiconview.moc"
