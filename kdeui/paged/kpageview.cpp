/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Tobias Koenig (tokoe@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kpageview.h"
#include "kpageview_p.h"

#include "kpagemodel.h"

#include <kdialog.h>
#include <kiconloader.h>
#include <ktitlewidget.h>

#include <QAbstractItemView>
#include <QGridLayout>
#include <QLabel>
#include <QSize>
#include <QStackedWidget>
#include <QTimer>

class KPageStackedWidget : public QStackedWidget
{
  public:
    KPageStackedWidget( QWidget *parent = 0 )
      : QStackedWidget( parent )
    {
    }

    void setMinimumSize( const QSize& size )
    {
      mMinimumSize = size;
    }

    virtual QSize minimumSizeHint () const
    {
      return mMinimumSize.expandedTo( QStackedWidget::minimumSizeHint() );
    }

  private:
    QSize mMinimumSize;
};

class KPageView::Private
{
  public:
    Private( KPageView *_parent )
      : parent( _parent ), model( 0 ),
        faceType( Auto ), view( 0 )
    {
    }

    void updateTitleWidget(const QModelIndex& index);

    KPageView* parent;

    // data
    KPageModel *model;
    FaceType faceType;

    // gui
    KPageStackedWidget *stack;
    KTitleWidget *titleWidget;
    QGridLayout *layout;

    QAbstractItemView *view;

    void rebuildGui();
    void updateSelection();
    void cleanupPages();
    QList<QWidget*> collectPages( const QModelIndex &parent = QModelIndex() );
    FaceType detectAutoFace() const;
    void modelChanged();
    void dataChanged( const QModelIndex&, const QModelIndex& );
    void pageSelected( const QModelIndex&, const QModelIndex& );
};

void KPageView::Private::rebuildGui()
{
  // clean up old view
  if ( view && view->selectionModel() ) {
    disconnect( view->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
                parent, SLOT( pageSelected( const QModelIndex&, const QModelIndex& ) ) );
  }

  delete view;
  view = parent->createView();

  Q_ASSERT( view );

  view->setSelectionBehavior( QAbstractItemView::SelectItems );
  view->setSelectionMode( QAbstractItemView::SingleSelection );

  if ( model ) {
    view->setModel( model );
  }

  // setup new view
  if ( view->selectionModel() ) {
    connect( view->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
             parent, SLOT( pageSelected( const QModelIndex&, const QModelIndex& ) ) );

    if ( model )
      view->selectionModel()->setCurrentIndex( model->index( 0, 0 ), QItemSelectionModel::Select );
  }

  titleWidget->setVisible(parent->showPageHeader());

  Qt::Alignment alignment = parent->viewPosition();
  if ( alignment & Qt::AlignTop )
    layout->addWidget( view, 0, 1 );
  else if ( alignment & Qt::AlignRight )
    layout->addWidget( view, 1, 2, 3, 1 );
  else if ( alignment & Qt::AlignBottom )
    layout->addWidget( view, 4, 1 );
  else if ( alignment & Qt::AlignLeft )
    layout->addWidget( view, 1, 0, 3, 1 );
}

void KPageView::Private::updateSelection()
{
  /**
   * Select the first item in the view if not done yet.
   */

  if ( !model )
    return;

  if ( !view || !view->selectionModel() )
    return;

  const QModelIndex index = view->selectionModel()->currentIndex();
  if ( !index.isValid() )
    view->selectionModel()->setCurrentIndex( model->index( 0, 0 ), QItemSelectionModel::Select );
}

void KPageView::Private::cleanupPages()
{
  /**
   * Remove all orphan pages from the stacked widget.
   */

  QList<QWidget*> widgets = collectPages();

  for ( int i = 0; i < stack->count(); ++i ) {
    QWidget *page = stack->widget( i );

    bool found = false;
    for ( int j = 0; j < widgets.count(); ++j ) {
      if ( widgets[ j ] == page )
        found = true;
    }

    if ( !found )
      stack->removeWidget( page );
  }
}

QList<QWidget*> KPageView::Private::collectPages( const QModelIndex &parentIndex )
{
  /**
   * Traverse through the model recursive and collect all widgets in
   * a list.
   */
  QList<QWidget*> retval;

  int rows = model->rowCount( parentIndex );
  for ( int j = 0; j < rows; ++j ) {
    const QModelIndex index = model->index( j, 0, parentIndex );
    retval.append( qvariant_cast<QWidget*>( model->data( index, KPageModel::WidgetRole ) ) );

    if ( model->rowCount( index ) > 0 ) {
      retval += collectPages( index );
    }
  }

  return retval;
}

KPageView::FaceType KPageView::Private::detectAutoFace() const
{
  if ( !model )
    return KPageView::Plain;

  /**
   * Check whether the model has sub pages.
   */
  bool hasSubPages = false;
  const int count = model->rowCount();
  for ( int i = 0; i < count; ++i ) {
    if ( model->rowCount( model->index( i, 0 ) ) > 0 ) {
      hasSubPages = true;
      break;
    }
  }

  if ( hasSubPages )
    return KPageView::Tree;

  if ( model->rowCount() > 1 )
    return KPageView::List;

  return KPageView::Plain;
}

void KPageView::Private::modelChanged()
{
  if ( !model )
    return;

  /**
   * If the face type is Auto, we rebuild the GUI whenever the layout
   * of the model changes.
   *
   * We have to decouple the method call here, since the view which
   * called use is deleted.
   */
  if ( faceType == Auto )
    QTimer::singleShot( 0, parent, SLOT( rebuildGui() ) );

  /**
   * Set the stack to the minimum size of the largest widget.
   */
  QSize size = stack->size();
  QList<QWidget*> widgets = collectPages();
  for ( int i = 0; i < widgets.count(); ++i ) {
    const QWidget *widget = widgets[ i ];
    if ( widget )
      size = size.expandedTo( widget->minimumSizeHint() );
  }
  stack->setMinimumSize( size );

  updateSelection();
}

void KPageView::Private::pageSelected( const QModelIndex &index, const QModelIndex &previous )
{
  if ( !model )
    return;

  if ( !index.isValid() )
    return;

  QWidget *widget = qvariant_cast<QWidget*>( model->data( index, KPageModel::WidgetRole ) );
  if ( widget ) {
    if ( stack->indexOf( widget ) == -1 ) { // not included yet
      stack->addWidget( widget );
    }

    stack->setCurrentWidget( widget );
  } else {
    //d->stack->setCurrentWidget( d->emptyWidget );
  }

  updateTitleWidget(index);

  emit parent->currentPageChanged( index, previous );
}

void KPageView::Private::updateTitleWidget(const QModelIndex& index)
{
  if (!parent->showPageHeader())
    return;

  QString header = model->data( index, KPageModel::HeaderRole ).toString();
  if ( header.isEmpty() ) {
    header = model->data( index, Qt::DisplayRole ).toString();
  }
  titleWidget->setText(header.remove('&'));
  titleWidget->setVisible(parent->showPageHeader());
  const QIcon icon = model->data( index, Qt::DecorationRole ).value<QIcon>();
  titleWidget->setPixmap(icon.pixmap(22, 22));
}

void KPageView::Private::dataChanged( const QModelIndex&, const QModelIndex& )
{
  /**
   * When data has changed we update the header and icon for the currently selected
   * page.
   */
  if ( !view )
    return;

  QModelIndex index = view->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return;

  updateTitleWidget(index);
}

/**
 * KPageView Implementation
 */
KPageView::KPageView( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  d->layout = new QGridLayout( this );
  d->stack = new KPageStackedWidget( this );

  d->titleWidget = new KTitleWidget(this);

  d->layout->addWidget(d->titleWidget, 1, 1);
  d->layout->addWidget( d->stack, 2, 1 );

  // stack should use most space
  d->layout->setColumnStretch( 1, 1 );
  d->layout->setRowStretch( 2, 1 );
}

KPageView::~KPageView()
{
  delete d;
}

void KPageView::setModel( KPageModel *model )
{
  // clean up old model
  if ( d->model ) {
    disconnect( d->model, SIGNAL( layoutChanged() ), this, SLOT( modelChanged() ) );
    disconnect( d->model, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
                this, SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
  }

  d->model = model;

  if ( d->model ) {
    connect( d->model, SIGNAL( layoutChanged() ), this, SLOT( modelChanged() ) );
    connect( d->model, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
             this, SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );

    // set new model in navigation view
    if ( d->view )
      d->view->setModel( model );
  }

  d->rebuildGui();
}

KPageModel* KPageView::model() const
{
  return d->model;
}

void KPageView::setFaceType( FaceType faceType )
{
  d->faceType = faceType;

  d->rebuildGui();
}

KPageView::FaceType KPageView::faceType() const
{
  return d->faceType;
}

void KPageView::setCurrentPage( const QModelIndex &index )
{
  if ( !d->view || !d->view->selectionModel() )
    return;

  d->view->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Select );
}

QModelIndex KPageView::currentPage() const
{
  if ( !d->view || !d->view->selectionModel() )
    return QModelIndex();

  return d->view->selectionModel()->currentIndex();
}

void KPageView::setItemDelegate( QAbstractItemDelegate *delegate )
{
  if ( d->view )
    d->view->setItemDelegate( delegate );
}

QAbstractItemDelegate* KPageView::itemDelegate() const
{
  if ( d->view )
    return d->view->itemDelegate();
  else
    return 0;
}


QAbstractItemView* KPageView::createView()
{
  if ( d->faceType == Auto ) {
    const FaceType faceType = d->detectAutoFace();

    if ( faceType == Plain )
      return new KDEPrivate::KPagePlainView( this );
    else if ( faceType == List )
      return new KDEPrivate::KPageListView( this );
    else if ( faceType == Tree )
      return new KDEPrivate::KPageTreeView( this );
    else // should never happen
      return 0;
  } else if ( d->faceType == Plain )
    return new KDEPrivate::KPagePlainView( this );
  else if ( d->faceType == List )
    return new KDEPrivate::KPageListView( this );
  else if ( d->faceType == Tree )
    return new KDEPrivate::KPageTreeView( this );
  else if ( d->faceType == Tabbed )
    return new KDEPrivate::KPageTabbedView( this );
  else
    return 0;
}

bool KPageView::showPageHeader() const
{
  FaceType faceType = d->faceType;

  if ( faceType == Auto )
    faceType = d->detectAutoFace();

  if ( faceType == Tabbed ) {
    return false;
  }
  else {
    return !d->titleWidget->text().isEmpty();
  }
}

Qt::Alignment KPageView::viewPosition() const
{
  FaceType faceType = d->faceType;

  if ( faceType == Auto )
    faceType = d->detectAutoFace();

  if ( faceType == Plain || faceType == Tabbed )
    return Qt::AlignTop;
  else
    return Qt::AlignLeft;
}

#include "kpageview.moc"
