
#include <kcmdlineargs.h>
#include <ktrader.h>
#include <kdebug.h>
#include <kapp.h>

static KCmdLineOptions options[] =
{
  { "+query", "the query", 0 },
  { "+[genericServiceType]", "Application (default), or KParts/ReadOnlyPart", 0 },
  { "+[constraint]", "constraint", 0 },
  { "+[preference]", "preference", 0 },
  { 0, 0, 0 }
};

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "ktradertest", "A KTrader testing tool", "0.0" );
 
  KCmdLineArgs::addCmdLineOptions( options );
  
  KApplication app;
  
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  
  QString query = QString::fromLocal8Bit( args->arg( 0 ) );

  QString genericServiceType, constraint, preference;

  if ( args->count() >= 2 )
    genericServiceType = QString::fromLocal8Bit( args->arg( 1 ) );

  if ( args->count() >= 3 )
    constraint = QString::fromLocal8Bit( args->arg( 2 ) );

  if ( args->count() == 4 )
    preference = QString::fromLocal8Bit( args->arg( 3 ) );


  kdDebug() << "query is : " << query << endl;
  kdDebug() << "genericServiceType is : " << genericServiceType << endl;
  kdDebug() << "constraint is : " << constraint << endl;
  kdDebug() << "preference is : " << preference << endl;
  
  KTrader::OfferList offers = KTrader::self()->query( query, genericServiceType, constraint, preference );
  
  kdDebug() << "got " << offers.count() << " offers." << endl;
  
  int i = 0;
  KTrader::OfferList::ConstIterator it = offers.begin();
  KTrader::OfferList::ConstIterator end = offers.end();
  for (; it != end; ++it )
  {
    kdDebug() << "---- Offer " << ++i << " ----" << endl;
    QStringList props = (*it)->propertyNames();
    QStringList::ConstIterator propIt = props.begin();
    QStringList::ConstIterator propEnd = props.end();
    for (; propIt != propEnd; ++propIt )
    {
      QVariant prop = (*it)->property( *propIt );
      
      if ( !prop.isValid() )
      {
        kdDebug() << "Invalid property " << *propIt << endl;
	continue;
      }
      
      QString outp = *propIt;
      outp += " : ";
      
      switch ( prop.type() )
      {
        case QVariant::String:
  	  outp += prop.toString();
	break;
        case QVariant::StringList:
  	  outp += prop.toStringList().join(" - ");
	break;
        default: outp = QString::null; break;
      }

      if ( !outp.isEmpty() )
        kdDebug() << outp << endl;
    }
  }
}
