/*
    This file is part of the KDE libraries

    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/
#ifndef _khtml_loader_h
#define _khtml_loader_h

#include <qlist.h>
#include <qptrdict.h>
#include <qdict.h>
#include <qpixmap.h>
#include <qmovie.h>
#include <qbuffer.h>
#include <qstringlist.h>

#include <kurl.h>

#include <dom/dom_string.h>

namespace KIO {
  class Job;
  class TransferJob;
}

namespace DOM
{
    class CSSStyleSheetImpl;
};

namespace khtml
{
    class CachedObject;

    /**
     * @internal
     *
     * a client who wants to load stylesheets or images from the web has to
     * inherit from this class and overload one of the 2 functions
     */
    class CachedObjectClient
    {
    public:
	virtual void setPixmap(const QPixmap &, CachedObject *) {}
	virtual void setStyleSheet(const DOM::DOMString &/*url*/, const DOM::DOMString &/*sheet*/) {}
    };

    /**
     * @internal
     *
     * A cached object. Classes who want to use this object should derive
     * from CachedObjectClient, to get the function calls in case the requested data has arrived.
     *
     * This class also does the actual communication with kio and loads the file.
     */
    class CachedObject
    {
    public:
	enum Type {
	    Image,
	    CSSStyleSheet
	};
	
	enum Status {
	    NotCached,    // this URL is not cached
	    Unknown,      // let imagecache decide what to do with it
	    New,          // inserting new image
	    Persistent,   // never delete this pixmap
	    Cached,       // regular case
	    Pending,      // only partially loaded
	    Uncacheable   // to big to be cached,
	};  	          // will be destroyed as soon as possible

	CachedObject(const DOM::DOMString &url, Type type)
	{
	    m_url = url;
	    m_type = type;
	    m_status = Pending;
	    m_size = 0;
	    m_free = false;
	}
	virtual ~CachedObject() {}

	virtual void data( QBuffer &buffer, bool eof) = 0;
	virtual void error( int err, const char *text ) = 0;
	
	const DOM::DOMString &url() { return m_url; }
	Type type() { return m_type; }
	
	virtual void ref(CachedObjectClient *consumer) = 0;
	virtual void deref(CachedObjectClient *consumer) = 0;
	
	int count() { return m_clients.count(); }

	void setStatus(Status s) { m_status = s; }
	Status status() { return m_status; }

	int size() { return m_size; }

	/*
	 * computes the status of an object after loading.
	 * the result depends on the objects size and the size of the cache
	 */
	void computeStatus();

        /*
         * Called by the cache if the object has been removed from the cache dict
         * while still being referenced. This means the object should kill itself
         * if its reference counter drops down to zero.
         */
        void setFree( bool b ) { m_free = b; }

	virtual bool isImage() const = 0;

    protected:
	DOM::DOMString m_url;
	Type m_type;
	Status m_status;
	QList<CachedObjectClient> m_clients;
	int m_size;
        bool m_free;
    };


    /**
     * a cached style sheet
     */
    class CachedCSSStyleSheet : public CachedObject
    {
    public:
	CachedCSSStyleSheet(const DOM::DOMString &url, const DOM::DOMString &baseURL);
	virtual ~CachedCSSStyleSheet();
	
	const DOM::DOMString &sheet() const { return m_sheet; }

	virtual void ref(CachedObjectClient *consumer);
	virtual void deref(CachedObjectClient *consumer);

	virtual void data( QBuffer &buffer, bool eof );
	virtual void error( int err, const char *text );

	void checkNotify();

	virtual bool isImage() const { return false; }
	
    protected:
	DOM::DOMString m_sheet;
	bool loading;
    };
	
    class ImageSource;

    /**
     * a cached image
     */
    class CachedImage : public QObject, public CachedObject
    {
	Q_OBJECT
    public:
	CachedImage(const DOM::DOMString &url, const DOM::DOMString &baseURL);
	virtual ~CachedImage();
	
	const QPixmap &pixmap() const;
	const QPixmap &tiled_pixmap() const;

	virtual void ref(CachedObjectClient *consumer);
	virtual void deref(CachedObjectClient *consumer);

	virtual void data( QBuffer &buffer, bool eof );
	virtual void error( int err, const char *text );
	
	/**
	 * tell the CachedObjectClient's, that the image is ready.
	 * if o = 0  notify all clients
	 */
	void notify(CachedObjectClient *c = 0);

	virtual bool isImage() const { return true; }

	void load();

    public slots:
	/**
	 * gets called, whenever a QMovie changes frame
	 */
	void movieUpdated( const QRect &rect );
	
    public:
	QPixmap *p;
	QMovie *m;

    protected:
	void clear();

	int width;
	int height;

	// Is the name of the movie format type
	const char* formatType;

	// Is set if movie format type ( incremental/animation) was checked
	bool typeChecked;
	bool gotFrame;
	
        // Used to cache a tiled version of the image
	QPixmap *bg;

	ImageSource* imgSource;

	DOM::DOMString m_baseURL;
    };
	
	
    /**
     * @internal
     */
    class Loader : public QObject
    {
	Q_OBJECT

    public:
	Loader();
	~Loader();

	void load(CachedObject *object, const DOM::DOMString &baseURL, bool incremental = true);

        int numRequests( const DOM::DOMString &baseURL );
        void cancelRequests( const DOM::DOMString &baseURL );	

    signals:
	void requestDone();

    protected slots:
	void slotFinished( KIO::Job * );
	void slotData( KIO::Job *, const QByteArray & );

    protected:
	void servePendingRequests();
	
	class Request
	{
	public:
	    Request(CachedObject *_object, const DOM::DOMString &baseURL, bool _incremental)
	    {
		object = _object;
		incremental = _incremental;
		m_baseURL = baseURL;
	    }
	    bool incremental;
	    QBuffer m_buffer;
	    CachedObject *object;
	    DOM::DOMString m_baseURL;
	};
	QList<Request> m_requestsPending;
	QPtrDict<Request> m_requestsLoading;
    };


    /**
     * @internal
     *
     * Provides a cache/loader for objects needed for displaying the html page.
     * At the moment these are stylesheets and images
     */
    class Cache
    {
    public:

        static void ref();
	static void deref();

	/**
	 * init the cache in case it's not already. This needs to get called once
	 * before using it.
	 */
	static void init();

	/**
	 * Ask the cache for some url. Will return a cachedObject, and
	 * load the requested data in case it's not cahced
	 */
	static CachedImage *requestImage( const DOM::DOMString &url, const DOM::DOMString &baseUrl);

	/**
	 * Ask the cache for some url. Will return a cachedObject, and
	 * load the requested data in case it's not cahced
	 */
	static CachedCSSStyleSheet *requestStyleSheet( const DOM::DOMString &url, const DOM::DOMString &baseUrl);

	/**
	 * sets the size of the cache. This will only hod approximately, since the size some
	 * cached objects (like stylesheets) take up in memory is not exaclty known.
	 */
	static void setSize( int bytes );
	/** returns the size of the cache */
	static int size() { return maxSize; };

	/** prints some statistics to stdout */
	static void statistics();

	/** clean up cache */
	static void flush();

	/**
	 * clears the cache
	 * Warning: call this only at the end of your program, to clean
	 * up memory (useful for finding memory holes)
	 */
	static void clear();

	static Loader *loader() { return m_loader; }

	static KURL completeURL(const DOM::DOMString &url, const DOM::DOMString &baseUrl);
	
    	static QPixmap *nullPixmap;

        static void removeCacheEntry( CachedObject *object );

        static void autoloadImages( bool enable );
	static bool autoloadImages();

    protected:
	/*
	 * @internal
	 */
	class LRUList : public QStringList
	{
	public:
	    /**
	     * implements the LRU list
	     * The least recently used item is at the beginning of the list.
	     */
	    void touch( const QString &url )
	    {
		remove( url );
		append( url );
	    }
	};


	static QDict<CachedObject> *cache;
	static LRUList *lru;

	static int maxSize;
	static int actSize;

	static Loader *m_loader;

        static bool s_autoloadImages;

        static unsigned long s_ulRefCnt;
    };



};

#endif
