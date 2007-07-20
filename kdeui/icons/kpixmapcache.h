/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KPIXMAPCACHE_H
#define KPIXMAPCACHE_H

#include <kdeui_export.h>

#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QSize>

class QString;
class QStringList;
class QPixmap;


/**
 * General-purpose pixmap cache for KDE.
 *
 * It can be used e.g. by applications wanting to cache pixmaps rendered from
 *  SVGs.
 */
class KDEUI_EXPORT KPixmapCache
{
public:
    /**
     * Constucts the pixmap cache object.
     * @param name unique name of the cache
     **/
    explicit KPixmapCache(const QString& name);
    virtual ~KPixmapCache();

    /**
     * Tries to load the specified pixmap from cache.
     * @return true when pixmap was found and loaded from cache, false otherwise
     **/
    virtual bool find(const QString& key, QPixmap& pix);
    /**
     * Insert specified pixmap into the cache.
     **/
    virtual void insert(const QString& key, const QPixmap& pix);

    /**
     * Loads pixmap from given file, using the cache.
     * If file's modified-time is more recent than cache's @ref timestamp() ,
     *  then the cache is discarded.
     **/
    QPixmap loadFromFile(const QString& filename);
    /**
     * Same as above, but uses SVG file instead.
     * @param size size of the pixmap where the SVG is render to. If not given
     *  then SVG's default size is used.
     **/
    QPixmap loadFromSVG(const QString& filename, const QSize& size = QSize());

    /**
     * @return timestamp of the cache, set using the @p setTimestamp method.
     * It can be used by the application to check whether app-specific cache
     *  has outdated.
     **/
    unsigned int timestamp() const;
    /**
     * Sets the timestamp of app-specific cache. It's saved in the cache file
     *  and can later be retrieved using @p timestamp() method.
     **/
    void setTimestamp(unsigned int time);
    /**
     * Sets whether QPixmapCache (memory cacheing) should be used in addition
     * to disk cache.
     * QPixmapCache is used by default
     **/
    void setUseQPixmapCache(bool use);
    /**
     * Whether QPixmapCache should be used to cache pixmaps in memory in
     *  addition to caching them on the disk.
     **/
    bool useQPixmapCache() const;
    /**
     * @return approximate size of the cache, in bytes.
     **/
    int size() const;
    /**
     * @return true when the cache is enabled.
     * Cache will be disabled when e.g.it's data file cannot be created or
     *  read.
     **/
    bool isEnabled() const;
    /**
     * @return true when the cache is ready to be used.
     * False usually means that some additional initing has to be done before
     *  the cache can be used.
     **/
    bool isValid() const;
    /**
     * Deletes a pixmap cache.
     * @param name unique name of the cache to be deleted
     **/
    static void deleteCache(const QString& name);
    /**
     * Deletes all entries and reinitializes this cache.
     **/
    void discard();

    enum RemoveStrategy { RemoveOldest, RemoveSeldomUsed, RemoveLeastRecentlyUsed };
    /**
     * Removes some of the entries in the cache.
     * @param newsize wanted size of te cache, in bytes.
     * @param strategy which entries to remove.
     *
     * Warning: this works by copying some entries to a new cache and then
     *  replacing the old cache with the new one. Thus it might be slow and
     *  will temporarily use extra disk space.
     **/
    void removeEntries(int newsize, RemoveStrategy strategy = RemoveLeastRecentlyUsed);

protected:
    void init();

    bool loadData(int offset, QPixmap& pix);
    /**
     * Can be used by subclasses to write custom data into the the stream.
     **/
    virtual bool loadCustomData(QDataStream& stream);

    int writeData(const QString& key, const QPixmap& pix);
    /**
     * Can be used by subclasses to load custom data from the the stream.
     **/
    virtual bool writeCustomData(QDataStream& stream);
    void writeIndex(const QString& key, int offset);

    /**
     * Can be used by subclasses to write custom data into cache's header.
     **/
    virtual bool loadCustomIndexHeader(QDataStream& stream);
    /**
     * Can be used by subclasses to load custom data from cache's header.
     **/
    virtual void writeCustomIndexHeader(QDataStream& stream);
    /**
     * Can be used by subclasses to indicate that cache needs some additional
     *  initing before it can be used.
     **/
    void setValid(bool valid);

    void ensureInited() const;
    bool recreateCacheFiles();

private:
    class Private;
    friend class Private;
    Private * const d;
};

#endif // KPIXMAPCACHE_H
