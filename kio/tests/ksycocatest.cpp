#include <kuserprofile.h>
#include <ktrader.h>
#include <kservice.h>
#include <kmimetype.h>
#include <assert.h>
#include <kstddirs.h>

#include <kapp.h>

int main(int argc, char *argv[])
{
   KApplication k(argc,argv,"whatever",false/*noGUI*/); // KMessageBox needs KApp for makeStdCaption

   debug("Trying to look for text/plain");
   KMimeType::Ptr s1 = KMimeType::mimeType("text/plain");
   if ( s1 )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(s1->comment(KURL(),false)));
   }
   else
   {
     debug("Not found !");
   }
     
   debug("Trying to look for application/x-zerosize");
   KMimeType::Ptr s0 = KMimeType::mimeType("application/x-zerosize");
   if ( s0 )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(s0->comment(KURL(),false)));
   }
   else
   {
     debug("Not found !");
   }
     
   debug("Trying to look for Desktop Pager");
   KService::Ptr se = KService::service("Desktop Pager");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
   }
   else
   {
     debug("Not found !");
   }

   debug("Trying to look for kpager");
   se = KService::service("kpager");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
   }
   else
   {
     debug("Not found !");
   }

   debug("Trying to look for System/kpager.desktop");
   se = KService::service("System/kpager.desktop");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
   }
   else
   {
     debug("Not found !");
   }

   debug("Trying to look for System/fake-entry.desktop");
   se = KService::service("System/fake-entry.desktop");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
   }
   else
   {
     debug("Not found !");
   }
   
   debug("Querying userprofile for services associated with text/plain");
   KServiceTypeProfile::OfferList offers = KServiceTypeProfile::offers("text/plain");
   debug("got %d offers", offers.count());
   KServiceTypeProfile::OfferList::Iterator it = offers.begin();
   for ( ; it != offers.end() ; it++ )
   {
     debug((*it).service()->name());
   }

   debug("Querying userprofile for services associated with KOfficeFilter");
   offers = KServiceTypeProfile::offers("KOfficeFilter");
   debug("got %d offers", offers.count());
   it = offers.begin();
   for ( ; it != offers.end() ; it++ )
   {
     debug((*it).service()->name());
   }

   debug("Querying trader for Konqueror/Plugin");
   KTrader::OfferList traderoffers = KTrader::self()->query("Konqueror/Plugin");
   debug("got %d offers", traderoffers.count());
   KTrader::OfferList::Iterator trit = traderoffers.begin();
   for ( ; trit != traderoffers.end() ; trit++ )
   {
     debug((*trit)->name());
   }


   //
   debug("\nTrying findByURL for $KDEDIR/bin/kdesktop");
   KMimeType::Ptr mf  = KMimeType::findByURL( KStandardDirs::findExe( "kdesktop" ), 0,
				 true, false );
   assert( mf );
   debug(QString("Name is %1").arg(mf->name()));
   debug(QString("Comment is %1").arg(mf->comment(KURL(),false)));

   //
   debug("\nTrying findByURL for home.png");
   mf  = KMimeType::findByURL( locate( "toolbar", "home.png" ), 0,
				 true, false );
   assert( mf );
   debug(QString("Name is %1").arg(mf->name()));
   debug(QString("Comment is %1").arg(mf->comment(KURL(),false)));

   //
   debug("\nTrying findByURL for Makefile.am");
   mf  = KMimeType::findByURL( KURL("/tmp/Makefile.am"), 0, true, false );
   assert( mf );
   debug(QString("Name is %1").arg(mf->name()));
   debug(QString("Comment is %1").arg(mf->comment(KURL(),false)));

   debug("\nTrying findByURL for Makefile.am");

   KMimeType::List mtl;

   mtl  = KMimeType::allMimeTypes( );
   assert( mtl.count() );
   debug(QString("Found %1 mime types.").arg(mtl.count()));

   KService::List sl;

   sl  = KService::allServices( );
   assert( sl.count() );
   debug(QString("Found %1 services.").arg(sl.count()));

   debug("done");
   return 0;
}
