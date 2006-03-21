#include <kiconloader.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>

#include <qdatetime.h>
#include <qpixmap.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  KAboutData about("kiconloadertest", "kiconloadertest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KIconLoader * mpLoader = KGlobal::iconLoader();
  K3Icon::Context mContext = K3Icon::Application;
  QTime dt;
  dt.start();
  int count = 0;
  for ( int mGroup = 0; mGroup < K3Icon::LastGroup ; ++mGroup )
  {
      kDebug() << "queryIcons " << mGroup << "," << mContext << endl;
      const QStringList filelist = mpLoader->queryIcons(mGroup, mContext);
      kDebug() << " -> found " << filelist.count() << " icons." << endl;
      int i=0;
      for(QStringList::ConstIterator it = filelist.begin();
          it != filelist.end() /*&& i<10*/;
          ++it, ++i )
      {
          //kDebug() << ( i==9 ? "..." : (*it) ) << endl;
          mpLoader->loadIcon( (*it), (K3Icon::Group)mGroup );
          ++count;
      }
  }
  kDebug() << "Loading " << count << " icons took " << (float)(dt.elapsed()) / 1000 << " seconds" << endl;
}

