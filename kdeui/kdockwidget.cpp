/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>

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
#include "kdockwidget.h"
#include "kdockwidget_private.h"
#include <kdocktabctl.h>

#include <qpopupmenu.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qobjcoll.h>

#include <kapp.h>
#include <kconfig.h>
#include <ktoolbar.h>

#define DOCK_CONFIG_VERSION "0.0.5"

static const char*close_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"#...#",
".#.#.",
"..#..",
".#.#.",
"#...#"};

static const char*not_close_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"#####",
"#...#",
"#...#",
"#...#",
"#####"};

static const char*dock_close_bottom[]={
"16 16 4 1",
"b c #c0c000",
"# c #c0c0c0",
"a c #868686",
". c #ffffff",
"...............#",
".##############a",
".##############a",
".##############a",
".##############a",
".##############a",
".##############a",
".##############a",
"#aaaaaaaaaaaaaaa",
"...............#",
".bbbbbbbbbbbbbba",
".bbbbbbbbbbbbbba",
".bbbbbbbbbbbbbba",
".bbbbbbbbbbbbbba",
".bbbbbbbbbbbbbba",
"#aaaaaaaaaaaaaaa"};

static const char*dock_close_left[]={
"16 16 4 1",
"a c #c0c000",
"# c #c0c0c0",
"b c #868686",
". c #ffffff",
"......#........#",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
".aaaaab.#######b",
"#bbbbbb#bbbbbbbb"};

static const char*dock_close_right[]={
"16 16 4 1",
"b c #c0c000",
"# c #c0c0c0",
"a c #868686",
". c #ffffff",
"........#......#",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
".#######a.bbbbba",
"#aaaaaaaa#aaaaaa"};

static const char*dock_close_top[]={
"16 16 4 1",
"a c #c0c000",
"# c #c0c0c0",
"b c #868686",
". c #ffffff",
"...............#",
".aaaaaaaaaaaaaab",
".aaaaaaaaaaaaaab",
".aaaaaaaaaaaaaab",
".aaaaaaaaaaaaaab",
".aaaaaaaaaaaaaab",
"#bbbbbbbbbbbbbbb",
"...............#",
".##############b",
".##############b",
".##############b",
".##############b",
".##############b",
".##############b",
".##############b",
"#bbbbbbbbbbbbbbb"};

/**
 * A special kind of KTMainWindow that is able to have dockwidget child widgets.
 *
 * The main widget should be a dockwidget where other dockwidgets can be docked to
 * the left, right, top, bottom or to the middle.
 * Furthermore, the KDockMainWindow has got the KDocManager and some data about the dock states.
 *
 * @author Max Judin.
 * @version $Id$
*/
KDockMainWindow::KDockMainWindow( const char *name )
:KTMainWindow( name )
{
  DockB.dock = 0L;
  DockB.pos  = KDockWidget::DockBottom;

  DockT.dock = 0L;
  DockT.pos  = KDockWidget::DockTop;

  DockR.dock = 0L;
  DockR.pos  = KDockWidget::DockRight;

  DockL.dock = 0L;
  DockL.pos  = KDockWidget::DockLeft;

  QString new_name = QString(name) + QString::fromLatin1("_DockManager");
  dockManager = new KDockManager( this, new_name.latin1());
  toolbar = 0L;
  mainDockWidget = 0L;

  connect( dockManager, SIGNAL(change()), SLOT(slotDockChange()) );
  connect( dockManager, SIGNAL(replaceDock(KDockWidget*,KDockWidget*)), SLOT(slotReplaceDock(KDockWidget*,KDockWidget*)) );
}

KDockMainWindow::~KDockMainWindow()
{
}

void KDockMainWindow::setMainDockWidget( KDockWidget* mdw )
{
  if ( mainDockWidget == mdw ) return;
  mainDockWidget = mdw;
  if ( mainDockWidget ){
    if ( !toolbar ){
      toolbar = new KToolBar( this );
      toolbar->insertButton( QPixmap(dock_close_top), 1, true, "Close top dock" );
      toolbar->insertButton( QPixmap(dock_close_left), 2, true, "Close left dock" );
      toolbar->insertButton( QPixmap(dock_close_right), 3, true, "Close right dock" );
      toolbar->insertButton( QPixmap(dock_close_bottom), 4, true, "Close bottom dock" );

      toolbar->setToggle(1);
      toolbar->setToggle(2);
      toolbar->setToggle(3);
      toolbar->setToggle(4);

      toolbar->setButton( 1, true );
      toolbar->setButton( 2, true );
      toolbar->setButton( 3, true );
      toolbar->setButton( 4, true );

      toolbar->setFullWidth( false );
      toolbar->show();
      connect( toolbar, SIGNAL(toggled(int)), SLOT(slotToggled(int)) );
      addToolBar( toolbar );
    }
  } else {
    delete toolbar;
    toolbar = 0L;
    updateRects();
  }
  slotDockChange();
}

void KDockMainWindow::setView( QWidget *view )
{
  if ( view->isA("KDockWidget") ){
    if ( view->parent() != this ) ((KDockWidget*)view)->applyToWidget( this );
  }
  KTMainWindow::setView(view);
  updateRects();
}

KDockWidget* KDockMainWindow::createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent )
{
  return new KDockWidget( dockManager, name.latin1(), pixmap, parent );
}

void KDockMainWindow::slotDockChange()
{
  if ( !mainDockWidget ) return;

  KDockWidget* DL = 0L;
  KDockWidget* DR = 0L;
  KDockWidget* DT = 0L;
  KDockWidget* DB = 0L;

  if ( mainDockWidget->parent() == this  || mainDockWidget->parent() == 0L ){
    if ( toolbar->isButtonOn(1) ) toolbar->setItemEnabled( 1, false );
    if ( toolbar->isButtonOn(2) ) toolbar->setItemEnabled( 2, false );
    if ( toolbar->isButtonOn(3) ) toolbar->setItemEnabled( 3, false );
    if ( toolbar->isButtonOn(4) ) toolbar->setItemEnabled( 4, false );
    return;
  }

  KDockWidget* base = mainDockWidget;

  while ( base != 0L && base->parent()!= 0L && base->parent()->inherits("KDockSplitter") )
  {
    KDockSplitter* s = (KDockSplitter*)base->parent();
    Orientation o = ((KDockWidget*)s->parent())->splitterOrientation;
    KDockWidget* another = (KDockWidget*)s->getAnother( base );
    if ( o == Horizontal ){
      if ( s->getFirst() == base ){
        DB = DB == 0L ? another : DB;
      } else {
        DT = DT == 0L ? another : DT;
      }
    } else { //Vertical
      if ( s->getFirst() == base ){
        DR = DR == 0L ? another : DR;
      } else {
        DL = DL == 0L ? another : DL;
      }
    }
    base = (KDockWidget*)s->parent();
  }
  if ( toolbar->isButtonOn(1) ) toolbar->setItemEnabled( 1, DT != 0L );
  if ( toolbar->isButtonOn(2) ) toolbar->setItemEnabled( 2, DL != 0L );
  if ( toolbar->isButtonOn(3) ) toolbar->setItemEnabled( 3, DR != 0L );
  if ( toolbar->isButtonOn(4) ) toolbar->setItemEnabled( 4, DB != 0L );

  /**********************/
  if ( DB != 0L && toolbar->isButtonOn(4) ){
    DockB.dock = DB;
    DockB.dropDock = (KDockWidget*)((KDockSplitter*)DB->parent())->getAnother( DB );
  }
  if ( DB == 0L && toolbar->isButtonOn(4) ) DockB.dock = 0L;
  /**********************/
  if ( DR != 0L && toolbar->isButtonOn(3) ){
    DockR.dock = DR;
    DockR.dropDock = (KDockWidget*)((KDockSplitter*)DR->parent())->getAnother( DR );
  }
  if ( DR == 0L && toolbar->isButtonOn(3) ) DockR.dock = 0L;
  /**********************/
  if ( DL != 0L && toolbar->isButtonOn(2) ){
    DockL.dock = DL;
    DockL.dropDock = (KDockWidget*)((KDockSplitter*)DL->parent())->getAnother( DL );
  }
  if ( DL == 0L && toolbar->isButtonOn(2) ) DockL.dock = 0L;
  /**********************/
  if ( DT != 0L && toolbar->isButtonOn(1) ){
    DockT.dock = DT;
    DockT.dropDock = (KDockWidget*)((KDockSplitter*)DT->parent())->getAnother( DT );
  }
  if ( DT == 0L && toolbar->isButtonOn(1) ) DockT.dock = 0L;
  /**********************/
}

void KDockMainWindow::slotToggled( int id )
{
  switch ( id ){
    case 1:
      toolBarManager( toolbar->isButtonOn(1), DockT );
      break;
    case 2:
      toolBarManager( toolbar->isButtonOn(2), DockL );
      break;
    case 3:
      toolBarManager( toolbar->isButtonOn(3), DockR );
      break;
    case 4:
      toolBarManager( toolbar->isButtonOn(4), DockB );
      break;
    default:
      break;
  }
}

void KDockMainWindow::toolBarManager( bool toggled, DockPosData &data )
{
  if ( data.dock == 0L || data.dropDock == 0L ) return;

  if ( toggled ){
    data.dock->manualDock( data.dropDock, data.pos, data.sepPos );
  } else {
    data.sepPos = ((KDockSplitter*)data.dock->parent())->separatorPos();
    data.dock->undock();
  }
}

void KDockMainWindow::slotReplaceDock( KDockWidget* oldDock, KDockWidget* newDock )
{
  DockB.dock = DockB.dock == oldDock ? newDock:DockB.dock;
  DockT.dock = DockT.dock == oldDock ? newDock:DockT.dock;
  DockR.dock = DockR.dock == oldDock ? newDock:DockR.dock;
  DockL.dock = DockL.dock == oldDock ? newDock:DockL.dock;

  DockB.dropDock = DockB.dropDock == oldDock ? newDock:DockB.dropDock;
  DockT.dropDock = DockT.dropDock == oldDock ? newDock:DockT.dropDock;
  DockR.dropDock = DockR.dropDock == oldDock ? newDock:DockR.dropDock;
  DockL.dropDock = DockL.dropDock == oldDock ? newDock:DockL.dropDock;
}

void KDockMainWindow::makeDockVisible( KDockWidget* dock )
{
  if ( !dock ) return;

  if ( toolbar ){
    toolbar->blockSignals( true );

    QWidget* testWidget = dock;
    bool found = false;

    while ( testWidget != 0L ){
      if ( testWidget->isA("KDockWidget") ){
        KDockWidget* test = (KDockWidget*)testWidget;
        if ( !toolbar->isButtonOn(1) && DockT.dock == test ){
          toolbar->toggleButton(1);
          toolBarManager( true, DockT );
          found = true;
        }
        if ( !toolbar->isButtonOn(2) && DockL.dock == test ){
          toolbar->toggleButton(2);
          toolBarManager( true, DockL );
          found = true;
        }
        if ( !toolbar->isButtonOn(3) && DockR.dock == test ){
          toolbar->toggleButton(3);
          toolBarManager( true, DockR );
          found = true;
        }
        if ( !toolbar->isButtonOn(4) && DockB.dock == test ){
          toolbar->toggleButton(4);
          toolBarManager( true, DockB );
          found = true;
        }
      }
      testWidget = testWidget->parentWidget();
    }

    dock->makeDockVisible();

    toolbar->blockSignals( false );
  } else {
    dock->makeDockVisible();
  }
}

void KDockMainWindow::makeWidgetDockVisible( QWidget* widget )
{
  makeDockVisible( dockManager->findWidgetParentDock(widget) );
}

void KDockMainWindow::writeDockConfig( KConfig* c, QString group )
{
  if ( toolbar ){
    toolbar->blockSignals( true );

    if ( !toolbar->isButtonOn(1) ){
      toolbar->toggleButton(1);
      toolBarManager( true, DockT );
    }
    if ( !toolbar->isButtonOn(2) ){
      toolbar->toggleButton(2);
      toolBarManager( true, DockL );
    }
    if ( !toolbar->isButtonOn(3) ){
      toolbar->toggleButton(3);
      toolBarManager( true, DockR );
    }
    if ( !toolbar->isButtonOn(4) ){
      toolbar->toggleButton(4);
      toolBarManager( true, DockB );
    }
    dockManager->writeConfig( c, group );

    toolbar->blockSignals( false );
  } else {
    dockManager->writeConfig( c, group );
  }
}

void KDockMainWindow::readDockConfig( KConfig* c, QString group )
{
  if ( toolbar ){
    toolbar->blockSignals( true );

    if ( !toolbar->isButtonOn(1) ){
      toolbar->toggleButton(1);
      toolBarManager( true, DockT );
    }
    if ( !toolbar->isButtonOn(2) ){
      toolbar->toggleButton(2);
      toolBarManager( true, DockL );
    }
    if ( !toolbar->isButtonOn(3) ){
      toolbar->toggleButton(3);
      toolBarManager( true, DockR );
    }
    if ( !toolbar->isButtonOn(4) ){
      toolbar->toggleButton(4);
      toolBarManager( true, DockB );
    }
    dockManager->readConfig( c, group );

    toolbar->blockSignals( false );
  } else {
    dockManager->readConfig( c, group );
  }
}


/*************************************************************************/
KDockWidgetAbstractHeaderDrag::KDockWidgetAbstractHeaderDrag( KDockWidgetAbstractHeader* parent, KDockWidget* dock, const char* name )
:QFrame( parent, name )
{
  d = dock;
  installEventFilter( dock->dockManager() );
}
/*************************************************************************/
KDockWidgetHeaderDrag::KDockWidgetHeaderDrag( KDockWidgetAbstractHeader* parent, KDockWidget* dock, const char* name )
:KDockWidgetAbstractHeaderDrag( parent, dock, name )
{
}

void KDockWidgetHeaderDrag::paintEvent( QPaintEvent* )
{
  QPixmap drawBuffer( width(), height() );
  QPainter paint;

  paint.begin( &drawBuffer );
  paint.fillRect( drawBuffer.rect(), QBrush(colorGroup().brush(QColorGroup::Background)) );

  paint.setPen( colorGroup().light() );
  paint.drawLine( 1, 3, 1, 2 );
  paint.drawLine( 1, 2, width(), 2 );

  paint.setPen( colorGroup().mid() );
  paint.drawLine( 1, 4, width(), 4 );
  paint.drawLine( width(), 4, width(), 3 );

  paint.setPen( colorGroup().light() );
  paint.drawLine( 1, 6, 1, 5 );
  paint.drawLine( 1, 5, width(), 5 );

  paint.setPen( colorGroup().mid() );
  paint.drawLine( 1, 7, width(), 7 );
  paint.drawLine( width(), 7, width(), 6 );

  bitBlt( this,0,0,&drawBuffer,0,0,width(),height() );
  paint.end();
}
/*************************************************************************/
KDockWidgetAbstractHeader::KDockWidgetAbstractHeader( KDockWidget* parent, const char* name )
:QFrame( parent, name )
{
}
/*************************************************************************/
KDockWidgetHeader::KDockWidgetHeader( KDockWidget* parent, const char* name )
:KDockWidgetAbstractHeader( parent, name )
{
  layout = new QHBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  drag = new KDockWidgetHeaderDrag( this, parent );

  closeButton = new KDockButton_Private( this, "DockCloseButton" );
  closeButton->setPixmap(close_xpm);
  closeButton->setFixedSize(9,9);
  connect( closeButton, SIGNAL(clicked()), parent, SLOT(undock()));

  stayButton = new KDockButton_Private( this, "DockStayButton" );
  stayButton->setToggleButton( true );
  stayButton->setPixmap(not_close_xpm);
  stayButton->setFixedSize(9,9);
  connect( stayButton, SIGNAL(clicked()), this, SLOT(slotStayClicked()));

  layout->addWidget( drag );
  layout->addWidget( stayButton );
  layout->addWidget( closeButton );
  layout->activate();
  drag->setFixedHeight( layout->minimumSize().height() );
}

void KDockWidgetHeader::setTopLevel( bool isTopLevel )
{
  if ( isTopLevel ){
    stayButton->hide();
    closeButton->hide();
    drag->setEnabled( true );
  } else {
    stayButton->show();
    closeButton->show();
  }
  layout->activate();
  updateGeometry();
}

void KDockWidgetHeader::slotStayClicked()
{
  closeButton->setEnabled( !stayButton->isOn() );
  drag->setEnabled( !stayButton->isOn() );
}

void KDockWidgetHeader::saveConfig( KConfig* c )
{
  c->writeEntry( QString("%1%2").arg(parent()->name()).arg(":stayButton"), stayButton->isOn() );
}

void KDockWidgetHeader::loadConfig( KConfig* c )
{
  stayButton->setOn( c->readBoolEntry( QString("%1%2").arg(parent()->name()).arg(":stayButton"), false ) );
  slotStayClicked();
}
/*************************************************************************/
KDockWidget::KDockWidget( KDockManager* dockManager, const char* name, const QPixmap &pixmap, QWidget* parent )
: QWidget( parent, name )
{
  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  manager = dockManager;
  manager->childDock->append( this );
  installEventFilter( manager );

  header = 0L;
  setHeader( new KDockWidgetHeader( this, "AutoCreatedDockHeader" ) );
  setCaption( name );

  eDocking = DockFullDocking;
  sDocking = DockFullSite;

  isGroup = false;
  isTabGroup = false;

  pix = new QPixmap(pixmap);
  widget = 0L;

  applyToWidget( parent, QPoint(0,0) );
}

KDockWidget::~KDockWidget()
{
  if ( !manager->undockProcess ){
    undock();
  }
  delete pix;
  manager->childDock->remove( this );
}

void KDockWidget::setHeader( KDockWidgetAbstractHeader* h )
{
  if ( !h ) return;

  if ( header ){
    delete header;
    delete layout;
    header = h;
    layout = new QVBoxLayout( this );
    layout->setResizeMode( QLayout::Minimum );
    layout->addWidget( header );
     setWidget( widget );    
  } else {
    header = h;
    layout->addWidget( header );
  }
}

void KDockWidget::setEnableDocking( int pos )
{
  eDocking = pos;
  updateHeader();
}

void KDockWidget::updateHeader()
{
  if ( parent() ){
    if ( (parent() == manager->main) || isGroup || (eDocking == KDockWidget::DockNone) ){
      header->hide();
    } else {
      header->setTopLevel( false );
      header->show();
    }
  } else {
    header->setTopLevel( true );
    header->show();
  }
}

void KDockWidget::applyToWidget( QWidget* s, const QPoint& p )
{
  if ( parent() != s ){
    hide();
    reparent(s, 0, QPoint(0,0), false);
  }

  if ( s && s->inherits("KDockMainWindow") ){
    ((KDockMainWindow*)s)->setView( this );
  }

  if ( s == manager->main ){
      setGeometry( QRect(QPoint(0,0), manager->main->geometry().size()) );
  }

  if ( !s ){
    move(p);
  }

  updateHeader();
}

void KDockWidget::show()
{
  if ( parent() || manager->main->isVisible() )
    if ( !parent() ){
     emit manager->setDockDefaultPos( this );
     emit setDockDefaultPos();
     if ( parent() ){
        makeDockVisible();
      } else {
        QWidget::show();
      }
    } else {
     QWidget::show();
    }
}

bool KDockWidget::event( QEvent *event )
{
  switch ( event->type() )
  {
    case QEvent::ChildRemoved:
      if ( widget == ((QChildEvent*)event)->child() ) widget = 0L;
      break;
    case QEvent::Show:
      if ( widget ) widget->show();
      emit manager->change();
      break;
    case QEvent::Hide:
      if ( widget ) widget->hide();
      emit manager->change();
      break;
    case QEvent::CaptionChange:
      if ( parentWidget() ){
        if ( parent()->inherits("KDockSplitter") ){
          ((KDockSplitter*)(parent()))->updateName();
        }
        if ( parentTabGroup() ){
          setDockTabName( parentTabGroup() );
          parentTabGroup()->setPageCaption( this, caption() );
        }
      }
      break;
    default:
      break;
  }
  return QWidget::event( event );
}

KDockWidget* KDockWidget::manualDock( KDockWidget* target, DockPosition dockPos, int spliPos, QPoint pos, bool check )
{
  bool succes = true; // tested flag

  // check allowed this dock submit this operations
  if ( !(eDocking & (int)dockPos) ){
    succes = false;
  }

  // check allowed target submit this operations
  if ( target && !(target->sDocking & (int)dockPos) ){
    succes = false;
  }

  if ( parent() && !parent()->inherits("KDockSplitter") && !parentTabGroup() ){
    succes = false;
  }

  if ( !succes ){
    // try to make another manualDock
    KDockWidget* dock_result = 0L;
    if ( target && !check ){
      KDockWidget::DockPosition another__dockPos = KDockWidget::DockNone;
      switch ( dockPos ){
        case KDockWidget::DockLeft  : another__dockPos = KDockWidget::DockRight ; break;
        case KDockWidget::DockRight : another__dockPos = KDockWidget::DockLeft  ; break;
        case KDockWidget::DockTop   : another__dockPos = KDockWidget::DockBottom; break;
        case KDockWidget::DockBottom: another__dockPos = KDockWidget::DockTop   ; break;
        default: break;
      }
      dock_result = target->manualDock( this, another__dockPos, spliPos, pos, true );
    }
    return dock_result;
  }
  // end check block

  undock();

  if ( !target ){
    move( pos );
    show();
    emit manager->change();
    return this;
  }

  KDockTabGroup* parentTab = target->parentTabGroup();
  if ( parentTab ){
    // add to existing TabGroup
    applyToWidget( parentTab );
    parentTab->insertPage( this, caption() );
    parentTab->setPixmap( this, *pix );
    setDockTabName( parentTab );

    emit manager->change();
    return (KDockWidget*)parentTab->parent();
  }

  QWidget* parentDock = target->parentWidget();
  KDockWidget* newDock = new KDockWidget( manager, "tempName", QPixmap(""), parentDock );

  if ( dockPos == KDockWidget::DockCenter ){
    newDock->isTabGroup = true;
  } else {
    newDock->isGroup = true;
  }
  newDock->eDocking = (target->eDocking & eDocking) & (~(int)KDockWidget::DockCenter);

  newDock->applyToWidget( parentDock );

  if ( !parentDock ){
    newDock->move( target->frameGeometry().topLeft() );
    newDock->resize( target->geometry().size() );
    if ( target->isVisibleToTLW() ) newDock->show();
  }

  if ( dockPos == KDockWidget::DockCenter )
  {
    KDockTabGroup* tab = new KDockTabGroup( newDock, "_dock_tab");
    newDock->setWidget( tab );

    target->applyToWidget( tab );
    applyToWidget( tab );

    tab->insertPage( target, target->caption() );
    tab->setPixmap( target, *(target->pix) );

    tab->insertPage( this, caption() );
    tab->setPixmap( this, *pix );

    setDockTabName( tab );
    tab->show();
  } else {
    KDockSplitter* panner = 0L;
    if ( dockPos == KDockWidget::DockTop  || dockPos == KDockWidget::DockBottom ) panner = new KDockSplitter( newDock, "_dock_split_", Horizontal, spliPos );
    if ( dockPos == KDockWidget::DockLeft || dockPos == KDockWidget::DockRight  ) panner = new KDockSplitter( newDock, "_dock_split_", Vertical , spliPos );
    newDock->setWidget( panner );

    panner->setFocusPolicy( NoFocus );
    target->applyToWidget( panner );
    applyToWidget( panner );
    if ( dockPos == KDockWidget::DockRight || dockPos == KDockWidget::DockBottom ) panner->activate( target, this );
    if ( dockPos == KDockWidget::DockTop   || dockPos == KDockWidget::DockLeft   ) panner->activate( this, target );
    target->show();
    show();
    panner->show();
  }

  if ( parentDock ){
    if ( parentDock->inherits("KDockSplitter") ){
      KDockSplitter* sp = (KDockSplitter*)parentDock;
      sp->deactivate();
      if ( sp->getFirst() == target )
        sp->activate( newDock, 0L );
      else
        sp->activate( 0L, newDock );
    }
  }

  newDock->show();
  emit target->docking( this, dockPos );
  emit manager->replaceDock( target, newDock );
  emit manager->change();

  return newDock;
}

KDockTabGroup* KDockWidget::parentTabGroup()
{
  if ( !parent() ) return 0L;
  QWidget* candidate = parentWidget()->parentWidget();
  if ( candidate && candidate->inherits("KDockTabGroup") ) return (KDockTabGroup*)candidate;
  return 0L;
}

void KDockWidget::undock()
{
  QWidget* parentW = parentWidget();
  if ( !parentW ){
    hide();
    return;
  }

  manager->blockSignals(true);
  manager->undockProcess = true;

  bool isV = parentW->isVisibleToTLW();

  KDockTabGroup* parentTab = parentTabGroup();
  if ( parentTab ){
    parentTab->removePage( this );
    applyToWidget( 0L );
    if ( parentTab->pageCount() == 1 ){

      /* last subdock widget in the tab control*/
      KDockWidget* lastTab = (KDockWidget*)parentTab->getFirstPage();
      parentTab->removePage( lastTab );
      lastTab->applyToWidget( 0L );
      lastTab->move( parentTab->mapToGlobal(parentTab->frameGeometry().topLeft()) );

      /* KDockTabGroup always have a parent is KDockWidget*/
      KDockWidget* parentOfTab = (KDockWidget*)parentTab->parent();
      delete parentTab; // KDockTabGroup

      QWidget* parentOfDockWidget = parentOfTab->parentWidget();
      if ( parentOfDockWidget == 0L ){
          if ( isV ) lastTab->show();
      } else {
        if ( parentOfDockWidget->inherits("KDockSplitter") ){
          KDockSplitter* split = (KDockSplitter*)parentOfDockWidget;
          lastTab->applyToWidget( split );
          split->deactivate();
          if ( split->getFirst() == parentOfTab ){
            split->activate( lastTab );
            if ( ((KDockWidget*)split->parent())->splitterOrientation == Vertical )
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockLeft );
            else
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockTop );
          } else {
            split->activate( 0L, lastTab );
            if ( ((KDockWidget*)split->parent())->splitterOrientation == Vertical )
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockRight );
            else
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockBottom );
          }
          split->show();
        } else {
          lastTab->applyToWidget( parentOfDockWidget );
        }
        lastTab->show();
      }
      manager->blockSignals(false);
      emit manager->replaceDock( parentOfTab, lastTab );
      manager->blockSignals(true);
      delete parentOfTab;

    } else {
      setDockTabName( parentTab );
    }
  } else {
/*********************************************************************************************/
    if ( parentW->inherits("KDockSplitter") ){
      KDockSplitter* parentSplitterOfDockWidget = (KDockSplitter*)parentW;

      applyToWidget( 0L );
      KDockWidget* secondWidget = (KDockWidget*)parentSplitterOfDockWidget->getAnother( this );
      KDockWidget* group        = (KDockWidget*)parentSplitterOfDockWidget->parentWidget();
      group->hide();

      if ( !group->parentWidget() ){
        secondWidget->applyToWidget( 0L, group->frameGeometry().topLeft() );
        secondWidget->resize( group->width(), group->height() );
      } else {
        QWidget* obj = group->parentWidget();
        secondWidget->applyToWidget( obj );
        if ( obj->inherits("KDockSplitter") ){
          KDockSplitter* parentOfGroup = (KDockSplitter*)obj;
          parentOfGroup->deactivate();

          if ( parentOfGroup->getFirst() == group )
            parentOfGroup->activate( secondWidget );
          else
            parentOfGroup->activate( 0L, secondWidget );
        }
      }
      delete parentSplitterOfDockWidget;
      manager->blockSignals(false);
      emit manager->replaceDock( group, secondWidget );
      manager->blockSignals(true);
      delete group;

      if ( isV ) secondWidget->show();
    } else {
      applyToWidget( 0L );
    }
/*********************************************************************************************/
  }
  manager->blockSignals(false);
  emit manager->change();
  manager->undockProcess = false;
}

void KDockWidget::setWidget( QWidget* mw )
{
  if ( !mw ) return;

  if ( mw->parent() != this ){
    mw->recreate(this, 0, QPoint(0,0), false);
  }

  widget = mw;
  delete layout;

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  layout->addWidget( header );
  layout->addWidget( widget,1 );
}

void KDockWidget::setDockTabName( KDockTabGroup* tab )
{
  QString listOfName;
  QString listOfCaption;
  for ( QWidget* w = tab->getFirstPage(); w; w = tab->getNextPage( w ) ){
    listOfCaption.append( w->caption() ).append(",");
    listOfName.append( w->name() ).append(",");
  }
  listOfCaption.remove( listOfCaption.length()-1, 1 );
  listOfName.remove( listOfName.length()-1, 1 );

  tab->parentWidget()->setName( listOfName.utf8() );
  tab->parentWidget()->setCaption( listOfCaption );

  tab->parentWidget()->repaint( false ); // KDockWidget->repaint
  if ( tab->parentWidget()->parent() )
    if ( tab->parentWidget()->parent()->inherits("KDockSplitter") )
      ((KDockSplitter*)(tab->parentWidget()->parent()))->updateName();
}

bool KDockWidget::mayBeHide()
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && isVisible() && ( eDocking != (int)KDockWidget::DockNone ) );
}

bool KDockWidget::mayBeShow()
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && !isVisible() );
}

void KDockWidget::changeHideShowState()
{
  if ( mayBeHide() ){
    undock();
    return;
  }

  if ( mayBeShow() ){
    if ( manager->main->inherits("KDockMainWindow") ){
      ((KDockMainWindow*)manager->main)->makeDockVisible(this);
    } else {
      makeDockVisible();
    }
  }
}

void KDockWidget::makeDockVisible()
{
  if ( isVisible() ) return;

  if ( parentTabGroup() ){
    parentTabGroup()->setVisiblePage( this );
  }
  QWidget* p = parentWidget();
  while ( p ){
    if ( !p->isVisible() ) p->show();
    p = p->parentWidget();
  }
  show();
}
/**************************************************************************************/

KDockManager::KDockManager( QWidget* mainWindow , const char* name )
:QObject( 0, name )
{
  main = mainWindow;
  main->installEventFilter( this );

  undockProcess = false;

  menuData = new QList<MenuDockData>;
  menuData->setAutoDelete( true );
  menuData->setAutoDelete( true );

  menu = new QPopupMenu();

  connect( menu, SIGNAL(aboutToShow()), SLOT(slotMenuPopup()) );
  connect( menu, SIGNAL(activated(int)), SLOT(slotMenuActivated(int)) );

  childDock = new QObjectList();
  childDock->setAutoDelete( false );
  mg = 0L;
  draging = false;
  dropCancel = false;
}

KDockManager::~KDockManager()
{
  delete menuData;
  delete menu;

  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ) {
    delete obj;
  }
  delete childDock;
}

void KDockManager::activate()
{
  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    if ( obj->widget ) obj->widget->show();
    if ( !obj->parentTabGroup() ){
        obj->show();
    }
  }
  if ( !main->inherits("QDialog") ) main->show();
}

bool KDockManager::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == main && event->type() == QEvent::Resize && main->children() ){
    QWidget* fc = (QWidget*)main->children()->getFirst();
    if ( fc )
      fc->setGeometry( QRect(QPoint(0,0), main->geometry().size()) );
  }

  #ifdef KeyPress
  #undef KeyPress
  #endif  
  if ( event->type() ==QEvent::KeyPress ){
    if ( ((QKeyEvent*)event)->key() == Qt::Key_Escape ){
      if ( draging ){
        dropCancel = true;
        draging = false;
        drop();
      }
    }
  }

  if ( obj->inherits("KDockWidgetAbstractHeaderDrag") ){
    KDockWidget* ww = 0L;
    KDockWidget* curdw = ((KDockWidgetAbstractHeaderDrag*)obj)->dockWidget();
    switch ( event->type() ){
      case QEvent::MouseButtonRelease:
        if ( ((QMouseEvent*)event)->button() == LeftButton ){
          if ( draging && !dropCancel ){
            drop();
          }
          draging = false;
          dropCancel = false;
        }
        break;
      case QEvent::MouseMove:
        if ( draging ) {
          ww = findDockWidgetAt( QCursor::pos() );
          KDockWidget* oldMoveWidget = currentMoveWidget;
          if ( currentMoveWidget  && ww == currentMoveWidget ) { //move
            dragMove( currentMoveWidget, currentMoveWidget->mapFromGlobal( QCursor::pos() ) );
            break;
          }

          if ( !ww && (curdw->eDocking & (int)KDockWidget::DockDesktop) == 0 ){
              currentMoveWidget = ww;
              curPos = KDockWidget::DockDesktop;
              mg->movePause();
          } else {
            if ( oldMoveWidget && ww != currentMoveWidget ) { //leave
              currentMoveWidget = ww;
              curPos = KDockWidget::DockDesktop;
              mg->resize( storeW, storeH );
              mg->moveContinue();
            }
          }

          if ( oldMoveWidget != ww && ww ) { //enter ww
            currentMoveWidget = ww;
            curPos = KDockWidget::DockDesktop;
            storeW = mg->width();
            storeH = mg->height();
            mg->movePause();
          }
        } else {
          if ( (((QMouseEvent*)event)->state() == LeftButton) &&  !dropCancel ){
            if ( curdw->eDocking != (int)KDockWidget::DockNone ){
              dropCancel = false;
              curdw->setFocus();
              qApp->processOneEvent();
              startDrag( curdw );
            }
          }
        }
        break;
      default:
        break;
    }
  }
  return QObject::eventFilter( obj, event );
}

KDockWidget* KDockManager::findDockWidgetAt( const QPoint& pos )
{
  if (currentDragWidget->eDocking == (int)KDockWidget::DockNone ) return 0L;

  QWidget* p = QApplication::widgetAt( pos );
  if ( !p ) return 0L;
  QWidget* w = 0L;
  findChildDockWidget( w, p, p->mapFromParent(pos) );
  if ( !w ){
    if ( !p->inherits("KDockWidget") ) return 0L;
    w = p;
  }
  if ( qt_find_obj_child( w, "KDockSplitter", "_dock_split_" ) ) return 0L;
  if ( qt_find_obj_child( w, "KDockTabGroup", "_dock_tab" ) ) return 0L;
  if ( childDockWidgetList->find(w) != -1 ) return 0L;
  if ( currentDragWidget->isGroup && ((KDockWidget*)w)->parentTabGroup() ) return 0L;

  KDockWidget* www = (KDockWidget*)w;
  if ( www->sDocking == (int)KDockWidget::DockNone ) return 0L;

  KDockWidget::DockPosition curPos = KDockWidget::DockDesktop;
  QPoint cpos  = www->mapFromGlobal( pos );

  int ww = www->widget->width() / 3;
  int hh = www->widget->height() / 3;

	if ( cpos.y() <= hh ){
    curPos = KDockWidget::DockTop;
	} else
    if ( cpos.y() >= 2*hh ){
      curPos = KDockWidget::DockBottom;
    } else
      if ( cpos.x() <= ww ){
        curPos = KDockWidget::DockLeft;
      } else
        if ( cpos.x() >= 2*ww ){
          curPos = KDockWidget::DockRight;
        } else
            curPos = KDockWidget::DockCenter;

  if ( !(www->sDocking & (int)curPos) ) return 0L;
  if ( !(currentDragWidget->eDocking & (int)curPos) ) return 0L;
  if ( www->manager != this ) return 0L;

  return www;
}

void KDockManager::findChildDockWidget( QWidget*& ww, const QWidget* p, const QPoint& pos )
{
  if ( p->children() ) {
    QWidget *w;
    QObjectListIt it( *p->children() );
    it.toLast();
    while ( it.current() ) {
      if ( it.current()->isWidgetType() ) {
        w = (QWidget*)it.current();
        if ( w->isVisible() && w->geometry().contains(pos) ) {
          if ( w->inherits("KDockWidget") ) ww = w;
          findChildDockWidget( ww, w, w->mapFromParent(pos) );
          return;
        }
      }
      --it;
    }
  }
  return;
}

void KDockManager::findChildDockWidget( const QWidget* p, WidgetList*& list )
{
  if ( p->children() ) {
    QWidget *w;
    QObjectListIt it( *p->children() );
    it.toLast();
    while ( it.current() ) {
      if ( it.current()->isWidgetType() ) {
        w = (QWidget*)it.current();
        if ( w->isVisible() ) {
          if ( w->inherits("KDockWidget") ) list->append( w );
          findChildDockWidget( w, list );
        }
      }
      --it;
    }
  }
  return;
}

void KDockManager::startDrag( KDockWidget* w )
{
  currentMoveWidget = 0L;
  currentDragWidget = w;
  childDockWidgetList = new WidgetList();
  childDockWidgetList->append( w );
  findChildDockWidget( w, childDockWidgetList );
  
  if ( mg ) delete mg;
  mg = new KDockMoveManager( w );
  curPos = KDockWidget::DockDesktop;
  draging = true;
  mg->doMove();
}

void KDockManager::dragMove( KDockWidget* dw, QPoint pos )
{
	QPoint p = dw->mapToGlobal( dw->widget->pos() );
  KDockWidget::DockPosition oldPos = curPos;

  QSize r = dw->widget->size();
  if ( dw->parentTabGroup() ){
    curPos = KDockWidget::DockCenter;
  	if ( oldPos != curPos ) mg->setGeometry( p.x()+2, p.y()+2, r.width()-4, r.height()-4 );
    return;
  }

  int w = r.width() / 3;
  int h = r.height() / 3;

  if ( pos.y() <= h ){
    curPos = KDockWidget::DockTop;
    w = r.width();
  } else
    if ( pos.y() >= 2*h ){
      curPos = KDockWidget::DockBottom;
      p.setY( p.y() + 2*h );
      w = r.width();
    } else
      if ( pos.x() <= w ){
        curPos = KDockWidget::DockLeft;
        h = r.height();
      } else
        if ( pos.x() >= 2*w ){
          curPos = KDockWidget::DockRight;
          p.setX( p.x() + 2*w );
          h = r.height();
        } else
          {
            curPos = KDockWidget::DockCenter;
            p.setX( p.x() + w );
            p.setY( p.y() + h );
          }

  if ( oldPos != curPos ) mg->setGeometry( p.x(), p.y(), w, h );
}

void KDockManager::drop()
{
  mg->stop();
  delete childDockWidgetList;
  if ( dropCancel ) return;
  if ( !currentMoveWidget && ((currentDragWidget->eDocking & (int)KDockWidget::DockDesktop) == 0) ) return;

  if ( !currentMoveWidget && !currentDragWidget->parent() )
    currentDragWidget->move( mg->x(), mg->y() );
  else
    currentDragWidget->manualDock( currentMoveWidget, curPos , 50, QPoint(mg->x(), mg->y()) );
}

void KDockManager::writeConfig( KConfig* c, QString group )
{
  //debug("BEGIN Write Config");
  if ( !c ) c = kapp->config();
  if ( group.isEmpty() ) group = "dock_setting_default";

  c->setGroup( group );
  c->writeEntry( "Version", DOCK_CONFIG_VERSION );

  QStrList nameList;
  QStrList findList;
  QObjectListIt it( *childDock );
  KDockWidget * obj;
  
  // collect KDockWidget's name
  QStrList nList;
  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    //debug("  +Add subdock %s", obj->name());
    nList.append( obj->name() );
    if ( obj->parent() == main )
      c->writeEntry( "Main:view", obj->name() );
  }

  nList.first();
  while ( nList.current() ){
    //debug("  -Try to save %s", nList.current());
    obj = getDockWidgetFromName( nList.current() );
    QString cname = obj->name();
    if ( obj->header ){
      obj->header->saveConfig( c );
    }
/*************************************************************************************************/
    if ( obj->isGroup ){
      if ( findList.find( obj->firstName.latin1() ) != -1 && findList.find( obj->lastName.latin1() ) != -1 ){

        c->writeEntry( cname+":type", "GROUP");
        if ( !obj->parent() ){
          c->writeEntry( cname+":parent", "___null___");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        c->writeEntry( cname+":first_name", obj->firstName );
        c->writeEntry( cname+":last_name", obj->lastName );
        c->writeEntry( cname+":orientation", (int)obj->splitterOrientation );
        c->writeEntry( cname+":sepPos", ((KDockSplitter*)obj->widget)->separatorPos() );

        nameList.append( obj->name() );
        findList.append( obj->name() );
        //debug("  Save %s", nList.current());
        nList.remove();
        nList.first();
      } else {
/*************************************************************************************************/
        //debug("  Skip %s", nList.current());
        //if ( findList.find( obj->firstName ) == -1 )
        //  debug("  ? Not found %s", obj->firstName);
        //if ( findList.find( obj->lastName ) == -1 )
        //  debug("  ? Not found %s", obj->lastName);
        nList.next();
        if ( !nList.current() ) nList.first();
      }
    } else {
/*************************************************************************************************/
      if ( obj->isTabGroup){
        c->writeEntry( cname+":type", "TAB_GROUP");
        if ( !obj->parent() ){
          c->writeEntry( cname+":parent", "___null___");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        QStrList list;
        for ( QWidget* w = ((KDockTabGroup*)obj->widget)->getFirstPage(); w; w = ((KDockTabGroup*)obj->widget)->getNextPage( w ) ){
          list.append( w->name() );
        }
        c->writeEntry( cname+":tabNames", list );
        c->writeEntry( cname+":curTab", ((KDockTabGroup*)obj->widget)->visiblePageId() );

        nameList.append( obj->name() );
        findList.append( obj->name() ); // not realy need !!!
        //debug("  Save %s", nList.current());
        nList.remove();
        nList.first();
      } else {
/*************************************************************************************************/
        if ( !obj->parent() ){
          c->writeEntry( cname+":type", "NULL_DOCK");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":type", "DOCK");
        }
        nameList.append( cname.latin1() );
        //debug("  Save %s", nList.current());
        findList.append( obj->name() );
        nList.remove();
        nList.first();
      }
    }
  }
  c->writeEntry( "NameList", nameList );

  c->writeEntry( "Main:Geometry", QRect(main->frameGeometry().topLeft(), main->size()) );
  c->writeEntry( "Main:visible", main->isVisible()); // curently nou use

  if ( main->inherits("KDockMainWindow") ){
    KDockMainWindow* dmain = (KDockMainWindow*)main;
    // for KDockMainWindow->setView() in reafConfig()
    c->writeEntry( "Main:view", dmain->view() ? dmain->view()->name():"" );
    c->writeEntry( "Main:dock", dmain->getMainDockWidget()     ? dmain->getMainDockWidget()->name()    :"" );
  }

  c->sync();
  //debug("END Write Config");
}

void KDockManager::readConfig( KConfig* c, QString group )
{
  if ( !c ) c = kapp->config();
  if ( group.isEmpty() ) group = "dock_setting_default";

  c->setGroup( group );
  QStrList nameList;
  c->readListEntry( "NameList", nameList );
  QString ver = c->readEntry( "Version", "0.0.1" );
  nameList.first();
  if ( !nameList.current() || ver != DOCK_CONFIG_VERSION ){
    activate();
    return;
  }

  autoCreateDock = new QObjectList();
  autoCreateDock->setAutoDelete( true );

  bool isMainVisible = main->isVisible();
  main->hide();

  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ){
    ++it;
    if ( !obj->isGroup && !obj->isTabGroup )
    {
      if ( obj->parent() ) obj->undock(); else obj->hide();
    }
  }

  nameList.first();
  while ( nameList.current() ){
    QString oname = nameList.current();
    QString type = c->readEntry( oname + ":type" );
    obj = 0L;

    if ( type == "GROUP" ){
      KDockWidget* first = getDockWidgetFromName( c->readEntry( oname + ":first_name" ) );
      KDockWidget* last  = getDockWidgetFromName( c->readEntry( oname + ":last_name"  ) );
      int sepPos = c->readNumEntry( oname + ":sepPos" );

      Orientation p = (Orientation)c->readNumEntry( oname + ":orientation" );
      if ( first  && last ){
        obj = first->manualDock( last, ( p == Vertical ) ? KDockWidget::DockLeft : KDockWidget::DockTop, sepPos );
        if (obj){
          obj->setName( oname.latin1() );
        }
      }
    }

    if ( type == "TAB_GROUP" ){
      QStrList list;
      KDockWidget* tabDockGroup = 0L;
      c->readListEntry( oname+":tabNames", list );
      KDockWidget* d1 = getDockWidgetFromName( list.first() );
      list.next();
      KDockWidget* d2 = getDockWidgetFromName( list.current() );
      tabDockGroup = d2->manualDock( d1, KDockWidget::DockCenter );
      if ( tabDockGroup ){
        KDockTabGroup* tab = (KDockTabGroup*)tabDockGroup->widget;
        list.next();
        while ( list.current() && tabDockGroup ){
          KDockWidget* tabDock = getDockWidgetFromName( list.current() );
          tabDockGroup = tabDock->manualDock( d1, KDockWidget::DockCenter );
          list.next();
        }
        if ( tabDockGroup ){
          tabDockGroup->setName( oname.latin1() );
          tab->setVisiblePage( c->readNumEntry( oname+":curTab" ) );
        }
      }
      obj = tabDockGroup;
    }

    if ( type == "NULL_DOCK" || c->readEntry( oname + ":parent") == "___null___" ){
      QRect r = c->readRectEntry( oname + ":geometry" );
      obj = getDockWidgetFromName( oname );
      obj->applyToWidget( 0L );
      obj->setGeometry(r);

      if ( c->readBoolEntry( oname + ":visible" ) ){
        obj->QWidget::show();
      }
    }

    if ( type == "DOCK"  ){
      obj = getDockWidgetFromName( oname );
    }

    if ( obj && obj->header){
      obj->header->loadConfig( c );
    }
    nameList.next();
	}

  // delete all autocreate dock
  delete autoCreateDock;
  autoCreateDock = 0L;

  if ( main->inherits("KDockMainWindow") ){
    KDockMainWindow* dmain = (KDockMainWindow*)main;

    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      KDockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( dmain );
      mvd->show();
      dmain->setView( mvd );
    }
    QString md = c->readEntry( "Main:dock" );
    if ( !md.isEmpty() && getDockWidgetFromName( md ) ){
      KDockWidget* mvd  = getDockWidgetFromName( md );
      dmain->setMainDockWidget( mvd );
    }
  } else {
    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      KDockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( main );
      mvd->show();
    }
  
  }

  QRect mr = c->readRectEntry("Main:Geometry");
  main->setGeometry(mr);
  if ( isMainVisible ) main->show();
}

KDockWidget* KDockManager::getDockWidgetFromName( const QString& dockName )
{
  QObjectListIt it( *childDock );
  KDockWidget * obj;
  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
		if ( QString(obj->name()) == dockName ) return obj;
  }

  KDockWidget* autoCreate = 0L;
  if ( autoCreateDock ){
    autoCreate = new KDockWidget( this, dockName.latin1(), QPixmap("") );
    autoCreateDock->append( autoCreate );
  }
	return autoCreate;
}

void KDockManager::slotMenuPopup()
{
  menu->clear();
  menuData->clear();

  QObjectListIt it( *childDock );
  KDockWidget * obj;
  int numerator = 0;
  while ( (obj=(KDockWidget*)it.current()) ) {
	  ++it;
    if ( obj->mayBeHide() )
    {
      menu->insertItem( *obj->pix, QString("Hide ") + obj->caption(), numerator++ );
      menuData->append( new MenuDockData( obj, true ) );
    }

    if ( obj->mayBeShow() )
    {
      menu->insertItem( *obj->pix, QString("Show ") + obj->caption(), numerator++ );
      menuData->append( new MenuDockData( obj, false ) );
    }
  }
}

void KDockManager::slotMenuActivated( int id )
{
  MenuDockData* data = menuData->at( id );
  data->dock->changeHideShowState();
}

KDockWidget* KDockManager::findWidgetParentDock( QWidget* w )
{
  QObjectListIt it( *childDock );
  KDockWidget * dock;
  KDockWidget * found = 0L;

  while ( (dock=(KDockWidget*)it.current()) ) {
	  ++it;
    if ( dock->widget == w ){ found  = dock; break; }
  }
  return found;
}
