/*
 * This file is part of the KDE Libraries
 * Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

// I (espen) prefer that header files are included alphabetically
#include <qhbox.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qwidget.h>

#include <kaboutdata.h>
#include <kaboutkde.h>
#include <kaction.h>
#include <kapp.h>
#include <kbugreport.h>
#include <kdialogbase.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaccel.h>
#include <kstdaction.h>

class KHelpMenuPrivate
{
public:
    KHelpMenuPrivate()
    {
    }
    ~KHelpMenuPrivate()
    {
    }

    const KAboutData *mAboutData;
};

KHelpMenu::KHelpMenu( QWidget *parent, const QString &aboutAppText,
		      bool showWhatsThis )
  : QObject(parent), mMenu(0), mAboutApp(0), mAboutKDE(0), mBugReport(0),
    d(new KHelpMenuPrivate)
{
  mParent = parent;
  mAboutAppText = aboutAppText;
  mShowWhatsThis = showWhatsThis;
  d->mAboutData = 0;
}

KHelpMenu::KHelpMenu( QWidget *parent, const KAboutData *aboutData,
		      bool showWhatsThis, KActionCollection *actions )
  : QObject(parent), mMenu(0), mAboutApp(0), mAboutKDE(0), mBugReport(0),
    d(new KHelpMenuPrivate)
{
  mParent = parent;
  mShowWhatsThis = showWhatsThis;

  d->mAboutData = aboutData;
  if (aboutData)
  {
    mAboutAppText = aboutData->programName() + " " + aboutData->version() +
                    "\n" + aboutData->shortDescription();

    if (!aboutData->homepage().isNull())
      mAboutAppText += "\n" + aboutData->homepage();

    QValueList<KAboutPerson>::ConstIterator it;
    for (it = aboutData->authors().begin();
         it != aboutData->authors().end(); ++it)
    {
      mAboutAppText += "\n" + (*it).name() + " <"+(*it).emailAddress()+">";
    }

    if (!aboutData->copyrightStatement().isNull())
      mAboutAppText += "\n" + aboutData->copyrightStatement();
    mAboutAppText += "\n";
  }
  else
    mAboutAppText = QString::null;

    if (actions)
    {
        KStdAction::helpContents(this, SLOT(appHelpActivated()), actions);
        if (showWhatsThis)
            KStdAction::whatsThis(this, SLOT(contextHelpActivated()), actions);
        KStdAction::reportBug(this, SLOT(reportBug()), actions);
        KStdAction::aboutApp(this, SLOT(aboutApplication()), actions);
        KStdAction::aboutKDE(this, SLOT(aboutKDE()), actions);
    }
}

KHelpMenu::~KHelpMenu()
{
  delete mMenu;
  delete mAboutApp;
  delete mAboutKDE;
  delete mBugReport;
  delete d;
}


QPopupMenu* KHelpMenu::menu()
{
  if( mMenu == 0 )
  {
    //
    // 1999-12-02 Espen Sand:
    // I use hardcoded menu id's here. Reason is to stay backward
    // compatible.
    //
    mMenu = new QPopupMenu();
    connect( mMenu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));

    mMenu->insertItem( BarIcon( "contents", KIcon::SizeSmall),
		       i18n( "&Contents" ),menuHelpContents );
    mMenu->connectItem( menuHelpContents, this, SLOT(appHelpActivated()) );
    mMenu->setAccel( KStdAccel::key(KStdAccel::Help), menuHelpContents );

    if( mShowWhatsThis == true )
    {
      QToolButton* wtb = QWhatsThis::whatsThisButton(0);
      mMenu->insertItem( wtb->iconSet(),i18n( "What's &This" ), menuWhatsThis);
      mMenu->connectItem( menuWhatsThis, this, SLOT(contextHelpActivated()) );
      delete wtb;
      mMenu->setAccel( SHIFT + Key_F1, menuWhatsThis );
    }

    mMenu->insertSeparator();

    mMenu->insertItem( i18n( "&Report Bug..." ), menuReportBug );
    mMenu->connectItem( menuReportBug, this, SLOT(reportBug()) );

    mMenu->insertSeparator();

    mMenu->insertItem( kapp->miniIcon(),
      i18n( "&About" ) + ' ' + QString::fromLatin1(kapp->name()) + 
      QString::fromLatin1("..."), menuAboutApp );
    mMenu->connectItem( menuAboutApp, this, SLOT( aboutApplication() ) );

    mMenu->insertItem( i18n( "About &KDE..." ), menuAboutKDE );
    mMenu->connectItem( menuAboutKDE, this, SLOT( aboutKDE() ) );
  }

  return( mMenu );
}



void KHelpMenu::appHelpActivated()
{
  kapp->invokeHTMLHelp( QString::fromLatin1(kapp->name()) + 
			QString::fromLatin1("/index.html"), 
			QString::fromLatin1("") );
}


void KHelpMenu::aboutApplication()
{
  if (d->mAboutData)
  {
    if( mAboutApp == 0 )
    {
      KAboutDialog *dlg = new KAboutDialog( KAboutDialog::AbtAppStandard,
		kapp->caption(), KDialogBase::Ok, KDialogBase::Ok,
		mParent, "about", false, true);
      mAboutApp = dlg;
      
//      dlg->setTitle( aboutData->programName() + " " + aboutData->version());
      dlg->setProduct( d->mAboutData->programName(), d->mAboutData->version(),
		       QString::null, QString::null);

      QString appPageText = d->mAboutData->shortDescription() + "\n";

      if (!d->mAboutData->homepage().isEmpty())
         appPageText += "\n" + d->mAboutData->homepage()+"\n";

      if (!d->mAboutData->otherText().isEmpty())
         appPageText += "\n" + d->mAboutData->otherText()+"\n";

      if (!d->mAboutData->copyrightStatement().isEmpty())
         appPageText += "\n" + d->mAboutData->copyrightStatement()+"\n";

      KAboutContainer *appPage = dlg->addContainerPage( i18n("&About"));
 
      QLabel *appPageLabel = new QLabel( appPageText, 0 );
      appPage->addWidget( appPageLabel );

      int authorCount = d->mAboutData->authors().count();
      if (authorCount)
      {
         QString authorPageTitle;
         if (authorCount == 1)
            authorPageTitle = i18n("A&uthor");
         else
            authorPageTitle = i18n("A&uthors");
         KAboutContainer *authorPage = dlg->addContainerPage( authorPageTitle );
         QValueList<KAboutPerson>::ConstIterator it;
         for (it = d->mAboutData->authors().begin();
              it != d->mAboutData->authors().end(); ++it)
         {
            authorPage->addPerson( (*it).name(), (*it).emailAddress(),
                                   (*it).webAddress(), (*it).task() );
         }
      }

      int creditsCount = d->mAboutData->credits().count();
      if (creditsCount)
      {
         KAboutContainer *creditsPage = dlg->addContainerPage( i18n("&Thanks to") );
         QValueList<KAboutPerson>::ConstIterator it;
         for (it = d->mAboutData->credits().begin();
              it != d->mAboutData->credits().end(); ++it)
         {
            creditsPage->addPerson( (*it).name(), (*it).emailAddress(),
                                   (*it).webAddress(), (*it).task() );
         }
      }

      if (!d->mAboutData->license().isEmpty())
      {
	 dlg->addTextPage( i18n("&License"), d->mAboutData->license());
      }
 
    }

    mAboutApp->show();
  }
  else if( mAboutAppText.isEmpty() )
  {
    emit showAboutApplication();
  }
  else
  {
    if( mAboutApp == 0 )
    {
      mAboutApp = new KDialogBase( QString::null, // Caption is defined below
				   KDialogBase::Yes, KDialogBase::Yes,
				   KDialogBase::Yes, mParent, "about",
				   false, true, i18n("&OK") );
      connect( mAboutApp, SIGNAL(hidden()), this, SLOT( dialogHidden()) );
      
      QHBox *hbox = new QHBox( mAboutApp );
      mAboutApp->setMainWidget( hbox );
      hbox->setSpacing(KDialog::spacingHint()*3);
      hbox->setMargin(KDialog::marginHint()*1);

      QLabel *label1 = new QLabel(hbox);
      label1->setPixmap( kapp->icon() );
      QLabel *label2 = new QLabel(hbox);
      label2->setText( mAboutAppText );

      mAboutApp->setPlainCaption( i18n("About %1").arg(kapp->caption()) );
      mAboutApp->disableResize();
    }

    mAboutApp->show();
  }
}


void KHelpMenu::aboutKDE()
{
  if( mAboutKDE == 0 )
  {
    mAboutKDE = new KAboutKDE( mParent, "aboutkde", false );
    connect( mAboutKDE, SIGNAL(hidden()), this, SLOT( dialogHidden()) );
  }    
  mAboutKDE->show();
}


void KHelpMenu::reportBug()
{
  if( mBugReport == 0 )
  {
    mBugReport = new KBugReport( mParent, false );
    connect( mBugReport, SIGNAL(hidden()),this,SLOT( dialogHidden()) );
  }
  mBugReport->show();
}


void KHelpMenu::dialogHidden()
{
  QTimer::singleShot( 0, this, SLOT(timerExpired()) );
}


void KHelpMenu::timerExpired()
{
  if( mAboutKDE != 0 && mAboutKDE->isVisible() == false )
  {
    delete mAboutKDE; mAboutKDE = 0;
  }

  if( mBugReport != 0 && mBugReport->isVisible() == false )
  {
    delete mBugReport; mBugReport = 0;
  }

  if( mAboutApp != 0 && mAboutApp->isVisible() == false )
  {
    delete mAboutApp; mAboutApp = 0;
  }
}


void KHelpMenu::menuDestroyed()
{
  mMenu = 0;
}


void KHelpMenu::contextHelpActivated()
{
  QWhatsThis::enterWhatsThisMode();
}


#include "khelpmenu.moc"
