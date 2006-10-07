/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000 Michael Matz <matz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "config.h"
#include "klibloader.h"

#include <qclipboard.h>
#include <qfile.h>
#include <qdir.h>
#include <qtimer.h>
#include <qlibrary.h>
#include <QStack>
#include <qapplication.h>

#include "kstandarddirs.h"
#include "kinstance.h"
#include "kdebug.h"
#include "klocale.h"

#include <stdlib.h> //getenv


namespace KLibLoader_cpp {
enum UnloadMode {UNKNOWN, UNLOAD, DONT_UNLOAD};
}

/* This helper class is needed, because KLibraries can go away without
   being unloaded. So we need some info about KLibraries even after its
   death. */
class KLibWrapPrivate
{
public:
    KLibWrapPrivate(KLibrary *l, QLibrary* h);

    KLibrary *lib;
    QLibrary* handle;
    KLibLoader_cpp::UnloadMode unload_mode;
    int ref_count;
    QString name;
    QString filename;
};

class KLibLoader::Private
{
public:
    ~Private() {
    	while (!loaded_stack.isEmpty())
	    delete loaded_stack.pop();
    }
    QStack<KLibWrapPrivate*> loaded_stack;
    QList<KLibWrapPrivate*> pending_close;
    KLibLoader_cpp::UnloadMode unload_mode;

    QString errorMessage;
};

KLibLoader* KLibLoader::s_self = 0;

// -------------------------------------------------------------------------

KLibFactory::KLibFactory( QObject* _parent )
    : QObject( _parent )
{
}

KLibFactory::~KLibFactory()
{
//    kDebug(150) << "Deleting KLibFactory " << this << endl;
}

QObject* KLibFactory::create( QObject* _parent, const char* classname, const QStringList &args )
{
    QObject* obj = createObject( _parent, classname, args );
    if ( obj )
	emit objectCreated( obj );
    return obj;
}


// -----------------------------------------------

class KLibraryPrivate
{
public:
    inline KLibraryPrivate(KLibrary *_q) : q(_q) {}
    KLibrary *q;
    QString libname;
    QString filename;
    QHash<QByteArray, KLibFactory*> factories;
    QLibrary* handle;
    QList<QObject*> objs;
    QTimer* timer;

    KLibFactory *kde3Factory(const QByteArray &factoryname);
    KLibFactory *kde4Factory();
};

KLibrary::KLibrary( const QString& libname, const QString& filename, QLibrary * handle )
{
    /* Make sure, we have a KLibLoader */
    (void) KLibLoader::self();
    d = new KLibraryPrivate(this);
    d->libname = libname;
    d->filename = filename;
    d->handle = handle;
    d->timer = 0;
}

KLibrary::~KLibrary()
{
//    kDebug(150) << "Deleting KLibrary " << this << "  " << d->libname << endl;
    if ( d->timer && d->timer->isActive() )
	d->timer->stop();

    // If any object is remaining, delete
    if ( !d->objs.isEmpty() )
	{
	    while (!d->objs.isEmpty())
		{
		    QObject *obj = d->objs.takeFirst();
		    kDebug(150) << "Factory still has object " << obj << " " << obj->objectName() << " Library = " << d->libname << endl;
		    disconnect( obj, SIGNAL( destroyed() ),
				this, SLOT( slotObjectDestroyed() ) );
		    delete obj;
		}
	}

    qDeleteAll( d->factories );
    d->factories.clear();
    delete d;
    d = 0;
}

QString KLibrary::name() const
{
    return d->libname;
}

QString KLibrary::fileName() const
{
    return d->filename;
}

KLibFactory* KLibraryPrivate::kde3Factory(const QByteArray &factoryname)
{
    QByteArray symname = "init_";
    if(!factoryname.isEmpty()) {
        symname += factoryname;
    } else {
        symname += libname.toLatin1();
    }

    if ( factories.contains( symname ) )
        return factories[ symname ];

    void* sym = q->symbol( symname );
    if ( !sym )
    {
        KLibLoader::self()->d->errorMessage = i18n( "The library %1 does not offer an %2 function.", libname, QLatin1String("init_") + libname );
        kWarning(150) << KLibLoader::self()->d->errorMessage << endl;
        return 0;
    }

    typedef KLibFactory* (*t_func)();
    t_func func = (t_func)sym;
    KLibFactory* factory = func();

    if( !factory )
    {
        KLibLoader::self()->d->errorMessage = i18n("The library %1 does not offer a KDE compatible factory." , libname);
        kWarning(150) << KLibLoader::self()->d->errorMessage << endl;
        return 0;
    }
    factories.insert( symname, factory );

    return factory;
}

KLibFactory *KLibraryPrivate::kde4Factory()
{
    const QByteArray symname("qt_plugin_instance");
    if ( factories.contains( symname ) )
        return factories[ symname ];

    void* sym = q->symbol( symname );
    if ( !sym )
    {
//        KLibLoader::self()->d->errorMessage = i18n("The library %1 does not offer an qt_plugin_instance function.", libname);
//        kWarning(150) << KLibLoader::self()->d->errorMessage << endl;
        return 0;
    }

    typedef QObject* (*t_func)();
    t_func func = (t_func)sym;
    QObject* instance = func();
    KLibFactory *factory = qobject_cast<KLibFactory *>(instance);

    if( !factory )
    {
//        KLibLoader::self()->d->errorMessage = i18n("The library %1 does not offer a KDE 4 compatible factory." , libname);
//        kWarning(150) << KLibLoader::self()->d->errorMessage << endl;
        return 0;
    }
    factories.insert( symname, factory );

    return factory;

}

KLibFactory* KLibrary::factory(const char* factoryname)
{
    KLibFactory *factory = d->kde4Factory();
    if (!factory)
        factory = d->kde3Factory(factoryname);

    if (!factory)
        return 0;

    connect(factory, SIGNAL(objectCreated(QObject *)),
             this, SLOT(slotObjectCreated(QObject * )));

    return factory;
}

void* KLibrary::resolve( const char* symname ) const
{
    void* sym = d->handle->resolve( symname );
    if ( !d->handle->isLoaded() || !sym )
    {
        KLibLoader::self()->d->errorMessage = QLatin1String("KLibrary: ") + d->handle->errorString();
        kWarning(150) << KLibLoader::self()->d->errorMessage << endl;
        return 0;
    }

    return sym;
}

bool KLibrary::hasSymbol( const char* symname ) const
{
    void* sym = d->handle->resolve(symname);
    return (sym != 0L );
}

void KLibrary::unload() const
{
   if (KLibLoader::s_self)
      KLibLoader::s_self->unloadLibrary(QFile::encodeName(name()));
}

void KLibrary::slotObjectCreated( QObject *obj )
{
  if ( !obj )
    return;

  if ( d->timer && d->timer->isActive() )
    d->timer->stop();

  if ( d->objs.contains( obj ) )
      return; // we know this object already

  connect( obj, SIGNAL( destroyed() ),
           this, SLOT( slotObjectDestroyed() ) );

  d->objs.append( obj );
}

void KLibrary::slotObjectDestroyed()
{
  d->objs.removeAll( sender() );

  if ( d->objs.count() == 0 )
  {
//    kDebug(150) << "KLibrary: shutdown timer for " << name() << " started!"
//                 << endl;

    if ( !d->timer )
    {
      d->timer = new QTimer( this );
      d->timer->setObjectName( QLatin1String("klibrary_shutdown_timer") );
      d->timer->setSingleShot(true);
      connect( d->timer, SIGNAL( timeout() ),
               this, SLOT( slotTimeout() ) );
    }

    // as long as it's not stable make the timeout short, for debugging
    // pleasure (matz)
    //d->timer->start( 1000*60 );
    d->timer->start( 1000*10 );
  }
}

void KLibrary::slotTimeout()
{
  if ( d->objs.count() != 0 )
    return;

  /* Don't go through KLibLoader::unloadLibrary(), because that uses the
     ref counter, but this timeout means to unconditionally close this library
     The destroyed() signal will take care to remove us from all lists.
  */
  delete this;
}

// -------------------------------------------------

KLibWrapPrivate::KLibWrapPrivate(KLibrary *l, QLibrary* h)
 : lib(l), ref_count(1), handle(h), name(l->name()), filename(l->fileName())
{
    unload_mode = KLibLoader_cpp::UNKNOWN;
    if (handle->resolve("__kde_do_not_unload") != 0) {
//        kDebug(150) << "Will not unload " << name << endl;
        unload_mode = KLibLoader_cpp::DONT_UNLOAD;
    } else if (handle->resolve("__kde_do_unload") != 0) {
        unload_mode = KLibLoader_cpp::UNLOAD;
    }
}

KLibLoader* KLibLoader::self()
{
    if ( !s_self )
        s_self = new KLibLoader;
    return s_self;
}

void KLibLoader::cleanUp()
{
  if ( !s_self )
    return;

  delete s_self;
  s_self = 0L;
}

KLibLoader::KLibLoader( QObject* _parent )
    : QObject( _parent ), d(new Private)
{
    s_self = this;
    d->unload_mode = KLibLoader_cpp::UNKNOWN;
    if (getenv("KDE_NOUNLOAD") != 0)
        d->unload_mode = KLibLoader_cpp::DONT_UNLOAD;
    else if (getenv("KDE_DOUNLOAD") != 0)
        d->unload_mode = KLibLoader_cpp::UNLOAD;
}

KLibLoader::~KLibLoader()
{
//    kDebug(150) << "Deleting KLibLoader " << this << "  " << name() << endl;

    for (QHash<QString, KLibWrapPrivate*>::Iterator it = m_libs.begin(); it != m_libs.end(); ++it)
    {
      Q_ASSERT((*it) != 0);
      kDebug(150) << "The KLibLoader contains the library " << (*it)->name
        << " (" << (*it)->lib << ")" << endl;
      d->pending_close.append(*it);
    }

    close_pending(0);

    delete d;
}

static inline QByteArray makeLibName( const char* name )
{
    QByteArray libname(name);
    int pos = libname.lastIndexOf('/');
    if (pos < 0)
      pos = 0;
    if (libname.indexOf('.', pos) < 0) {
        const char* const extList[] = { ".so", ".dylib", ".bundle", ".dll", ".sl" };
        for (int i = 0; i < sizeof(extList) / sizeof(*extList); ++i) {
           if (QLibrary::isLibrary(libname + extList[i]))
               return libname + extList[i];
        }
    }
    return libname;
}

//static
QString KLibLoader::findLibrary( const char * name, const KInstance * instance )
{
    QByteArray libname = makeLibName( name );

    // only look up the file if it is not an absolute filename
    // (mhk, 20000228)
    QString libfile;
    if (!QDir::isRelativePath(libname)) {
      libfile = QFile::decodeName( libname );
    }
    else
    {
      libfile = instance->dirs()->findResource( "module", libname );
      if ( libfile.isEmpty() )
      {
        libfile = instance->dirs()->findResource( "lib", libname );
#ifndef NDEBUG
        if ( !libfile.isEmpty() && libname.startsWith( "lib" ) ) // don't warn for kdeinit modules
          kDebug(150) << "library " << libname << " not found under 'module' but under 'lib'" << endl;
#endif
      }
    }
    return libfile;
}


KLibrary* KLibLoader::globalLibrary( const char *_name )
{
    return library(_name, QLibrary::ExportExternalSymbolsHint);
}


KLibrary* KLibLoader::library( const char *_name, QLibrary::LoadHints hint )
{
    if (!_name)
        return 0;

    if (m_libs.contains(_name)) {
      /* Nothing to do to load the library.  */
      m_libs[_name]->ref_count++;
      return m_libs[_name]->lib;
    }

    KLibWrapPrivate* wrap = 0;
    /* Test if this library was loaded at some time, but got
       unloaded meanwhile, whithout being dlclose()'ed.  */
    QStack<KLibWrapPrivate*>::Iterator it = d->loaded_stack.begin();
    for (; it != d->loaded_stack.end(); ++it) {
      if ((*it)->name == QLatin1String(_name))
        wrap = *it;
    }

    if (wrap) {
      d->pending_close.removeAll(wrap);
      if (!wrap->lib) {
        /* This lib only was in loaded_stack, but not in m_libs.  */
        wrap->lib = new KLibrary( QLatin1String(_name), wrap->filename, wrap->handle );
      }
      wrap->ref_count++;
    } else {
      QString libfile = findLibrary( _name );
      if ( libfile.isEmpty() )
      {
        const QByteArray libname ( _name );
#ifndef NDEBUG
        kDebug(150) << "library=" << _name << ": No file named " << libname << " found in paths." << endl;
#endif
        d->errorMessage = i18n("Library files for \"%1\" not found in paths.",  QString(libname) );
        return 0;
      }

      QLibrary* handle = new QLibrary(libfile);
      handle->setLoadHints(hint);
      if ( !handle->load() || !handle->isLoaded() )
      {
        d->errorMessage = handle->errorString();
        return 0;
      }
      else
        d->errorMessage.clear();

      KLibrary *lib = new KLibrary( QLatin1String(_name), libfile, handle);
      wrap = new KLibWrapPrivate(lib, handle);
      d->loaded_stack.push(wrap);
    }
    m_libs.insert( _name, wrap );

    connect( wrap->lib, SIGNAL( destroyed() ),
             this, SLOT( slotLibraryDestroyed() ) );

    return wrap->lib;
}

QString KLibLoader::lastErrorMessage() const
{
    return d->errorMessage;
}

void KLibLoader::unloadLibrary( const char *libname )
{
  if (!m_libs.contains(libname))
    return;

  KLibWrapPrivate *wrap = m_libs[ libname ];
  if (--wrap->ref_count)
    return;

//  kDebug(150) << "closing library " << libname << endl;

  m_libs.remove( libname );

  disconnect( wrap->lib, SIGNAL( destroyed() ),
              this, SLOT( slotLibraryDestroyed() ) );
  close_pending( wrap );
}

KLibFactory* KLibLoader::factory( const char* _name )
{
    KLibrary* lib = library( _name );
    if ( !lib )
        return 0;

    return lib->factory();
}

void KLibLoader::slotLibraryDestroyed()
{
  const KLibrary *lib = static_cast<const KLibrary *>( sender() );

  for (QHash<QString, KLibWrapPrivate*>::Iterator it = m_libs.begin(); it != m_libs.end(); ++it)
    if ( (*it)->lib == lib )
    {
      KLibWrapPrivate *wrap = *it;
      wrap->lib = 0;  /* the KLibrary object is already away */
      m_libs.remove( it.key() );
      close_pending( wrap );
      return;
    }
}

void KLibLoader::close_pending(KLibWrapPrivate *wrap)
{
  if (wrap && !d->pending_close.contains( wrap ))
    d->pending_close.append( wrap );

  /* First delete all KLibrary objects in pending_close, but _don't_ unload
     the DSO behind it.  */
  for (QList<KLibWrapPrivate*>::Iterator it = d->pending_close.begin();
       it != d->pending_close.end(); ++it) {
    wrap = *it;
    if (wrap->lib) {
      disconnect( wrap->lib, SIGNAL( destroyed() ),
                  this, SLOT( slotLibraryDestroyed() ) );
      KLibrary* to_delete = wrap->lib;
      wrap->lib = 0L; // unset first, because KLibrary dtor can cause
      delete to_delete; // recursive call to close_pending()
    }
  }

  if (d->unload_mode == KLibLoader_cpp::DONT_UNLOAD) {
    while (!d->pending_close.isEmpty())
        delete d->pending_close.takeFirst();
    return;
  }

  bool deleted_one = false;
  while (!d->loaded_stack.isEmpty()) {
    wrap = d->loaded_stack.top();
    /* Let's first see, if we want to try to unload this lib.
       If the env. var KDE_DOUNLOAD is set, we try to unload every lib.
       If not, we look at the lib itself, and unload it only, if it exports
       the symbol __kde_do_unload. */
    if (d->unload_mode != KLibLoader_cpp::UNLOAD && wrap->unload_mode != KLibLoader_cpp::UNLOAD)
      break;

    /* Now ensure, that the libs are only unloaded in the reverse direction
       they were loaded.  */
    if (!d->pending_close.contains( wrap )) {
      if (!deleted_one)
        /* Only diagnose, if we really haven't deleted anything. */
//        kDebug(150) << "try to dlclose " << wrap->name << ": not yet" << endl;
      break;
    }

//    kDebug(150) << "try to dlclose " << wrap->name << ": yes, done." << endl;

    if ( !deleted_one ) {
      /* Only do the hack once in this loop.
         WABA: *HACK*
         We need to make sure to clear the clipboard before unloading a DSO
         because the DSO could have defined an object derived from QMimeSource
         and placed that on the clipboard. */
      /*qApp->clipboard()->clear();*/

      /* Well.. let's do something more subtle... convert the clipboard context
         to text. That should be safe as it only uses objects defined by Qt. */
      if( qApp->clipboard()->ownsSelection()) {
	qApp->clipboard()->setText(
            qApp->clipboard()->text( QClipboard::Selection ), QClipboard::Selection );
      }
      if( qApp->clipboard()->ownsClipboard()) {
	qApp->clipboard()->setText(
            qApp->clipboard()->text( QClipboard::Clipboard ), QClipboard::Clipboard );
      }
    }

    deleted_one = true;
    delete wrap->handle;
    d->pending_close.removeAll(wrap);
    delete d->loaded_stack.pop();
  }
}

QString KLibLoader::errorString( int componentLoadingError )
{
    switch ( componentLoadingError ) {
    case ErrNoServiceFound:
        return i18n( "No service matching the requirements was found" );
    case ErrServiceProvidesNoLibrary:
        return i18n( "The service provides no library, the Library key is missing in the .desktop file" );
    case ErrNoLibrary:
        return KLibLoader::self()->lastErrorMessage();
    case ErrNoFactory:
        return i18n( "The library does not export a factory for creating components" );
    case ErrNoComponent:
        return i18n( "The factory does not support creating components of the specified type" );
    default:
        return i18n( "KLibLoader: Unknown error" );
    }
}

#include "klibloader.moc"
// vim: sw=4 sts=4 et
