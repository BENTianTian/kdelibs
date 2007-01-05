/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
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

#ifndef QT_NO_CAST_FROM_ASCII
#define QT_NO_CAST_FROM_ASCII
#endif
#ifndef QT_NO_CAST_TO_ASCII
#define QT_NO_CAST_TO_ASCII
#endif

#include "kdebug.h"

#ifdef NDEBUG
#undef kDebug
#undef kBacktrace
#endif

#include "kglobal.h"
#include "kinstance.h"
#include "kstandarddirs.h"
#include "kdatetime.h"

#include <QtGui/QWidget>
#include <kmessage.h>
#include <klocale.h>
#include <qfile.h>
#include <qhash.h>

#include <qstring.h>

#include <kurl.h>

#include <stdlib.h>	// abort
#include <unistd.h>	// getpid
#include <stdarg.h>	// vararg stuff
#include <ctype.h>      // isprint
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <kconfig.h>
#include "kstaticdeleter.h"
#include <config.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include "kdebugdbusiface_p.h"

struct KDebugEntry
{
    KDebugEntry (unsigned int n = 0, const QByteArray& d = QByteArray()) {number=n; descr=d;}
    unsigned int number;
    QByteArray descr;
};

typedef QHash<unsigned int, KDebugEntry> debug_cache;
static debug_cache *KDebugCache;

KDECORE_EXPORT bool kde_kdebug_enable_dbus_interface = false;

static KStaticDeleter< debug_cache > kdd;

static QByteArray getDescrFromNum(unsigned int _num)
{
  if (!KDebugCache) {
    kdd.setObject(KDebugCache, new debug_cache);
    // Do not call this deleter from ~KApplication
    KGlobal::unregisterStaticDeleter(&kdd);
  }

  if ( KDebugCache->contains( _num ) )
    return KDebugCache->value( _num ).descr;

  if ( !KDebugCache->isEmpty() ) // areas already loaded
    return QByteArray();

  QString filename(KStandardDirs::locate("config", QLatin1String("kdebug.areas")));
  if (filename.isEmpty())
      return QByteArray();

  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning("Couldn't open %s", filename.toLocal8Bit().constData());
    file.close();
    return QByteArray();
  }

  uint lineNumber=0;
  QByteArray line(1024, 0);
  int len;

  while (( len = file.readLine(line.data(),line.size()-1) ) > 0) {
      int i=0;
      ++lineNumber;

      while (line[i] && line[i] <= ' ')
        i++;

      unsigned char ch=line[i];

      if ( !ch || ch =='#' || ch =='\n')
          continue; // We have an eof, a comment or an empty line

      if (ch < '0' && ch > '9') {
          qWarning("Syntax error: no number (line %u)",lineNumber);
          continue;
      }

      const int numStart=i;
      do {
          ch=line[++i];
      } while ( ch >= '0' && ch <= '9');

      unsigned int number = line.mid(numStart, i).toUInt(); // ###

      while (line[i] && line[i] <= ' ')
        i++;

      KDebugCache->insert(number, KDebugEntry(number, line.mid(i, len-i-1)));
  }
  file.close();

  if ( KDebugCache->contains( _num ) )
      return KDebugCache->value( _num ).descr;

  return QByteArray();
}

enum DebugLevels {
    KDEBUG_INFO=    0,
    KDEBUG_WARN=    1,
    KDEBUG_ERROR=   2,
    KDEBUG_FATAL=   3
};


struct kDebugPrivate {
  kDebugPrivate() :
	oldarea(0), config(0) { }

  ~kDebugPrivate() { delete config; }

  QByteArray aAreaName;
  unsigned int oldarea;
  KConfig *config;
};

static kDebugPrivate *kDebug_data = 0;
static KStaticDeleter<kDebugPrivate> pcd;
static KStaticDeleter<KDebugDBusIface> dbussd;
static KDebugDBusIface* kDebugDBusIface = 0;

// ######## KDE4: kDebug is not threadsafe!  Races and crashes apps that call
//                it from both threads.  Let's rework this altogether, and
//                maybe make it faster too.
static void kDebugBackend( unsigned short nLevel, unsigned int nArea, const char *data)
{
  if ( !kDebug_data )
  {
      pcd.setObject(kDebug_data, new kDebugPrivate());
      // Do not call this deleter from ~KApplication
      KGlobal::unregisterStaticDeleter(&pcd);

      // Create the dbus interface if it has not been created yet
      // But only register to DBus if we are in a process with a dbus event loop,
      // otherwise introspection will just hang.
      // Examples of processes without a dbus event loop: kioslaves and the main kdeinit process.
      //
      // How to know that we have a real event loop? That's tricky.
      // We could delay registration in kDebugDBusIface with a QTimer, but
      // it would still get triggered by kioslaves that use enterLoop/exitLoop
      // to run kio jobs synchronously.
      //
      // Solution: we have a bool that is set by KApplication
      // (kioslaves should use QCoreApplication but not KApplication).
      if (!kDebugDBusIface && kde_kdebug_enable_dbus_interface)
      {
          kDebugDBusIface = dbussd.setObject(kDebugDBusIface, new KDebugDBusIface);
      }
  }

  if (!kDebug_data->config && KGlobal::_instance )
  {
      kDebug_data->config = new KConfig(QLatin1String("kdebugrc"), false, false);
      kDebug_data->config->setGroup( QLatin1String("0") );

      //AB: this is necessary here, otherwise all output with area 0 won't be
      //prefixed with anything, unless something with area != 0 is called before
      if ( KGlobal::_instance )
        kDebug_data->aAreaName = KGlobal::instance()->instanceName();
  }

  if (kDebug_data->config && kDebug_data->oldarea != nArea) {
    kDebug_data->config->setGroup( QString::number(nArea) );
    kDebug_data->oldarea = nArea;
    if ( nArea > 0 && KGlobal::_instance )
      kDebug_data->aAreaName = getDescrFromNum(nArea);
    if ((nArea == 0) || kDebug_data->aAreaName.isEmpty())
      if ( KGlobal::_instance )
        kDebug_data->aAreaName = KGlobal::instance()->instanceName();
  }

  int nPriority = 0;
  QString aCaption;

    /* Determine output */

  QString key;
  switch( nLevel )
  {
  case KDEBUG_INFO:
      key = QLatin1String( "InfoOutput" );
      aCaption = QLatin1String( "Info" );
      nPriority = LOG_INFO;
      break;
  case KDEBUG_WARN:
      key = QLatin1String( "WarnOutput" );
      aCaption = QLatin1String( "Warning" );
      nPriority = LOG_WARNING;
	break;
  case KDEBUG_FATAL:
      key = QLatin1String( "FatalOutput" );
      aCaption = QLatin1String( "Fatal Error" );
      nPriority = LOG_CRIT;
      break;
  case KDEBUG_ERROR:
  default:
      /* Programmer error, use "Error" as default */
      key = QLatin1String( "ErrorOutput" );
      aCaption = QLatin1String( "Error" );
      nPriority = LOG_ERR;
      break;
  }

  short nOutput = kDebug_data->config ? kDebug_data->config->readEntry(key, 2) : 2;

  if ( nOutput == 4 && nLevel != KDEBUG_FATAL )
      return;

  const int BUFSIZE = 4096;
  char buf[BUFSIZE];
#ifdef Q_WS_WIN
  sprintf(buf,"[%d] ",getpid());
#else
  strlcpy(buf,"",BUFSIZE);
#endif

  if ( !kDebug_data->aAreaName.isEmpty() ) {
      strlcat( buf, kDebug_data->aAreaName.data(), BUFSIZE );
      strlcat( buf, ": ", BUFSIZE );
      strlcat( buf, data, BUFSIZE );
  }
  else
      strlcat( buf, data, BUFSIZE );


  // Output
  switch( nOutput )
  {
  case 0: // File
  {
      const char* aKey;
      switch( nLevel )
      {
      case KDEBUG_INFO:
          aKey = "InfoFilename";
          break;
      case KDEBUG_WARN:
          aKey = "WarnFilename";
          break;
      case KDEBUG_FATAL:
          aKey = "FatalFilename";
          break;
      case KDEBUG_ERROR:
      default:
          aKey = "ErrorFilename";
          break;
      }
      QFile aOutputFile( kDebug_data->config->readPathEntry(aKey, QLatin1String( "kdebug.dbg" ) ) );
      aOutputFile.open( QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered );
      aOutputFile.write( buf, strlen( buf ) );
      aOutputFile.close();
      break;
  }
  case 1: // Message Box
  {
      // Since we are in kdecore here, we cannot use KMsgBox 
      if ( !kDebug_data->aAreaName.isEmpty() )
          aCaption += QString::fromAscii("(%1)").arg( QString::fromUtf8( kDebug_data->aAreaName.data() ) );
      KMessage::message( KMessage::Information , QString::fromUtf8( data ) , aCaption );
      break;
  }
  case 2: // Shell
  {
      write( 2, buf, strlen( buf ) );  //fputs( buf, stderr );
      break;
  }
  case 3: // syslog
  {
      syslog( nPriority, "%s", buf);
      break;
  }
  }

  // check if we should abort
  if( ( nLevel == KDEBUG_FATAL )
      && ( !kDebug_data->config ||
           kDebug_data->config->readEntry( "AbortFatal", true ) ) )
        abort();
}

kdbgstream &perror( kdbgstream &s)
{
    return s << QString::fromLocal8Bit(strerror(errno));
}
kdbgstream kDebug(int area)
{
    return kdbgstream(area, KDEBUG_INFO);
}
kdbgstream kDebug(bool cond, int area)
{
    if (cond)
        return kDebug(area);
    return kdbgstream(0, 0, false);
}

kdbgstream kError(int area)
{
    return kdbgstream("ERROR: ", area, KDEBUG_ERROR);
}
kdbgstream kError(bool cond, int area)
{
    if (cond)
        return kError(area);
    return kdbgstream(0,0,false);
}

kdbgstream kWarning(int area)
{
    return kdbgstream("WARNING: ", area, KDEBUG_WARN);
}
kdbgstream kWarning(bool cond, int area)
{
     if (cond)
         return kWarning(area);
     return kdbgstream(0,0,false);
}

kdbgstream kFatal(int area)
{
    return kdbgstream("FATAL: ", area, KDEBUG_FATAL);
}
kdbgstream kFatal(bool cond, int area)
{
    if (cond)
        return kFatal(area);
    return kdbgstream(0,0,false);
}

class kdbgstream::Private
{
public:
    QString output;
    unsigned int area, level;
    bool print;
    Private(const Private& p)
        : output(p.output), area(p.area), level(p.level), print(p.print) { ; }
    Private(const QString& str, uint a, uint lvl, bool p)
        : output(str), area(a), level(lvl), print(p)  { ; }
    Private(unsigned int a, unsigned int l, bool p)
        : area(a), level(l), print(p)  { ; }
};

kdbgstream::kdbgstream(unsigned int _area, unsigned int _level, bool _print)
    : d(new Private(_area, _level, _print))
{
}

kdbgstream::kdbgstream(const char * initialString, unsigned int _a,
                       unsigned int _lvl, bool _p)
    : d(new Private(QLatin1String(initialString), _a, _lvl, _p))
{
}

kdbgstream::kdbgstream(const kdbgstream &str)
    : d(new Private(*(str.d)))
{
    str.d->output.truncate(0);
}

void kdbgstream::flush() {
    if (d->output.isEmpty() || !d->print)
	return;
    kDebugBackend( d->level, d->area, d->output.toLocal8Bit().constData() );
    d->output.clear();
}

kdbgstream &kdbgstream::form(const char *format, ...)
{
    if (!d->print)
        return *this;

    char buf[4096];
    va_list arguments;
    va_start( arguments, format );
    qvsnprintf( buf, sizeof(buf), format, arguments );
    va_end(arguments);
    *this << buf;

    return *this;
}

kdbgstream::~kdbgstream()
{
    if (!d->output.isEmpty()) {
	fprintf(stderr, "ASSERT: debug output not ended with \\n\n");
        fprintf(stderr, "%s", qPrintable( kBacktrace() ) );
	*this << "\n";
    }
    delete d;
}

kdbgstream& kdbgstream::operator << (QChar ch)
{
    if (!d->print)
        return *this;

    if (!ch.isPrint())
        d->output += QLatin1String("\\x")
	          + QString::number(ch.unicode(), 16).rightJustified(2, QLatin1Char('0') );
    else {
        d->output += ch;
        if ( ch == QLatin1Char( '\n' ) )
            flush();
    }

    return *this;
}

kdbgstream& kdbgstream::operator<<(const QString& string)
{
    if ( !d->print )
        return *this;

    d->output += string;
    if ( d->output.length() && d->output.at(d->output.length() -1 ) == QLatin1Char('\n') )
        flush();
    return *this;
}

kdbgstream& kdbgstream::operator << (const QWidget* widget)
{
  if (!d->print)
      return *this;

  if(widget==0) {
      d->output += QLatin1String("[Null pointer]");
  } else {
      d->output += QString::fromAscii("[%1 pointer(0x%2)")
                         .arg(QString::fromUtf8(widget->metaObject()->className()))
                         .arg(QString::number(ulong(widget), 16)
		              .rightJustified(8, QLatin1Char('0')));
      if(widget->objectName().isEmpty()) {
	  d->output += QLatin1String( " to unnamed widget, " );
      } else {
	  d->output += QString::fromAscii(" to widget %1, ")
	                    .arg(widget->objectName());
      }
      d->output += QString::fromAscii("geometry=%1x%2+%3+%4]")
                       .arg(widget->width()).arg(widget->height())
                       .arg(widget->x()).arg(widget->y());
  }
  return *this;
}

/*
 * When printing a string:
 *  - if no newline can possibly be in the string, use d->output directly
 *  - otherwise you have two choices:
 *      a) use d->output and do the flush if needed
 *      b) or use the QString operator which calls the char* operator
 */
kdbgstream& kdbgstream::operator<<( const KDateTime& time) {
    if ( d->print ) {
        if ( time.isDateOnly() )
            d->output += time.toString(KDateTime::QtTextDate);
        else
            d->output += time.toString(KDateTime::ISODate);
    }
    return *this;
}
kdbgstream& kdbgstream::operator<<( KDBGFUNC f ) {
    if ( d->print )
        return (*f)(*this);
    return *this;
}
kdbgstream& kdbgstream::operator<<( const KUrl& u ) {
    if ( d->print )
        d->output += u.prettyUrl();
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QByteArray& data) {
    if (!d->print) return *this;
    bool isBinary = false;
    for ( int i = 0; i < data.size() && !isBinary ; ++i ) {
        if ( data[i] < 32 || (unsigned char)data[i] > 127 )
            isBinary = true;
    }
    if ( isBinary ) {
        d->output += QLatin1Char('[');
        int sz = qMin( data.size(), 64 );
        for ( int i = 0; i < sz ; ++i ) {
            d->output += QString::number( (unsigned char) data[i], 16 ).rightJustified(2, QLatin1Char('0'));
            if ( i < sz )
                d->output += QLatin1Char(' ');
        }
        if ( sz < data.size() )
            d->output += QLatin1String("...");
        d->output += QLatin1Char(']');
    } else {
        d->output += QLatin1String( data ); // using ascii as advertised
    }
    return *this;
}

QString kBacktrace(int levels)
{
    QString s;
#ifdef HAVE_BACKTRACE
    void* trace[256];
    int n = backtrace(trace, 256);
    if (!n)
	return s;
    char** strings = backtrace_symbols (trace, n);

    if ( levels != -1 )
        n = qMin( n, levels );
    s = QLatin1String("[\n");

    for (int i = 0; i < n; ++i)
        s += QString::number(i) +
             QLatin1String(": ") +
             QLatin1String(strings[i]) + QLatin1String("\n");
    s += QLatin1String("]\n");
    if (strings)
        free (strings);
#endif
    return s;
}

void kClearDebugConfig()
{
    if (!kDebug_data) return;
    delete kDebug_data->config;
    kDebug_data->config = 0;
}

// Needed for --enable-final
#ifdef NDEBUG
#define kDebug kndDebug
#endif
