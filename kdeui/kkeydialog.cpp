/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@via.ecp.fr>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qdrawutil.h>
#include <qradiobutton.h>


#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qpopupmenu.h>
#include <qgroupbox.h>



#include <kckey.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlgui.h>
#include <qdom.h>

#include "kkeydialog.h"

#include <kaction.h>

/*****************************************************************************/
/* KSplitListItem                                                            */
/*                                                                           */
/* Added by Mark Donohoe <donohoe@kde.org>                                   */
/*                                                                           */
/*****************************************************************************/

KSplitListItem::KSplitListItem( const QString& s, int _id )
	:  QListBoxItem()
{
	id = _id;
	setText( s );
	
	QString str( s );
	int i = str.find( ':' );
	
	actionName = str.left( i );
	actionName.simplifyWhiteSpace();
	
	str.remove( 0, i+1 );
	
	keyName = str.simplifyWhiteSpace();
	
	halfWidth = 0;
}

void KSplitListItem::setWidth( int newWidth )
{
	halfWidth = newWidth/2;
}

void KSplitListItem::paint( QPainter *p )
{
        QFontMetrics fm = p->fontMetrics();
        int yPos;                       // vertical text position
        yPos = fm.ascent() + fm.leading()/2;
        p->drawText( 5, yPos, actionName );
	p->drawText( 5 + halfWidth, yPos, keyName );
}

int KSplitListItem::height(const QListBox *lb ) const
{
    return lb->fontMetrics().lineSpacing() + 1;
}

int KSplitListItem::width(const QListBox *lb ) const
{
    return lb->fontMetrics().width( text() ) + 6;
}

/***********************************************************************/
/* KSplitList                                                          */
/*                                                                     */
/* Added by Mark Donohoe <donohoe@kde.org>                             */
/*                                                                     */
/***********************************************************************/
KSplitList::KSplitList( QWidget *parent , const char *name )
  : KListBox( parent, name )
{
  setFocusPolicy( QWidget::StrongFocus );
  if( style() == MotifStyle )
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
  else
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
}

void KSplitList::resizeEvent( QResizeEvent *e )
{
  emit newWidth( width() );
  KListBox::resizeEvent( e );
}

void KSplitList::styleChange( GUIStyle )
{
  if( style() == MotifStyle )
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
  else
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
}

void KSplitList::paletteChange ( const QPalette & oldPalette )
{
  KListBox::paletteChange( oldPalette );
}


void KSplitList::setVisibleItems( int numItem )
{
  int h = fontMetrics().lineSpacing()+1;
  setMinimumHeight( h * QMAX(1,numItem) + lineWidth() * 2 );
}






/***********************************************************************/
/* KKeyButton                                                          */
/*                                                                     */
/* Added by Mark Donohoe <donohoe@kde.org>                             */
/*                                                                     */
/***********************************************************************/

KKeyButton::KKeyButton(QWidget *parent, const char *name)
  : QPushButton( parent, name )
{
  setFocusPolicy( QWidget::StrongFocus );
  editing = FALSE;
}

KKeyButton::~KKeyButton ()
{
}

void KKeyButton::setText( const QString& text )
{
  QPushButton::setText( text );
  setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}


void KKeyButton::setEditing(bool _editing)
{
  editing = _editing;
  repaint();
}


bool KKeyButton::isEditing() const
{
  return editing;
}


void KKeyButton::drawButton( QPainter *painter )
{
  QPointArray a( 4 );
  a.setPoint( 0, 0, 0) ;
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, 0, 0 );

  QRegion r1( a );
  painter->setClipRegion( r1 );
  painter->setBrush( backgroundColor().light() );
  painter->drawRoundRect( 0, 0, width(), height(), 20, 20);

  a.setPoint( 0, width(), height() );
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, width(), height() );

  QRegion r2( a );
  painter->setClipRegion( r2 );
  painter->setBrush( backgroundColor().dark() );
  painter->drawRoundRect( 0, 0, width(), height(), 20, 20 );
  
  painter->setClipping( FALSE );
  if( width() > 12 && height() > 8 )
    qDrawShadePanel( painter, 6, 4, width() - 12, height() - 8,
		     colorGroup(), TRUE, 1, 0L );
  if ( editing ) 
  {
    painter->setPen( colorGroup().base() );
    painter->setBrush( colorGroup().base() );
  } 
  else 
  {
    painter->setPen( backgroundColor() );
    painter->setBrush( backgroundColor() );
  }
  if( width() > 14 && height() > 10 )
    painter->drawRect( 7, 5, width() - 14, height() - 10 );
  
  drawButtonLabel( painter );
	
  painter->setPen( colorGroup().text() );
  painter->setBrush( NoBrush );
  if( hasFocus() || editing ) 
  {
    if( width() > 16 && height() > 12 )
      painter->drawRect( 8, 6, width() - 16, height() - 12 );
  }
	
}

/************************************************************************/
/* KKeyDialog                                                           */
/*                                                                      */
/* Originally by Nicolas Hadacek <hadacek@via.ecp.fr>                   */
/*                                                                      */
/* Substantially revised by Mark Donohoe <donohoe@kde.org>              */
/*                                                                      */
/* And by Espen Sand <espen@kde.org> 1999-10-19                         */
/* (by using KDialogBase there is almost no code left ;)                */
/*                                                                      */
/************************************************************************/
KKeyDialog::KKeyDialog( QDict<KKeyEntry> *aKeyDict, QWidget *parent,
			bool check_against_std_keys)
  : KDialogBase( parent, 0, TRUE, i18n("Configure Key Bindings"), 
		 Help|Default|Ok|Cancel, Ok )
{
  KKeyChooser *kc =  new KKeyChooser( aKeyDict, this, check_against_std_keys );
  setMainWidget(kc);
  connect( this, SIGNAL(defaultClicked()), kc, SLOT(allDefault()) );
  enableButton ( Help, false );
}



int KKeyDialog::configureKeys( KAccel *keys, bool save_settings, 
			       QWidget *parent )
{
  QDict<KKeyEntry> dict = keys->keyDict();
  KKeyDialog *kd = new KKeyDialog( &dict, parent );
  CHECK_PTR( kd );
  int retcode = kd->exec();
  delete kd;

  if( retcode == Accepted ) 
  {
    keys->setKeyDict( dict );
    if (save_settings)
      keys->writeSettings();
  }
  return retcode;
}

int KKeyDialog::configureKeys( KGlobalAccel *keys, bool save_settings, 
			       QWidget *parent )
{
  QDict<KKeyEntry> dict = keys->keyDict();

  KKeyDialog *kd = new KKeyDialog( &dict, parent );
  CHECK_PTR( kd );
  int retcode = kd->exec();
  delete kd;

  if( retcode == Accepted ) 
  {
    keys->setKeyDict( dict );
    if (save_settings)
      keys->writeSettings();
  }
  return retcode;
}

int KKeyDialog::configureKeys( KActionCollection *coll, const QString& file,
                               bool save_settings, QWidget *parent )
{
  QDict<KKeyEntry> *dict = coll->keyDict();

  KKeyDialog *kd = new KKeyDialog( dict, parent );
  CHECK_PTR( kd );
  int retcode = kd->exec();
  delete kd;

  if( retcode != Accepted ) 
    return retcode;

  if (!save_settings)
  {
    coll->setKeyDict( *dict );
    return retcode;
  }

  // let's start saving this info
  QString raw_xml(KXMLGUIFactory::readConfigFile(file));
  QDomDocument doc;
  doc.setContent(raw_xml);

  QString tagActionProp = QString::fromLatin1( "ActionProperties" );
  QString tagAction     = QString::fromLatin1( "Action" );
  QString attrName      = QString::fromLatin1( "name" );
  QString attrAccel     = QString::fromLatin1( "accel" );

  // first, lets see if we have existing properties
  QDomElement elem;
  QDomElement it = doc.firstChild().firstChild().toElement();
  for ( ; !it.isNull(); it = it.nextSibling().toElement() )
  {
    if ( it.tagName() == tagActionProp )
    {
      elem = it;
      break;
    }
  }

  // if there was none, create one
  if ( elem.isNull() )
  {
    elem = doc.createElement( tagActionProp );
    doc.firstChild().appendChild(elem);
  }

  // now, iterate through our actions
  for (unsigned int i = 0; i < coll->count(); i++)
  {
    KAction *action = coll->action(i);

    // see if we changed
    KKeyEntry *key = (*dict)[action->name()];
    if (key->aCurrentKeyCode == key->aConfigKeyCode)
      continue;

    // now see if this element already exists
    QDomElement act_elem;
    for ( it = elem.firstChild().toElement(); !it.isNull(); it = it.nextSibling().toElement() )
    {
      if ( it.attribute( attrName ) == action->name() )
      {
        act_elem = it;
        break;
      }
    }

    // nope, create a new one
    if ( act_elem.isNull() )
    {
      act_elem = doc.createElement( tagAction );
      act_elem.setAttribute( attrName, action->name() );
    }
    act_elem.setAttribute( attrAccel,
                           KAccel::keyToString( key->aConfigKeyCode ) );

    elem.appendChild( act_elem );
  }

  // finally, write out the result
  KXMLGUIFactory::saveConfigFile(doc, file);

  coll->setKeyDict( *dict );

  return retcode;
}

KKeyChooser::KKeyChooser( QDict<KKeyEntry> *aKeyDict, QWidget *parent,
			  bool check_against_std_keys)
  : QWidget( parent )
{
	
  bKeyIntercept = FALSE;
  kbMode = NoKey;
	
  aKeyIt = new QDictIterator<KKeyEntry> ( *aKeyDict );

  //
  // TOP LAYOUT MANAGER
  //
  // The following layout is used for the dialog
  // 		LIST LABELS LAYOUT
  //		SPLIT LIST BOX WIDGET
  //		CHOOSE KEY GROUP BOX WIDGET
  //		BUTTONS LAYOUT
  // Items are added to topLayout as they are created.
  //

  QBoxLayout *topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  QGridLayout *stackLayout = new QGridLayout(2, 2, 2);
  topLayout->addLayout( stackLayout, 10 );
  stackLayout->setRowStretch( 1, 10 ); // Only list will stretch

  //
  // CREATE SPLIT LIST BOX
  //
  // Copy all currentKeyCodes to configKeyCodes
  // and fill up the split list box with the action/key pairs.
  //
  wList = new KSplitList( this );
  wList->setVisibleItems( 5 );
  wList->setAutoUpdate(FALSE);
  wList->setFocus();
  stackLayout->addMultiCellWidget( wList, 1, 1, 0, 1 );

  //
  // CREATE LIST LABELS
  //
  actLabel = new QLabel( wList, i18n("&Action"), this );
  stackLayout->addWidget(actLabel, 0, 0);

  keyLabel = new QLabel( i18n("Current key"), this );
  stackLayout->addWidget(keyLabel, 0, 1);

  //
  // Add all "keys" to the list
  //
  aIt = aKeyIt;
  aIt->toFirst();
  int id = 0;
  while ( aIt->current() ) 
  {
    aIt->current()->aConfigKeyCode = aIt->current()->aCurrentKeyCode;
		
    KSplitListItem *sli = new KSplitListItem(
      item( aIt->current()->aConfigKeyCode, aIt->current()->descr ), id);
		
    connect( wList, SIGNAL( newWidth( int ) ),sli, SLOT( setWidth( int ) ) );
    wList->inSort( sli );
		
    ++ ( *aIt );
    ++id;
  }

  //
  // Make sure there is no horizontal scrollbar on startup
  //
  wList->setMinimumWidth( wList->sizeHint().width() +
			  wList->verticalScrollBar()->sizeHint().width() +
			  wList->lineWidth() * 2 );


  if ( wList->count() == 0 ) wList->setEnabled( FALSE );
  //connect( wList, SIGNAL( selected( int ) ), SLOT( toChange( int ) ) );
  connect( wList, SIGNAL( highlighted( int ) ), SLOT( updateAction( int ) ) );

  //
  // CREATE CHOOSE KEY GROUP
  //
  fCArea = new QGroupBox( this );
  topLayout->addWidget( fCArea, 1 );

  fCArea->setTitle( i18n("Choose a key for the selected action") );
  fCArea->setFrameStyle( QFrame::Box | QFrame::Sunken );

  //
  // CHOOSE KEY GROUP LAYOUT MANAGER	
  //
  QGridLayout *grid = new QGridLayout( fCArea, 6, 4, KDialog::spacingHint() );
  grid->setRowStretch(0,10);
  grid->setRowStretch(1,10);
  grid->setRowStretch(2,10);
  grid->setRowStretch(3,10);
  grid->setRowStretch(4,10);
  grid->setRowStretch(5,10);

  grid->setColStretch(0,0);
  grid->setColStretch(1,10);
  grid->setColStretch(2,90);
  grid->setColStretch(3,0);
	
  grid->addRowSpacing(0,15);
  grid->addRowSpacing(5,1);
	
	
  kbGroup = new QButtonGroup( fCArea );
  kbGroup->hide();
  kbGroup->setExclusive( true );
	
  QRadioButton *rb = new QRadioButton( i18n("&No key"), fCArea );
  kbGroup->insert( rb, NoKey );
  grid->addMultiCellWidget( rb, 1, 1, 1, 2 );
	
  rb = new QRadioButton( i18n("&Default key"), fCArea );
  kbGroup->insert( rb, DefaultKey );
  grid->addMultiCellWidget( rb, 2, 2, 1, 2 );
	
  rb = new QRadioButton( i18n("&Custom key"), fCArea );
  kbGroup->insert( rb, CustomKey );
  connect( kbGroup, SIGNAL( clicked( int ) ), SLOT( keyMode( int ) ) );
  grid->addMultiCellWidget( rb, 3, 3, 1, 2 );

  QBoxLayout *pushLayout = new QHBoxLayout( KDialog::spacingHint() );
  grid->addLayout( pushLayout, 4, 2 );
	
  cShift = new QCheckBox( fCArea );
  cShift->setText( QString::fromLatin1("SHIFT") );
  cShift->setEnabled( FALSE );
  connect( cShift, SIGNAL( clicked() ), SLOT( shiftClicked() ) );
	
  cCtrl = new QCheckBox( fCArea );
  cCtrl->setText( QString::fromLatin1("CTRL") );
  cCtrl->setEnabled( FALSE );
  connect( cCtrl, SIGNAL( clicked() ), SLOT( ctrlClicked() ) );
	
  cAlt = new QCheckBox( fCArea );
  cAlt->setText( QString::fromLatin1("ALT") );
  cAlt->setEnabled( FALSE );
  connect( cAlt, SIGNAL( clicked() ), SLOT( altClicked() ) );
  
  bChange = new KKeyButton(fCArea, "key");
  bChange->setEnabled( FALSE );
  connect( bChange, SIGNAL( clicked() ), SLOT( changeKey() ) );
	
  //
  // Add widgets to the geometry manager
  //
  pushLayout->addWidget( cShift );
  pushLayout->addWidget( cCtrl );
  pushLayout->addWidget( cAlt );
  pushLayout->addSpacing( KDialog::spacingHint()*2 );
  pushLayout->addWidget( bChange );
  pushLayout->addStretch( 10 );


  lNotConfig = new QLabel(fCArea);
  lNotConfig->resize(0,0);
  QFont f = KGlobalSettings::generalFont();
  f.setPointSize(f.pointSize()+2);
  f.setBold(true);
  lNotConfig->setFont( f );
  lNotConfig->setAlignment( AlignCenter );
  lNotConfig->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  if ( wList->count()==0 )
    lNotConfig->setText( i18n("No keys defined") );
  else 
  {
    lNotConfig->setText( i18n("Not configurable") );
    lNotConfig->hide();
  }
  lNotConfig->hide();
  
  lInfo = new QLabel(fCArea);
  resize(0,0);
  lInfo->setAlignment( AlignCenter );
  lInfo->setEnabled( FALSE );
  lInfo->hide();
	
  wList->setAutoUpdate(TRUE);
  wList->update();
	
  globalDict = new QDict<int> ( 100, false );
  globalDict->setAutoDelete( true );
  readGlobalKeys();
  stdDict = new QDict<int> ( 100, false );
  stdDict->setAutoDelete( true );
  if (check_against_std_keys)
    readStdKeys();
  wList->setCurrentItem( 0 );
}



KKeyChooser::~KKeyChooser()
{
}

void KKeyChooser::updateAction( int index )
{
	aIt->toFirst();
	
	(*aIt) += wList->getId(index);
	sEntryKey = aIt->currentKey();
	pEntry = aIt->current();
	
	if ( pEntry->aConfigKeyCode == 0 )
		kbMode = NoKey;
	else if ( pEntry->aConfigKeyCode == pEntry->aDefaultKeyCode )
		kbMode = DefaultKey;
	else kbMode = CustomKey;

	toChange( index );
}

void KKeyChooser::readGlobalKeys()
{
  //debug("KKeyChooser::readGlobalKeys()");
  
  globalDict->clear();
  
  
  // Insert all global keys into globalDict
  int *keyCode;
  KConfig pConfig;
  QMap<QString, QString> tmpMap = pConfig.entryMap(i18n("Global Keys"));
  QMap<QString, QString>::Iterator gIt(tmpMap.begin());
  for (; gIt != tmpMap.end(); ++gIt) {
    keyCode = new int;
	  *keyCode = KAccel::stringToKey( *gIt );
	  globalDict->insert( gIt.key(), keyCode);
  }
  
  // Only insert global keys which don't appear in the dictionary to be configured
  aIt->toFirst();
  while ( aIt->current() ) {
    if ( globalDict->find( aIt->currentKey() ) ) {
      globalDict->remove( aIt->currentKey());
    }
    ++ ( *aIt );
  }
}

void KKeyChooser::readStdKeys()
{
  // debug("KKeyChooser::readStdKeys()");
  
  stdDict->clear();
  
  
  // Insert all standard keys into globalDict
  int *keyCode;
  KConfig pConfig;
  QMap<QString, QString> tmpMap = pConfig.entryMap(i18n("Keys"));
  QMap<QString, QString>::Iterator sIt(tmpMap.begin());
  for (; sIt != tmpMap.end(); ++sIt) {
    keyCode = new int;
    *keyCode = KAccel::stringToKey( *sIt );
    stdDict->insert( sIt.key(), keyCode);
  }
  
  // Only insert std keys which don't appear in the dictionary to be configured
  aIt->toFirst();
  while ( aIt->current() ) {
    if ( stdDict->find( aIt->currentKey() ) ) {
      stdDict->remove( aIt->currentKey());
    }
    ++ ( *aIt );
  }
}

void KKeyChooser::toChange( int index )
{
	bKeyIntercept = FALSE;
	
	/* get the entry */
	aIt->toFirst();
	(*aIt) += wList->getId(index);

	sEntryKey = aIt->currentKey();
	pEntry = aIt->current();
	
	//eKey->setEnabled( FALSE );
	
	/* Is the key configurable or has the user turned it off ? */
	if ( !pEntry->bConfigurable || kbMode == NoKey ) {
		lInfo->setEnabled( FALSE );
		cShift->setEnabled( FALSE ); cCtrl->setEnabled( FALSE ); cAlt->setEnabled( FALSE );
		bChange->setEnabled( FALSE );  //bDefault->setEnabled( FALSE );
		lNotConfig->setEnabled( TRUE );
		
		uint kCode = pEntry->aConfigKeyCode;
		uint kSCode = kCode & ~(SHIFT | CTRL | ALT);
		
		if ( kSCode == Key_Shift ) cShift->setChecked(FALSE);
		else cShift->setChecked( kCode & SHIFT );
		if ( kSCode == Key_Control ) cCtrl->setChecked(FALSE);
		else cCtrl->setChecked( kCode & CTRL );
		if ( kSCode == Key_Alt ) cAlt->setChecked(FALSE);
		else cAlt->setChecked( kCode & ALT );
		
		QString str = KAccel::keyToString( kSCode );
		bChange->setText(str);
		
	} else {
		lNotConfig->setEnabled( FALSE );
		lInfo->setText(QString::null); lInfo->setEnabled( TRUE );
		
		uint kCode = pEntry->aConfigKeyCode;
		uint kSCode = kCode & ~(SHIFT | CTRL | ALT);
		
		//cShift->setEnabled( TRUE ); cCtrl->setEnabled( TRUE ); cAlt->setEnabled( TRUE );
		if ( kSCode == Key_Shift ) cShift->setChecked(FALSE);
		else cShift->setChecked( kCode & SHIFT );
		if ( kSCode == Key_Control ) cCtrl->setChecked(FALSE);
		else cCtrl->setChecked( kCode & CTRL );
		if ( kSCode == Key_Alt ) cAlt->setChecked(FALSE);
		else cAlt->setChecked( kCode & ALT );
		
		QString str = KAccel::keyToString( kSCode );
		bChange->setText(str); //eKey->setText(str);
		//bChange->setEnabled( TRUE ); //bDefault->setEnabled( TRUE );
		
		if ( isKeyPresent() ) {
			lInfo->setText(i18n("Attention : key already used") );
		}
		
		if ( kbMode == DefaultKey ) {
			cAlt->setEnabled( false );
			cShift->setEnabled( false );
			cCtrl->setEnabled( false );
			bChange->setEnabled( false );
		} else {
			cAlt->setEnabled( true );
			cShift->setEnabled( true );
			cCtrl->setEnabled( true );
			bChange->setEnabled( true );
		}
	}
		
	((QRadioButton *)kbGroup->find(NoKey))->setChecked( kbMode == NoKey );
	((QRadioButton *)kbGroup->find(DefaultKey))->setChecked( kbMode == DefaultKey );
	((QRadioButton *)kbGroup->find(CustomKey))->setChecked( kbMode == CustomKey );
	
	if ( !pEntry->bConfigurable ) {
		cAlt->setEnabled( false );
		cShift->setEnabled( false );
		cCtrl->setEnabled( false );
		bChange->setEnabled( false );
		((QRadioButton *)kbGroup->find(NoKey))->setEnabled( false );
		((QRadioButton *)kbGroup->find(DefaultKey))->setEnabled( false );
		((QRadioButton *)kbGroup->find(CustomKey))->setEnabled( false );
	} else {
		((QRadioButton *)kbGroup->find(NoKey))->setEnabled( true );
		((QRadioButton *)kbGroup->find(DefaultKey))->setEnabled( pEntry->aDefaultKeyCode != 0);
		((QRadioButton *)kbGroup->find(CustomKey))->setEnabled( true );
	}	
}

void KKeyChooser::fontChange( const QFont & )
{
	actLabel->setFixedHeight( actLabel->sizeHint().height() );
	keyLabel->setFixedHeight( keyLabel->sizeHint().height() );

	cAlt->setFixedHeight( bChange->sizeHint().height() );
	cShift->setFixedHeight( bChange->sizeHint().height() );
	cCtrl->setFixedHeight( bChange->sizeHint().height() );
	
	fCArea->setMinimumHeight( 4*bChange->sizeHint().height() );
	
	int widget_width = 0;
	
	setMinimumWidth( 20+5*(widget_width+10) );
}

void KKeyChooser::keyMode( int m )
{
	kbMode = m;
	switch( kbMode ) {
		case NoKey:
			noKey();
			break;
		case DefaultKey:
			defaultKey();
			break;
		case CustomKey: default:
			toChange( wList->currentItem() );
			break;
	}
}

void KKeyChooser::noKey()
{
  if (!pEntry)
    return;
	pEntry->aConfigKeyCode = 0;
	
	/* update the list and the change area */
	
	KSplitListItem *sli = new KSplitListItem(
		 item(pEntry->aConfigKeyCode, pEntry->descr),
		 wList->getId(wList->currentItem() )
	);
		
	//connect( wList, SIGNAL( newWidth( int ) ),
	//		 	sli, SLOT( setWidth( int ) ) );
				
	sli->setWidth( wList->width() );
	
	if ( bChange->isEditing() )
            bChange->setEditing(false);

	wList->changeItem( sli, wList->currentItem()  );
	toChange(wList->currentItem());
	emit keyChange();
}

void KKeyChooser::defaultKey()
{
  if (!pEntry)
    return;
	/* change the configKeyCode */
	pEntry->aConfigKeyCode = pEntry->aDefaultKeyCode;
	
	/* update the list and the change area */
	
	KSplitListItem *sli = new KSplitListItem(
		 item(pEntry->aConfigKeyCode, pEntry->descr),
		 wList->getId(wList->currentItem())
	);
		
	//connect( wList, SIGNAL( newWidth( int ) ),
	//		 	sli, SLOT( setWidth( int ) ) );
				
	sli->setWidth( wList->width() );

	if ( bChange->isEditing() )
            bChange->setEditing(false);

	wList->changeItem( sli, wList->currentItem()  );
	toChange(wList->currentItem());
	emit keyChange();
}

void KKeyChooser::allDefault()
{
	// Change all configKeyCodes to default values
	
	//int idx = wList->currentItem();

	disconnect( wList, SIGNAL( highlighted( int ) ),
		this, SLOT( updateAction( int ) ) );
	wList->setAutoUpdate(FALSE);
	wList->clear();
	
	aIt->toFirst();
	int id = 0;
	while ( aIt->current() ) {
		if ( aIt->current()->bConfigurable )
			aIt->current()->aConfigKeyCode = aIt->current()->aDefaultKeyCode;
		
		KSplitListItem *sli = new KSplitListItem(
		 	item(aIt->current()->aConfigKeyCode, aIt->current()->descr),
			id);

		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				
		sli->setWidth( wList->width() );
		
		wList->inSort( sli );
		
		++(*aIt);
		++id;
	}
	
	connect( wList, SIGNAL( highlighted( int ) ), SLOT( updateAction( int ) ) );
	wList->setAutoUpdate( true );
	wList->update();
	wList->setCurrentItem( 0 );
	emit keyChange();
}

void KKeyChooser::listSync()
{
	disconnect( wList, SIGNAL( highlighted( int ) ),
		this, SLOT( updateAction( int ) ) );
	wList->setAutoUpdate(FALSE);
	wList->clear();
	
	aIt->toFirst();
	int id = 0;
	while ( aIt->current() ) {
		
		KSplitListItem *sli = new KSplitListItem(
		 	item(aIt->current()->aCurrentKeyCode, aIt->current()->descr),
			id);

		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				
		sli->setWidth( wList->width() );
		
		wList->inSort( sli );
		
		++(*aIt);
		++id;
	}
	
	connect( wList, SIGNAL( highlighted( int ) ), SLOT( updateAction( int ) ) );
	wList->setAutoUpdate( true );
	wList->update();
	wList->setCurrentItem( 0 );
}

const QString KKeyChooser::item( uint keyCode, const QString& entryKey )
{
	QString str = entryKey;
	str = str.leftJustify(MAX_FCTN_LENGTH, ' ', TRUE);
	str += QString::fromLatin1(" : ");
	str += KAccel::keyToString(keyCode, true);
	str = str.leftJustify( MAX_FCTN_LENGTH + 3 +
						   MAX_KEY_LENGTH+MAX_KEY_MODIFIER_LENGTH, ' ', TRUE );
	return str;
}

void KKeyChooser::shiftClicked()
{
  if (!pEntry)
    return;
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Shift ) {
		if ( cShift->isOn() )
			pEntry->aConfigKeyCode |= SHIFT;
		else
			pEntry->aConfigKeyCode &= ~SHIFT;
			
		KSplitListItem *sli = new KSplitListItem(
		 	item(pEntry->aConfigKeyCode, pEntry->descr),
			wList->getId(wList->currentItem() )
		);
		
		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				
					
		sli->setWidth( wList->width() );
		
		wList->changeItem( sli, wList->currentItem() );
	}
	toChange(wList->currentItem());
	emit keyChange();
}

void KKeyChooser::ctrlClicked()
{
  if (!pEntry)
    return;
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Control ) {
		if ( cCtrl->isOn() )
			pEntry->aConfigKeyCode |= CTRL;
		else
			pEntry->aConfigKeyCode &= ~CTRL;
			
		KSplitListItem *sli = new KSplitListItem(
		 	item(pEntry->aConfigKeyCode, pEntry->descr),
			wList->getId(wList->currentItem() )
		);
		
		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				
					
		sli->setWidth( wList->width() );
		
		wList->changeItem( sli, wList->currentItem() );
	}
	toChange(wList->currentItem());
	emit keyChange();
}

void KKeyChooser::altClicked()
{
  if (!pEntry)
    return;
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Alt ) {
		if ( cAlt->isOn() )
			pEntry->aConfigKeyCode |= ALT;
		else
			pEntry->aConfigKeyCode &= ~ALT;
			
		KSplitListItem *sli = new KSplitListItem(
		 	item(pEntry->aConfigKeyCode, pEntry->descr),
			wList->getId(wList->currentItem() )
		);
		
		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				
					
		sli->setWidth( wList->width() );
			
		wList->changeItem( sli, wList->currentItem() );
	}
	toChange(wList->currentItem());
	emit keyChange();
}

void KKeyChooser::changeKey()
{
	bChange->setEditing(true);
	lInfo->setText( i18n("Press the wanted key") );
	lInfo->setEnabled( TRUE );
	/* give the focus to the widget */
	
	//eKey->setGeometry(bChange->x()+6, bChange->y()+4, bChange->width()-12,
		//bChange->height()-8);
	//eKey->show();
	//eKey->setEnabled(TRUE);
	setFocus();
	bKeyIntercept = TRUE;
}

void KKeyChooser::keyPressEvent( QKeyEvent *e )
{
  /* the keys are processed if the change button was pressed */
  if( !bKeyIntercept )
  {
    e->ignore();
    return;
  }
	
  uint kCode = e->key() & ~(SHIFT | CTRL | ALT);
  /* check the given key :
     if it is a non existent key (=0) : keep the old value and say
     what happened. 
  */
  if ( KAccel::keyToString(kCode).isNull() ) 
  {
    lInfo->setText( i18n("Undefined key") );
    return;
  }
	
  bKeyIntercept = FALSE;
  //eKey->hide();
  //eKey->setEnabled(FALSE);
  bChange->setEditing(true);
  bChange->setFocus();
  setKey(kCode);
}

void KKeyChooser::setKey( uint kCode)
{
  if (!pEntry)
    return;
	// uint kOldCode = pEntry->aConfigKeyCode;
	
	/* add the current modifier to the key */
	if ( kCode!=Key_Shift ) kCode |= (pEntry->aConfigKeyCode & SHIFT);
	if ( kCode!=Key_Control ) kCode |= (pEntry->aConfigKeyCode & CTRL);
	if ( kCode!=Key_Alt ) kCode |= (pEntry->aConfigKeyCode & ALT);
	
	/* set the list and the change button */
	pEntry->aConfigKeyCode = kCode;
	
	if ( isKeyPresent() ) {
		lInfo->setText( i18n("Attention : key already used") );
		return;
	}
	
	KSplitListItem *sli = new KSplitListItem(
	 	item(pEntry->aConfigKeyCode, pEntry->descr),
		wList->getId(wList->currentItem() )
	);
		
	connect( wList, SIGNAL( newWidth( int ) ),
			 sli, SLOT( setWidth( int ) ) );
				
				
	sli->setWidth( wList->width() );
	
	wList->changeItem( sli, wList->currentItem() );
	toChange(wList->currentItem());
	emit keyChange();
}

void KKeyChooser::editKey()
{
	bChange->setEnabled( FALSE ); //eKey->setEnabled( TRUE );
	lInfo->setText( i18n("Press 'Return' to quit editing") );
}

void KKeyChooser::editEnd()
{
	debug("Called editEnd() which relies on eKey widget");
	
	//uint kCode = KAccel::stringToKey(eKey->text());
	uint kCode = 0;
	if ( kCode==0 || (kCode & (SHIFT | CTRL | ALT)) ) {
		lInfo->setText( i18n("Incorrect key") );
		return;
	}
	setKey(kCode);
}

bool KKeyChooser::isKeyPresent()
{
  if (!pEntry->aConfigKeyCode)
    return false;
	// Search the global key codes to find if this keyCode is already used
	//  elsewhere
	
	QDictIterator<int> gIt( *globalDict );
	
	gIt.toFirst();
	while ( gIt.current() ) {
		debug("current %s:%d code %d", gIt.currentKey().ascii(), *gIt.current(), pEntry->aConfigKeyCode);
		if ( (unsigned int)(*gIt.current()) == pEntry->aConfigKeyCode && *gIt.current() != 0 ) {
			QString actionName( gIt.currentKey() );
			actionName.stripWhiteSpace();

			QString keyName = KAccel::keyToString( *gIt.current() );
			
			QString str =
			    i18n("The %1 key combination has already been "
				 "allocated\n"
				 "to the global %2 action.\n\n"
				 "Please choose a unique key combination.").
			    arg(keyName).arg(actionName);
				
			KMessageBox::sorry( this, i18n("Global key conflict"), str);
			
			return TRUE;
		}
		++gIt;
	}
	
	// Search the std key codes to find if this keyCode is already used
	//  elsewhere
	
	QDictIterator<int> sIt( *stdDict );

	sIt.toFirst();
	while ( sIt.current() ) {
		debug("current %s:%d code %d", sIt.currentKey().ascii(), *sIt.current(), pEntry->aConfigKeyCode);
		if ( (unsigned int)(*sIt.current()) == pEntry->aConfigKeyCode && *sIt.current() != 0 ) {
			QString actionName( sIt.currentKey() );
			actionName.stripWhiteSpace();

			QString keyName = KAccel::keyToString( *sIt.current() );
			
			QString str = 
			    i18n("The %1 key combination has already "
				 "been allocated\n"
				 "to the standard %2 action.\n"
				 "\n"
				 "Please choose a unique key combination.").
			    arg(keyName).arg(actionName);
			
			KMessageBox::sorry( this, str, i18n("Standard key conflict"));
			
			return TRUE;
		}
		++sIt;
	}

	// Search the aConfigKeyCodes to find if this keyCode is already used
	// elsewhere
	aIt->toFirst();
	while ( aIt->current() ) {
		if ( aIt->current() != pEntry
				&& aIt->current()->aConfigKeyCode == pEntry->aConfigKeyCode ) {
			QString actionName( aIt->currentKey() );
			actionName.stripWhiteSpace();

			QString keyName = KAccel::keyToString( aIt->current()->aConfigKeyCode );
			
			QString str = 
			    i18n("The %1 key combination has already "
				 "been allocated\n"
				 "to the %2 action.\n"
				 "\n"
				 "Please choose a unique key combination.").
			    arg(keyName).arg(actionName);
				
			KMessageBox::sorry( this, str, i18n("Key conflict"));
			
			return TRUE;
		}
		++(*aIt);
	}
	
	//	emit keyChange();
	
	return FALSE;
}
#include "kkeydialog.moc"

