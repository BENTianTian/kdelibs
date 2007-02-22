 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <QtCore/QFile>

#include <kdebug.h>
#include <kcmdlineargs.h>

#include "kdirwatchtest.h"

static const KCmdLineOptions options[] =
{
  {"+[directory ...]", "Directory(ies) to watch", 0},
  KCmdLineLastOption
};


int main (int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, "kdirwatchtest", "KDirWatchTest",
		     "Test for KDirWatch", "1.0");
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs::addStdCmdLineOptions();

  KApplication a;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  myTest testObject;

  KDirWatch *dirwatch1 = KDirWatch::self();
  KDirWatch *dirwatch2 = new KDirWatch;

  testObject.connect(dirwatch1, SIGNAL( dirty( const QString &)), SLOT( dirty( const QString &)) );
  testObject.connect(dirwatch1, SIGNAL( created( const QString &)), SLOT( created( const QString &)) );
  testObject.connect(dirwatch1, SIGNAL( deleted( const QString &)), SLOT( deleted( const QString &)) );

  if (args->count() >0) {
    for(int i = 0; i < args->count(); i++) {
      kDebug() << "Watching: " << args->arg(i) << endl;
      dirwatch2->addDir( QFile::decodeName( args->arg(i)));
    }
  }

  QString home = QString(getenv ("HOME")) + '/';
  QString desk = home + "Desktop/";
  kDebug() << "Watching: " << home << endl;
  dirwatch1->addDir(home);
  kDebug() << "Watching file: " << home << "foo " << endl;
  dirwatch1->addFile(home+"foo");
  kDebug() << "Watching: " << desk << endl;
  dirwatch1->addDir(desk);
  QString test = home + "test/";
  kDebug() << "Watching: (but skipped) " << test << endl;
  dirwatch1->addDir(test);

  dirwatch1->startScan();
  dirwatch2->startScan();

  if(!dirwatch1->stopDirScan(home))
      kDebug() << "stopDirscan: " << home << " error!" << endl;
  if(!dirwatch1->restartDirScan(home))
      kDebug() << "restartDirScan: " << home << "error!" << endl;
  if (!dirwatch1->stopDirScan(test))
     kDebug() << "stopDirScan: error" << endl;

  KDirWatch::statistics();

  delete dirwatch2;

  KDirWatch::statistics();

  return a.exec();
}
#include "kdirwatchtest.moc"
