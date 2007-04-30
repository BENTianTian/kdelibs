/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2000-2002 Stephan Kulow (coolo@kde.org)
                  2002 Holger Freyther (freyther@kde.org)

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

#ifndef _KDEBUG_H_
#define _KDEBUG_H_

#include <kdecore_export.h>

#include <QtCore/QDebug>
#include <QtCore/QCharRef>

class QObject;
class QWidget;
class KDateTime;
class KUrl;

class kdbgstream;
class kndbgstream;

/**
 * \addtogroup kdebug Debug message generators
 *  @{
 * KDE debug message streams let you and the user control just how many debug
 * messages you see. Debug message printing is controlled by (un)defining
 * NDEBUG when compiling your source. If NDEBUG is defined then no debug
 * messages are printed.
 */

typedef kdbgstream & (*KDBGFUNC)(kdbgstream &);    ///< manipulator function
typedef kndbgstream & (*KNDBGFUNC)(kndbgstream &); ///< manipulator function

/**
 * An indicator of where you are in a source file, to be used in
 * warnings (perhaps debug messages too).
 *
 * Extra pretty with GNU C, same as k_lineinfo without.
 */
#ifdef __GNUC__
#define k_funcinfo "[" << __PRETTY_FUNCTION__ << "] "
#else
#define k_funcinfo "[" << __FILE__ << ":" << __LINE__ << "] "
#endif

/**
 * An indicator of where you are in a source file, to be used in
 * warnings (perhaps debug messages too). Gives an accurate
 * idea of where the message comes from. Not suitable for
 * user-visible messages.
 *
 */
#define k_lineinfo "[" << __FILE__ << ":" << __LINE__ << "] "

/**
 * kdbgstream is a text stream that allows you to print debug messages.
 * Using the overloaded "<<" operator you can send messages. Usually
 * you do not create the kdbgstream yourself, but use kDebug()
 * kWarning(), kError() or kFatal() to obtain one.
 *
 * Example:
 * \code
 *    int i = 5;
 *    kDebug() << "The value of i is " << i << endl;
 * \endcode
 */
class KDECORE_EXPORT kdbgstream {
 public:
    /**
     * Create for a given area and severity; suppress printing if
     * @p _print is @c false.
     * @internal
     */
    kdbgstream(unsigned int _area, unsigned int _level, bool _print = true);
    /**
     * Create for a given area and severity; suppress printing if
     * @p _print is @c false. The string @p initialString is printed
     * before each line of output.
     * @internal
     */
    kdbgstream(const char * initialString, unsigned int _area, unsigned int _level, bool _print = true);
    /// Copy constructor
    kdbgstream(const kdbgstream &str);
    virtual ~kdbgstream();

    /**
     * Prints the given value.
     * @param i the boolean to print (as "true" or "false")
     * @return this stream
     */
    kdbgstream &operator<<(bool i)  {
        return *this << QString::fromLatin1(i ? "true" : "false" );
    }
    /**
     * Prints the given value.
     * @param i the short to print
     * @return this stream
     */
    kdbgstream &operator<<(short i)  {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the unsigned short to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned short i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param ch the char to print
     * @return this stream
     */
    kdbgstream& operator<<(char ch) {
        return *this << QLatin1Char(ch);
    }
    /**
     * Prints the given value.
     * @param ch the unsigned char to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned char ch) {
        return *this << QLatin1Char(ch);
    }
    /**
     * Prints the given value.
     * @param i the int to print
     * @return this stream
     */
    kdbgstream &operator<<(int i)  {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the unsigned int to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned int i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the long to print
     * @return this stream
     */
    kdbgstream &operator<<(long i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the unsigned long to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned long i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the long long to print
     * @return this stream
     */
    kdbgstream &operator<<(qlonglong i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the unsigned long long to print
     * @return this stream
     */
    kdbgstream &operator<<(qulonglong i) {
        return *this << QString::number( i );
    }

    /**
     * Flushes the output.
     */
    virtual void flush();

    /**
     * Prints the given value.
     * @param ch the char to print
     * @return this stream
     */
    kdbgstream &operator<<(QChar ch);
    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    kdbgstream &operator<<(const QString& string);
    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    kdbgstream &operator<<(const char *string) {
        return *this << QString::fromUtf8(string);
    }
    /**
     * Prints the given value.
     * @param p a pointer to print (in number form)
     * @return this stream
     */
    kdbgstream& operator<<(const void * p) {
        return form("%p", p);
    }
    /**
     * Invokes the given function.
     * @param f the function to invoke
     * @return the return value of @p f
     */
    kdbgstream& operator<<(KDBGFUNC f);
    /**
     * Prints the given value.
     * @param f the double to print
     * @return this stream
     */
    kdbgstream& operator<<(double f) {
        return *this << QString::number( f );
    }
    /**
     * Prints the string @p format which can contain
     * printf-style formatted values.
     * @param format the printf-style format
     * @return this stream
     */
    kdbgstream& form(const char *format, ...)
#ifdef __GNUC__
      __attribute__ ( ( format ( printf, 2, 3 ) ) )
#endif
     ;

    /** Operator to print out basic information about a QObject or QWidget.
     *  Output of class names only works if the class is moc'ified.
     *  Prints the geometry of the object if it is a QWidget
     * @param widget the widget to print
     * @return this stream
     */
    kdbgstream& operator<<( QObject* object );
    kdbgstream& operator<<( const QObject* object );
    kdbgstream& operator<<( QWidget* object );
    kdbgstream& operator<<( const QWidget* object );

    /**
     * Prints the given value.
     * @param dateTime the datetime to print
     * @return this stream
     */
    kdbgstream& operator << ( const KDateTime& dateTime );

    /**
     * Prints the given value.
     * @param url the url to print
     * @return this stream
     */
    kdbgstream& operator << ( const KUrl& url );

    /**
     * Prints the given bytearray value, interpreting it as either
     * a raw byte array (if it contains non-printable characters)
     * or as a ascii string otherwise.
     * @param data the byte array to print
     * @return this stream
     */
    kdbgstream& operator << ( const QByteArray& data );

    /**
     * Fallback that invokes QDebug for all types that don't have
     * a kdebstream operator.
     * @param t the value to be printed
     * @return this stream
     */
    template <typename T>
    kdbgstream& operator << (const T &t);

 private:
    class Private;
    Private* const d;
};

template <typename T>
kdbgstream &kdbgstream::operator<<(const T &t)
{
#if defined(QT_NO_DEBUG_STREAM)
    return *this;
#else
    QString out;
    QDebug qdbg(&out);
    qdbg << t;
    return operator<<(out);
#endif
}

/**
 * \relates KGlobal
 * Prints a newline to the stream.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 */
inline kdbgstream &endl( kdbgstream &s) { return s << "\n"; }

/**
 * \relates KGlobal
 * Flushes the stream.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 */
inline kdbgstream &flush( kdbgstream &s) { s.flush(); return s; }

/**
 * \relates KGlobal
 * Print a message describing the last system error.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 * @see perror(3)
 */
KDECORE_EXPORT kdbgstream &perror( kdbgstream &s);

/**
 * @internal
 * kndbgstream is a dummy variant of kdbgstream, it is only here to allow
 * compiling with/without debugging messages.
 * All functions do nothing.
 * @see kdbgstream
 */
class KDECORE_EXPORT kndbgstream { //krazy:exclude=dpointer (dummy class - the rules don't apply)
 public:
    // Do not add dummy API docs to all methods, just ensure the whole class is skipped by doxygen

    kndbgstream() {}
    ~kndbgstream() {}
    kndbgstream& operator<<(KNDBGFUNC) { return *this; }
    template <typename T>
    kndbgstream& operator<<(const T &) { return *this; }
    void flush() {}
    kndbgstream &form(const char *, ...) { return *this; }
};

inline kndbgstream &endl( kndbgstream & s) { return s; }
inline kndbgstream &flush( kndbgstream & s) { return s; }
inline kndbgstream &perror( kndbgstream & s) { return s; }

/**
 * \relates KGlobal
 * Returns a debug stream. You can use it to print debug
 * information.
 * @param area an id to identify the output, 0 for default
 * @see kndDebug()
 */
KDECORE_EXPORT kdbgstream kDebug(int area = 0);
static inline KDE_DEPRECATED kdbgstream kdDebug(int area = 0) { return kDebug( area ); }
/**
 * \relates KGlobal
 * Returns a debug stream. You can use it to conditionally
 * print debug information.
 * @param cond the condition to test, if true print debugging info
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kDebug(bool cond, int area = 0);
static inline KDE_DEPRECATED kdbgstream kdDebug(bool cond, int area = 0) { return kDebug( cond, area ); }

/**
 * \relates KGlobal
 * Returns a backtrace.
 * @param levels the number of levels of the backtrace
 * @return a backtrace
 */
KDECORE_EXPORT QString kBacktrace(int levels=-1);
static inline KDE_DEPRECATED QString kdBacktrace(int levels=-1) { return kBacktrace( levels ); }

inline kndbgstream kndDebug(int = 0) { return kndbgstream(); }
inline kndbgstream kndDebug(bool , int  = 0) { return kndbgstream(); }
inline QString kndBacktrace(int = -1) { return QString(); }

/**
 * \relates KGlobal
 * Returns a warning stream. You can use it to print warning
 * information.
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kWarning(int area = 0);
static inline KDE_DEPRECATED kdbgstream kdWarning(int area = 0) { return kWarning( area ); }
/**
 * \relates KGlobal
 * Returns a warning stream. You can use it to conditionally
 * print warning information.
 * @param cond the condition to test, if true print warning
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kWarning(bool cond, int area = 0);
static inline KDE_DEPRECATED kdbgstream kdWarning(bool cond, int area = 0) { return kWarning( cond, area ); }
/**
 * \relates KGlobal
 * Returns an error stream. You can use it to print error
 * information.
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kError(int area = 0);
static inline KDE_DEPRECATED kdbgstream kdError(int area = 0) { return kError( area ); }
/**
 * \relates KGlobal
 * Returns an error stream. You can use it to conditionally
 * print error information
 * @param cond the condition to test, if true print error
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kError(bool cond, int area = 0);
static inline KDE_DEPRECATED kdbgstream kdError(bool cond, int area = 0) { return kError( cond, area ); }
/**
 * \relates KGlobal
 * Returns a fatal error stream. You can use it to print fatal error
 * information.
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kFatal(int area = 0);
static inline KDE_DEPRECATED kdbgstream kdFatal(int area = 0) { return kFatal( area ); }
/**
 * \relates KGlobal
 * Returns a fatal error stream. You can use it to conditionally
 * print error information
 * @param cond the condition to test, if true print error
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kFatal(bool cond, int area = 0);
static inline KDE_DEPRECATED kdbgstream kdFatal(bool cond, int area = 0) { return kFatal( cond, area ); }

/**
 * \relates KGlobal
 * Deletes the kdebugrc cache and therefore forces KDebug to reread the
 * config file
 */
KDECORE_EXPORT void kClearDebugConfig();
static inline KDE_DEPRECATED void kdClearDebugConfig() { return kClearDebugConfig(); }

/** @} */

#ifdef NDEBUG
#define kDebug kndDebug
#define kBacktrace kndBacktrace
#endif

#endif

