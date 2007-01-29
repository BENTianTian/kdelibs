/*
    This file is part of KNewStuff.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <kconfig.h>
#include <kdebug.h>
#include <kjobuidelegate.h>
#include <kio/job.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qhash.h>
#include <qcoreapplication.h>

#include "provider.h"
#include "provider.moc"

using namespace KNS;

// BCI for KDE 3.5 only
class ProviderPrivate
{
  public:
  ProviderPrivate(){}
  KUrl mDownloadUrlLatest;
  KUrl mDownloadUrlScore;
  KUrl mDownloadUrlDownloads;
};

static QHash<void*, ProviderPrivate*> *d_ptr_prov = 0;

static void cleanup_d_ptr_prov()
{
  delete d_ptr_prov;
  d_ptr_prov = 0; // not in BIC guide - add there
}

static ProviderPrivate *d_prov(const Provider *p)
{
  if(!d_ptr_prov)
  {
    d_ptr_prov = new QHash<void*, ProviderPrivate*>();
    qAddPostRoutine(cleanup_d_ptr_prov);
  }
  ProviderPrivate *ret = d_ptr_prov->value((void*)p);
  if(!ret)
  {
    ret = new ProviderPrivate();
    d_ptr_prov->insert((void*)p, ret);
  }
  return ret;
}

KUrl Provider::downloadUrlVariant( QString variant ) const
{
  if((variant == "latest") && (d_prov(this)->mDownloadUrlLatest.isValid()))
    return d_prov(this)->mDownloadUrlLatest;
  if((variant == "score") && (d_prov(this)->mDownloadUrlScore.isValid()))
    return d_prov(this)->mDownloadUrlScore;
  if((variant == "downloads") && (d_prov(this)->mDownloadUrlDownloads.isValid()))
    return d_prov(this)->mDownloadUrlDownloads;

  return mDownloadUrl;
}

// BCI part ends here

Provider::Provider() : mNoUpload( false )
{
}

Provider::Provider( const QDomElement &e ) : mNoUpload( false )
{
  parseDomElement( e );
}

Provider::~Provider()
{
}


void Provider::setName( const QString &name )
{
  mName = name;
}

QString Provider::name() const
{
  return mName;
}


void Provider::setIcon( const KUrl &url )
{
  mIcon = url;
}

KUrl Provider::icon() const
{
  return mIcon;
}


void Provider::setDownloadUrl( const KUrl &url )
{
  mDownloadUrl = url;
}

KUrl Provider::downloadUrl() const
{
  return mDownloadUrl;
}


void Provider::setUploadUrl( const KUrl &url )
{
  mUploadUrl = url;
}

KUrl Provider::uploadUrl() const
{
  return mUploadUrl;
}


void Provider::setNoUploadUrl( const KUrl &url )
{
  mNoUploadUrl = url;
}

KUrl Provider::noUploadUrl() const
{
  return mNoUploadUrl;
}


void Provider::setNoUpload( bool enabled )
{
  mNoUpload = enabled;
}

bool Provider::noUpload() const
{
  return mNoUpload;
}


void Provider::parseDomElement( const QDomElement &element )
{
  if ( element.tagName() != "provider" ) return;

  setDownloadUrl( KUrl( element.attribute("downloadurl") ) );
  setUploadUrl( KUrl( element.attribute("uploadurl") ) );
  setNoUploadUrl( KUrl( element.attribute("nouploadurl") ) );

  d_prov(this)->mDownloadUrlLatest = KUrl( element.attribute("downloadurl-latest") );
  d_prov(this)->mDownloadUrlScore = KUrl( element.attribute("downloadurl-score") );
  d_prov(this)->mDownloadUrlDownloads = KUrl( element.attribute("downloadurl-downloads") );

  KUrl iconurl( element.attribute("icon") );
  if(!iconurl.isValid()) iconurl.setPath( element.attribute("icon") );
  setIcon( iconurl );

  QDomNode n;
  for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement p = n.toElement();

    if ( p.tagName() == "noupload" ) setNoUpload( true );
    if ( p.tagName() == "title" ) setName( p.text().trimmed() );
  }
}

QDomElement Provider::createDomElement( QDomDocument &doc, QDomElement &parent )
{
  QDomElement entry = doc.createElement( "stuff" );
  parent.appendChild( entry );

  QDomElement n = doc.createElement( "name" );
  n.appendChild( doc.createTextNode( name() ) );
  entry.appendChild( n );

  return entry;
}


ProviderLoader::ProviderLoader( QWidget *parentWidget ) :
  mParentWidget( parentWidget )
{
  //mProviders.setAutoDelete( true );
}

ProviderLoader::~ProviderLoader()
{
		qDeleteAll(mProviders);
		mProviders.clear();
}

void ProviderLoader::load( const QString &category, const QString &providersList )
{
  kDebug() << "ProviderLoader::load()" << endl;

  mProviders.clear();
  mJobData = "";

  KSharedConfig::Ptr cfg = KGlobal::config();
  cfg->setGroup("KNewStuff");

  QString providersUrl = providersList;
  if( providersUrl.isEmpty() )
  	providersUrl = cfg->readEntry( "ProvidersUrl" );

  if ( providersUrl.isEmpty() ) {
    // TODO: Replace the default by the real one.
    QString server = cfg->readEntry( "MasterServer",
                                     "http://korganizer.kde.org" );

    providersUrl = server + "/knewstuff/" + category + "/providers.xml";
  }

  kDebug() << "ProviderLoader::load(): providersUrl: " << providersUrl << endl;

  KIO::TransferJob *job = KIO::get( KUrl( providersUrl ), false, false );
  connect( job, SIGNAL( result( KJob * ) ),
           SLOT( slotJobResult( KJob * ) ) );
  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );

//  job->dumpObjectInfo();
}

void ProviderLoader::slotJobData( KIO::Job *, const QByteArray &data )
{
  kDebug() << "ProviderLoader::slotJobData()" << endl;

  if ( data.size() == 0 ) return;

  mJobData.append( QString::fromUtf8( data ) ); // ####### The fromUtf8 conversion should be done at the end, not chunk by chunk
}

void ProviderLoader::slotJobResult( KJob *job )
{
  if ( job->error() ) {
     job->uiDelegate()->showErrorMessage();
  }

  kDebug() << "--PROVIDERS-START--" << endl << mJobData << "--PROV_END--"
            << endl;

  QDomDocument doc;
  if ( !doc.setContent( mJobData ) ) {
    KMessageBox::error( mParentWidget, i18n("Error parsing providers list.") );
    return;
  }

  QDomElement providers = doc.documentElement();

  if ( providers.isNull() ) {
    kDebug() << "No document in Providers.xml." << endl;
  }

  QDomNode n;
  for ( n = providers.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement p = n.toElement();

    if ( p.tagName() == "provider" ) {
      mProviders.append( new Provider( p ) );
    }
  }

  emit providersLoaded( &mProviders );
}
