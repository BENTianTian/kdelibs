// -*- c++ -*-
// vim: ts=4 sw=4 et
/*  This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>
                  2000 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Malte Starostik <malte.starostik@t-online.de>

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

#include "previewjob.h"

#include <sys/stat.h>
#ifdef __FreeBSD__
    #include <machine/param.h>
#endif
#include <sys/types.h>

#ifdef Q_OS_UNIX
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qtimer.h>
#include <qregexp.h>

#include <kfileitem.h>
#include <kapplication.h>
#include <ktemporaryfile.h>
#include <kservicetypetrader.h>
#include <kcodecs.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <qlinkedlist.h>

#include "previewjob.moc"

namespace KIO { struct PreviewItem; }
using namespace KIO;

struct KIO::PreviewItem
{
    KFileItem *item;
    KService::Ptr plugin;
};

struct KIO::PreviewJobPrivate
{
    enum { STATE_STATORIG, // if the thumbnail exists
           STATE_GETORIG, // if we create it
           STATE_CREATETHUMB // thumbnail:/ slave
    } state;
    KFileItemList initialItems;
    const QStringList *enabledPlugins;
    // Our todo list :)
    // We remove the first item at every step, so use QLinkedList
    QLinkedList<PreviewItem> items;
    // The current item
    PreviewItem currentItem;
    // The modification time of that URL
    time_t tOrig;
    // Path to thumbnail cache for the current size
    QString thumbPath;
    // Original URL of current item in TMS format
    // (file:///path/to/file instead of file:/path/to/file)
    QString origName;
    // Thumbnail file name for current item
    QString thumbName;
    // Size of thumbnail
    int width;
    int height;
    // Unscaled size of thumbnail (128 or 256 if cache is enabled)
    int cacheWidth;
    int cacheHeight;
    // Whether the thumbnail should be scaled
    bool bScale;
    // Whether we should save the thumbnail
    bool bSave;
    // If the file to create a thumb for was a temp file, this is its name
    QString tempName;
    // Over that, it's too much
    long long maximumSize;
    // the size for the icon overlay
    int iconSize;
    // the transparency of the blended mimetype icon
    int iconAlpha;
	// Shared memory segment Id. The segment is allocated to a size
	// of extent x extent x 4 (32 bit image) on first need.
	int shmid;
	// And the data area
	uchar *shmaddr;
    // Delete the KFileItems when done?
    bool deleteItems;
    bool succeeded;
    // Root of thumbnail cache
    QString thumbRoot;
    bool ignoreMaximumSize;
};

PreviewJob::PreviewJob( const KFileItemList &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const QStringList *enabledPlugins, bool deleteItems )
    : KIO::Job( false /* no GUI */ ),d(new PreviewJobPrivate)
{
    d->tOrig = 0;
    d->shmid = -1;
    d->shmaddr = 0;
    d->initialItems = items;
    d->enabledPlugins = enabledPlugins;
    d->width = width;
    d->height = height ? height : width;
    d->cacheWidth = d->width;
    d->cacheHeight = d->height;
    d->iconSize = iconSize;
    d->iconAlpha = iconAlpha;
    d->deleteItems = deleteItems;
    d->bScale = scale;
    d->bSave = save && scale;
    d->succeeded = false;
    d->currentItem.item = 0;
    d->thumbRoot = QDir::homePath() + "/.thumbnails/";
    d->ignoreMaximumSize = false;

    // Return to event loop first, determineNextFile() might delete this;
    QTimer::singleShot(0, this, SLOT(startPreview()));
}

PreviewJob::~PreviewJob()
{
#ifdef Q_OS_UNIX
    if (d->shmaddr) {
        shmdt((char*)d->shmaddr);
        shmctl(d->shmid, IPC_RMID, 0);
    }
#endif
    delete d;
}

void PreviewJob::startPreview()
{
    // Load the list of plugins to determine which mimetypes are supported
    const KService::List plugins = KServiceTypeTrader::self()->query("ThumbCreator");
    QMap<QString, KService::Ptr> mimeMap;

    for (KService::List::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        if (!d->enabledPlugins || d->enabledPlugins->contains((*it)->desktopEntryName()))
    {
        const QStringList mimeTypes = (*it)->property("MimeTypes").toStringList();
        for (QStringList::ConstIterator mt = mimeTypes.begin(); mt != mimeTypes.end(); ++mt)
            mimeMap.insert(*mt, *it);
    }

    // Look for images and store the items in our todo list :)
    bool bNeedCache = false;
    KFileItemList::const_iterator kit = d->initialItems.begin();
    const KFileItemList::const_iterator kend = d->initialItems.end();
    for ( ; kit != kend; ++kit )
    {
        PreviewItem item;
        item.item = *kit;
        const QString mimeType = item.item->mimetype();
        QMap<QString, KService::Ptr>::ConstIterator plugin = mimeMap.find(mimeType);
        if (plugin == mimeMap.end())
        {
            QString groupMimeType = mimeType;
            groupMimeType.replace(QRegExp("/.*"), "/*");
            plugin = mimeMap.find(groupMimeType);

            if (plugin == mimeMap.end())
            {
                // check mime type inheritance
                const KMimeType::Ptr mimeInfo = KMimeType::mimeType(mimeType);
                QString parentMimeType = mimeInfo->parentMimeType();
                while (!parentMimeType.isEmpty())
                {
                    plugin = mimeMap.find(parentMimeType);
                    if (plugin != mimeMap.end()) break;

                    KMimeType::Ptr parentMimeInfo = KMimeType::mimeType(parentMimeType);
                    if (!parentMimeInfo) break;

                    parentMimeType = parentMimeInfo->parentMimeType();
                }
            }

            if (plugin == mimeMap.end())
            {
                // check X-KDE-Text property
                KMimeType::Ptr mimeInfo = KMimeType::mimeType(mimeType);
                QVariant textProperty = mimeInfo->property("X-KDE-text");
                if (textProperty.isValid() && textProperty.type() == QVariant::Bool)
                {
                    if (textProperty.toBool())
                    {
                        plugin = mimeMap.find("text/plain");
                        if (plugin == mimeMap.end())
                        {
                            plugin = mimeMap.find( "text/*" );
                        }
                    }
                }
            }
        }

        if (plugin != mimeMap.end())
        {
            item.plugin = *plugin;
            d->items.append(item);
            if (!bNeedCache && d->bSave &&
                ((*kit)->url().protocol() != "file" ||
                 !(*kit)->url().directory( KUrl::AppendTrailingSlash ).startsWith(d->thumbRoot)) &&
                (*plugin)->property("CacheThumbnail").toBool())
                bNeedCache = true;
        }
        else
        {
            emitFailed( *kit );
            if (d->deleteItems)
                delete *kit;
        }
    }

  // Read configuration value for the maximum allowed size
    KConfigGroup cg( KGlobal::config(), "PreviewSettings" );
    d->maximumSize = cg.readEntry( "MaximumSize", 1024*1024LL /* 1MB */ );

    if (bNeedCache)
    {
        if (d->width <= 128 && d->height <= 128) d->cacheWidth = d->cacheHeight = 128;
        else d->cacheWidth = d->cacheHeight = 256;
        d->thumbPath = d->thumbRoot + (d->cacheWidth == 128 ? "normal/" : "large/");
        KStandardDirs::makeDir(d->thumbPath, 0700);
    }
    else
        d->bSave = false;

    d->initialItems.clear();
    determineNextFile();
}

void PreviewJob::removeItem( const KFileItem *item )
{
    for (QLinkedList<PreviewItem>::Iterator it = d->items.begin(); it != d->items.end(); ++it)
        if ((*it).item == item)
        {
            d->items.erase(it);
            break;
        }

    if (d->currentItem.item == item)
    {
        KJob* job = subjobs().first();
        job->kill();
        removeSubjob( job );
        determineNextFile();
    }
}

void PreviewJob::setIgnoreMaximumSize(bool ignoreSize)
{
    d->ignoreMaximumSize = ignoreSize;
}

void PreviewJob::determineNextFile()
{
    if (d->currentItem.item)
    {
        if (!d->succeeded)
            emitFailed();
        if (d->deleteItems) {
            delete d->currentItem.item;
            d->currentItem.item = 0L;
        }
    }
    // No more items ?
    if ( d->items.isEmpty() )
    {
        emitResult();
        return;
    }
    else
    {
        // First, stat the orig file
        d->state = PreviewJobPrivate::STATE_STATORIG;
        d->currentItem = d->items.first();
        d->succeeded = false;
        d->items.removeFirst();
        KIO::Job *job = KIO::stat( d->currentItem.item->url(), false );
        job->addMetaData( "no-auth-prompt", "true" );
        addSubjob(job);
    }
}

void PreviewJob::slotResult( KJob *job )
{
    removeSubjob(job);
    Q_ASSERT ( !hasSubjobs() ); // We should have only one job at a time ...
    switch ( d->state )
    {
        case PreviewJobPrivate::STATE_STATORIG:
        {
            if (job->error()) // that's no good news...
            {
                // Drop this one and move on to the next one
                determineNextFile();
                return;
            }
            const KIO::UDSEntry entry = static_cast<KIO::StatJob*>(job)->statResult();
            d->tOrig = entry.numberValue( KIO::UDS_MODIFICATION_TIME, 0 );
            if ( !d->ignoreMaximumSize &&
                 entry.numberValue( KIO::UDS_SIZE, 0 ) > d->maximumSize &&
                 !d->currentItem.plugin->property("IgnoreMaximumSize").toBool()
                ) {
                determineNextFile();
                return;
            }

            if ( !d->currentItem.plugin->property( "CacheThumbnail" ).toBool() )
            {
                // This preview will not be cached, no need to look for a saved thumbnail
                // Just create it, and be done
                getOrCreateThumbnail();
                return;
            }

            if ( statResultThumbnail() )
                return;

            getOrCreateThumbnail();
            return;
        }
        case PreviewJobPrivate::STATE_GETORIG:
        {
            if (job->error())
            {
                determineNextFile();
                return;
            }

            createThumbnail( static_cast<KIO::FileCopyJob*>(job)->destUrl().path() );
            return;
        }
        case PreviewJobPrivate::STATE_CREATETHUMB:
        {
            if (!d->tempName.isEmpty())
            {
                QFile::remove(d->tempName);
                d->tempName.clear();
            }
            determineNextFile();
            return;
        }
    }
}

bool PreviewJob::statResultThumbnail()
{
    if ( d->thumbPath.isEmpty() )
        return false;

    KUrl url = d->currentItem.item->url();
    // Don't include the password if any
    url.setPass(QString());
    d->origName = url.url();

    KMD5 md5( QFile::encodeName( d->origName ) );
    d->thumbName = QFile::encodeName( md5.hexDigest() ) + ".png";

    QImage thumb;
    if ( !thumb.load( d->thumbPath + d->thumbName ) ) return false;

    if ( thumb.text( "Thumb::URI", 0 ) != d->origName ||
         thumb.text( "Thumb::MTime", 0 ).toInt() != d->tOrig ) return false;

    // Found it, use it
    emitPreview( thumb );
    d->succeeded = true;
    determineNextFile();
    return true;
}


void PreviewJob::getOrCreateThumbnail()
{
    // We still need to load the orig file ! (This is getting tedious) :)
    const KFileItem* item = d->currentItem.item;
    const QString localPath = item->localPath();
    if ( !localPath.isEmpty() )
        createThumbnail( localPath );
    else
    {
        d->state = PreviewJobPrivate::STATE_GETORIG;
        KTemporaryFile localFile;
        localFile.setAutoRemove(false);
        localFile.open();
        KUrl localURL;
        localURL.setPath( d->tempName = localFile.fileName() );
        const KUrl currentURL = item->url();
        KIO::Job * job = KIO::file_copy( currentURL, localURL, -1, true,
                                         false, false /* No GUI */ );
        job->addMetaData("thumbnail","1");
        addSubjob(job);
    }
}

void PreviewJob::createThumbnail( const QString &pixPath )
{
    d->state = PreviewJobPrivate::STATE_CREATETHUMB;
    KUrl thumbURL;
    thumbURL.setProtocol("thumbnail");
    thumbURL.setPath(pixPath);
    KIO::TransferJob *job = KIO::get(thumbURL, false, false);
    addSubjob(job);
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), SLOT(slotThumbData(KIO::Job *, const QByteArray &)));
    bool save = d->bSave && d->currentItem.plugin->property("CacheThumbnail").toBool();
    job->addMetaData("mimeType", d->currentItem.item->mimetype());
    job->addMetaData("width", QString().setNum(save ? d->cacheWidth : d->width));
    job->addMetaData("height", QString().setNum(save ? d->cacheHeight : d->height));
    job->addMetaData("iconSize", QString().setNum(save ? 64 : d->iconSize));
    job->addMetaData("iconAlpha", QString().setNum(d->iconAlpha));
    job->addMetaData("plugin", d->currentItem.plugin->library());
#ifdef Q_OS_UNIX
    if (d->shmid == -1)
    {
        if (d->shmaddr) {
            shmdt((char*)d->shmaddr);
            shmctl(d->shmid, IPC_RMID, 0);
        }
        d->shmid = shmget(IPC_PRIVATE, d->cacheWidth * d->cacheHeight * 4, IPC_CREAT|0600);
        if (d->shmid != -1)
        {
            d->shmaddr = (uchar *)(shmat(d->shmid, 0, SHM_RDONLY));
            if (d->shmaddr == (uchar *)-1)
            {
                shmctl(d->shmid, IPC_RMID, 0);
                d->shmaddr = 0;
                d->shmid = -1;
            }
        }
        else
            d->shmaddr = 0;
    }
    if (d->shmid != -1)
        job->addMetaData("shmid", QString().setNum(d->shmid));
#endif
}

void PreviewJob::slotThumbData(KIO::Job *, const QByteArray &data)
{
    bool save = d->bSave &&
                d->currentItem.plugin->property("CacheThumbnail").toBool() &&
                (d->currentItem.item->url().protocol() != "file" ||
                 !d->currentItem.item->url().directory( KUrl::AppendTrailingSlash ).startsWith(d->thumbRoot));
    QImage thumb;
#ifdef Q_OS_UNIX
    if (d->shmaddr)
    {
        // Keep this in sync with kdebase/kioslave/thumbnail.cpp
        QDataStream str(data);
        int width, height;
        quint8 iFormat;
        str >> width >> height >> iFormat;
        QImage::Format format = static_cast<QImage::Format>( iFormat );
        thumb = QImage(d->shmaddr, width, height, format );
    }
    else
#endif
        thumb.loadFromData(data);

    if (save)
    {
        thumb.setText("Thumb::URI", d->origName);
        thumb.setText("Thumb::MTime", QString::number(d->tOrig));
        thumb.setText("Thumb::Size", number(d->currentItem.item->size()));
        thumb.setText("Thumb::Mimetype", d->currentItem.item->mimetype());
        thumb.setText("Software", "KDE Thumbnail Generator");
        KTemporaryFile temp;
        temp.setPrefix(d->thumbPath + "kde-tmp-");
        temp.setSuffix(".png");
        temp.setAutoRemove(false);
        if (temp.open()) //Only try to write out the thumbnail if we
        {                //actually created the temp file.
            thumb.save(temp.fileName(), "PNG");
            rename(QFile::encodeName(temp.fileName()), QFile::encodeName(d->thumbPath + d->thumbName));
        }
    }
    emitPreview( thumb );
    d->succeeded = true;
}

void PreviewJob::emitPreview(const QImage &thumb)
{
    QPixmap pix;
    if (thumb.width() > d->width || thumb.height() > d->height)
        pix = QPixmap::fromImage( thumb.scaled(thumb.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    else
        pix = QPixmap::fromImage( thumb );
    emit gotPreview(d->currentItem.item, pix);
}

void PreviewJob::emitFailed(const KFileItem *item)
{
    if (!item)
        item = d->currentItem.item;
    emit failed(item);
}

QStringList PreviewJob::availablePlugins()
{
    QStringList result;
    const KService::List plugins = KServiceTypeTrader::self()->query("ThumbCreator");
    for (KService::List::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        if (!result.contains((*it)->desktopEntryName()))
            result.append((*it)->desktopEntryName());
    return result;
}

QStringList PreviewJob::supportedMimeTypes()
{
    QStringList result;
    const KService::List plugins = KServiceTypeTrader::self()->query("ThumbCreator");
    for (KService::List::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        result += (*it)->property("MimeTypes").toStringList();
    return result;
}

PreviewJob *KIO::filePreview( const KFileItemList &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const QStringList *enabledPlugins )
{
    return new PreviewJob(items, width, height, iconSize, iconAlpha,
                          scale, save, enabledPlugins);
}

PreviewJob *KIO::filePreview( const KUrl::List &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const QStringList *enabledPlugins )
{
    KFileItemList fileItems;
    for (KUrl::List::ConstIterator it = items.begin(); it != items.end(); ++it)
        fileItems.append(new KFileItem(KFileItem::Unknown, KFileItem::Unknown, *it, true));
    return new PreviewJob(fileItems, width, height, iconSize, iconAlpha,
                          scale, save, enabledPlugins, true);
}

