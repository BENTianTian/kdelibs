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

#include <kdecore_export.h>
#include <QtCore/QAtomicPointer>

class KComponentData;
class KCharsets;
class KConfig;
class KLocale;
class KStandardDirs;
class KStaticDeleterBase;
class QString;
class KSharedConfigPtr;

/**
 * \internal
 */
typedef void (*KdeCleanUpFunction)();

/**
 * \internal
 *
 * Helper class for K_GLOBAL_STATIC to clean up the object on library unload or application
 * shutdown.
 */
class KCleanUpGlobalStatic
{
    public:
        KdeCleanUpFunction func;

        inline ~KCleanUpGlobalStatic() { func(); }
};

#ifdef Q_CC_MSVC
/**
 * \internal
 *
 * MSVC seems to give anonymous structs the same name which fails at link time. So instead we name
 * the struct and hope that by adding the line number to the name it's unique enough to never clash.
 */
# define K_GLOBAL_STATIC_STRUCT_NAME(NAME) _k_##NAME##__LINE__
#else
/**
 * \internal
 *
 * Make the struct of the K_GLOBAL_STATIC anonymous.
 */
# define K_GLOBAL_STATIC_STRUCT_NAME(NAME)
#endif

/**
 * This macro makes it easy to use non-POD types as global statics.
 * The object is created on first use and creation is threadsafe.
 *
 * The object is destructed on library unload or application exit.
 * Be careful with calling other objects in the destructor of the class
 * as you have to be sure that they (or objects they depend on) are not already destructed.
 *
 * \param TYPE The type of the global static object. Do not add a *.
 * \param NAME The name of the function to get a pointer to the global static object.
 *
 * If you have code that might be called after the global object has been destroyed you can check
 * for that using the isDestroyed() function.
 *
 * If needed you can also install a post routine (\ref qAddPostRoutine) to clean up the object
 * using the destroy() method.
 *
 * Example:
 * \code
 * class A { ... };
 *
 * K_GLOBAL_STATIC(A, globalA)
 * // The above creates a new globally static variable named 'globalA' which you
 * // can use as a pointer to an instance of A.
 *
 * void doSomething()
 * {
 *     //  The first time you acess globalA a new instance of A will be created automatically.
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
 *     // A post routine can be used to delete the object when QCoreApplication destructs,
 *     // not adding such a post routine will delete the object normally at program unload
 *     qAddPostRoutine(globalA.destroy);
 * }
 * \endcode
 *
 * A common case for the need of deletion on lib unload/app shutdown are Singleton classes. Here's
 * an example how to do it:
 * \code
 * class MySingletonPrivate;
 * class EXPORT_MACRO MySingleton
 * {
 * friend class MySingletonPrivate;
 * public:
 *     static MySingleton *self();
 *     QString someFunction();
 *
 * private:
 *     MySingleton();
 *     ~MySingleton();
 * };
 * \endcode
 * in the .cpp file:
 * \code
 * // This class will be instantiated and referenced as a singleton in this example
 * class MySingletonPrivate
 * {
 * public:
 *     QString foo;
 *     MySingleton instance;
 * };
 *
 * K_GLOBAL_STATIC(MySingletonPrivate, mySingletonPrivate)
 *
 * MySingleton *MySingleton::self()
 * {
 *     // returns the singleton; automatically creates a new instance if that has not happened yet.
 *     return &mySingletonPrivate->instance;
 * }
 * QString MySingleton::someFunction()
 * {
 *     // Refencing the singleton directly is possible for your convenience
 *     return mySingletonPrivate->foo;
 * }
 * \endcode
 *
 * Instead of the above you can use also the following pattern (ignore the name of the namespace):
 * \code
 * namespace MySingleton
 * {
 *     EXPORT_MACRO QString someFunction();
 * }
 * \endcode
 * in the .cpp file:
 * \code
 * class MySingletonPrivate
 * {
 * public:
 *     QString foo;
 * };
 *
 * K_GLOBAL_STATIC(MySingletonPrivate, mySingletonPrivate)
 *
 * QString MySingleton::someFunction()
 * {
 *     return mySingletonPrivate->foo;
 * }
 * \endcode
 *
 * Now code that wants to call someFunction() doesn't have to do
 * \code
 * MySingleton::self()->someFunction();
 * \endcode
 * anymore but instead:
 * \code
 * MySingleton::someFunction();
 * \endcode
 */
#define K_GLOBAL_STATIC(TYPE, NAME) K_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ())

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
 * class A
 * {
 * public:
 *     A(const char *s, int i);
 *     ...
 * };
 *
 * K_GLOBAL_STATIC_WITH_ARG(A, globalA, ("foo", 0))
 * // The above creates a new globally static variable named 'globalA' which you
 * // can use as a pointer to an instance of A.
 *
 * void doSomething()
 * {
 *     //  The first time you acess globalA a new instance of A will be created automatically.
 *     A *a = globalA;
 *     ...
 * }
 * \endcode
 */
#define K_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                            \
static QBasicAtomicPointer<TYPE > _k_static_##NAME = Q_ATOMIC_INIT(0);         \
static bool _k_static_##NAME##_destroyed;                                      \
static struct K_GLOBAL_STATIC_STRUCT_NAME(NAME)                                \
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
            if (!_k_static_##NAME.testAndSet(0, x)                             \
                && _k_static_##NAME != x ) {                                   \
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
     * Returns the global component data.  There is always at least
     * one instance of a component in one application (in most
     * cases the application itself).
     * @return the global component data
     */
    static const KComponentData &mainComponent();

    /**
     * \internal
     * Returns whether a main KComponentData is available.
     */
    static bool hasMainComponent();

    /**
     *  Returns the application standard dirs object.
     * @return the global standard dir object
     */
    static KStandardDirs *dirs();

    /**
     *  Returns the general config object.
     * @return the global configuration object.
     */
    static KSharedConfigPtr config();

    /**
     * Returns the global locale object.
     * @return the global locale object
     */
    static KLocale              *locale();
    /**
     * \internal
     * Returns whether KGlobal has a valid KLocale object
     */
    static bool hasLocale();

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

    /**
     * The component currently active (useful in a multi-component
     * application, such as a KParts application).
     * Don't use this - it's mainly for KAboutDialog and KBugReport.
     * @internal
     */
    static const KComponentData &activeComponent();

    /**
     * Set the active component for use by KAboutDialog and KBugReport.
     * To be used only by a multi-component (KParts) application.
     *
     * @see activeComponent()
     */
    static void setActiveComponent(const KComponentData &d);

    /**
     * Returns a text for the window caption.
     *
     * This may be set by
     * "-caption", otherwise it will be equivalent to the name of the
     * executable.
     * @return the text for the window caption
     */
    static QString caption();

    ///@internal
    static void setLocale(KLocale *);
private:
    friend class KComponentData;
    ///@internal
    static void newComponentData(KComponentData *c);
    static void deletedComponentData(KComponentData *c);
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

