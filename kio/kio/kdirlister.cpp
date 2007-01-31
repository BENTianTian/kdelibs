/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2003-2005 David Faure <faure@kde.org>
                 2001-2006 Michael Brade <brade@kde.org>

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

#include "kdirlister.h"

#include <qregexp.h>
#include <q3ptrlist.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstaticdeleter.h>
#include "kprotocolmanager.h"

#include "kdirlister_p.h"

#include <assert.h>

KDirListerCache* KDirListerCache::s_pSelf = 0;
static KStaticDeleter<KDirListerCache> sd_KDirListerCache;

// Enable this to get printDebug() called often, to see the contents of the cache
//#define DEBUG_CACHE

// Make really sure it doesn't get activated in the final build
#ifdef NDEBUG
#undef DEBUG_CACHE
#endif

KDirListerCache::KDirListerCache( int maxCount )
  : itemsCached( maxCount )
{
  kDebug(7004) << "+KDirListerCache" << endl;

  itemsInUse.setAutoDelete( false );
  itemsCached.setAutoDelete( true );
  urlsCurrentlyListed.setAutoDelete( true );
  urlsCurrentlyHeld.setAutoDelete( true );
  pendingUpdates.setAutoDelete( true );

  connect( kdirwatch, SIGNAL( dirty( const QString& ) ),
           this, SLOT( slotFileDirty( const QString& ) ) );
  connect( kdirwatch, SIGNAL( created( const QString& ) ),
           this, SLOT( slotFileCreated( const QString& ) ) );
  connect( kdirwatch, SIGNAL( deleted( const QString& ) ),
           this, SLOT( slotFileDeleted( const QString& ) ) );

  kdirnotify = new org::kde::KDirNotify(QString(), QString(), QDBusConnection::sessionBus(), this);
  connect(kdirnotify, SIGNAL(FileRenamed(QString,QString)), SLOT(slotFileRenamed(QString,QString)));
  connect(kdirnotify, SIGNAL(FilesAdded(QString)), SLOT(slotFilesAdded(QString)));
  connect(kdirnotify, SIGNAL(FilesChanged(QStringList)), SLOT(slotFilesChanged(QStringList)));
  connect(kdirnotify, SIGNAL(FilesRemoved(QStringList)), SLOT(slotFilesRemoved(QStringList)));
}

KDirListerCache::~KDirListerCache()
{
  kDebug(7004) << "-KDirListerCache" << endl;

  itemsInUse.setAutoDelete( true );
  itemsInUse.clear();
  itemsCached.clear();
  urlsCurrentlyListed.clear();
  urlsCurrentlyHeld.clear();

  if ( KDirWatch::exists() )
    kdirwatch->disconnect( this );
}

// setting _reload to true will emit the old files and
// call updateDirectory
bool KDirListerCache::listDir( KDirLister *lister, const KUrl& _u,
                               bool _keep, bool _reload )
{
  // like this we don't have to worry about trailing slashes any further
  KUrl _url = _u;
  _url.cleanPath(); // kill consecutive slashes
  _url.adjustPath(KUrl::RemoveTrailingSlash);
  QString urlStr = _url.url();

  if ( !validUrl( lister, _url ) )
    return false;

#ifdef DEBUG_CACHE
  printDebug();
#endif
  kDebug(7004) << k_funcinfo << lister << " url=" << _url
                << " keep=" << _keep << " reload=" << _reload << endl;

  if ( !_keep )
  {
    // stop any running jobs for lister
    stop( lister );

    // clear our internal list for lister
    forgetDirs( lister );

    lister->d->rootFileItem = 0;
  }
  else if ( lister->d->lstDirs.contains( _url ) )
  {
    // stop the job listing _url for this lister
    stop( lister, _url );

    // remove the _url as well, it will be added in a couple of lines again!
    // forgetDirs with three args does not do this
    // TODO: think about moving this into forgetDirs
    lister->d->lstDirs.removeAll( _url );

    // clear _url for lister
    forgetDirs( lister, _url, true );

    if ( lister->d->url == _url )
      lister->d->rootFileItem = 0;
  }

  lister->d->lstDirs.append( _url );

  if ( lister->d->url.isEmpty() || !_keep ) // set toplevel URL only if not set yet
    lister->d->url = _url;

  DirItem *itemU = itemsInUse[urlStr];
  DirItem *itemC;

  if ( !urlsCurrentlyListed[urlStr] )
  {
    // if there is an update running for _url already we get into
    // the following case - it will just be restarted by updateDirectory().

    if ( itemU )
    {
      kDebug(7004) << "listDir: Entry already in use: " << _url << endl;

      bool oldState = lister->d->complete;
      lister->d->complete = false;

      emit lister->started( _url );

      if ( !lister->d->rootFileItem && lister->d->url == _url )
        lister->d->rootFileItem = itemU->rootItem;

      lister->addNewItems( itemU->lstItems );
      lister->emitItems();

      lister->d->complete = oldState;

      emit lister->completed( _url );
      if ( lister->d->complete )
        emit lister->completed();

      // _url is already in use, so there is already an entry in urlsCurrentlyHeld
      assert( urlsCurrentlyHeld[urlStr] );
      urlsCurrentlyHeld[urlStr]->append( lister );

      if ( _reload || !itemU->complete )
        updateDirectory( _url );
    }
    else if ( !_reload && (itemC = itemsCached.take( urlStr )) )
    {
      kDebug(7004) << "listDir: Entry in cache: " << _url << endl;

      itemC->decAutoUpdate();
      itemsInUse.insert( urlStr, itemC );
      itemU = itemC;

      bool oldState = lister->d->complete;
      lister->d->complete = false;

      emit lister->started( _url );

      if ( !lister->d->rootFileItem && lister->d->url == _url )
        lister->d->rootFileItem = itemC->rootItem;

      lister->addNewItems( itemC->lstItems );
      lister->emitItems();

      lister->d->complete = oldState;

      emit lister->completed( _url );
      if ( lister->d->complete )
        emit lister->completed();

      Q_ASSERT( !urlsCurrentlyHeld[urlStr] );
      Q3PtrList<KDirLister> *list = new Q3PtrList<KDirLister>;
      list->append( lister );
      urlsCurrentlyHeld.insert( urlStr, list );

      if ( !itemC->complete )
        updateDirectory( _url );
    }
    else  // dir not in cache or _reload is true
    {
      kDebug(7004) << "listDir: Entry not in cache or reloaded: " << _url << endl;

      Q3PtrList<KDirLister> *list = new Q3PtrList<KDirLister>;
      list->append( lister );
      urlsCurrentlyListed.insert( urlStr, list );

      itemsCached.remove( urlStr );
      itemU = new DirItem( _url );
      itemsInUse.insert( urlStr, itemU );

//        // we have a limit of MAX_JOBS_PER_LISTER concurrently running jobs
//        if ( lister->numJobs() >= MAX_JOBS_PER_LISTER )
//        {
//          lstPendingUpdates.append( _url );
//        }
//        else
//        {

      if ( lister->d->url == _url )
        lister->d->rootFileItem = 0;

      KIO::ListJob* job = KIO::listDir( _url, false /* no default GUI */ );
      jobs.insert( job, KIO::UDSEntryList() );

      lister->jobStarted( job );
      lister->connectJob( job );

      if ( lister->d->window )
        job->ui()->setWindow( lister->d->window );

      connect( job, SIGNAL( entries( KIO::Job *, const KIO::UDSEntryList & ) ),
               this, SLOT( slotEntries( KIO::Job *, const KIO::UDSEntryList & ) ) );
      connect( job, SIGNAL( result( KJob * ) ),
               this, SLOT( slotResult( KJob * ) ) );
      connect( job, SIGNAL( redirection( KIO::Job *, const KUrl & ) ),
               this, SLOT( slotRedirection( KIO::Job *, const KUrl & ) ) );

      emit lister->started( _url );

//        }
    }
  }
  else
  {
    kDebug(7004) << "listDir: Entry currently being listed: " << _url << endl;

    emit lister->started( _url );

    urlsCurrentlyListed[urlStr]->append( lister );

    KIO::ListJob *job = jobForUrl( urlStr );
    Q_ASSERT( job );

    lister->jobStarted( job );
    lister->connectJob( job );

    Q_ASSERT( itemU );

    if ( !lister->d->rootFileItem && lister->d->url == _url )
      lister->d->rootFileItem = itemU->rootItem;

    lister->addNewItems( itemU->lstItems );
    lister->emitItems();
  }

  // automatic updating of directories
  if ( lister->d->autoUpdate )
    itemU->incAutoUpdate();

  return true;
}

bool KDirListerCache::validUrl( const KDirLister *lister, const KUrl& url ) const
{
  if ( !url.isValid() )
  {
    if ( lister->d->autoErrorHandling )
    {
      QString tmp = i18n("Malformed URL\n%1",  url.prettyUrl() );
      KMessageBox::error( lister->d->errorParent, tmp );
    }
    return false;
  }

  if ( !KProtocolManager::supportsListing( url ) )
  {
    if ( lister->d->autoErrorHandling )
    {
      QString tmp = i18n("URL cannot be listed\n%1",  url.prettyUrl() );
      KMessageBox::error( lister->d->errorParent, tmp );
    }
    return false;
  }

  return true;
}

void KDirListerCache::stop( KDirLister *lister )
{
#ifdef DEBUG_CACHE
  printDebug();
#endif
  kDebug(7004) << k_funcinfo << "lister: " << lister << endl;
  bool stopped = false;

  Q3DictIterator< Q3PtrList<KDirLister> > it( urlsCurrentlyListed );
  Q3PtrList<KDirLister> *listers;
  while ( (listers = it.current()) )
  {
    if ( listers->findRef( lister ) > -1 )
    {
      // lister is listing url
      QString url = it.currentKey();

      //kDebug(7004) << k_funcinfo << " found lister in list - for " << url << endl;
      bool ret = listers->removeRef( lister );
      Q_ASSERT( ret );

      KIO::ListJob *job = jobForUrl( url );
      if ( job )
        lister->jobDone( job );

      // move lister to urlsCurrentlyHeld
      Q3PtrList<KDirLister> *holders = urlsCurrentlyHeld[url];
      if ( !holders )
      {
        holders = new Q3PtrList<KDirLister>;
        urlsCurrentlyHeld.insert( url, holders );
      }

      holders->append( lister );

      emit lister->canceled( KUrl( url ) );

      //kDebug(7004) << k_funcinfo << "remaining list: " << listers->count() << " listers" << endl;

      if ( listers->isEmpty() )
      {
        // kill the job since it isn't used any more
        if ( job )
          killJob( job );

        urlsCurrentlyListed.remove( url );
      }

      stopped = true;
    }
    else
      ++it;
  }

  if ( stopped )
  {
    emit lister->canceled();
    lister->d->complete = true;
  }

  // this is wrong if there is still an update running!
  //Q_ASSERT( lister->d->complete );
}

void KDirListerCache::stop( KDirLister *lister, const KUrl& _u )
{
  QString urlStr( _u.url(KUrl::RemoveTrailingSlash) );
  KUrl _url( urlStr );

  // TODO: consider to stop all the "child jobs" of _url as well
  kDebug(7004) << k_funcinfo << lister << " url=" << _url << endl;

  Q3PtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr];
  if ( !listers || !listers->removeRef( lister ) )
    return;

  // move lister to urlsCurrentlyHeld
  Q3PtrList<KDirLister> *holders = urlsCurrentlyHeld[urlStr];
  if ( !holders )
  {
    holders = new Q3PtrList<KDirLister>;
    urlsCurrentlyHeld.insert( urlStr, holders );
  }

  holders->append( lister );


  KIO::ListJob *job = jobForUrl( urlStr );
  if ( job )
    lister->jobDone( job );

  emit lister->canceled( _url );

  if ( listers->isEmpty() )
  {
    // kill the job since it isn't used any more
    if ( job )
      killJob( job );

    urlsCurrentlyListed.remove( urlStr );
  }

  if ( lister->numJobs() == 0 )
  {
    lister->d->complete = true;

    // we killed the last job for lister
    emit lister->canceled();
  }
}

void KDirListerCache::setAutoUpdate( KDirLister *lister, bool enable )
{
  // IMPORTANT: this method does not check for the current autoUpdate state!

  for ( KUrl::List::Iterator it = lister->d->lstDirs.begin();
        it != lister->d->lstDirs.end(); ++it )
  {
    if ( enable )
      itemsInUse[(*it).url()]->incAutoUpdate();
    else
      itemsInUse[(*it).url()]->decAutoUpdate();
  }
}

void KDirListerCache::forgetDirs( KDirLister *lister )
{
  kDebug(7004) << k_funcinfo << lister << endl;

  emit lister->clear();
  // clear lister->d->lstDirs before calling forgetDirs(), so that
  // it doesn't contain things that itemsInUse doesn't. When emitting
  // the canceled signals, lstDirs must not contain anything that
  // itemsInUse does not contain. (otherwise it might crash in findByName()).
  KUrl::List lstDirsCopy = lister->d->lstDirs;
  lister->d->lstDirs.clear();

  for ( KUrl::List::Iterator it = lstDirsCopy.begin();
        it != lstDirsCopy.end(); ++it )
  {
    forgetDirs( lister, *it, false );
  }
}

void KDirListerCache::forgetDirs( KDirLister *lister, const KUrl& _url, bool notify )
{
  kDebug(7004) << k_funcinfo << lister << " _url: " << _url << endl;

  KUrl url( _url );
  url.adjustPath( KUrl::RemoveTrailingSlash );
  QString urlStr = url.url();
  Q3PtrList<KDirLister> *holders = urlsCurrentlyHeld[urlStr];
  Q_ASSERT( holders );
  holders->removeRef( lister );

  DirItem *item = itemsInUse[urlStr];
  Q_ASSERT( item );

  if ( holders->isEmpty() )
  {
    urlsCurrentlyHeld.remove( urlStr ); // this deletes the (empty) holders list
    if ( !urlsCurrentlyListed[urlStr] )
    {
      // item not in use anymore -> move into cache if complete
      itemsInUse.remove( urlStr );

      // this job is a running update
      KIO::ListJob *job = jobForUrl( urlStr );
      if ( job )
      {
        lister->jobDone( job );
        killJob( job );
        kDebug(7004) << k_funcinfo << "Killing update job for " << urlStr << endl;

        emit lister->canceled( url );
        if ( lister->numJobs() == 0 )
        {
          lister->d->complete = true;
          emit lister->canceled();
        }
      }

      if ( notify )
      {
        lister->d->lstDirs.removeAll( url );
        emit lister->clear( url );
      }

      if ( item->complete )
      {
        kDebug(7004) << k_funcinfo << lister << " item moved into cache: " << url << endl;
        itemsCached.insert( urlStr, item ); // TODO: may return false!!

        // Should we forget the dir for good, or keep a watch on it?
        // Generally keep a watch, except when it would prevent
        // unmounting a removable device (#37780)
        const bool isLocal = item->url.isLocalFile();
        const bool isManuallyMounted = isLocal && KIO::manually_mounted( item->url.path() );
        bool containsManuallyMounted = false;
        if ( !isManuallyMounted && isLocal )
        {
          // Look for a manually-mounted directory inside
          // If there's one, we can't keep a watch either, FAM would prevent unmounting the CDROM
          // I hope this isn't too slow (manually_mounted caches the last device so most
          // of the time this is just a stat per subdir)
          KFileItemList::const_iterator kit = item->lstItems.begin();
          const KFileItemList::const_iterator kend = item->lstItems.end();
          for ( ; kit != kend && !containsManuallyMounted; ++kit )
            if ( (*kit)->isDir() && KIO::manually_mounted( (*kit)->url().path() ) )
              containsManuallyMounted = true;
        }

        if ( isManuallyMounted || containsManuallyMounted )
        {
          kDebug(7004) << "Not adding a watch on " << item->url << " because it " <<
            ( isManuallyMounted ? "is manually mounted" : "contains a manually mounted subdir" ) << endl;
          item->complete = false; // set to "dirty"
        }
        else
          item->incAutoUpdate(); // keep watch
      }
      else
      {
        delete item;
        item = 0;
      }
    }
  }

  if ( item && lister->d->autoUpdate )
    item->decAutoUpdate();
}

void KDirListerCache::updateDirectory( const KUrl& _dir )
{
  kDebug(7004) << k_funcinfo << _dir << endl;

  QString urlStr = _dir.url(KUrl::RemoveTrailingSlash);
  if ( !checkUpdate( urlStr ) )
    return;

  // A job can be running to
  //   - only list a new directory: the listers are in urlsCurrentlyListed
  //   - only update a directory: the listers are in urlsCurrentlyHeld
  //   - update a currently running listing: the listers are in urlsCurrentlyListed
  //     and urlsCurrentlyHeld

  Q3PtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr];
  Q3PtrList<KDirLister> *holders = urlsCurrentlyHeld[urlStr];

  // restart the job for _dir if it is running already
  bool killed = false;
  QWidget *window = 0;
  KIO::ListJob *job = jobForUrl( urlStr );
  if ( job )
  {
     window = job->ui()->window();

     killJob( job );
     killed = true;

     if ( listers )
        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
           kdl->jobDone( job );

     if ( holders )
        for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
           kdl->jobDone( job );
  }
  kDebug(7004) << k_funcinfo << "Killed = " << killed << endl;

  // we don't need to emit canceled signals since we only replaced the job,
  // the listing is continuing.

  Q_ASSERT( !listers || (listers && killed) );

  job = KIO::listDir( _dir, false /* no default GUI */ );
  jobs.insert( job, KIO::UDSEntryList() );

  connect( job, SIGNAL(entries( KIO::Job *, const KIO::UDSEntryList & )),
           this, SLOT(slotUpdateEntries( KIO::Job *, const KIO::UDSEntryList & )) );
  connect( job, SIGNAL(result( KJob * )),
           this, SLOT(slotUpdateResult( KJob * )) );

  kDebug(7004) << k_funcinfo << "update started in " << _dir << endl;

  if ( listers )
     for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        kdl->jobStarted( job );

  if ( holders )
  {
     if ( !killed )
     {
        bool first = true;
        for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
        {
           kdl->jobStarted( job );
           if ( first && kdl->d->window )
           {
              first = false;
              job->ui()->setWindow( kdl->d->window );
           }
           emit kdl->started( _dir );
        }
     }
     else
     {
        job->ui()->setWindow( window );

        for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
           kdl->jobStarted( job );
     }
  }
}

bool KDirListerCache::checkUpdate( const QString& _dir )
{
  if ( !itemsInUse[_dir] )
  {
    DirItem *item = itemsCached[_dir];
    if ( item && item->complete )
    {
      item->complete = false;
      item->decAutoUpdate();
      // Hmm, this debug output might include login/password from the _dir URL.
      //kDebug(7004) << k_funcinfo << "directory " << _dir << " not in use, marked dirty." << endl;
    }
    //else
      //kDebug(7004) << k_funcinfo << "aborted, directory " << _dir << " not in cache." << endl;

    return false;
  }
  else
    return true;
}

KFileItem *KDirListerCache::itemForUrl( const KUrl& url ) const
{
  return findByUrl( 0, url );
}

KFileItemList *KDirListerCache::itemsForDir( const KUrl& _dir ) const
{
  QString urlStr = _dir.url(KUrl::RemoveTrailingSlash);
  DirItem *item = itemsInUse[ urlStr ];
  if ( !item )
    item = itemsCached[ urlStr ];
  return item ? &item->lstItems : 0;
}

KFileItem *KDirListerCache::findByName( const KDirLister *lister, const QString& _name ) const
{
  Q_ASSERT( lister );

  for ( KUrl::List::Iterator it = lister->d->lstDirs.begin();
        it != lister->d->lstDirs.end(); ++it )
  {
    KFileItem* item = itemsInUse[(*it).url()]->lstItems.findByName( _name );
    if ( item )
      return item;
  }

  return 0;
}

KFileItem *KDirListerCache::findByUrl( const KDirLister *lister, const KUrl& _u ) const
{
  KUrl _url = _u;
  _url.adjustPath(KUrl::RemoveTrailingSlash);

  KUrl parentDir( _url );
  parentDir.setPath( parentDir.directory() );

  // If lister is set, check that it contains this dir
  if ( lister && !lister->d->lstDirs.contains( parentDir ) )
      return 0;

  const KFileItemList *itemList = itemsForDir( parentDir );
  if ( itemList )
    return itemList->findByUrl( _url );
  return 0;
}

void KDirListerCache::slotFilesAdded( const QString &dir ) // from KDirNotify signals
{
  kDebug(7004) << k_funcinfo << dir << endl;
  updateDirectory( KUrl(dir) );
}

void KDirListerCache::slotFilesRemoved( const QStringList &fileList ) // from KDirNotify signals
{
  kDebug(7004) << k_funcinfo << endl;
  QStringList::ConstIterator it = fileList.begin();
  for ( ; it != fileList.end() ; ++it )
  {
    // emit the deleteItem signal if this file was shown in any view
    KFileItem *fileitem = 0L;
    KUrl url( *it );
    KUrl parentDir( url );
    parentDir.setPath( parentDir.directory() );
    KFileItemList *lstItems = itemsForDir( parentDir );
    if ( lstItems )
    {
      for ( KFileItemList::iterator fit = lstItems->begin(), fend = lstItems->end() ; fit != fend ; ++fit ) {
        if ( (*fit )->url() == url ) {
          fileitem = *fit;
          lstItems->erase( fit ); // remove fileitem from list
          break;
        }
      }
    }

    // Tell the views about it before deleting the KFileItems. They might need the subdirs'
    // file items (see the dirtree).
    if ( fileitem )
    {
      Q3PtrList<KDirLister> *listers = urlsCurrentlyHeld[parentDir.url()];
      if ( listers )
        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
          kdl->emitDeleteItem( fileitem );
    }

    // If we found a fileitem, we can test if it's a dir. If not, we'll go to deleteDir just in case.
    if ( !fileitem || fileitem->isDir() )
    {
      // in case of a dir, check if we have any known children, there's much to do in that case
      // (stopping jobs, removing dirs from cache etc.)
      deleteDir( url );
    }

    // now remove the item itself
    delete fileitem;
  }
}

void KDirListerCache::slotFilesChanged( const QStringList &fileList ) // from KDirNotify signals
{
  KUrl::List dirsToUpdate;
  kDebug(7004) << k_funcinfo << "only half implemented" << endl;
  QStringList::ConstIterator it = fileList.begin();
  for ( ; it != fileList.end() ; ++it )
  {
    KUrl url( *it );
    if ( url.isLocalFile() )
    {
      kDebug(7004) << "KDirListerCache::slotFilesChanged " << url << endl;
      KFileItem *fileitem = findByUrl( 0, url );
      if ( fileitem )
      {
          // we need to refresh the item, because e.g. the permissions can have changed.
          aboutToRefreshItem( fileitem );
          fileitem->refresh();
          emitRefreshItem( fileitem );
      }
      else
          kDebug(7004) << "item not found" << endl;
    } else {
      // For remote files, refresh() won't be able to figure out the new information.
      // Let's update the dir.
      KUrl dir( url );
      dir.setPath( dir.directory() );
      if ( !dirsToUpdate.contains( dir ) )
        dirsToUpdate.prepend( dir );
    }
  }

  KUrl::List::ConstIterator itdir = dirsToUpdate.begin();
  for ( ; itdir != dirsToUpdate.end() ; ++itdir )
    updateDirectory( *itdir );
  // ## TODO problems with current jobs listing/updating that dir
  // ( see kde-2.2.2's kdirlister )
}

void KDirListerCache::slotFileRenamed( const QString &_src, const QString &_dst ) // from KDirNotify signals
{
  KUrl src( _src );
  KUrl dst( _dst );
  kDebug(7004) << k_funcinfo << src.prettyUrl() << " -> " << dst.prettyUrl() << endl;
#ifdef DEBUG_CACHE
  printDebug();
#endif

  // Somehow this should only be called if src is a dir. But how could we know if it is?
  // (Note that looking into itemsInUse isn't good enough. One could rename a subdir in a view.)
  renameDir( src, dst );

  // Now update the KFileItem representing that file or dir (not exclusive with the above!)
  KUrl oldurl( src );
  oldurl.adjustPath( KUrl::RemoveTrailingSlash );
  KFileItem *fileitem = findByUrl( 0, oldurl );
  if ( fileitem )
  {
    if ( !fileitem->isLocalFile() && !fileitem->localPath().isEmpty() ) // it uses UDS_LOCAL_PATH? ouch, needs an update then
        slotFilesChanged( QStringList() << src.url() );
    else
    {
        aboutToRefreshItem( fileitem );
        fileitem->setUrl( dst );
        fileitem->refreshMimeType();
        emitRefreshItem( fileitem );
    }
  }
#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::aboutToRefreshItem( KFileItem *fileitem )
{
  // Look whether this item was shown in any view, i.e. held by any dirlister
  KUrl parentDir( fileitem->url() );
  parentDir.setPath( parentDir.directory() );
  QString parentDirURL = parentDir.url();
  Q3PtrList<KDirLister> *listers = urlsCurrentlyHeld[parentDirURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
      kdl->aboutToRefreshItem( fileitem );

  // Also look in urlsCurrentlyListed, in case the user manages to rename during a listing
  listers = urlsCurrentlyListed[parentDirURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
      kdl->aboutToRefreshItem( fileitem );
}

void KDirListerCache::emitRefreshItem( KFileItem *fileitem )
{
  // Look whether this item was shown in any view, i.e. held by any dirlister
  KUrl parentDir( fileitem->url() );
  parentDir.setPath( parentDir.directory() );
  QString parentDirURL = parentDir.url();
  Q3PtrList<KDirLister> *listers = urlsCurrentlyHeld[parentDirURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->addRefreshItem( fileitem );
      kdl->emitItems();
    }

  // Also look in urlsCurrentlyListed, in case the user manages to rename during a listing
  listers = urlsCurrentlyListed[parentDirURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->addRefreshItem( fileitem );
      kdl->emitItems();
    }
}

KDirListerCache* KDirListerCache::self()
{
  if ( !s_pSelf )
    s_pSelf = sd_KDirListerCache.setObject( s_pSelf, new KDirListerCache );

  return s_pSelf;
}

// private slots

// _file can also be a directory being currently held!
void KDirListerCache::slotFileDirty( const QString& _file )
{
  kDebug(7004) << k_funcinfo << _file << endl;

  if ( !pendingUpdates[_file] )
  {
    KUrl dir;
    dir.setPath( _file );
    if ( checkUpdate( dir.url(KUrl::RemoveTrailingSlash) ) )
      updateDirectory( dir );

    // the parent directory of _file
    dir.setPath( dir.directory() );
    if ( checkUpdate( dir.url() ) )
    {
      QTimer *timer = new QTimer( this );
      // Nice hack to save memory: use the qt object name to store the filename
      timer->setObjectName( _file );
      timer->setSingleShot( true );
      connect( timer, SIGNAL(timeout()), this, SLOT(slotFileDirtyDelayed()) );
      pendingUpdates.insert( _file, timer );
      timer->start( 500 );
    }
  }
}

// delayed updating of files, FAM is flooding us with events
void KDirListerCache::slotFileDirtyDelayed()
{
  QString file = sender()->objectName(); // file name stored as timer object name

  kDebug(7004) << k_funcinfo << file << endl;

  // TODO: do it better: don't always create/delete the QTimer but reuse it.
  // Delete the timer after the parent directory is removed from the cache.
  pendingUpdates.remove( file );

  KUrl u;
  u.setPath( file );
  KFileItem *item = findByUrl( 0, u ); // search all items
  if ( item )
  {
    // we need to refresh the item, because e.g. the permissions can have changed.
    aboutToRefreshItem( item );
    item->refresh();
    emitRefreshItem( item );
  }
}

void KDirListerCache::slotFileCreated( const QString& _file )
{
  kDebug(7004) << k_funcinfo << _file << endl;
  // XXX: how to avoid a complete rescan here?
  KUrl u;
  u.setPath( _file );
  u.setPath( u.directory() );
  updateDirectory( u );
}

void KDirListerCache::slotFileDeleted( const QString& _file ) // from KDirWatch
{
  kDebug(7004) << k_funcinfo << _file << endl;
  KUrl u;
  u.setPath( _file );
  slotFilesRemoved( QStringList() << u.url() );
}

void KDirListerCache::slotEntries( KIO::Job *job, const KIO::UDSEntryList &entries )
{
  KUrl url = joburl( static_cast<KIO::ListJob *>(job) );
  url.adjustPath(KUrl::RemoveTrailingSlash);
  QString urlStr = url.url();

  kDebug(7004) << k_funcinfo << "new entries for " << url << endl;

  DirItem *dir = itemsInUse[urlStr];
  Q_ASSERT( dir );

  Q3PtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr];
  Q_ASSERT( listers );
  Q_ASSERT( !listers->isEmpty() );

  // check if anyone wants the mimetypes immediately
  bool delayedMimeTypes = true;
  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    delayedMimeTypes &= kdl->d->delayedMimeTypes;

  KIO::UDSEntryList::ConstIterator it = entries.begin();
  KIO::UDSEntryList::ConstIterator end = entries.end();

  for ( ; it != end; ++it )
  {
    const QString name = (*it).stringValue( KIO::UDS_NAME );

    Q_ASSERT( !name.isEmpty() );
    if ( name.isEmpty() )
      continue;

    if ( name == "." )
    {
      Q_ASSERT( !dir->rootItem );
      dir->rootItem = new KFileItem( *it, url, delayedMimeTypes, true  );

      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        if ( !kdl->d->rootFileItem && kdl->d->url == url )
          kdl->d->rootFileItem = dir->rootItem;
    }
    else if ( name != ".." )
    {
      KFileItem* item = new KFileItem( *it, url, delayedMimeTypes, true );
      Q_ASSERT( item );

      //kDebug(7004)<< "Adding item: " << item->url() << endl;
      dir->lstItems.append( item );

      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        kdl->addNewItem( item );
    }
  }

  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    kdl->emitItems();
}

void KDirListerCache::slotResult( KJob *j )
{
  Q_ASSERT( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );
  jobs.remove( job );

  KUrl jobUrl = joburl( job );
  jobUrl.adjustPath(KUrl::RemoveTrailingSlash);  // need remove trailing slashes again, in case of redirections
  QString jobUrlStr = jobUrl.url();

  kDebug(7004) << k_funcinfo << "finished listing " << jobUrl << endl;
#ifdef DEBUG_CACHE
  printDebug();
#endif

  Q3PtrList<KDirLister> *listers = urlsCurrentlyListed.take( jobUrlStr );
  Q_ASSERT( listers );

  // move the directory to the held directories, do it before emitting
  // the signals to make sure it exists in KDirListerCache in case someone
  // calls listDir during the signal emission
  Q_ASSERT( !urlsCurrentlyHeld[jobUrlStr] );
  urlsCurrentlyHeld.insert( jobUrlStr, listers );

  KDirLister *kdl;

  if ( job->error() )
  {
    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->jobDone( job );
      kdl->handleError( job );
      emit kdl->canceled( jobUrl );
      if ( kdl->numJobs() == 0 )
      {
        kdl->d->complete = true;
        emit kdl->canceled();
      }
    }
  }
  else
  {
    DirItem *dir = itemsInUse[jobUrlStr];
    Q_ASSERT( dir );
    dir->complete = true;

    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->jobDone( job );
      emit kdl->completed( jobUrl );
      if ( kdl->numJobs() == 0 )
      {
        kdl->d->complete = true;
        emit kdl->completed();
      }
    }
  }

  // TODO: hmm, if there was an error and job is a parent of one or more
  // of the pending urls we should cancel it/them as well
  processPendingUpdates();

#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::slotRedirection( KIO::Job *j, const KUrl& url )
{
  Q_ASSERT( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );

  KUrl oldUrl = job->url();  // here we really need the old url!
  KUrl newUrl = url;

  // strip trailing slashes
  oldUrl.adjustPath(KUrl::RemoveTrailingSlash);
  newUrl.adjustPath(KUrl::RemoveTrailingSlash);

  if ( oldUrl == newUrl )
  {
    kDebug(7004) << k_funcinfo << "New redirection url same as old, giving up." << endl;
    return;
  }

  kDebug(7004) << k_funcinfo << oldUrl.prettyUrl() << " -> " << newUrl.prettyUrl() << endl;

#ifdef DEBUG_CACHE
  printDebug();
#endif

  // I don't think there can be dirItems that are children of oldUrl.
  // Am I wrong here? And even if so, we don't need to delete them, right?
  // DF: redirection happens before listDir emits any item. Makes little sense otherwise.

  // oldUrl cannot be in itemsCached because only completed items are moved there
  DirItem *dir = itemsInUse.take( oldUrl.url() );
  Q_ASSERT( dir );

  Q3PtrList<KDirLister> *listers = urlsCurrentlyListed.take( oldUrl.url() );
  Q_ASSERT( listers );
  Q_ASSERT( !listers->isEmpty() );

  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
  {
    // TODO: put in own method?
    if ( kdl->d->url.equals( oldUrl, KUrl::CompareWithoutTrailingSlash ) )
    {
      kdl->d->rootFileItem = 0;
      kdl->d->url = newUrl;
    }

    KUrl::List& lstDirs = kdl->d->lstDirs;
    lstDirs[ lstDirs.indexOf( oldUrl ) ] = newUrl;

    if ( lstDirs.count() == 1 )
    {
      emit kdl->clear();
      emit kdl->redirection( newUrl );
      emit kdl->redirection( oldUrl, newUrl );
    }
    else
    {
      emit kdl->clear( oldUrl );
      emit kdl->redirection( oldUrl, newUrl );
    }
  }

  // when a lister was stopped before the job emits the redirection signal, the old url will
  // also be in urlsCurrentlyHeld
  Q3PtrList<KDirLister> *holders = urlsCurrentlyHeld.take( oldUrl.url() );
  if ( holders )
  {
    Q_ASSERT( !holders->isEmpty() );

    for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
    {
      kdl->jobStarted( job );

      // do it like when starting a new list-job that will redirect later
      emit kdl->started( oldUrl );

      // TODO: maybe don't emit started if there's an update running for newUrl already?

      if ( kdl->d->url.equals( oldUrl, KUrl::CompareWithoutTrailingSlash ) )
      {
        kdl->d->rootFileItem = 0;
        kdl->d->url = newUrl;
      }

      KUrl::List& lstDirs = kdl->d->lstDirs;
      lstDirs[ lstDirs.indexOf( oldUrl ) ] = newUrl;

      if ( kdl->d->lstDirs.count() == 1 )
      {
        emit kdl->clear();
        emit kdl->redirection( newUrl );
        emit kdl->redirection( oldUrl, newUrl );
      }
      else
      {
        emit kdl->clear( oldUrl );
        emit kdl->redirection( oldUrl, newUrl );
      }
    }
  }

  DirItem *newDir = itemsInUse[newUrl.url()];
  if ( newDir )
  {
    kDebug(7004) << "slotRedirection: " << newUrl.url() << " already in use" << endl;

    // only in this case there can newUrl already be in urlsCurrentlyListed or urlsCurrentlyHeld
    delete dir;

    // get the job if one's running for newUrl already (can be a list-job or an update-job), but
    // do not return this 'job', which would happen because of the use of redirectionURL()
    KIO::ListJob *oldJob = jobForUrl( newUrl.url(), job );

    // listers of newUrl with oldJob: forget about the oldJob and use the already running one
    // which will be converted to an updateJob
    Q3PtrList<KDirLister> *curListers = urlsCurrentlyListed[newUrl.url()];
    if ( curListers )
    {
      kDebug(7004) << "slotRedirection: and it is currently listed" << endl;

      Q_ASSERT( oldJob );  // ?!

      for ( KDirLister *kdl = curListers->first(); kdl; kdl = curListers->next() )  // listers of newUrl
      {
        kdl->jobDone( oldJob );

        kdl->jobStarted( job );
        kdl->connectJob( job );
      }

      // append listers of oldUrl with newJob to listers of newUrl with oldJob
      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        curListers->append( kdl );
    }
    else
      urlsCurrentlyListed.insert( newUrl.url(), listers );

    if ( oldJob )         // kill the old job, be it a list-job or an update-job
      killJob( oldJob );

    // holders of newUrl: use the already running job which will be converted to an updateJob
    Q3PtrList<KDirLister> *curHolders = urlsCurrentlyHeld[newUrl.url()];
    if ( curHolders )
    {
      kDebug(7004) << "slotRedirection: and it is currently held." << endl;

      for ( KDirLister *kdl = curHolders->first(); kdl; kdl = curHolders->next() )  // holders of newUrl
      {
        kdl->jobStarted( job );
        emit kdl->started( newUrl );
      }

      // append holders of oldUrl to holders of newUrl
      if ( holders )
        for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
          curHolders->append( kdl );
    }
    else if ( holders )
      urlsCurrentlyHeld.insert( newUrl.url(), holders );


    // emit old items: listers, holders. NOT: newUrlListers/newUrlHolders, they already have them listed
    // TODO: make this a separate method?
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      if ( !kdl->d->rootFileItem && kdl->d->url == newUrl )
        kdl->d->rootFileItem = newDir->rootItem;

      kdl->addNewItems( newDir->lstItems );
      kdl->emitItems();
    }

    if ( holders )
    {
      for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
      {
        if ( !kdl->d->rootFileItem && kdl->d->url == newUrl )
          kdl->d->rootFileItem = newDir->rootItem;

        kdl->addNewItems( newDir->lstItems );
        kdl->emitItems();
      }
    }
  }
  else if ( (newDir = itemsCached.take( newUrl.url() )) )
  {
    kDebug(7004) << "slotRedirection: " << newUrl.url() << " is unused, but already in the cache." << endl;

    delete dir;
    itemsInUse.insert( newUrl.url(), newDir );
    urlsCurrentlyListed.insert( newUrl.url(), listers );
    if ( holders )
      urlsCurrentlyHeld.insert( newUrl.url(), holders );

    // emit old items: listers, holders
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      if ( !kdl->d->rootFileItem && kdl->d->url == newUrl )
        kdl->d->rootFileItem = newDir->rootItem;

      kdl->addNewItems( newDir->lstItems );
      kdl->emitItems();
    }

    if ( holders )
    {
      for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
      {
        if ( !kdl->d->rootFileItem && kdl->d->url == newUrl )
          kdl->d->rootFileItem = newDir->rootItem;

        kdl->addNewItems( newDir->lstItems );
        kdl->emitItems();
      }
    }
  }
  else
  {
    kDebug(7004) << "slotRedirection: " << newUrl.url() << " has not been listed yet." << endl;

    delete dir->rootItem;
    dir->rootItem = 0;
    qDeleteAll( dir->lstItems );
    dir->lstItems.clear();
    dir->redirect( newUrl );
    itemsInUse.insert( newUrl.url(), dir );
    urlsCurrentlyListed.insert( newUrl.url(), listers );

    if ( holders )
      urlsCurrentlyHeld.insert( newUrl.url(), holders );
    else
    {
#ifdef DEBUG_CACHE
      printDebug();
#endif
      return; // only in this case the job doesn't need to be converted,
    }
  }

  // make the job an update job
  job->disconnect( this );

  connect( job, SIGNAL(entries( KIO::Job *, const KIO::UDSEntryList & )),
           this, SLOT(slotUpdateEntries( KIO::Job *, const KIO::UDSEntryList & )) );
  connect( job, SIGNAL(result( KJob * )),
           this, SLOT(slotUpdateResult( KJob * )) );

  // FIXME: autoUpdate-Counts!!

#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::renameDir( const KUrl &oldUrl, const KUrl &newUrl )
{
  kDebug(7004) << k_funcinfo << oldUrl.prettyUrl() << " -> " << newUrl.prettyUrl() << endl;
  QString oldUrlStr = oldUrl.url(KUrl::RemoveTrailingSlash);
  QString newUrlStr = newUrl.url(KUrl::RemoveTrailingSlash);

  // Not enough. Also need to look at any child dir, even sub-sub-sub-dir.
  //DirItem *dir = itemsInUse.take( oldUrlStr );
  //emitRedirections( oldUrl, url );

  // Look at all dirs being listed/shown
  Q3DictIterator<DirItem> itu( itemsInUse );
  bool goNext;
  while ( itu.current() )
  {
    goNext = true;
    DirItem *dir = itu.current();
    KUrl oldDirUrl ( itu.currentKey() );
    //kDebug(7004) << "itemInUse: " << oldDirUrl.prettyUrl() << endl;
    // Check if this dir is oldUrl, or a subfolder of it
    if ( oldUrl.isParentOf( oldDirUrl ) )
    {
      // TODO should use KUrl::cleanpath like isParentOf does
      QString relPath = oldDirUrl.path().mid( oldUrl.path().length() );

      KUrl newDirUrl( newUrl ); // take new base
      if ( !relPath.isEmpty() )
        newDirUrl.addPath( relPath ); // add unchanged relative path
      //kDebug(7004) << "KDirListerCache::renameDir new url=" << newDirUrl.prettyUrl() << endl;

      // Update URL in dir item and in itemsInUse
      dir->redirect( newDirUrl );
      itemsInUse.remove( itu.currentKey() ); // implies ++itu
      // TODO when porting to a Qt4 collection class: itu = itemsInUse.erase( itu );
      // but double check the iteration over all...
      itemsInUse.insert( newDirUrl.url(KUrl::RemoveTrailingSlash), dir );
      goNext = false; // because of the implied ++itu above
      // Rename all items under that dir

      for ( KFileItemList::iterator kit = dir->lstItems.begin(), kend = dir->lstItems.end() ; kit != kend ; ++kit )
      {
        KUrl oldItemUrl = (*kit)->url();
        QString oldItemUrlStr( oldItemUrl.url(KUrl::RemoveTrailingSlash) );
        KUrl newItemUrl( oldItemUrl );
        newItemUrl.setPath( newDirUrl.path() );
        newItemUrl.addPath( oldItemUrl.fileName() );
        kDebug(7004) << "KDirListerCache::renameDir renaming " << oldItemUrlStr << " to " << newItemUrl.url() << endl;
        (*kit)->setUrl( newItemUrl );
      }
      emitRedirections( oldDirUrl, newDirUrl );
    }
    if ( goNext )
      ++itu;
  }

  // Is oldUrl a directory in the cache?
  // Remove any child of oldUrl from the cache - even if the renamed dir itself isn't in it!
  removeDirFromCache( oldUrl );
  // TODO rename, instead.
}

void KDirListerCache::emitRedirections( const KUrl &oldUrl, const KUrl &url )
{
  kDebug(7004) << k_funcinfo << oldUrl.prettyUrl() << " -> " << url.prettyUrl() << endl;
  QString oldUrlStr = oldUrl.url(KUrl::RemoveTrailingSlash);
  QString urlStr = url.url(KUrl::RemoveTrailingSlash);

  KIO::ListJob *job = jobForUrl( oldUrlStr );
  if ( job )
    killJob( job );

  // Check if we were listing this dir. Need to abort and restart with new name in that case.
  Q3PtrList<KDirLister> *listers = urlsCurrentlyListed.take( oldUrlStr );
  if ( listers )
  {
    // Tell the world that the job listing the old url is dead.
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      if ( job )
        kdl->jobDone( job );

      emit kdl->canceled( oldUrl );
    }

    urlsCurrentlyListed.insert( urlStr, listers );
  }

  // Check if we are currently displaying this directory (odds opposite wrt above)
  // Update urlsCurrentlyHeld dict with new URL
  Q3PtrList<KDirLister> *holders = urlsCurrentlyHeld.take( oldUrlStr );
  if ( holders )
  {
    if ( job )
      for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
        kdl->jobDone( job );

    urlsCurrentlyHeld.insert( urlStr, holders );
  }

  if ( listers )
  {
    updateDirectory( url );

    // Tell the world about the new url
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
      emit kdl->started( url );
  }

  if ( holders )
  {
    // And notify the dirlisters of the redirection
    for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
    {
      KUrl::List& lstDirs = kdl->d->lstDirs;
      lstDirs[ lstDirs.indexOf( oldUrl ) ] = url;

      if ( lstDirs.count() == 1 )
        emit kdl->redirection( url );

      emit kdl->redirection( oldUrl, url );
    }
  }
}

void KDirListerCache::removeDirFromCache( const KUrl& dir )
{
  kDebug(7004) << "KDirListerCache::removeDirFromCache " << dir.prettyUrl() << endl;
  Q3CacheIterator<DirItem> itc( itemsCached );
  while ( itc.current() )
  {
    if ( dir.isParentOf( KUrl( itc.currentKey() ) ) )
      itemsCached.remove( itc.currentKey() );
    else
      ++itc;
  }
}

void KDirListerCache::slotUpdateEntries( KIO::Job* job, const KIO::UDSEntryList& list )
{
  jobs[static_cast<KIO::ListJob*>(job)] += list;
}

void KDirListerCache::slotUpdateResult( KJob * j )
{
  Q_ASSERT( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );

  KUrl jobUrl = joburl( job );
  jobUrl.adjustPath(KUrl::RemoveTrailingSlash);  // need remove trailing slashes again, in case of redirections
  QString jobUrlStr = jobUrl.url();

  kDebug(7004) << k_funcinfo << "finished update " << jobUrl << endl;

  KDirLister *kdl;

  Q3PtrList<KDirLister> *listers = urlsCurrentlyHeld[jobUrlStr];
  Q3PtrList<KDirLister> *tmpLst = urlsCurrentlyListed.take( jobUrlStr );

  if ( tmpLst )
  {
    if ( listers )
      for ( kdl = tmpLst->first(); kdl; kdl = tmpLst->next() )
      {
        Q_ASSERT( listers->containsRef( kdl ) == 0 );
        listers->append( kdl );
      }
    else
    {
      listers = tmpLst;
      urlsCurrentlyHeld.insert( jobUrlStr, listers );
    }
  }

  // once we are updating dirs that are only in the cache this will fail!
  Q_ASSERT( listers );

  if ( job->error() )
  {
    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->jobDone( job );

      //don't bother the user
      //kdl->handleError( job );

      emit kdl->canceled( jobUrl );
      if ( kdl->numJobs() == 0 )
      {
        kdl->d->complete = true;
        emit kdl->canceled();
      }
    }

    jobs.remove( job );

    // TODO: if job is a parent of one or more
    // of the pending urls we should cancel them
    processPendingUpdates();
    return;
  }

  DirItem *dir = itemsInUse[jobUrlStr];
  dir->complete = true;


  // check if anyone wants the mimetypes immediately
  bool delayedMimeTypes = true;
  for ( kdl = listers->first(); kdl; kdl = listers->next() )
    delayedMimeTypes &= kdl->d->delayedMimeTypes;

  QHash<QString, KFileItem *> fileItems;

  // Unmark all items in url
  for ( KFileItemList::iterator kit = dir->lstItems.begin(), kend = dir->lstItems.end() ; kit != kend ; ++kit )
  {
    (*kit)->unmark();
    fileItems.insert( (*kit)->url().url(), *kit );
  }

  KFileItem *tmp;

  KIO::UDSEntryList buf = jobs.value( job );
  KIO::UDSEntryList::ConstIterator it = buf.begin();
  const KIO::UDSEntryList::ConstIterator end = buf.end();
  for ( ; it != end; ++it )
  {
    // Form the complete url
    KFileItem item( *it, jobUrl, delayedMimeTypes, true );

    const QString name = item.name();
    Q_ASSERT( !name.isEmpty() );

    // we duplicate the check for dotdot here, to avoid iterating over
    // all items again and checking in matchesFilter() that way.
    if ( name.isEmpty() || name == ".." )
      continue;

    if ( name == "." )
    {
      // if the update was started before finishing the original listing
      // there is no root item yet
      if ( !dir->rootItem )
      {
        dir->rootItem = new KFileItem(item);

        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
          if ( !kdl->d->rootFileItem && kdl->d->url == jobUrl )
            kdl->d->rootFileItem = dir->rootItem;
      }
      continue;
    }

    // Find this item
    if ( (tmp = fileItems.value(item.url().url())) )
    {
      // check if something changed for this file
      if ( !tmp->cmp( item ) )
      {
        for ( kdl = listers->first(); kdl; kdl = listers->next() )
          kdl->aboutToRefreshItem( tmp );

        //kDebug(7004) << "slotUpdateResult: file changed: " << tmp->name() << endl;
        *tmp = item;

        for ( kdl = listers->first(); kdl; kdl = listers->next() )
          kdl->addRefreshItem( tmp );
      }
      tmp->mark();
    }
    else // this is a new file
    {
      //kDebug(7004) << "slotUpdateResult: new file: " << name << endl;

      KFileItem* pitem = new KFileItem(item); // we're not using kfileitem by value yet
      pitem->mark();
      dir->lstItems.append( pitem );

      for ( kdl = listers->first(); kdl; kdl = listers->next() )
        kdl->addNewItem( pitem );
    }
  }

  jobs.remove( job );

  deleteUnmarkedItems( listers, dir->lstItems );

  for ( kdl = listers->first(); kdl; kdl = listers->next() )
  {
    kdl->emitItems();

    kdl->jobDone( job );

    emit kdl->completed( jobUrl );
    if ( kdl->numJobs() == 0 )
    {
      kdl->d->complete = true;
      emit kdl->completed();
    }
  }

  // TODO: hmm, if there was an error and job is a parent of one or more
  // of the pending urls we should cancel it/them as well
  processPendingUpdates();
}

// private

KIO::ListJob *KDirListerCache::jobForUrl( const QString& url, KIO::ListJob *not_job )
{
  KIO::ListJob *job;
  QMap< KIO::ListJob *, KIO::UDSEntryList >::ConstIterator it = jobs.begin();
  while ( it != jobs.end() )
  {
    job = it.key();
    if ( joburl( job ).url(KUrl::RemoveTrailingSlash) == url && job != not_job )
       return job;
    ++it;
  }
  return 0;
}

const KUrl& KDirListerCache::joburl( KIO::ListJob *job )
{
  if ( job->redirectionUrl().isValid() )
     return job->redirectionUrl();
  else
     return job->url();
}

void KDirListerCache::killJob( KIO::ListJob *job )
{
  jobs.remove( job );
  job->disconnect( this );
  job->kill();
}

void KDirListerCache::deleteUnmarkedItems( Q3PtrList<KDirLister> *listers, KFileItemList &lstItems )
{
  // Find all unmarked items and delete them
  QMutableListIterator<KFileItem *> kit( lstItems );
  while ( kit.hasNext() )
  {
    KFileItem* item = kit.next();
    if ( !item->isMarked() )
    {
      //kDebug() << k_funcinfo << item->name() << endl;
      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        kdl->emitDeleteItem( item );

      if ( item->isDir() )
        deleteDir( item->url() );

      // finally actually delete the item
      delete item;
      kit.remove();
    }
  }
}

void KDirListerCache::deleteDir( const KUrl& dirUrl )
{
  //kDebug() << k_funcinfo << dirUrl.prettyUrl() << endl;
  // unregister and remove the children of the deleted item.
  // Idea: tell all the KDirListers that they should forget the dir
  //       and then remove it from the cache.

  Q3DictIterator<DirItem> itu( itemsInUse );
  while ( itu.current() )
  {
    KUrl deletedUrl( itu.currentKey() );
    if ( dirUrl.isParentOf( deletedUrl ) )
    {
      // stop all jobs for deletedUrl

      Q3PtrList<KDirLister> *kdls = urlsCurrentlyListed[deletedUrl.url()];
      if ( kdls )  // yeah, I lack good names
      {
        // we need a copy because stop modifies the list
        kdls = new Q3PtrList<KDirLister>( *kdls );
        for ( KDirLister *kdl = kdls->first(); kdl; kdl = kdls->next() )
          stop( kdl, deletedUrl );

        delete kdls;
      }

      // tell listers holding deletedUrl to forget about it
      // this will stop running updates for deletedUrl as well

      kdls = urlsCurrentlyHeld[deletedUrl.url()];
      if ( kdls )
      {
        // we need a copy because forgetDirs modifies the list
        kdls = new Q3PtrList<KDirLister>( *kdls );

        for ( KDirLister *kdl = kdls->first(); kdl; kdl = kdls->next() )
        {
          // lister's root is the deleted item
          if ( kdl->d->url == deletedUrl )
          {
            // tell the view first. It might need the subdirs' items (which forgetDirs will delete)
            if ( kdl->d->rootFileItem )
              emit kdl->deleteItem( kdl->d->rootFileItem );
            forgetDirs( kdl );
            kdl->d->rootFileItem = 0;
          }
          else
          {
            bool treeview = kdl->d->lstDirs.count() > 1;
            if ( !treeview )
            {
              emit kdl->clear();
              kdl->d->lstDirs.clear();
            }
            else
              kdl->d->lstDirs.removeAll( deletedUrl );

            forgetDirs( kdl, deletedUrl, treeview );
          }
        }

        delete kdls;
      }

      // delete the entry for deletedUrl - should not be needed, it's in
      // items cached now

      DirItem *dir = itemsInUse.take( deletedUrl.url() );
      Q_ASSERT( !dir );
      if ( !dir ) // take didn't find it - move on
          ++itu;
    }
    else
      ++itu;
  }

  // remove the children from the cache
  removeDirFromCache( dirUrl );
}

void KDirListerCache::processPendingUpdates()
{
  // TODO
}

#ifndef NDEBUG
void KDirListerCache::printDebug()
{
  kDebug(7004) << "Items in use: " << endl;
  Q3DictIterator<DirItem> itu( itemsInUse );
  for ( ; itu.current() ; ++itu ) {
      kDebug(7004) << "   " << itu.currentKey() << "  URL: " << itu.current()->url
                    << " rootItem: " << ( itu.current()->rootItem ? itu.current()->rootItem->url() : KUrl() )
                    << " autoUpdates refcount: " << itu.current()->autoUpdates
                    << " complete: " << itu.current()->complete
                    << QString(" with %1 items.").arg(itu.current()->lstItems.count()) << endl;
  }

  kDebug(7004) << "urlsCurrentlyHeld: " << endl;
  Q3DictIterator< Q3PtrList<KDirLister> > it( urlsCurrentlyHeld );
  for ( ; it.current() ; ++it )
  {
    QString list;
    for ( Q3PtrListIterator<KDirLister> listit( *it.current() ); listit.current(); ++listit )
      list += " 0x" + QString::number( (long)listit.current(), 16 );
    kDebug(7004) << "   " << it.currentKey() << "  " << it.current()->count() << " listers: " << list << endl;
  }

  kDebug(7004) << "urlsCurrentlyListed: " << endl;
  Q3DictIterator< Q3PtrList<KDirLister> > it2( urlsCurrentlyListed );
  for ( ; it2.current() ; ++it2 )
  {
    QString list;
    for ( Q3PtrListIterator<KDirLister> listit( *it2.current() ); listit.current(); ++listit )
      list += " 0x" + QString::number( (long)listit.current(), 16 );
    kDebug(7004) << "   " << it2.currentKey() << "  " << it2.current()->count() << " listers: " << list << endl;
  }

  QMap< KIO::ListJob *, KIO::UDSEntryList >::Iterator jit = jobs.begin();
  kDebug(7004) << "Jobs: " << endl;
  for ( ; jit != jobs.end() ; ++jit )
    kDebug(7004) << "   " << jit.key() << " listing " << joburl( jit.key() ).prettyUrl() << ": " << (*jit).count() << " entries." << endl;

  kDebug(7004) << "Items in cache: " << endl;
  Q3CacheIterator<DirItem> itc( itemsCached );
  for ( ; itc.current() ; ++itc )
    kDebug(7004) << "   " << itc.currentKey() << "  rootItem: "
                  << ( itc.current()->rootItem ? itc.current()->rootItem->url().prettyUrl() : QString("NULL") )
                  << QString(" with %1 items.").arg(itc.current()->lstItems.count()) << endl;
}
#endif

/*********************** -- The new KDirLister -- ************************/


KDirLister::KDirLister( QObject* parent )
    : QObject(parent), d(new KDirListerPrivate)
{
  kDebug(7003) << "+KDirLister" << endl;

  d->complete = true;

  setAutoUpdate( true );
  setDirOnlyMode( false );
  setShowingDotFiles( false );

  setAutoErrorHandlingEnabled( true, 0 );
}

KDirLister::~KDirLister()
{
  kDebug(7003) << "-KDirLister" << endl;

  // Stop all running jobs
  stop();
  s_pCache->forgetDirs( this );

  delete d;
}

bool KDirLister::openUrl( const KUrl& _url, bool _keep, bool _reload )
{
  kDebug(7003) << k_funcinfo << _url.prettyUrl()
                << " keep=" << _keep << " reload=" << _reload << endl;

  // emit the current changes made to avoid an inconsistent treeview
  if ( d->changes != NONE && _keep )
    emitChanges();

  d->changes = NONE;

  return s_pCache->listDir( this, _url, _keep, _reload );
}

void KDirLister::stop()
{
  kDebug(7003) << k_funcinfo << endl;
  s_pCache->stop( this );
}

void KDirLister::stop( const KUrl& _url )
{
  kDebug(7003) << k_funcinfo << _url.prettyUrl() << endl;
  s_pCache->stop( this, _url );
}

bool KDirLister::autoUpdate() const
{
  return d->autoUpdate;
}

void KDirLister::setAutoUpdate( bool _enable )
{
  if ( d->autoUpdate == _enable )
    return;

  d->autoUpdate = _enable;
  s_pCache->setAutoUpdate( this, _enable );
}

bool KDirLister::showingDotFiles() const
{
  return d->isShowingDotFiles;
}

void KDirLister::setShowingDotFiles( bool _showDotFiles )
{
  if ( d->isShowingDotFiles == _showDotFiles )
    return;

  d->isShowingDotFiles = _showDotFiles;
  d->changes ^= DOT_FILES;
}

bool KDirLister::dirOnlyMode() const
{
  return d->dirOnlyMode;
}

void KDirLister::setDirOnlyMode( bool _dirsOnly )
{
  if ( d->dirOnlyMode == _dirsOnly )
    return;

  d->dirOnlyMode = _dirsOnly;
  d->changes ^= DIR_ONLY_MODE;
}

bool KDirLister::autoErrorHandlingEnabled() const
{
  return d->autoErrorHandling;
}

void KDirLister::setAutoErrorHandlingEnabled( bool enable, QWidget* parent )
{
  d->autoErrorHandling = enable;
  d->errorParent = parent;
}

const KUrl& KDirLister::url() const
{
  return d->url;
}

const KUrl::List& KDirLister::directories() const
{
  return d->lstDirs;
}

void KDirLister::emitChanges()
{
  if ( d->changes == NONE )
    return;

  for ( KUrl::List::Iterator it = d->lstDirs.begin();
        it != d->lstDirs.end(); ++it )
  {
    const KFileItemList* itemList = s_pCache->itemsForDir( *it );
    KFileItemList::const_iterator kit = itemList->begin();
    const KFileItemList::const_iterator kend = itemList->end();
    for ( ; kit != kend; ++kit )
    {
      if ( (*kit)->text() == "." || (*kit)->text() == ".." )
        continue;

      bool oldMime = true, newMime = true;

      if ( d->changes & MIME_FILTER )
      {
        const QString mimetype = (*kit)->mimetype();
        oldMime = doMimeFilter( mimetype, d->oldMimeFilter )
                && doMimeExcludeFilter( mimetype, d->oldMimeExcludeFilter );
        newMime = doMimeFilter( mimetype, d->mimeFilter )
                && doMimeExcludeFilter( mimetype, d->mimeExcludeFilter );

        if ( oldMime && !newMime )
        {
          emit deleteItem( *kit );
          continue;
        }
      }

      if ( d->changes & DIR_ONLY_MODE )
      {
        // the lister switched to dirOnlyMode
        if ( d->dirOnlyMode )
        {
          if ( !(*kit)->isDir() )
            emit deleteItem( *kit );
        }
        else if ( !(*kit)->isDir() )
          addNewItem( *kit );

        continue;
      }

      if ( (*kit)->isHidden() )
      {
        if ( d->changes & DOT_FILES )
        {
          // the lister switched to dot files mode
          if ( d->isShowingDotFiles )
            addNewItem( *kit );
          else
            emit deleteItem( *kit );

          continue;
        }
      }
      else if ( d->changes & NAME_FILTER )
      {
        bool oldName = (*kit)->isDir() ||
                       d->oldFilters.isEmpty() ||
                       doNameFilter( (*kit)->text(), d->oldFilters );

        bool newName = (*kit)->isDir() ||
                       d->lstFilters.isEmpty() ||
                       doNameFilter( (*kit)->text(), d->lstFilters );

        if ( oldName && !newName )
        {
          emit deleteItem( *kit );
          continue;
        }
        else if ( !oldName && newName )
          addNewItem( *kit );
      }

      if ( (d->changes & MIME_FILTER) && !oldMime && newMime )
        addNewItem( *kit );
    }

    emitItems();
  }

  d->changes = NONE;
}

void KDirLister::updateDirectory( const KUrl& _u )
{
  s_pCache->updateDirectory( _u );
}

bool KDirLister::isFinished() const
{
  return d->complete;
}

KFileItem *KDirLister::rootItem() const
{
  return d->rootFileItem;
}

KFileItem *KDirLister::findByUrl( const KUrl& _url ) const
{
  return s_pCache->findByUrl( this, _url );
}

KFileItem *KDirLister::findByName( const QString& _name ) const
{
  return s_pCache->findByName( this, _name );
}


// ================ public filter methods ================ //

void KDirLister::setNameFilter( const QString& nameFilter )
{
  if ( !(d->changes & NAME_FILTER) )
  {
    d->oldFilters = d->lstFilters;
  }

  d->lstFilters.clear();

  d->nameFilter = nameFilter;

  // Split on white space
  const QStringList list = nameFilter.split( ' ', QString::SkipEmptyParts );
  for ( QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
    d->lstFilters.append( QRegExp(*it, Qt::CaseInsensitive, QRegExp::Wildcard ) );

  d->changes |= NAME_FILTER;
}

const QString& KDirLister::nameFilter() const
{
  return d->nameFilter;
}

void KDirLister::setMimeFilter( const QStringList& mimeFilter )
{
  if ( !(d->changes & MIME_FILTER) )
    d->oldMimeFilter = d->mimeFilter;

  if ( mimeFilter.contains("all/allfiles") ||
       mimeFilter.contains("all/all") )
    d->mimeFilter.clear();
  else
    d->mimeFilter = mimeFilter;

  d->changes |= MIME_FILTER;
}

void KDirLister::setMimeExcludeFilter( const QStringList& mimeExcludeFilter )
{
  if ( !(d->changes & MIME_FILTER) )
    d->oldMimeExcludeFilter = d->mimeExcludeFilter;

  d->mimeExcludeFilter = mimeExcludeFilter;
  d->changes |= MIME_FILTER;
}


void KDirLister::clearMimeFilter()
{
  if ( !(d->changes & MIME_FILTER) )
  {
    d->oldMimeFilter = d->mimeFilter;
    d->oldMimeExcludeFilter = d->mimeExcludeFilter;
  }
  d->mimeFilter.clear();
  d->mimeExcludeFilter.clear();
  d->changes |= MIME_FILTER;
}

const QStringList& KDirLister::mimeFilters() const
{
  return d->mimeFilter;
}

bool KDirLister::matchesFilter( const QString& name ) const
{
  return doNameFilter( name, d->lstFilters );
}

bool KDirLister::matchesMimeFilter( const QString& mime ) const
{
  return doMimeFilter( mime, d->mimeFilter ) && doMimeExcludeFilter(mime,d->mimeExcludeFilter);
}

// ================ protected methods ================ //

bool KDirLister::matchesFilter( const KFileItem *item ) const
{
  Q_ASSERT( item );

  if ( item->text() == ".." )
    return false;

  if ( !d->isShowingDotFiles && item->isHidden() )
    return false;

  if ( item->isDir() || d->lstFilters.isEmpty() )
    return true;

  return matchesFilter( item->text() );
}

bool KDirLister::matchesMimeFilter( const KFileItem *item ) const
{
  Q_ASSERT( item );
  // Don't lose time determining the mimetype if there is no filter
  if ( d->mimeFilter.isEmpty() && d->mimeExcludeFilter.isEmpty() )
      return true;
  return matchesMimeFilter( item->mimetype() );
}

bool KDirLister::doNameFilter( const QString& name, const QList<QRegExp>& filters ) const
{
  for ( QList<QRegExp>::const_iterator it = filters.begin(); it != filters.end(); ++it )
    if ( (*it).exactMatch( name ) )
      return true;

  return false;
}

bool KDirLister::doMimeFilter( const QString& mime, const QStringList& filters ) const
{
  if ( filters.isEmpty() )
    return true;

  KMimeType::Ptr mimeptr = KMimeType::mimeType(mime);
  //kDebug(7004) << "doMimeFilter: investigating: "<<mimeptr->name()<<endl;
  QStringList::ConstIterator it = filters.begin();
  for ( ; it != filters.end(); ++it )
    if ( mimeptr->is(*it) )
      return true;
    //else   kDebug(7004) << "doMimeFilter: compared without result to  "<<*it<<endl;


  return false;
}

bool KDirLister::doMimeExcludeFilter( const QString& mime, const QStringList& filters ) const
{
  if ( filters.isEmpty() )
    return true;

  QStringList::ConstIterator it = filters.begin();
  for ( ; it != filters.end(); ++it )
    if ( (*it) == mime )
      return false;

  return true;
}

void KDirLister::handleError( KIO::Job *job )
{
  if ( d->autoErrorHandling )
    job->uiDelegate()->showErrorMessage();
}


// ================= private methods ================= //

void KDirLister::addNewItem( KFileItem *item )
{
  if ( ( d->dirOnlyMode && !item->isDir() ) || !matchesFilter( item ) )
    return; // No reason to continue... bailing out here prevents a mimetype scan.

  if ( matchesMimeFilter( item ) )
  {
    if ( !d->lstNewItems )
      d->lstNewItems = new KFileItemList;

    d->lstNewItems->append( item );            // items not filtered
  }
  else
  {
    if ( !d->lstMimeFilteredItems )
      d->lstMimeFilteredItems = new KFileItemList;

    d->lstMimeFilteredItems->append( item );   // only filtered by mime
  }
}

void KDirLister::addNewItems( const KFileItemList& items )
{
  // TODO: make this faster - test if we have a filter at all first
  // DF: was this profiled? The matchesFoo() functions should be fast, w/o filters...
  // Of course if there is no filter and we can do a range-insertion instead of a loop, that might be good.
  // But that's for Qt4, not possible with QPtrList.
  KFileItemList::const_iterator kit = items.begin();
  const KFileItemList::const_iterator kend = items.end();
  for ( ; kit != kend; ++kit )
    addNewItem( *kit );
}

void KDirLister::aboutToRefreshItem( const KFileItem *item )
{
  // The code here follows the logic in addNewItem
  if ( ( d->dirOnlyMode && !item->isDir() ) || !matchesFilter( item ) )
    d->refreshItemWasFiltered = true;
  else if ( !matchesMimeFilter( item ) )
    d->refreshItemWasFiltered = true;
  else
    d->refreshItemWasFiltered = false;
}

void KDirLister::addRefreshItem( KFileItem *item )
{
  bool isExcluded = (d->dirOnlyMode && !item->isDir()) || !matchesFilter( item );

  if ( !isExcluded && matchesMimeFilter( item ) )
  {
    if ( d->refreshItemWasFiltered )
    {
      if ( !d->lstNewItems )
        d->lstNewItems = new KFileItemList;

      d->lstNewItems->append( item );
    }
    else
    {
      if ( !d->lstRefreshItems )
        d->lstRefreshItems = new KFileItemList;

      d->lstRefreshItems->append( item );
    }
  }
  else if ( !d->refreshItemWasFiltered )
  {
    if ( !d->lstRemoveItems )
      d->lstRemoveItems = new KFileItemList;

    // notify the user that the mimetype of a file changed that doesn't match
    // a filter or does match an exclude filter
    d->lstRemoveItems->append( item );
  }
}

void KDirLister::emitItems()
{
  KFileItemList *tmpNew = d->lstNewItems;
  d->lstNewItems = 0;

  KFileItemList *tmpMime = d->lstMimeFilteredItems;
  d->lstMimeFilteredItems = 0;

  KFileItemList *tmpRefresh = d->lstRefreshItems;
  d->lstRefreshItems = 0;

  KFileItemList *tmpRemove = d->lstRemoveItems;
  d->lstRemoveItems = 0;

  if ( tmpNew )
  {
    emit newItems( *tmpNew );
    delete tmpNew;
  }

  if ( tmpMime )
  {
    emit itemsFilteredByMime( *tmpMime );
    delete tmpMime;
  }

  if ( tmpRefresh )
  {
    emit refreshItems( *tmpRefresh );
    delete tmpRefresh;
  }

  if ( tmpRemove )
  {
    KFileItemList::const_iterator kit = tmpRemove->begin();
    const KFileItemList::const_iterator kend = tmpRemove->end();
    for ( ; kit != kend; ++kit )
      emit deleteItem( *kit );
    delete tmpRemove;
  }
}

void KDirLister::emitDeleteItem( KFileItem *item )
{
  if ( ( d->dirOnlyMode && !item->isDir() ) || !matchesFilter( item ) )
    return; // No reason to continue... bailing out here prevents a mimetype scan.
  if ( matchesMimeFilter( item ) )
    emit deleteItem( item );
}


// ================ private slots ================ //

void KDirLister::slotInfoMessage( KJob *, const QString& message )
{
  emit infoMessage( message );
}

void KDirLister::slotPercent( KJob *job, unsigned long pcnt )
{
  d->jobData[static_cast<KIO::ListJob *>(job)].percent = pcnt;

  int result = 0;

  KIO::filesize_t size = 0;

  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).percent * (*dataIt).totalSize;
    size += (*dataIt).totalSize;
    ++dataIt;
  }

  if ( size != 0 )
    result /= size;
  else
    result = 100;
  emit percent( result );
}

void KDirLister::slotTotalSize( KJob *job, qulonglong size )
{
  d->jobData[static_cast<KIO::ListJob *>(job)].totalSize = size;

  KIO::filesize_t result = 0;
  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).totalSize;
    ++dataIt;
  }

  emit totalSize( result );
}

void KDirLister::slotProcessedSize( KJob *job, qulonglong size )
{
  d->jobData[static_cast<KIO::ListJob *>(job)].processedSize = size;

  KIO::filesize_t result = 0;
  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).processedSize;
    ++dataIt;
  }

  emit processedSize( result );
}

void KDirLister::slotSpeed( KJob *job, unsigned long spd )
{
  d->jobData[static_cast<KIO::ListJob *>(job)].speed = spd;

  int result = 0;
  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).speed;
    ++dataIt;
  }

  emit speed( result );
}

uint KDirLister::numJobs()
{
  return d->jobData.count();
}

void KDirLister::jobDone( KIO::ListJob *job )
{
  d->jobData.remove( job );
}

void KDirLister::jobStarted( KIO::ListJob *job )
{
  KDirListerPrivate::JobData jobData;
  jobData.speed = 0;
  jobData.percent = 0;
  jobData.processedSize = 0;
  jobData.totalSize = 0;

  d->jobData.insert( job, jobData );
  d->complete = false;
}

void KDirLister::connectJob( KIO::ListJob *job )
{
  connect( job, SIGNAL(infoMessage( KJob *, const QString&, const QString& )),
           this, SLOT(slotInfoMessage( KJob *, const QString& )) );
  connect( job, SIGNAL(percent( KJob *, unsigned long )),
           this, SLOT(slotPercent( KJob *, unsigned long )) );
  connect( job, SIGNAL(totalSize( KJob *, qulonglong )),
           this, SLOT(slotTotalSize( KJob *, qulonglong )) );
  connect( job, SIGNAL(processedSize( KJob *, qulonglong )),
           this, SLOT(slotProcessedSize( KJob *, qulonglong )) );
  connect( job, SIGNAL(speed( KJob *, unsigned long )),
           this, SLOT(slotSpeed( KJob *, unsigned long )) );
}

void KDirLister::setMainWindow( QWidget *window )
{
  d->window = window;
}

QWidget *KDirLister::mainWindow()
{
  return d->window;
}

KFileItemList KDirLister::items( WhichItems which ) const
{
    return itemsForDir( url(), which );
}

KFileItemList KDirLister::itemsForDir( const KUrl& dir, WhichItems which ) const
{
    KFileItemList *allItems = s_pCache->itemsForDir( dir );
    if ( !allItems )
        return KFileItemList();

    if ( which == AllItems )
        return *allItems;
    else // only items passing the filters
    {
        KFileItemList result;
        KFileItemList::const_iterator kit = allItems->begin();
        const KFileItemList::const_iterator kend = allItems->end();
        for ( ; kit != kend; ++kit )
        {
            KFileItem *item = *kit;
            bool isExcluded = (d->dirOnlyMode && !item->isDir()) || !matchesFilter( item );
            if ( !isExcluded && matchesMimeFilter( item ) )
                result.append( item );
        }
        return result;
    }
}

bool KDirLister::delayedMimeTypes() const
{
    return d->delayedMimeTypes;
}

void KDirLister::setDelayedMimeTypes( bool delayedMimeTypes )
{
    d->delayedMimeTypes = delayedMimeTypes;
}

#include "kdirlister.moc"
#include "kdirlister_p.moc"
