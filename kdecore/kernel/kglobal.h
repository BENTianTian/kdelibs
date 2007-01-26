/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>

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
#ifndef _KGLOBAL_H
#define _KGLOBAL_H

#include "kdelibs_export.h"
#include <QAtomicPointer>

class KInstance;
class KCharsets;
class KConfig;
class KSharedConfig;
class KLocale;
class KStandardDirs;
class KStaticDeleterBase;
class KStaticDeleterList;
class KStringDict;
class QString;

typedef void (*KdeCleanUpFunction)();

class KCleanUpGlobalStatic
{
    public:
        KdeCleanUpFunction func;

        inline ~KCleanUpGlobalStatic() { func(); }
};

/**
 * This macro makes it easy to use non-POD types as global statics.
 * The object is created on first use and creation is threadsafe.
 *
 * The object is destructed on library unload or application exit.
 * Be careful with calling other objects in the destructor of the class
 * as you have to be sure that they are not already destructed.
 *
 * \param TYPE The type of the global static object. Do not add a *.
 * \param NAME The name of the function to get a pointer to the global static object.
 *
 * If you have code that might be called after the global object has been destroyed you can check
 * for that using the isDestroyed() function.
 *
 * If needed you can also install a qPostRoutine to clean up the object using the destroy() method.
 *
 * Example:
 * \code
 * class A { ... };
 *
 * K_GLOBAL_STATIC(A, globalA);
 *
 * void doSomething()
 * {
 *     A *a = globalA;
 *     ...
 * }
 *
 * void doSomethingElse()
 * {
 *     if (globalA.isDestroyed()) {
 *         return;
 *     }
 *     A *a = globalA;
 *     ...
 * }
 *
 * void installPostRoutine()
 * {
 *     qAddPostRoutine(globalA.destroy);
 * }
 * \endcode
 */
#define K_GLOBAL_STATIC(TYPE, NAME)                                            \
static QBasicAtomicPointer<TYPE > _k_static_##NAME = Q_ATOMIC_INIT(0);         \
static bool _k_static_##NAME##_destroyed = false;                              \
static struct                                                                  \
{                                                                              \
    inline bool isDestroyed()                                                  \
    {                                                                          \
        return _k_static_##NAME##_destroyed;                                   \
    }                                                                          \
    inline operator TYPE*()                                                    \
    {                                                                          \
        return operator->();                                                   \
    }                                                                          \
    inline TYPE *operator->()                                                  \
    {                                                                          \
        if (!_k_static_##NAME) {                                               \
            if (isDestroyed()) {                                               \
                qFatal("Fatal Error: Accessed global static '%s *%s()' after destruction. " \
                       "Defined at %s:%d", #TYPE, #NAME, __FILE__, __LINE__);  \
            }                                                                  \
            TYPE *x = new TYPE;                                                \
            if (!_k_static_##NAME.testAndSet(0, x)) {                          \
                delete x;                                                      \
            } else {                                                           \
                static KCleanUpGlobalStatic cleanUpObject = { destroy };       \
            }                                                                  \
        }                                                                      \
        return _k_static_##NAME;                                               \
    }                                                                          \
    inline TYPE &operator*()                                                   \
    {                                                                          \
        return *operator->();                                                  \
    }                                                                          \
    static void destroy()                                                      \
    {                                                                          \
        _k_static_##NAME##_destroyed = true;                                   \
        TYPE *x = _k_static_##NAME;                                            \
        _k_static_##NAME.init(0);                                              \
        delete x;                                                              \
    }                                                                          \
} NAME;

/**
 * @overload
 * This is the same as K_GLOBAL_STATIC,  but can take arguments that are passed 
 * to the object's constructor
 * 
 * \param TYPE The type of the global static object. Do not add a *.
 * \param NAME The name of the function to get a pointer to the global static object.
 * \param ARGS the list of arguments, between brackets
 * 
 * Example:
 * \code
 * class A {
 *      public:   A(const char *s , int i);
 *   ...  };
 *
 * K_GLOBAL_STATIC_WITH_ARG(A, globalA, ("foo", 0) )
 *
 * void doSomething()
 * {
 *     A *a = globalA();
 *     ...
 * }
 * \endcode
 */
#define K_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                            \
static QBasicAtomicPointer<TYPE > _k_static_##NAME = Q_ATOMIC_INIT(0);         \
static bool _k_static_##NAME##_destroyed;                                      \
static struct                                                                  \
{                                                                              \
    bool isDestroyed()                                                         \
    {                                                                          \
        return _k_static_##NAME##_destroyed;                                   \
    }                                                                          \
    inline operator TYPE*()                                                    \
    {                                                                          \
        return operator->();                                                   \
    }                                                                          \
    inline TYPE *operator->()                                                  \
    {                                                                          \
        if (!_k_static_##NAME) {                                               \
            if (isDestroyed()) {                                               \
                qFatal("Fatal Error: Accessed global static '%s *%s()' after destruction. " \
                       "Defined at %s:%d", #TYPE, #NAME, __FILE__, __LINE__);  \
            }                                                                  \
            TYPE *x = new TYPE ARGS;                                           \
            if (!_k_static_##NAME.testAndSet(0, x)) {                          \
                delete x;                                                      \
            } else {                                                           \
                static KCleanUpGlobalStatic cleanUpObject = { destroy };       \
            }                                                                  \
        }                                                                      \
        return _k_static_##NAME;                                               \
    }                                                                          \
    inline TYPE &operator*()                                                   \
    {                                                                          \
        return *operator->();                                                  \
    }                                                                          \
    static void destroy()                                                      \
    {                                                                          \
        _k_static_##NAME##_destroyed = true;                                   \
        TYPE *x = _k_static_##NAME;                                            \
        _k_static_##NAME.init(0);                                              \
        delete x;                                                              \
    }                                                                          \
} NAME;

/**
 * Access to the KDE global objects.
 * KGlobal provides you with pointers of many central
 * objects that exist only once in the process. It is also
 * responsible for managing instances of KStaticDeleterBase.
 *
 * @see KStaticDeleterBase
 * @author Sirtaj Singh Kang (taj@kde.org)
 */
class KDECORE_EXPORT KGlobal
{
public:

    /**
     * Returns the global instance.  There is always at least
     * one instance of a component in one application (in most
     * cases the application itself).
     * @return the global instance
     */
    static KInstance            *instance();

    /**
     *  Returns the application standard dirs object.
     * @return the global standard dir object
     */
    static KStandardDirs	*dirs();

    /**
     *  Returns the general config object.
     * @return the global configuration object.
     */
    static KConfig		*config();

    /**
     *  Returns the general config object.
     * @return the global configuration object.
     */
    static KSharedConfig        *sharedConfig();

    /**
     * Returns the global locale object.
     * @return the global locale object
     */
    static KLocale              *locale();

    /**
     * The global charset manager.
     * @return the global charset manager
     */
    static KCharsets	        *charsets();

    /**
     * Creates a static QString.
     *
     * To be used inside functions(!) like:
     * @code
     * static const QString &myString = KGlobal::staticQString("myText");
     * @endcode
     *
     * @attention Do @b NOT use code such as:
     * @code
     * static QString myString = KGlobal::staticQString("myText");
     * @endcode
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     * @param str the string to create
     * @return the static string
     */
    static const QString&        staticQString(const char *str);

    /**
     * Creates a static QString.
     *
     * To be used inside functions(!) like:
     * @code
     * static const QString &myString = KGlobal::staticQString(i18n("My Text"));
     * @endcode
     *
     * @attention Do @b NOT use code such as:
     * @code
     * static QString myString = KGlobal::staticQString(i18n("myText"));
     * @endcode
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     * @param str the string to create
     * @return the static string
     */
    static const QString&        staticQString(const QString &str);

    /**
     * Registers a static deleter.
     * @param d the static deleter to register
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    static void registerStaticDeleter(KStaticDeleterBase *d);

    /**
     * Unregisters a static deleter.
     * @param d the static deleter to unregister
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    static void unregisterStaticDeleter(KStaticDeleterBase *d);

    /**
     * Calls KStaticDeleterBase::destructObject() on all
     * registered static deleters and unregisters them all.
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    static void deleteStaticDeleters();

    /**
     * Tells KGlobal about one more operation that should be finished
     * before the application exits. The standard behavior is to exit on the
     * "last window closed" event, but some events should outlive the last window closed
     * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
     *
     * Note that for this to happen you must call qApp->setQuitOnLastWindowClosed(false),
     * in main() for instance.
     */
    static void ref();

    /**
     * Tells KGlobal that one operation such as those described in ref() just finished.
     * This call makes the QApplication quit if the counter is back to 0.
     */
    static void deref();

    //private:
    /** Data which ought to be private but isn't. @internal */
    static  KStringDict         *_stringDict;
    /** Data which ought to be private but isn't. @internal */
    static  KInstance           *_instance;
    /** Data which ought to be private but isn't. @internal */
    static  KLocale             *_locale;
    /** Data which ought to be private but isn't. @internal */
    static  KCharsets	        *_charsets;
    /** Data which ought to be private but isn't. @internal */
    static  KStaticDeleterList  *_staticDeleters;

    /**
     * The instance currently active (useful in a multi-instance
     * application, such as a KParts application).
     * Don't use this - it's mainly for KAboutDialog and KBugReport.
     * @internal
     */
    static KInstance *activeInstance() { return _activeInstance; }

    /**
     * Set the active instance for use by KAboutDialog and KBugReport.
     * To be used only by a multi-instance (KParts) application.
     *
     * @see activeInstance()
     */
    static void setActiveInstance(KInstance *d);

private:
    friend class KInstance;
    ///@internal
    static void setMainInstance( KInstance* i );

    static  KInstance           *_activeInstance;
};

#ifdef KDE_SUPPORT
/**
 * \relates KGlobal
 * A typesafe function to find the smaller of the two arguments.
 * @deprecated, used qMin instead
 */
#define KMIN(a,b)	qMin(a,b)
/**
 * \relates KGlobal
 * A typesafe function to find the larger of the two arguments.
 * @deprecated, used qMax instead
 */
#define KMAX(a,b)	qMax(a,b)
/**
 * \relates KGlobal
 * A typesafe function to determine the absolute value of the argument.
 * @deprecated, used qAbs instead
 */
#define KABS(a)	qAbs(a)
/**
 * \relates KGlobal
 * A typesafe function that returns x if it's between low and high values.
 * low if x is smaller than low and high if x is bigger than high.
 * @deprecated, used qBound instead. Warning, the argument order differs.
 */
#define KCLAMP(x,low,high) qBound(low,x,high)

#define kMin qMin
#define kMax qMax
#define kAbs qAbs

/**
 * \relates KGlobal
 * A typesafe function that returns x if it's between low and high values.
 * low if x is smaller than low and high if x is bigger than high.
 * @deprecated, used qBound instead. Warning, the argument order differs.
 */

template<class T>
inline KDE_DEPRECATED T kClamp( const T& x, const T& low, const T& high )
{
    if ( x < low )       return low;
    else if ( high < x ) return high;
                         return x;
}

#endif

#endif // _KGLOBAL_H

