/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Mirko Sucker (mirko@kde.org) and 
 *  Espen Sand (espen@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include <iostream.h>
#include <stdlib.h>

#include <qgrid.h>
#include <qhbox.h>
#include <qlayout.h> 
#include <qlist.h> 
#include <qtooltip.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qtimer.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kseparator.h>
#include <kurllabel.h>

#include "kdialogbase.h"

KDialogBaseTile *KDialogBase::mTile = 0;

template class QList<KDialogBaseButton>;

KDialogBase::KDialogBase( QWidget *parent, const char *name, bool modal,
			  const QString &caption, int buttonMask, 
			  ButtonCode defaultButton, bool separator, 
			  const QString &user1, const QString &user2,
			  const QString &user3 )
  :KDialog( parent, name, modal, /*WStyle_Customize|*/WStyle_DialogBorder),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(false)
{
  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

  enableButtonSeparator( separator );
  makeButtonBox( buttonMask, defaultButton, user1, user2, user3 );
  
  mIsActivated = true;
  setupLayout();
}


KDialogBase::KDialogBase( int dialogFace, const QString &caption, 
			  int buttonMask, ButtonCode defaultButton, 
			  QWidget *parent, const char *name, bool modal, 
			  bool separator, const QString &user1, 
			  const QString &user2, const QString &user3 )
  :KDialog( parent, name, modal, /*WStyle_Customize|*/WStyle_DialogBorder ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(false)
{
  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );
  
  mJanus = new KJanusWidget( this, "janus", dialogFace );
  if( mJanus == 0 || mJanus->isValid() == false ) { return; }

  enableButtonSeparator( separator );
  makeButtonBox( buttonMask, defaultButton, user1, user2, user3 );

  mIsActivated = true;
  setupLayout();
}


KDialogBase::KDialogBase( const QString &caption, int buttonMask, 
			  ButtonCode defaultButton, ButtonCode escapeButton, 
			  QWidget *parent, const char *name, bool modal, 
			  bool separator, QString yes,
			  QString no, QString cancel )
  :KDialog( parent, name, modal, /*WStyle_Customize|*/WStyle_DialogBorder ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(true),
   mEscapeButton(escapeButton)
{
  if (yes.isEmpty())
     yes = i18n("&Yes");
  if (no.isEmpty())
     no = i18n("&No");
  if (cancel.isEmpty())
     cancel = i18n("&Cancel");

  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

  enableButtonSeparator( separator );

  buttonMask &= Yes|No|Cancel;

  makeButtonBox( buttonMask, defaultButton, no, yes, QString::null );
  setButtonCancelText( cancel );

  mIsActivated = true;
  setupLayout();
}



KDialogBase::~KDialogBase()
{
}

void
KDialogBase::delayedDestruct()
{
   if (isVisible())
      hide();
   QTimer::singleShot( 0, this, SLOT(slotDelayedDestruct()));
}

void
KDialogBase::slotDelayedDestruct()
{
   delete this;
}

void KDialogBase::setupLayout()
{
  if( mTopLayout != 0 ) 
  {
    delete mTopLayout;
  }
  mTopLayout = new QVBoxLayout( this, marginHint(), spacingHint() );

  if( mUrlHelp != 0 )
  {
    mTopLayout->addWidget( mUrlHelp, 0, AlignRight );
  }

  if( mJanus != 0 )
  {
    mTopLayout->addWidget( mJanus, 10 );
  }
  else if( mMainWidget != 0 )
  {
    mTopLayout->addWidget( mMainWidget, 10 );
  }

  if( mActionSep != 0 )
  {
    mTopLayout->addWidget( mActionSep );
  }

  if( mButton.box != 0 )
  {
    mTopLayout->addWidget( mButton.box );
  }
}



void KDialogBase::makeRelay()
{
  if( mTile != 0 )
  {
    connect( mTile, SIGNAL(pixmapChanged()), this, SLOT(updateBackground()) );
    return;
  }

  mTile = new KDialogBaseTile;
  if( mTile != 0 )
  {
    connect( mTile, SIGNAL(pixmapChanged()), this, SLOT(updateBackground()) );
    connect( kapp, SIGNAL(aboutToQuit()), mTile, SLOT(cleanup()) );
  }
}


void KDialogBase::enableButtonSeparator( bool state )
{
  if( state == true )
  {
    if( mActionSep != 0 )
    {
      return;
    }
    mActionSep = new KSeparator( this );
    mActionSep->setFocusPolicy(QWidget::NoFocus);
    mActionSep->show();
  }
  else
  {
    if( mActionSep == 0 )
    {
      return;
    }
    delete mActionSep; mActionSep = 0;
  }
 
  if( mIsActivated == true )
  {
    setupLayout();
  }
}



QFrame *KDialogBase::plainPage()
{
  return( mJanus == 0 ? 0 : mJanus->plainPage() );
}



void KDialogBase::adjustSize()
{
  int m = marginHint();
  int s = spacingHint();

  QSize s1(0,0);
  QSize s2(0,0);

  //
  // Url help area
  //
  if( mUrlHelp != 0 )
  {
    s2 = mUrlHelp->minimumSize() + QSize( 0, s );
  }
  s1.rwidth()   = QMAX( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  //
  // User widget
  //
  if( mJanus != 0 )
  {
    s2 = mJanus->minimumSizeHint() + QSize( 0, s );
  }
  else if( mMainWidget != 0 )
  {
    s2 = mMainWidget->sizeHint() + QSize( 0, s );
    s2 = s2.expandedTo( mMainWidget->minimumSize() );
    s2 = s2.expandedTo( mMainWidget->minimumSizeHint() );
    if( s2.isEmpty() == true )
    {
      s2 = QSize( 100, 100+s );
    }
  }
  else
  {
    s2 = QSize( 100, 100+s );
  }
  s1.rwidth()  = QMAX( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  //
  // Button separator
  //
  if( mActionSep != 0 ) 
  { 
    s1.rheight() += mActionSep->minimumSize().height() + s;
  }

  //
  // The button box
  //
  s2 = mButton.box->minimumSize();
  s1.rwidth()   = QMAX( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  //
  // Outer margings
  //
  s1.rheight() += 2*m;
  s1.rwidth()  += 2*m;

  resize( s1 );
}


void KDialogBase::disableResize()
{
  adjustSize();
  setFixedSize( sizeHint() );
}


void KDialogBase::setInitialSize( const QSize &s, bool noResize )
{
  adjustSize();
  if( noResize == true )
  {
    setFixedSize( s.expandedTo( sizeHint() ) );
  }
  else
  {
    resize( s.expandedTo( sizeHint() ) );
  }
}


void KDialogBase::incInitialSize( const QSize &s, bool noResize )
{
  adjustSize();
  if( noResize == true )
  {
    setFixedSize( s.expandedTo(QSize(0,0))+sizeHint() );
  }
  else
  {
    resize( s.expandedTo(QSize(0,0))+sizeHint() );
  }
}


void KDialogBase::makeButtonBox( int buttonMask, ButtonCode defaultButton,
				 const QString &user1, const QString &user2,
				 const QString &user3 )
{
  if( buttonMask & Cancel ) { buttonMask &= ~Close; }
  if( buttonMask & Apply ) { buttonMask &= ~Try; }

  if( mMessageBoxMode == false )
  {
    mEscapeButton = (buttonMask&Cancel) ? Cancel : Close;
  }

  mButton.box = new QWidget( this );

  mButton.mask = buttonMask;
  if( mButton.mask & Help )
  {
    QPushButton *pb = mButton.append( Help, i18n("&Help") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotHelp()) );
  }
  if( mButton.mask & Default )
  {
    QPushButton *pb = mButton.append( Default, i18n("&Default") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotDefault()) );
  }
  if( mButton.mask & User3 )
  {
    QPushButton *pb = mButton.append( User3, user3 );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotUser3()) );
  }
  if( mButton.mask & User2 )
  {
    QPushButton *pb = mButton.append( User2, user2 );
    if( mMessageBoxMode == true )
    { 
      connect( pb, SIGNAL(clicked()), this, SLOT(slotYes()) );
    }
    else
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotUser2()) );
    }
  }
  if( mButton.mask & User1 )
  {
    QPushButton *pb = mButton.append( User1, user1 );
    if( mMessageBoxMode == true )
    { 
      connect( pb, SIGNAL(clicked()), this, SLOT(slotNo()) );
    }
    else
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotUser1()) );
    }
  }
  if( mButton.mask & Ok )
  {
    QPushButton *pb = mButton.append( Ok, i18n("&OK") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotOk()) );
  }
  if( mButton.mask & Apply )
  {
    QPushButton *pb = mButton.append( Apply, i18n("&Apply") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotApply()) );
    connect( pb, SIGNAL(clicked()), this, SLOT(applyPressed()) );
  }
  if( mButton.mask & Try )
  {
    QPushButton *pb = mButton.append( Try, i18n("&Try") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotTry()) );
  }
  if( mButton.mask & Cancel )
  {
    QPushButton *pb = mButton.append( Cancel, i18n("&Cancel") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotCancel()) );
  }
  if( mButton.mask & Close )
  {
    QPushButton *pb = mButton.append( Close, i18n("&Close") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotClose()) );
  }
  
  QPushButton *pb = actionButton( defaultButton );
  if( pb != 0 )
  {
    setButtonFocus( pb, true, true );
  }

  setButtonStyle( ActionStyle0 );
}





void KDialogBase::setButtonStyle( int style )
{
  if( style < 0 || style > ActionStyleMAX ) { style = ActionStyle0; }
  mButton.style = style;

  static int layoutRule[5][8] =
  {
    {Help,Default|Stretch,User3,User2,User1,Ok,Apply|Try,Cancel|Close},
    {Help,Default|Stretch,User3,User2,User1,Cancel|Close,Apply|Try,Ok},
    {Help,Default|Stretch,User3,User2,User1,Apply|Try,Cancel|Close,Ok},
    {Ok,Apply|Try,Cancel|Close,User3,User2,User1|Stretch,Default,Help},
    {Ok,Cancel|Close,Apply|Try,User3,User2,User1|Stretch,Default,Help},
  };
  const int *layout = layoutRule[ mButton.style ];


  if( mButton.box->layout() )
  {
    delete mButton.box->layout();
  }
  QHBoxLayout *hbox = new QHBoxLayout( mButton.box, 0, spacingHint() );

  if( mMessageBoxMode == true )
  {
    hbox->addStretch(1);
  }

  int numButton = 0;
  for( uint i=0; i<8; i++ )
  {
    if( i>0 && (layout[i-1]&Stretch) && mMessageBoxMode == false )
    {
      hbox->addStretch(1);
    }

    if( mButton.mask & Help & layout[i] )
    {
      hbox->addWidget( actionButton( Help ) ); numButton++;
    }
    else if( mButton.mask & Default & layout[i] )
    {
      hbox->addWidget( actionButton( Default ) ); numButton++;
    }
    else if( mButton.mask & User3 & layout[i] )
    {
      hbox->addWidget( actionButton( User3 ) ); numButton++;
    }
    else if( mButton.mask & User2 & layout[i] )
    {
      hbox->addWidget( actionButton( User2 ) ); numButton++;
    }
    else if( mButton.mask & User1 & layout[i] )
    {
      hbox->addWidget( actionButton( User1 ) ); numButton++;
    }
    else if( mButton.mask & Ok & layout[i] )
    {
      hbox->addWidget( actionButton( Ok ) ); numButton++;
    }
    else if( mButton.mask & Apply & layout[i] )
    {
      hbox->addWidget( actionButton( Apply ) ); numButton++;
    }
    else if( mButton.mask & Try & layout[i] )
    {
      hbox->addWidget( actionButton( Try ) ); numButton++;
    } 
    else if( mButton.mask & Cancel & layout[i] )
    {
      hbox->addWidget( actionButton( Cancel ) ); numButton++;
    }
    else if( mButton.mask & Close & layout[i] )
    {
      hbox->addWidget( actionButton( Close ) ); numButton++;
    }
    else
    {
      continue;
    }

    if( mMessageBoxMode == true )
    {
      hbox->addStretch(1);
    }

  }

  mButton.resize( true, 0, spacingHint() );
}


QPushButton *KDialogBase::actionButton( ButtonCode id )
{
  return( mButton.button(id) );
}


void KDialogBase::enableButton( ButtonCode id, bool state )
{
  QPushButton *pb = actionButton( id );
  if( pb != 0 )
  {
    pb->setEnabled( state );
  }
}


void KDialogBase::enableButtonOK( bool state )
{
  enableButton( Ok, state );
}


void KDialogBase::enableButtonApply( bool state )
{
  enableButton( Apply, state );
}


void KDialogBase::enableButtonCancel( bool state )
{
  enableButton( Cancel, state );
}


void KDialogBase::showButton( ButtonCode id, bool state )
{
  QPushButton *pb = actionButton( id );
  if( pb != 0 )
  {
    state ? pb->show() : pb->hide();
  }
}


void KDialogBase::showButtonOK( bool state )
{
  showButton( Ok, state );
}


void KDialogBase::showButtonApply( bool state )
{
  showButton( Apply, state );
}


void KDialogBase::showButtonCancel( bool state )
{
  showButton( Cancel, state );
}


void KDialogBase::setButtonOKText( const QString &text, 
				   const QString &tooltip,
				   const QString &quickhelp )
{
  QPushButton *pb = actionButton( Ok );
  if( pb == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "If you press the <b>OK</b> button, all changes\n"
    "you made will be used to proceed.");

  pb->setText( text.isEmpty() ? i18n("&OK") : text );
  QToolTip::add( pb, tooltip.isEmpty() ? i18n("Accept settings") : tooltip );
  QWhatsThis::add( pb, quickhelp.isEmpty() ? whatsThis : quickhelp );
}



void KDialogBase::setButtonApplyText( const QString &text, 
				      const QString &tooltip,
				      const QString &quickhelp )
{
  QPushButton *pb = actionButton( Apply );
  if( pb == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "When clicking <b>Apply</b>, the settings will be\n"
    "handed over to the program, but the dialog\n"
    "will not be closed."
    "Use this to try different settings. ");

  pb->setText( text.isEmpty() ? i18n("&Apply") : text );
  QToolTip::add( pb, tooltip.isEmpty() ? i18n("Apply settings") : tooltip );
  QWhatsThis::add( pb, quickhelp.isEmpty() ? whatsThis : quickhelp );
}


void KDialogBase::setButtonCancelText( const QString& text, 
				       const QString& tooltip, 
				       const QString& quickhelp )
{
  QPushButton *pb = actionButton( Cancel );
  if( pb == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "If you press the <b>Cancel</b> button, all changes\n"
    "you made will be abandoned and the dialog\n"
    "will be closed.\n"
    "The program will be in the state before\n"
    "opening the dialog.");

  pb->setText( text.isEmpty() ? i18n("&Cancel") : text );
  QToolTip::add( pb, tooltip.isEmpty() ? i18n("Cancel settings") : tooltip);
  QWhatsThis::add( pb, quickhelp.isEmpty() ? whatsThis : quickhelp );
}


void KDialogBase::setButtonText( ButtonCode id, const QString &text )
{
  QPushButton *pb = actionButton( id );
  if( pb != 0 )
  {
    pb->setText( text );
  }
}


void KDialogBase::setButtonTip( ButtonCode id, const QString &text )
{
  QPushButton *pb = actionButton( id );
  if( pb != 0 )
  {
    QToolTip::add( pb, text );
  }
}


void KDialogBase::setButtonWhatsThis( ButtonCode id, const QString &text )
{
  QPushButton *pb = actionButton( id );
  if( pb != 0 )
  {
    QWhatsThis::add( pb, text );
  }
}


void KDialogBase::setButtonFocus( QPushButton *p,bool isDefault, bool isFocus )
{ 
  p->setDefault( isDefault );
  isFocus ? p->setFocus() : p->clearFocus();
}


void KDialogBase::setTreeListAutoResize( bool state )
{
  if( mJanus != 0 )
  {
    mJanus->setTreeListAutoResize( state );
  }
}


void KDialogBase::setIconListAllVisible( bool state )
{
  if( mJanus != 0 )
  {
    mJanus->setIconListAllVisible( state );
  }
}


void KDialogBase::slotHelp() 
{
  emit helpClicked();
  if( mHelpPath.isEmpty() == true )
  {
    kapp->beep();
  } 
  else 
  {
    kapp->invokeHTMLHelp( mHelpPath, mHelpTopic );
  }
}


void KDialogBase::slotDefault() 
{
  emit defaultClicked();
}


void KDialogBase::slotOk() 
{
  emit okClicked();
  accept();
}


void KDialogBase::slotApply() 
{
  emit applyClicked();
}


void KDialogBase::slotTry() 
{
  emit tryClicked();
}


void KDialogBase::slotUser3() 
{
  emit user3Clicked();
}


void KDialogBase::slotUser2() 
{
  emit user2Clicked();
}


void KDialogBase::slotUser1()
{
  emit user1Clicked();
}


void KDialogBase::slotYes() 
{
  emit yesClicked();
  done( Yes );
}


void KDialogBase::slotNo()
{
  emit noClicked();
  done( No );
}


void KDialogBase::slotCancel() 
{
  emit cancelClicked();
  done( mMessageBoxMode == true ? Cancel : Rejected );
}


void KDialogBase::slotClose() 
{
  emit closeClicked();
  reject();
}


void KDialogBase::helpClickedSlot( const QString & )
{
  slotHelp(); 
}


void KDialogBase::applyPressed()
{
  emit apply();
}


void KDialogBase::enableLinkedHelp( bool state )
{
  if( state == true )
  {
    if( mUrlHelp != 0 )
    {
      return;
    }
  
    mUrlHelp = new KURLLabel( this, "url" );
    mUrlHelp->setText( helpLinkText() );
    mUrlHelp->setFloat(true);
    mUrlHelp->setUnderline(true);
    if( mShowTile == true && mTile->get() != 0 )
    { 
      mUrlHelp->setBackgroundPixmap(*mTile->get());
    }
    mUrlHelp->setMinimumHeight( fontMetrics().height() + marginHint() );
    connect(mUrlHelp,SIGNAL(leftClickedURL(const QString &)),
	    SLOT(helpClickedSlot(const QString &)));
    mUrlHelp->show();
  }
  else
  {
    if( mUrlHelp == 0 )
    {
      return;
    }
    delete mUrlHelp; mUrlHelp = 0;
  }
 
  if( mIsActivated == true )
  {
    setupLayout();
  }
}


void KDialogBase::setHelp( const QString &path, const QString &topic )
{
  mHelpPath  = path;
  mHelpTopic = topic;
}


void KDialogBase::setHelpLinkText( const QString &text )
{
  mHelpLinkText = text;
  if( mUrlHelp != 0 )
  {
    mUrlHelp->setText( helpLinkText() );
  }
}


QFrame *KDialogBase::addPage( const QString &itemName, const QString &header,
			      const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addPage( itemName, header, pixmap ) );
}


QVBox *KDialogBase::addVBoxPage( const QString &itemName, 
				 const QString &header, const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addVBoxPage( itemName, header, pixmap) );
}


QHBox *KDialogBase::addHBoxPage( const QString &itemName, 
				 const QString &header,
				 const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addHBoxPage( itemName, header, pixmap ) );
}


QGrid *KDialogBase::addGridPage( int n, QGrid::Direction dir, 
				 const QString &itemName, 
				 const QString &header, const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addGridPage( n, dir, itemName, header,
						 pixmap) );
}


QFrame *KDialogBase::makeMainWidget()
{
  if( mJanus != 0 || mMainWidget != 0 )
  {
    printMakeMainWidgetError();
    return( 0 );
  }
  
  QFrame *mainWidget = new QFrame( this );
  setMainWidget( mainWidget );
  return( mainWidget );
}


QVBox *KDialogBase::makeVBoxMainWidget()
{
  if( mJanus != 0 || mMainWidget != 0 )
  {
    printMakeMainWidgetError();
    return( 0 );
  }

  QVBox *mainWidget = new QVBox( this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return( mainWidget );
}


QHBox *KDialogBase::makeHBoxMainWidget()
{
  if( mJanus != 0 || mMainWidget != 0 )
  {
    printMakeMainWidgetError();
    return( 0 );
  }

  QHBox *mainWidget = new QHBox( this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return( mainWidget );
}


QGrid *KDialogBase::makeGridMainWidget( int n, QGrid::Direction dir )
{
  if( mJanus != 0 || mMainWidget != 0 )
  {
    printMakeMainWidgetError();
    return( 0 );
  }

  QGrid *mainWidget = new QGrid( n, dir, this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return( mainWidget );
}  


void KDialogBase::printMakeMainWidgetError()
{
  if( mJanus != 0 )
  {
    qDebug("makeMainWidget: Illegal mode (wrong constructor)");
  }
  else if( mMainWidget != 0 )
  {
    qDebug("makeMainWidget: Main widget already defined");
  }
}


void KDialogBase::setMainWidget( QWidget *widget )
{
  if( mJanus != 0 )
  {
    if( mJanus->setSwallowedWidget(widget) == true )
    {
      mMainWidget = widget;
    }
  }
  else
  {
    mMainWidget = widget;
    if( mIsActivated == true )
    {
      setupLayout();
    }
  }
}


QWidget *KDialogBase::getMainWidget() 
{
  return( mMainWidget );
}


bool KDialogBase::showPage( int index )
{
  return( mJanus == 0 ? false : mJanus->showPage(index) );
}


int KDialogBase::activePageIndex() const
{
  return( mJanus == 0 ? -1 : mJanus->activePageIndex() );
}


int KDialogBase::pageIndex( QWidget *widget ) const
{
  return( mJanus == 0 ? -1 : mJanus->pageIndex( widget) );
}



QRect KDialogBase::getContentsRect()
{
  QRect r;
  r.setLeft( marginHint() );
  r.setTop( marginHint() + (mUrlHelp != 0 ? mUrlHelp->height() : 0) );
  r.setRight( width() - marginHint() );
  int h = (mActionSep==0?0:mActionSep->minimumSize().height()+marginHint());
  r.setBottom( height() - mButton.box->minimumSize().height() - h );
  return(r);
}



void KDialogBase::getBorderWidths(int& ulx, int& uly, int& lrx, int& lry) const
{
  ulx = marginHint();
  uly = marginHint();
  if( mUrlHelp != 0  )
  {
    uly += mUrlHelp->minimumSize().height();
  }

  lrx = marginHint();
  lry = mButton.box->minimumSize().height();
  if( mActionSep != 0 ) 
  { 
    lry += mActionSep->minimumSize().height() + marginHint();
  }
}


QSize KDialogBase::calculateSize(int w, int h)
{
  int ulx, uly, lrx, lry;
  getBorderWidths(ulx, uly, lrx, lry);
  return( QSize(ulx+w+lrx,uly+h+lry) );
}


QString KDialogBase::helpLinkText()
{
  return( mHelpLinkText==QString::null ? i18n("Get help...") : mHelpLinkText );
}


void KDialogBase::updateGeometry()
{
  if( mTopLayout != 0 )
  {
    mTopLayout->setMargin( marginHint() );
    mTopLayout->setSpacing(spacingHint() );
  }
}



void KDialogBase::keyPressEvent( QKeyEvent *e )
{
  //
  // Reimplemented from KDialog to remove the default behavior 
  // and to add F1 (help) sensitivity and some animation. 
  //
  if( e->state() == 0 )
  {
    if( e->key() == Key_F1 )
    {
      QPushButton *pb = actionButton( Help );
      if( pb != 0 )
      {
	pb->animateClick();
	e->accept();
	return;
      }
    }
    if( e->key() == Key_Escape )
    {
      QPushButton *pb = actionButton( mEscapeButton );
      if( pb != 0 )
      {
	pb->animateClick();
	e->accept();
	return;
      }

    }
  }
  else if( e->key() == Key_F1 && e->state() == ShiftButton )
  {
    QWhatsThis::enterWhatsThisMode();
    e->accept();
    return;
  }


  //
  // Do the default action instead. Note KDialog::keyPressEvent is bypassed
  //
  QDialog::keyPressEvent(e);
}



void KDialogBase::hideEvent( QHideEvent * )
{
  emit hidden();
}



void KDialogBase::closeEvent( QCloseEvent *e )
{
  QPushButton *pb = actionButton( mEscapeButton );
  if( pb != 0 )
  {
    pb->animateClick();
  }
  else
  {
    QDialog::closeEvent(e);
  }
}


bool KDialogBase::haveBackgroundTile()
{
  return( mTile == 0 || mTile->get() == 0 ? false : true );
}


const QPixmap *KDialogBase::getBackgroundTile()
{
  return( mTile == 0 ? 0 : mTile->get() );
}


void KDialogBase::setBackgroundTile( const QPixmap *pix )
{
  if( mTile != 0 )
  {
    mTile->set( pix );
  }
}


void KDialogBase::updateBackground()
{
  if( mTile == 0 || mTile->get() == 0 )
  {
    QPixmap nullPixmap;
    setBackgroundPixmap(nullPixmap);
    mButton.box->setBackgroundPixmap(nullPixmap);      
    setBackgroundMode(PaletteBackground);
    mButton.box->setBackgroundMode(PaletteBackground);
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
    mButton.box->setBackgroundPixmap(*pix);
    showTile( mShowTile );
  }
}


void KDialogBase::showTile( bool state )
{
  mShowTile = state;
  if( mShowTile == false || mTile == 0 || mTile->get() == 0 )
  {
    setBackgroundMode(PaletteBackground);
    mButton.box->setBackgroundMode(PaletteBackground);
    if( mUrlHelp != 0 )
    {
      mUrlHelp->setBackgroundMode(PaletteBackground);
    }
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
    mButton.box->setBackgroundPixmap(*pix);
    if( mUrlHelp != 0 )
    {
      mUrlHelp->setBackgroundPixmap(*pix);
    }
  }
}



KDialogBaseButton::KDialogBaseButton( const QString &text, int key,
				      QWidget *parent,  const char *name )
  : QPushButton( text, parent, name )
{
  mKey = key;
}




KDialogBaseTile::KDialogBaseTile( QObject *parent, const char *name )
  : QObject( parent, name )
{
  mPixmap = 0;
}


KDialogBaseTile::~KDialogBaseTile()
{
  cleanup();
}


void KDialogBaseTile::set( const QPixmap *pix )
{
  if( pix == 0 )
  {
    cleanup();
  }
  else
  {
    if( mPixmap == 0 )
    {
      mPixmap = new QPixmap(*pix);
    }
    else
    {
      *mPixmap = *pix;
    }
  }

  emit pixmapChanged();
}


const QPixmap *KDialogBaseTile::get() const
{
  return( mPixmap );
}


void KDialogBaseTile::cleanup()
{
  delete mPixmap; mPixmap = 0;
}

#include "kdialogbase.moc"
