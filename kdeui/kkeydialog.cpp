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
#include <qwhatsthis.h>
#include <klistview.h>

#include <kckey.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kglobalaccel.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlgui.h>
#include <qdom.h>
#include <kdebug.h>
#include "kkeydialog.h"

#include <kaction.h>
#include "kkeybutton.h"

class KKeyChooserPrivate {
public:
    QDict<int> *globalDict;
    QDict<int> *stdDict;
    KListView *wList;
    QLabel *lInfo;
    QLabel *lNotConfig;
    KKeyButton *bChange;
    QCheckBox *cShift;
    QCheckBox *cCtrl;
    QCheckBox *cAlt;
    QGroupBox *fCArea;
    QButtonGroup *kbGroup;
    KKeyEntryMap *map;

    QMap<QListViewItem*, KKeyEntryMap::Iterator> actionMap;
    bool bKeyIntercept;

    int kbMode;
};

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
  editing = false;
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

  painter->setClipping( false );
  if( width() > 12 && height() > 8 )
    qDrawShadePanel( painter, 6, 4, width() - 12, height() - 8,
		     colorGroup(), true, 1, 0L );
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
KKeyDialog::KKeyDialog( KKeyEntryMap *aKeyMap, QWidget *parent,
			bool check_against_std_keys)
  : KDialogBase( parent, 0, true, i18n("Configure Key Bindings"),
		 Help|Default|Ok|Cancel, Ok )
{
  KKeyChooser *kc =  new KKeyChooser( aKeyMap, this, check_against_std_keys );
  setMainWidget(kc);
  connect( this, SIGNAL(defaultClicked()), kc, SLOT(allDefault()) );
  enableButton ( Help, false );
}

KKeyDialog::~KKeyDialog()
{
}

int KKeyDialog::configureKeys( KAccel *keys, bool save_settings,
			       QWidget *parent )
{
    KKeyEntryMap map = keys->keyDict();
    KKeyDialog kd( &map, parent );
    int retcode = kd.exec();

    if( retcode == Accepted ) {
        keys->setKeyDict( map );
        if (save_settings)
            keys->writeSettings();
    }
    return retcode;
}

int KKeyDialog::configureKeys( KGlobalAccel *keys, bool save_settings,
			       QWidget *parent )
{
    KKeyEntryMap dict = keys->keyDict();

    KKeyDialog kd( &dict, parent );
    int retcode = kd.exec();

    if( retcode == Accepted ) {
        keys->setKeyDict( dict );
        if (save_settings)
            keys->writeSettings();
    }
    return retcode;
}

int KKeyDialog::configureKeys( KActionCollection *coll, const QString& file,
                               bool save_settings, QWidget *parent )
{
    KKeyEntryMap map = coll->keyMap();

    KKeyDialog kd( &map, parent );
    int retcode = kd.exec();

    if( retcode != Accepted )
        return retcode;

    if (!save_settings) {
        coll->setKeyMap( map );
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
        if ( it.tagName() == tagActionProp ) {
            elem = it;
            break;
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
    KKeyEntry key = map[action->name()];
    if (key.aCurrentKeyCode == key.aConfigKeyCode)
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
                           KAccel::keyToString( key.aConfigKeyCode ) );

    elem.appendChild( act_elem );
  }

  // finally, write out the result
  KXMLGUIFactory::saveConfigFile(doc, file);

  coll->setKeyMap( map );

  return retcode;
}

KKeyChooser::KKeyChooser( KKeyEntryMap *aKeyMap, QWidget *parent,
			  bool check_against_std_keys)
    : QWidget( parent )
{

  d = new KKeyChooserPrivate();

  d->bKeyIntercept = false;
  d->kbMode = NoKey;
  d->map = aKeyMap;

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
  d->wList = new KListView( this );
  d->wList->setFocus();

  stackLayout->addMultiCellWidget( d->wList, 1, 1, 0, 1 );
  QString wtstr = i18n("Here you can see a list of key bindings, \n"
                       "i.e. associations between actions (e.g. 'Copy')\n"
                       "shown in the left column and keys or combination\n"
                       "of keys (e.g. CTRL-V) shown in the right column.");

  QWhatsThis::add( d->wList, wtstr );

  d->wList->addColumn(i18n("Action"));
  d->wList->addColumn(i18n("Current key"));

  //
  // Add all "keys" to the list
  //
  for (KKeyEntryMap::Iterator it = aKeyMap->begin();
       it != aKeyMap->end(); ++it)
  {
    (*it).aConfigKeyCode = (*it).aCurrentKeyCode;

    QListViewItem * s = new QListViewItem(d->wList,
                                          (*it).descr,
                                          KAccel::keyToString((*it).aConfigKeyCode));
    d->actionMap[s] = it;
  }

  connect( d->wList, SIGNAL( currentChanged( QListViewItem * ) ),
           SLOT( updateAction( QListViewItem * ) ) );

  //
  // CREATE CHOOSE KEY GROUP
  //
  d->fCArea = new QGroupBox( this );
  topLayout->addWidget( d->fCArea, 1 );

  d->fCArea->setTitle( i18n("Choose a key for the selected action") );
  d->fCArea->setFrameStyle( QFrame::Box | QFrame::Sunken );

  //
  // CHOOSE KEY GROUP LAYOUT MANAGER
  //
  QGridLayout *grid = new QGridLayout( d->fCArea, 6, 4, KDialog::spacingHint() );
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


  d->kbGroup = new QButtonGroup( d->fCArea );
  d->kbGroup->hide();
  d->kbGroup->setExclusive( true );

  QRadioButton *rb = new QRadioButton( i18n("&No key"), d->fCArea );
  d->kbGroup->insert( rb, NoKey );
  grid->addMultiCellWidget( rb, 1, 1, 1, 2 );
  QWhatsThis::add( rb, i18n("The selected action will not be associated with any key.") );

  rb = new QRadioButton( i18n("&Default key"), d->fCArea );
  d->kbGroup->insert( rb, DefaultKey );
  grid->addMultiCellWidget( rb, 2, 2, 1, 2 );
  QWhatsThis::add( rb, i18n("This will bind the default key to the selected action. Usually a reasonable choice.") );

  rb = new QRadioButton( i18n("&Custom key"), d->fCArea );
  d->kbGroup->insert( rb, CustomKey );
  connect( d->kbGroup, SIGNAL( clicked( int ) ), SLOT( keyMode( int ) ) );
  grid->addMultiCellWidget( rb, 3, 3, 1, 2 );
  QWhatsThis::add( rb, i18n("If this option is selected you can create a customized key binding for the"
    " selected action using the buttons below.") );

  QBoxLayout *pushLayout = new QHBoxLayout( KDialog::spacingHint() );
  grid->addLayout( pushLayout, 4, 2 );

  d->cShift = new QCheckBox( d->fCArea );
  d->cShift->setText( QString::fromLatin1("SHIFT") );
  d->cShift->setEnabled( false );
  connect( d->cShift, SIGNAL( clicked() ), SLOT( shiftClicked() ) );

  d->cCtrl = new QCheckBox( d->fCArea );
  d->cCtrl->setText( QString::fromLatin1("CTRL") );
  d->cCtrl->setEnabled( false );
  connect( d->cCtrl, SIGNAL( clicked() ), SLOT( ctrlClicked() ) );

  d->cAlt = new QCheckBox( d->fCArea );
  d->cAlt->setText( QString::fromLatin1("ALT") );
  d->cAlt->setEnabled( false );
  connect( d->cAlt, SIGNAL( clicked() ), SLOT( altClicked() ) );

  d->bChange = new KKeyButton(d->fCArea, "key");
  d->bChange->setEnabled( false );
  connect( d->bChange, SIGNAL( clicked() ), SLOT( changeKey() ) );

  wtstr = i18n("If 'Custom key' is selected, you can set a combination of keys here. Click on"
    " the rightmost button once and then press a key to select it for this binding. You can"
    " check the SHIFT, CTRL and ALT boxes to combine the selected key with modifier keys.");
  QWhatsThis::add( d->cShift, wtstr );
  QWhatsThis::add( d->cCtrl, wtstr );
  QWhatsThis::add( d->cAlt, wtstr );
  QWhatsThis::add( d->bChange, wtstr );

  //
  // Add widgets to the geometry manager
  //
  pushLayout->addWidget( d->cShift );
  pushLayout->addWidget( d->cCtrl );
  pushLayout->addWidget( d->cAlt );
  pushLayout->addSpacing( KDialog::spacingHint()*2 );
  pushLayout->addWidget( d->bChange );
  pushLayout->addStretch( 10 );


  d->lNotConfig = new QLabel(d->fCArea);
  d->lNotConfig->resize(0,0);
  QFont f = KGlobalSettings::generalFont();
  f.setPointSize(f.pointSize()+2);
  f.setBold(true);
  d->lNotConfig->setFont( f );
  d->lNotConfig->setAlignment( AlignCenter );
  d->lNotConfig->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  if ( d->wList->childCount()==0 )
    d->lNotConfig->setText( i18n("No keys defined") );
  else
  {
    d->lNotConfig->setText( i18n("Not configurable") );
    d->lNotConfig->hide();
  }
  d->lNotConfig->hide();

  d->lInfo = new QLabel(d->fCArea);
  resize(0,0);
  d->lInfo->setAlignment( AlignCenter );
  d->lInfo->setEnabled( false );
  d->lInfo->hide();
	
  d->globalDict = new QDict<int> ( 100, false );
  d->globalDict->setAutoDelete( true );
  readGlobalKeys();
  d->stdDict = new QDict<int> ( 100, false );
  d->stdDict->setAutoDelete( true );
  if (check_against_std_keys)
    readStdKeys();
}



KKeyChooser::~KKeyChooser()
{
  delete d->globalDict;
  delete d->stdDict;
  delete d->wList;
  delete d;
}

void KKeyChooser::updateAction( QListViewItem *item )
{
    KKeyEntryMap::Iterator it = d->actionMap[item];

    if ( (*it).aConfigKeyCode == 0 )
        d->kbMode = NoKey;
    else if ( (*it).aConfigKeyCode == (*it).aDefaultKeyCode )
        d->kbMode = DefaultKey;
    else d->kbMode = CustomKey;

    toChange( item );
}

void KKeyChooser::readGlobalKeys()
{
  //debug("KKeyChooser::readGlobalKeys()");

  d->globalDict->clear();


  // Insert all global keys into d->globalDict
  int *keyCode;
  KConfig pConfig;
  QMap<QString, QString> tmpMap = pConfig.entryMap(i18n("Global Keys"));
  QMap<QString, QString>::Iterator gIt(tmpMap.begin());
  for (; gIt != tmpMap.end(); ++gIt) {
    keyCode = new int;
    *keyCode = KAccel::stringToKey( *gIt );
    d->globalDict->insert( gIt.key(), keyCode);
  }

  // Only insert global keys which don't appear in the dictionary to be configured
  for (KKeyEntryMap::ConstIterator it = d->map->begin();
       it != d->map->end(); ++it)
    if ( d->globalDict->find( it.key() ))
      d->globalDict->remove( it.key() );
}

void KKeyChooser::readStdKeys()
{
  // debug("KKeyChooser::readStdKeys()");

  d->stdDict->clear();


  // Insert all standard keys into d->globalDict
  int *keyCode;
  KConfig pConfig;
  QMap<QString, QString> tmpMap = pConfig.entryMap(i18n("Keys"));
  QMap<QString, QString>::Iterator sIt(tmpMap.begin());
  for (; sIt != tmpMap.end(); ++sIt) {
    keyCode = new int;
    *keyCode = KAccel::stringToKey( *sIt );
    d->stdDict->insert( sIt.key(), keyCode);
  }

  // Only insert std keys which don't appear in the dictionary to be configured
  for (KKeyEntryMap::ConstIterator it = d->map->begin();
       it != d->map->end(); ++it)
    if ( d->stdDict->find( it.key() ) )
      d->stdDict->remove( it.key() );
}

void KKeyChooser::toChange( QListViewItem *item )
{
	d->bKeyIntercept = false;
	
	/* get the entry */
        KKeyEntryMap::Iterator it = d->actionMap[item];

	/* Is the key configurable or has the user turned it off ? */
	if ( !(*it).bConfigurable || d->kbMode == NoKey ) {
		d->lInfo->setEnabled( false );
		d->cShift->setEnabled( false );
                d->cCtrl->setEnabled( false );
                d->cAlt->setEnabled( false );
		d->bChange->setEnabled( false );  //bDefault->setEnabled( false );
		d->lNotConfig->setEnabled( true );
		
		int kCode = (*it).aConfigKeyCode;
		int kSCode = kCode & ~(SHIFT | CTRL | ALT);
		
		if ( kSCode == Key_Shift ) d->cShift->setChecked(false);
		else d->cShift->setChecked( kCode & SHIFT );
		if ( kSCode == Key_Control ) d->cCtrl->setChecked(false);
		else d->cCtrl->setChecked( kCode & CTRL );
		if ( kSCode == Key_Alt ) d->cAlt->setChecked(false);
		else d->cAlt->setChecked( kCode & ALT );
		
		QString str = KAccel::keyToString( kSCode );
		d->bChange->setText(str);
		
	} else {
		d->lNotConfig->setEnabled( false );
		d->lInfo->setText(QString::null); d->lInfo->setEnabled( true );
		
		int kCode = (*it).aConfigKeyCode;
		int kSCode = kCode & ~(SHIFT | CTRL | ALT);
		
		//d->cShift->setEnabled( true ); d->cCtrl->setEnabled( true ); d->cAlt->setEnabled( true );
		if ( kSCode == Key_Shift ) d->cShift->setChecked(false);
		else d->cShift->setChecked( kCode & SHIFT );
		if ( kSCode == Key_Control ) d->cCtrl->setChecked(false);
		else d->cCtrl->setChecked( kCode & CTRL );
		if ( kSCode == Key_Alt ) d->cAlt->setChecked(false);
		else d->cAlt->setChecked( kCode & ALT );
		
		QString str = KAccel::keyToString( kSCode );
		d->bChange->setText(str); //eKey->setText(str);
		//d->bChange->setEnabled( true ); //bDefault->setEnabled( true );
		
		if ( isKeyPresent(kSCode) ) {
			d->lInfo->setText(i18n("Attention : key already used") );
		}
		
		if ( d->kbMode == DefaultKey ) {
			d->cAlt->setEnabled( false );
			d->cShift->setEnabled( false );
			d->cCtrl->setEnabled( false );
			d->bChange->setEnabled( false );
		} else {
			d->cAlt->setEnabled( true );
			d->cShift->setEnabled( true );
			d->cCtrl->setEnabled( true );
			d->bChange->setEnabled( true );
		}
	}
		
	((QRadioButton *)d->kbGroup->find(NoKey))->setChecked( d->kbMode == NoKey );
	((QRadioButton *)d->kbGroup->find(DefaultKey))->setChecked( d->kbMode == DefaultKey );
	((QRadioButton *)d->kbGroup->find(CustomKey))->setChecked( d->kbMode == CustomKey );
	
	if ( !(*it).bConfigurable ) {
		d->cAlt->setEnabled( false );
		d->cShift->setEnabled( false );
		d->cCtrl->setEnabled( false );
		d->bChange->setEnabled( false );
		((QRadioButton *)d->kbGroup->find(NoKey))->setEnabled( false );
		((QRadioButton *)d->kbGroup->find(DefaultKey))->setEnabled( false );
		((QRadioButton *)d->kbGroup->find(CustomKey))->setEnabled( false );
	} else {
		((QRadioButton *)d->kbGroup->find(NoKey))->setEnabled( true );
		((QRadioButton *)d->kbGroup->find(DefaultKey))->setEnabled( (*it).aDefaultKeyCode != 0);
		((QRadioButton *)d->kbGroup->find(CustomKey))->setEnabled( true );
	}	
}

void KKeyChooser::fontChange( const QFont & )
{

	d->cAlt->setFixedHeight( d->bChange->sizeHint().height() );
	d->cShift->setFixedHeight( d->bChange->sizeHint().height() );
	d->cCtrl->setFixedHeight( d->bChange->sizeHint().height() );
	
	d->fCArea->setMinimumHeight( 4*d->bChange->sizeHint().height() );
	
	int widget_width = 0;
	
	setMinimumWidth( 20+5*(widget_width+10) );
}

void KKeyChooser::keyMode( int m )
{
	d->kbMode = m;
	switch( d->kbMode ) {
		case NoKey:
			noKey();
			break;
		case DefaultKey:
			defaultKey();
			break;
		case CustomKey: default:
			toChange( d->wList->currentItem() );
			break;
	}
}

void KKeyChooser::noKey()
{
    (*d->actionMap[d->wList->currentItem()]).aConfigKeyCode = 0;
	
    /* update the list and the change area */

    if ( d->bChange->isEditing() )
        d->bChange->setEditing(false);

    toChange(d->wList->currentItem());
    emit keyChange();
}

void KKeyChooser::defaultKey()
{
    /* update the list and the change area */

    QListViewItem *item = d->wList->currentItem();
    if (!item)
        return;

    (*d->actionMap[item]).aConfigKeyCode =
        (*d->actionMap[item]).aDefaultKeyCode;

    item->setText(1, KAccel::keyToString((*d->actionMap[item]).
                                         aConfigKeyCode));
		
    if ( d->bChange->isEditing() )
        d->bChange->setEditing(false);

    toChange(d->wList->currentItem());
    emit keyChange();
}


void KKeyChooser::allDefault()
{
    // Change all configKeyCodes to default values
	
    QListViewItem *at = d->wList->firstChild();
    while (at) {

        KKeyEntryMap::Iterator it = d->actionMap[at];

        if ( (*it).bConfigurable ) {
            (*it).aConfigKeyCode = (*it).aDefaultKeyCode;
            at->setText(1, KAccel::keyToString((*it).aConfigKeyCode));
        }

        at = at->nextSibling();
    }
    emit keyChange();
}

void KKeyChooser::shiftClicked()
{
    QListViewItem *item = d->wList->currentItem();
    if (!item)
        return;

    KKeyEntryMap::Iterator it = d->actionMap[item];

    int kSCode = (*it).aConfigKeyCode & ~(SHIFT | CTRL | ALT);
    if ( kSCode != Key_Shift ) {
        if ( d->cShift->isOn() )
            (*it).aConfigKeyCode |= SHIFT;
        else
            (*it).aConfigKeyCode &= ~SHIFT;

        item->setText(1,
                      KAccel::keyToString((*it).aConfigKeyCode));

    }
    toChange(item);
    emit keyChange();
}


void KKeyChooser::listSync()
{
    QListView *item = d->wList->firstItem();

    while (item) {
        KKeyEntryMap::Iterator it = d->actionMap[item];

        if ( (*it).bConfigurable )
            item->setText(1, KAccel::keyToString((*it).aConfigKeyCode));

        item = item->nextSibling();
    }

}

void KKeyChooser::ctrlClicked()
{
    QListViewItem *item = d->wList->currentItem();
    if (!item)
        return;

    KKeyEntryMap::Iterator it = d->actionMap[item];

    int kSCode = (*it).aConfigKeyCode & ~(SHIFT | CTRL | ALT);

    if ( kSCode != Key_Control ) {
        if ( d->cCtrl->isOn() )
            (*it).aConfigKeyCode |= CTRL;
        else
            (*it).aConfigKeyCode &= ~CTRL;

        item->setText(1, KAccel::keyToString((*it).aConfigKeyCode));
    }
    toChange(item);
    emit keyChange();
}

void KKeyChooser::altClicked()
{
    QListViewItem *item = d->wList->currentItem();
    if (!item)
        return;

    KKeyEntryMap::Iterator it = d->actionMap[item];

    int kSCode = (*it).aConfigKeyCode & ~(SHIFT | CTRL | ALT);
    if ( kSCode != Key_Alt ) {
        if ( d->cAlt->isOn() )
            (*it).aConfigKeyCode |= ALT;
        else
            (*it).aConfigKeyCode &= ~ALT;

        item->setText(1, KAccel::keyToString((*it).aConfigKeyCode));
    }
    toChange(d->wList->currentItem());
    emit keyChange();
}

void KKeyChooser::changeKey()
{
    d->bChange->setEditing(true);
    d->lInfo->setText( i18n("Press the wanted key") );
    d->lInfo->setEnabled( true );
    /* give the focus to the widget */

    //eKey->setGeometry(d->bChange->x()+6, d->bChange->y()+4, d->bChange->width()-12,
    //d->bChange->height()-8);
    //eKey->show();
    //eKey->setEnabled(true);
    setFocus();
    d->bKeyIntercept = true;
}

void KKeyChooser::keyPressEvent( QKeyEvent *e )
{
  /* the keys are processed if the change button was pressed */
  if( !d->bKeyIntercept )
  {
    e->ignore();
    return;
  }
	
  int kCode = e->key() & ~(SHIFT | CTRL | ALT);
  /* check the given key :
     if it is a non existent key (=0) : keep the old value and say
     what happened.
  */
  if ( KAccel::keyToString(kCode).isNull() )
  {
    d->lInfo->setText( i18n("Undefined key") );
    return;
  }
	
  d->bKeyIntercept = false;
  //eKey->hide();
  //eKey->setEnabled(false);
  d->bChange->setEditing(true);
  d->bChange->setFocus();
  setKey(kCode);
}

void KKeyChooser::setKey( int kCode)
{
    QListViewItem *item = d->wList->currentItem();
    if (!item)
        return;

    KKeyEntryMap::Iterator it = d->actionMap[item];

    /* add the current modifier to the key */
    if ( kCode!=Key_Shift ) kCode |= ((*it).aConfigKeyCode & SHIFT);
    if ( kCode!=Key_Control ) kCode |= ((*it).aConfigKeyCode & CTRL);
    if ( kCode!=Key_Alt ) kCode |= ((*it).aConfigKeyCode & ALT);

    if ( isKeyPresent(kCode) ) {
        d->lInfo->setText( i18n("Attention : key already used") );
        return;
    }

    /* set the list and the change button */
    (*it).aConfigKeyCode = kCode;
    item->setText(1, KAccel::keyToString((*it).aConfigKeyCode));
	
    toChange(item);
    emit keyChange();
}

void KKeyChooser::editKey()
{
    d->bChange->setEnabled( false ); //eKey->setEnabled( true );
    d->lInfo->setText( i18n("Press 'Return' to quit editing") );
}

void KKeyChooser::editEnd()
{
    kdDebug(125) << "Called editEnd() which relies on eKey widget" << endl;

    //int kCode = KAccel::stringToKey(eKey->text());
    int kCode = 0;
    if ( kCode==0 || (kCode & (SHIFT | CTRL | ALT)) ) {
        d->lInfo->setText( i18n("Incorrect key") );
        return;
    }
    setKey(kCode);
}

bool KKeyChooser::isKeyPresent( int kcode)
{
    if (!kcode)
        return false;

    // Search the global key codes to find if this keyCode is already used
    //  elsewhere

    QDictIterator<int> gIt( *d->globalDict );

    gIt.toFirst();
    while ( gIt.current() ) {
        kdDebug(125) << "current " << gIt.currentKey() << ":" << *gIt.current() << " code " << kcode << endl;
        if ( (*gIt.current()) == kcode && *gIt.current() != 0 ) {
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

            return true;
        }
        ++gIt;
    }

    // Search the std key codes to find if this keyCode is already used
    //  elsewhere

    QDictIterator<int> sIt( *d->stdDict );

    sIt.toFirst();
    while ( sIt.current() ) {
        kdDebug(125) << "current " << sIt.currentKey() << ":" << *sIt.current() << " code " << kcode << endl;
        if ( (*sIt.current()) == kcode && *sIt.current() != 0 ) {
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

            return true;
        }
        ++sIt;
    }

    // Search the aConfigKeyCodes to find if this keyCode is already used
    // elsewhere
    for (KKeyEntryMap::ConstIterator it = d->map->begin();
         it != d->map->end(); ++it) {
        if ( it != d->actionMap[d->wList->currentItem()]
             && (*it).aConfigKeyCode == kcode ) {
            QString actionName( it.key() );
            actionName.stripWhiteSpace();

            QString keyName = KAccel::keyToString( kcode );

            QString str =
                i18n("The %1 key combination has already "
                     "been allocated\n"
                     "to the %2 action.\n"
                     "\n"
                     "Please choose a unique key combination.").
                arg(keyName).arg(actionName);

            KMessageBox::sorry( this, str, i18n("Key conflict"));

            return true;
        }
    }

    //	emit keyChange();

    return false;
}
#include "kkeydialog.moc"
#include "kkeybutton.moc"
