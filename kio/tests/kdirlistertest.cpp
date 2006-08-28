/* This file is part of the KDE desktop environment

   Copyright (C) 2001, 2002 Michael Brade <brade@kde.org>

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

#include <qlayout.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <kdirlister.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include "kdirlistertest.h"

#include <cstdlib>


KDirListerTest::KDirListerTest( QWidget *parent )
  : QWidget( parent )
{
  lister = new KDirLister( false /* true */ );
  debug = new PrintSignals;

  QVBoxLayout* layout = new QVBoxLayout( this );

  QPushButton* startH = new QPushButton( "Start listing $HOME", this );
  QPushButton* startR= new QPushButton( "Start listing /", this );
  QPushButton* test = new QPushButton( "Many", this );
  QPushButton* startT = new QPushButton( "tarfile", this );

  layout->addWidget( startH );
  layout->addWidget( startR );
  layout->addWidget( startT );
  layout->addWidget( test );
  resize( layout->sizeHint() );

  connect( startR, SIGNAL( clicked() ), SLOT( startRoot() ) );
  connect( startH, SIGNAL( clicked() ), SLOT( startHome() ) );
  connect( startT, SIGNAL( clicked() ), SLOT( startTar() ) );
  connect( test, SIGNAL( clicked() ), SLOT( test() ) );

  connect( lister, SIGNAL( started( const KUrl & ) ),
           debug,  SLOT( started( const KUrl & ) ) );
  connect( lister, SIGNAL( completed() ),
           debug,  SLOT( completed() ) );
  connect( lister, SIGNAL( completed( const KUrl & ) ),
           debug,  SLOT( completed( const KUrl & ) ) );
  connect( lister, SIGNAL( canceled() ),
           debug,  SLOT( canceled() ) );
  connect( lister, SIGNAL( canceled( const KUrl & ) ),
           debug,  SLOT( canceled( const KUrl & ) ) );
  connect( lister, SIGNAL( redirection( const KUrl & ) ),
           debug,  SLOT( redirection( const KUrl & ) ) );
  connect( lister, SIGNAL( redirection( const KUrl &, const KUrl & ) ),
           debug,  SLOT( redirection( const KUrl &, const KUrl & ) ) );
  connect( lister, SIGNAL( clear() ),
           debug,  SLOT( clear() ) );
  connect( lister, SIGNAL( newItems( const KFileItemList & ) ),
           debug,  SLOT( newItems( const KFileItemList & ) ) );
  connect( lister, SIGNAL( itemsFilteredByMime( const KFileItemList & ) ),
           debug,  SLOT( itemsFilteredByMime( const KFileItemList & ) ) );
  connect( lister, SIGNAL( deleteItem( KFileItem * ) ),
           debug,  SLOT( deleteItem( KFileItem * ) ) );
  connect( lister, SIGNAL( refreshItems( const KFileItemList & ) ),
           debug,  SLOT( refreshItems( const KFileItemList & ) ) );
  connect( lister, SIGNAL( infoMessage( const QString& ) ),
           debug,  SLOT( infoMessage( const QString& ) ) );
  connect( lister, SIGNAL( percent( int ) ),
           debug,  SLOT( percent( int ) ) );
  connect( lister, SIGNAL( totalSize( KIO::filesize_t ) ),
           debug,  SLOT( totalSize( KIO::filesize_t ) ) );
  connect( lister, SIGNAL( processedSize( KIO::filesize_t ) ),
           debug,  SLOT( processedSize( KIO::filesize_t ) ) );
  connect( lister, SIGNAL( speed( int ) ),
           debug,  SLOT( speed( int ) ) );

  connect( lister, SIGNAL( completed() ),
           this,  SLOT( completed() ) );
}

KDirListerTest::~KDirListerTest()
{
  delete lister;
}

void KDirListerTest::startHome()
{
  KUrl home( getenv( "HOME" ) );
  lister->openUrl( home, false, false );
//  lister->stop();
}

void KDirListerTest::startRoot()
{
  KUrl root( "file:/" );
  lister->openUrl( root, true, true );
// lister->stop( root );
}

void KDirListerTest::startTar()
{
  KUrl root( "file:/home/jowenn/aclocal_1.tgz" );
  lister->openUrl( root, true, true );
// lister->stop( root );
}

void KDirListerTest::test()
{
  KUrl home( getenv( "HOME" ) );
  KUrl root( "file:/" );
/*  lister->openURL( home, true, false );
  lister->openURL( root, true, true );
  lister->openURL( KUrl("file:/etc"), true, true );
  lister->openURL( root, true, true );
  lister->openURL( KUrl("file:/dev"), true, true );
  lister->openURL( KUrl("file:/tmp"), true, true );
  lister->openURL( KUrl("file:/usr/include"), true, true );
  lister->updateDirectory( KUrl("file:/usr/include") );
  lister->updateDirectory( KUrl("file:/usr/include") );
  lister->openURL( KUrl("file:/usr/"), true, true );
*/
  lister->openUrl( KUrl("file:/dev"), true, true );
}

void KDirListerTest::completed()
{
    if ( lister->url().path() == "/")
    {
        KFileItem* item = lister->findByUrl( KUrl( "/tmp" ) );
        if ( item )
            kDebug() << "Found /tmp: " << item << endl;
        else
            kWarning() << "/tmp not found! Bug in findByURL?" << endl;
    }
}

int main ( int argc, char *argv[] )
{
  KCmdLineArgs::init( argc, argv, "kdirlistertest", 0, 0, 0, 0 );
  KApplication app;

  KDirListerTest *test = new KDirListerTest( 0 );
  test->show();
  return app.exec();
}

#include "kdirlistertest.moc"
