/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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
 **/

#include <qdir.h>

#include "kded.h"

#include <kbuildservicetypefactory.h>
#include <kbuildservicefactory.h>
#include <kresourcelist.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <qfile.h>
#include <qtimer.h>

#include <dcopclient.h>

#include <kuniqueapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstddirs.h>

static void runBuildSycoca()
{
   // Avoid relying on $PATH and on /bin/sh -> don't use system()
   KProcess proc;
   proc << locate("exe","kbuildsycoca");
   proc << "--incremental";
   proc.start( KProcess::Block );
}


Kded::Kded()
  : KSycoca( true )
{
  QString path = KGlobal::dirs()->saveLocation("tmp")+"ksycoca";
  QCString cPath = QFile::encodeName(path);
  m_pTimer = new QTimer(this);
  connect (m_pTimer, SIGNAL(timeout()), this, SLOT(recreate()));

  m_pDirWatch = 0;
}

Kded::~Kded()
{
  m_pTimer->stop();
  delete m_pTimer;
  delete m_pDirWatch;
}

void Kded::build()
{
  delete m_pDirWatch;
  m_pDirWatch = new KDirWatch;

  QObject::connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
           this, SLOT(update(const QString&)));
  QObject::connect( m_pDirWatch, SIGNAL(deleted(const QString&)),
           this, SLOT(dirDeleted(const QString&)));

  // It is very important to build the servicetype one first
  // Both are registered in KSycoca, no need to keep the pointers
  KSycocaFactory *stf = new KBuildServiceTypeFactory;
  KBuildServiceGroupFactory *bsgf = new KBuildServiceGroupFactory;
  (void) new KBuildServiceFactory(stf, bsgf);
  // We don't include KImageIOFactory here because it doesn't add
  // new resourceList entries anyway.
  //(void) KImageIOFactory::self();
  // Same for KBuildProtocolInfoFactory

  // For each factory
  QListIterator<KSycocaFactory> factit ( *m_lstFactories );
  for (KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->first() )
  {
    // For each resource the factory deals with
    for( KSycocaResourceList::ConstIterator it1 = factory->resourceList()->begin();
         it1 != factory->resourceList()->end();
         ++it1 )
    {
      KSycocaResource res = (*it1);
      QStringList dirs = KGlobal::dirs()->resourceDirs( res.resource.ascii() );
      // For each resource the factory deals with
      for( QStringList::ConstIterator it2 = dirs.begin();
           it2 != dirs.end();
           ++it2 )
      {
         readDirectory( *it2 );
      }
    }
    m_lstFactories->removeRef(factory);
  }
}

void Kded::recreate()
{
   // Using KLauncher here is difficult since we might not have a
   // database

   build(); // Update tree first, to be sure to miss nothing.

   runBuildSycoca();

   while( !m_requests.isEmpty())
   {
      QCString replyType = "void";
      QByteArray replyData;
      kapp->dcopClient()->endTransaction(m_requests.first(), replyType, replyData);
      m_requests.remove(m_requests.begin());
   }
}

void Kded::dirDeleted(const QString& /*path*/)
{
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 2000, true /* single shot */ );
}

void Kded::update(const QString& )
{
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 2000, true /* single shot */ );
}

bool Kded::process(const QCString &fun, const QByteArray &/*data*/,
                           QCString &replyType, QByteArray &/*replyData*/)
{
  if (fun == "recreate()") {
    kdDebug() << "got a recreate signal!" << endl;
    if (m_requests.isEmpty())
    {
       m_pTimer->start(0, true /* single shot */ );
    }
    m_requests.append(kapp->dcopClient()->beginTransaction());
    replyType = "void";
    return true;
  } else
    return false;
    // don't call KSycoca::process - this is for other apps, not kded
}


void Kded::readDirectory( const QString& _path )
{
  // kdDebug(7020) << QString("reading %1").arg(_path) << endl;

  QString path( _path );
  if ( path.right(1) != "/" )
    path += "/";

  if ( m_pDirWatch->contains( path ) ) // Already seen this one?
     return;

  QDir d( _path, QString::null, QDir::Unsorted, QDir::AccessMask | QDir::Dirs );
  // set QDir ...


  //************************************************************************
  //                           Setting dirs
  //************************************************************************

  m_pDirWatch->addDir(path);          // add watch on this dir

  if ( !d.exists() )                            // exists&isdir?
  {
    kdDebug(7020) << QString("Does not exist! (%1)").arg(_path) << endl;
    return;                             // return false
  }

  // Note: If some directory is gone, dirwatch will delete it from the list.

  //************************************************************************
  //                               Reading
  //************************************************************************
  QString file;
  unsigned int i;                           // counter and string length.
  unsigned int count = d.count();
  for( i = 0; i < count; i++ )                        // check all entries
  {
     if (d[i] == "." || d[i] == ".." || d[i] == "magic")
       continue;                          // discard those ".", "..", "magic"...

     file = path;                           // set full path
     file += d[i];                          // and add the file name.

     readDirectory( file );      // yes, dive into it.
  }
}

static void sighandler(int /*sig*/)
{
    kapp->quit();
}

static KCmdLineOptions options[] =
{
  { "check", I18N_NOOP("Check sycoca database only once."), 0 },
  { 0, 0, 0 }
};

int main(int argc, char *argv[])
{
     KAboutData aboutData( "kded", I18N_NOOP("KDE Daemon"),
        "$Id$",
        I18N_NOOP("KDE Daemon - triggers Sycoca database updates when needed."));

     KCmdLineArgs::init(argc, argv, &aboutData);

     KUniqueApplication::addCmdLineOptions();

     KCmdLineArgs::addCmdLineOptions( options );

     // this program is in kdelibs so it uses kdelibs as catalogue
     KLocale::setMainCatalogue("kdelibs");

     // WABA: Make sure not to enable session management.
     putenv(strdup("SESSION_MANAGER="));

     KInstance *instance = new KInstance(&aboutData);
     (void) instance->config(); // Enable translations.

     KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

     if (args->isSet("check"))
     {
        runBuildSycoca();
        exit(0);
     }

     if (!KUniqueApplication::start())
     {
        fprintf(stderr, "KDE Daemon (kded) already running.\n");
        exit(0);
     }

     Kded *kded = new Kded(); // Build data base

     kded->recreate();

     signal(SIGTERM, sighandler);
     KUniqueApplication k( false, false ); // No styles, no GUI

     kapp->dcopClient()->setDaemonMode( true );

     // During startup kdesktop waits for KDED to finish.
     // Send a notifyDatabaseChanged signal even if the database hasn't
     // changed.
     // If the database changed, kbuildsycoca's signal didn't go anywhere
     // anyway, because it was too early, so let's send this signal
     // unconditionnally (David)
     QByteArray data;
     kapp->dcopClient()->send( "*", "ksycoca", "notifyDatabaseChanged()", data );

     return k.exec(); // keep running
}

#include "kded.moc"
