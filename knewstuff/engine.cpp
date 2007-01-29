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

#include <qdom.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <kglobal.h>
#include <kcomponentdata.h>
#include <ktoolinvocation.h>
#include <kdebug.h>
#include <kjobuidelegate.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "knewstuff.h"
#include "downloaddialog.h"
#include "uploaddialog.h"
#include "providerdialog.h"

#include "engine.h"
#include "engine.moc"

using namespace KNS;

struct Engine::Private
{
    bool mIgnoreInstallResult;
    KNewStuff *mNewStuff;
};

Engine::Engine( KNewStuff *newStuff, const QString &category,
                QWidget *parentWidget ) :
  mParentWidget( parentWidget ), mDownloadDialog( 0 ),
  mUploadDialog( 0 ), mProviderDialog( 0 ), mUploadProvider( 0 ),
  d(new Private), mCategory( category )
{
  d->mNewStuff = newStuff;
  mProviderLoader = new ProviderLoader( mParentWidget );

}

Engine::Engine( KNewStuff *newStuff, const QString &category,
                const QString &providerList, QWidget *parentWidget ) :
                mParentWidget( parentWidget ),
		mDownloadDialog( 0 ), mUploadDialog( 0 ),
		mProviderDialog( 0 ), mUploadProvider( 0 ),
                mProviderList( providerList ), d(new Private),
		mCategory( category )
{
  d->mNewStuff = newStuff;
  d->mIgnoreInstallResult = false;
  mProviderLoader = new ProviderLoader( mParentWidget );
}

Engine::~Engine()
{
  qDeleteAll(mNewStuffList);
  mNewStuffList.clear();
  delete d;
  delete mProviderLoader;

  delete mUploadDialog;
  delete mDownloadDialog;
}

void Engine::download()
{
  kDebug() << "Engine::download()" << endl;

  connect( mProviderLoader,
           SIGNAL( providersLoaded( Provider::List * ) ),
           SLOT( getMetaInformation( Provider::List * ) ) );
  mProviderLoader->load( mCategory, mProviderList );
}

void Engine::getMetaInformation( Provider::List *providers )
{
  mProviderLoader->disconnect();

  mNewStuffJobData.clear();

  if ( !mDownloadDialog ) {
    mDownloadDialog = new DownloadDialog( this, mParentWidget );
    mDownloadDialog->show();
  }
  mDownloadDialog->clear();

  Provider *p;
  for (int i = 0; i < providers->size(); ++i) {
	p = providers->at(i);
    if ( p->downloadUrl().isEmpty() ) continue;

    KIO::TransferJob *job = KIO::get( p->downloadUrl(), false, false );
    connect( job, SIGNAL( result( KJob * ) ),
             SLOT( slotNewStuffJobResult( KJob * ) ) );
    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             SLOT( slotNewStuffJobData( KIO::Job *, const QByteArray & ) ) );

    mNewStuffJobData.insert( job, "" );
    mProviderJobs[ job ] = p;
  }
}

void Engine::slotNewStuffJobData( KIO::Job *job, const QByteArray &data )
{
  if ( data.isEmpty() ) return;

  kDebug() << "Engine:slotNewStuffJobData()" << endl;

  mNewStuffJobData[ job ].append( QString::fromUtf8( data ) ); // ####### The fromUtf8 conversion should be done at the end, not chunk by chunk
}

void Engine::slotNewStuffJobResult( KJob *job )
{
  if ( job->error() ) {
    kDebug() << "Error downloading new stuff descriptions." << endl;
    job->uiDelegate()->showErrorMessage();
  } else {
    QString knewstuffDoc = mNewStuffJobData[ job ];

    kDebug() << "---START---" << endl << knewstuffDoc << "---END---" << endl;

    mDownloadDialog->addProvider( mProviderJobs[ job ] );

    QDomDocument doc;
    if ( !doc.setContent( knewstuffDoc ) ) {
      kDebug() << "Error parsing knewstuff.xml." << endl;
      return;
    } else {
      QDomElement knewstuff = doc.documentElement();

      if ( knewstuff.isNull() ) {
        kDebug() << "No document in knewstuffproviders.xml." << endl;
      } else {
        QDomNode p;
        for ( p = knewstuff.firstChild(); !p.isNull(); p = p.nextSibling() ) {
          QDomElement stuff = p.toElement();
          if ( stuff.tagName() != "stuff" ) continue;
          if ( stuff.attribute("category", mCategory) != mCategory ) {
            if ( stuff.attribute("type", mCategory) != mCategory ) {
              continue;
            }
          }

          Entry *entry = new Entry( stuff );
          mNewStuffList.append( entry );

          mDownloadDialog->show();

          mDownloadDialog->addEntry( entry );

          kDebug() << "KNEWSTUFF: " << entry->name() << endl;

          kDebug() << "  SUMMARY: " << entry->summary() << endl;
          kDebug() << "  VERSION: " << entry->version() << endl;
          kDebug() << "  RELEASEDATE: " << entry->releaseDate().toString() << endl;
          kDebug() << "  RATING: " << entry->rating() << endl;

          kDebug() << "  LANGS: " << entry->langs().join(", ") << endl;
        }
      }
    }
  }

  mNewStuffJobData.remove( job );
  mProviderJobs.remove( job );

  if ( mNewStuffJobData.count() == 0 ) {
    mDownloadDialog->show();
    mDownloadDialog->raise();
  }
}

void Engine::download( Entry *entry )
{
  kDebug() << "Engine::download(entry)" << endl;

  KUrl source = entry->payload();
  mDownloadDestination = d->mNewStuff->downloadDestination( entry );

  if ( mDownloadDestination.isEmpty() ) {
    kDebug() << "Empty downloadDestination. Cancelling download." << endl;
    return;
  }

  KUrl destination = KUrl( mDownloadDestination );

  kDebug() << "  SOURCE: " << source.url() << endl;
  kDebug() << "  DESTINATION: " << destination.url() << endl;

  KIO::FileCopyJob *job = KIO::file_copy( source, destination, -1, true );
  connect( job, SIGNAL( result( KJob * ) ),
           SLOT( slotDownloadJobResult( KJob * ) ) );
}

void Engine::slotDownloadJobResult( KJob *job )
{
  if ( job->error() ) {
    kDebug() << "Error downloading new stuff payload." << endl;
    job->uiDelegate()->showErrorMessage();
    return;
  }

  if ( d->mNewStuff->install( mDownloadDestination ) ) {
    if ( !d->mIgnoreInstallResult ) {
      KMessageBox::information( mParentWidget,
                                i18n("Successfully installed hot new stuff.") );
    }
  } else
    if ( !d->mIgnoreInstallResult ){
      KMessageBox::error( mParentWidget,
                          i18n("Failed to install hot new stuff.") );
  }
}

void Engine::upload(const QString &fileName, const QString &previewName )
{
  mUploadFile = fileName;
  mPreviewFile = previewName;

  connect( mProviderLoader,
           SIGNAL( providersLoaded( Provider::List * ) ),
           SLOT( selectUploadProvider( Provider::List * ) ) );
  mProviderLoader->load( mCategory );
}

void Engine::selectUploadProvider( Provider::List *providers )
{
  kDebug() << "Engine:selectUploadProvider()" << endl;

  mProviderLoader->disconnect();

  if ( !mProviderDialog ) {
    mProviderDialog = new ProviderDialog( this, mParentWidget );
  }

  mProviderDialog->clear();

  mProviderDialog->show();
  mProviderDialog->raise();

  for (int i = 0; i < providers->size(); ++i) {
		  Provider *p = providers->at(i);
		mProviderDialog->addProvider( p );
  }
}

void Engine::requestMetaInformation( Provider *provider )
{
  mUploadProvider = provider;

  if ( !mUploadDialog ) {
    mUploadDialog = new UploadDialog( this, mParentWidget );
  }
  mUploadDialog->setPreviewFile( mPreviewFile );
  mUploadDialog->setPayloadFile( mUploadFile );
  mUploadDialog->show();
  mUploadDialog->raise();
}

void Engine::upload( Entry *entry )
{
  if ( mUploadFile.isNull()) {
     mUploadFile = KStandardDirs::locateLocal( "data", QString(KGlobal::mainComponent().componentName()) + "/upload/" + entry->fullName() );

    if ( !d->mNewStuff->createUploadFile( mUploadFile ) ) {
      KMessageBox::error( mParentWidget, i18n("Unable to create file to upload.") );
      emit uploadFinished( false );
      return;
    }
  }

  QString lang = entry->langs().first();
  QFileInfo fi( mUploadFile );
  entry->setPayload( KUrl( fi.fileName() ), lang );

  if ( !createMetaFile( entry ) ) {
    emit uploadFinished( false );
    return;
  }

  QString text = i18n("The files to be uploaded have been created at:\n");
  text.append( i18n("Data file: %1\n",  mUploadFile) );
  if (!mPreviewFile.isEmpty()) {
    text.append( i18n("Preview image: %1\n",  mPreviewFile) );
  }
  text.append( i18n("Content information: %1\n",  mUploadMetaFile) );
  text.append( i18n("Those files can now be uploaded.\n") );
  text.append( i18n("Beware that any people might have access to them at any time.") );

  QString caption = i18n("Upload Files");

  if ( mUploadProvider->noUpload() ) {
    KUrl noUploadUrl = mUploadProvider->noUploadUrl();
    if ( noUploadUrl.isEmpty() ) {
      text.append( i18n("Please upload the files manually.") );
      KMessageBox::information( mParentWidget, text, caption );
    } else {
      int result = KMessageBox::questionYesNo( mParentWidget, text, caption,
                                               KGuiItem(i18n("Upload Info")),
                                               KStandardGuiItem::close() );
      if ( result == KMessageBox::Yes ) {
        KToolInvocation::invokeBrowser( noUploadUrl.url() );
      }
    }
  } else {
    int result = KMessageBox::questionYesNo( mParentWidget, text, caption,
                                             KGuiItem(i18n("&Upload")), KStandardGuiItem::cancel() );
    if ( result == KMessageBox::Yes ) {
      KUrl destination = mUploadProvider->uploadUrl();
      destination.setFileName( fi.fileName() );

      KIO::FileCopyJob *job = KIO::file_copy( KUrl( mUploadFile ), destination );
      connect( job, SIGNAL( result( KJob * ) ),
               SLOT( slotUploadPayloadJobResult( KJob * ) ) );
    } else {
      emit uploadFinished( false );
    }
  }
}

bool Engine::createMetaFile( Entry *entry )
{
  QDomDocument doc("knewstuff");
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement de = doc.createElement("knewstuff");
  doc.appendChild( de );

  entry->setCategory(category());
  de.appendChild( entry->createDomElement( doc, de ) );

  kDebug() << "--DOM START--" << endl << doc.toString()
            << "--DOM_END--" << endl;

  if ( mUploadMetaFile.isNull() ) {
    mUploadMetaFile = entry->fullName() + ".meta";
    mUploadMetaFile = KStandardDirs::locateLocal( "data", QString(KGlobal::mainComponent().componentName()) + "/upload/" + mUploadMetaFile );
  }

  QFile f( mUploadMetaFile );
  if ( !f.open( QIODevice::WriteOnly ) ) {
    mUploadMetaFile.clear();
    return false;
  }

  QTextStream ts( &f );
  ts.setCodec( "UTF-8" );
  ts << doc.toString();

  f.close();

  return true;
}

void Engine::slotUploadPayloadJobResult( KJob *job )
{
  if ( job->error() ) {
    kDebug() << "Error uploading new stuff payload." << endl;
    job->uiDelegate()->showErrorMessage();
    emit uploadFinished( false );
    return;
  }

  if (mPreviewFile.isEmpty()) {
    slotUploadPreviewJobResult(job);
    return;
  }

  QFileInfo fi( mPreviewFile );

  KUrl previewDestination = mUploadProvider->uploadUrl();
  previewDestination.setFileName( fi.fileName() );

  KIO::FileCopyJob *newJob = KIO::file_copy( KUrl( mPreviewFile ), previewDestination );
  connect( newJob, SIGNAL( result( KJob * ) ),
           SLOT( slotUploadPreviewJobResult( KJob * ) ) );
}

void Engine::slotUploadPreviewJobResult( KJob *job )
{
  if ( job->error() ) {
    kDebug() << "Error uploading new stuff preview." << endl;
    job->uiDelegate()->showErrorMessage();
    emit uploadFinished( true );
    return;
  }

  QFileInfo fi( mUploadMetaFile );

  KUrl metaDestination = mUploadProvider->uploadUrl();
  metaDestination.setFileName( fi.fileName() );

  KIO::FileCopyJob *newJob = KIO::file_copy( KUrl( mUploadMetaFile ), metaDestination );
  connect( newJob, SIGNAL( result( KJob * ) ),
           SLOT( slotUploadMetaJobResult( KJob * ) ) );
}

void Engine::slotUploadMetaJobResult( KJob *job )
{
  mUploadMetaFile.clear();
  if ( job->error() ) {
    kDebug() << "Error uploading new stuff metadata." << endl;
    job->uiDelegate()->showErrorMessage();
    emit uploadFinished( false );
    return;
  }

  KMessageBox::information( mParentWidget,
                            i18n("Successfully uploaded new stuff.") );
  emit uploadFinished( true );
}

void Engine::ignoreInstallResult(bool ignore)
{
  d->mIgnoreInstallResult = ignore;
}
