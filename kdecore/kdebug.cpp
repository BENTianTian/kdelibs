/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
*/

// $Id$

// Include our header without NDEBUG defined to avoid having the kDebugInfo
// functions inlined to noops (which would then conflict with their definition
// here).

#ifdef NDEBUG
#define NODEBUG
#undef NDEBUG
#endif
#include "kdebug.h"
#ifdef NODEBUG
#define NDEBUG
#undef NODEBUG
#endif

// Other includes needed by kDebug* functions.

#include "kglobal.h"
#include "kinstance.h"
#include "kstddirs.h"
#include <qmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdlib.h>	// abort
#include <stdarg.h>	// vararg stuff
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <kconfig.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class KDebugEntry;
template class QList<KDebugEntry>;

class KDebugEntry
{
public:
  KDebugEntry (int n, QString d) {number=n; descr=d.copy();}
  KDebugEntry (QString d, int n) {number=n; descr=d.copy();}
  unsigned int number;
  QString descr;
};

static QList<KDebugEntry> *KDebugCache;
#define MAX_CACHE 20

static QString getDescrFromNum(unsigned short _num)
{
  if (!KDebugCache)
      KDebugCache = new QList<KDebugEntry>;

  for ( KDebugEntry *ent = KDebugCache->first();
		  ent != 0; ent = KDebugCache->next()) {
	  if (ent->number == _num) {
		  return ent->descr;
	  }
  }

  QString data, filename(locate("config","kdebug.areas"));
  QFile file(filename);
  if (!file.open(IO_ReadOnly)) {
    warning("Couldn't open %s", filename.local8Bit().data());
    file.close();
    return "";
  }

  unsigned long number;
  bool longOK;

  QTextStream *ts = new QTextStream(&file);
  while (!ts->eof()) {
    data = ts->readLine().simplifyWhiteSpace();

    int pos = data.find("#");
    if ( pos != -1 )
      data.truncate( pos );

    if (data.isEmpty())
      continue;

    int space = data.find(" ");
    if (space == -1)
      continue;

    number = data.left(space).toULong(&longOK);
    if (!longOK)
      continue; // The first part wasn't a number

    if (number != _num)
      continue; // Not the number we're looking for

    data.remove(0, space+1);

    if (KDebugCache->count() >= MAX_CACHE)
      KDebugCache->removeFirst();
    KDebugCache->append(new KDebugEntry(number,data));
    delete ts;
    file.close();
    return data;
  }

  delete ts;
  file.close();
  return "";
}

enum DebugLevels {
    KDEBUG_INFO=    0,
    KDEBUG_WARN=    1,
    KDEBUG_ERROR=   2,
    KDEBUG_FATAL=   3
};

static void kDebugBackend( unsigned short nLevel, unsigned short nArea, const char *data)
{
  static KConfig * pConfig = 0L;
  if ( !pConfig && KGlobal::_instance )
  {
      pConfig = new KConfig( "kdebugrc", false );
      pConfig->setGroup("0");
  }

  static QString aAreaName;
  static int oldarea = 0;
  if (pConfig && oldarea != nArea) {
    pConfig->setGroup( QString::number(static_cast<int>(nArea)) );
    oldarea = nArea;
    if ( nArea > 0)
      aAreaName = getDescrFromNum(nArea);
    if ((nArea == 0) || aAreaName.isEmpty())
      aAreaName = KGlobal::instance()->instanceName();
  }

  static short nOutput = 2;
  static ushort oldLevel = KDEBUG_FATAL + 1;
  static int nPriority = 0;
  static QString aCaption;

  if (oldLevel != nLevel) {
    /* Determine output */

    QString key;
    switch( nLevel )
      {
      case KDEBUG_INFO:
	key = "InfoOutput";
	aCaption = "Info";
	nPriority = LOG_INFO;
	break;
      case KDEBUG_WARN:
	key = "WarnOutput";
	aCaption = "Warning";
	nPriority = LOG_WARNING;
	break;
      case KDEBUG_FATAL:
	key = "FatalOutput";
	aCaption = "Fatal Error";
	nPriority = LOG_CRIT;
	break;
      case KDEBUG_ERROR:
      default:
	/* Programmer error, use "Error" as default */
	key = "ErrorOutput";
	aCaption = "Error";
	nPriority = LOG_ERR;
	break;
      }

    nOutput = pConfig ? pConfig->readNumEntry(key, 2) : 2;
  }



  // Output
  switch( nOutput )
        {
        case 0: // File
          {
                QString aKey;
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
                QString aOutputFileName = pConfig->readEntry(aKey, "kdebug.dbg");

                char buf[4096] = "";
		int nSize;
                if ( !aAreaName.isEmpty() )
		    nSize = sprintf( buf, "%s: %s", aAreaName.ascii(), data);
		else
		    nSize = sprintf( buf, "%s", data);

                QFile aOutputFile( aOutputFileName );
                aOutputFile.open( IO_WriteOnly );
                aOutputFile.writeBlock( buf, nSize+1 );
                aOutputFile.close();
                break;
          }
        case 1: // Message Box
          {
                // Since we are in kdecore here, we cannot use KMsgBox and use
                // QMessageBox instead
	      if ( !aAreaName.isEmpty() ) aCaption += QString("(")+aAreaName+")";
	      QMessageBox::warning( 0L, aCaption, data, i18n("&OK") );
	      break;
          }
        case 2: // Shell
          {
	      if ( !aAreaName.isEmpty() ) fprintf( stderr, "%s: ", aAreaName.ascii() );
	      fputs(  data, stderr);
	      break;
          }
        case 3: // syslog
          {
	      syslog( nPriority, data);
          }
        case 4: // nothing
          {
          }
        }

  // check if we should abort
  if( ( nLevel == KDEBUG_FATAL ) && pConfig &&
          ( pConfig->readNumEntry( "AbortFatal", 1 ) ) )
        abort();

#ifdef NDEBUG
  delete pConfig;
  pConfig = 0;
#endif
}

static void kDebugBackend2( unsigned short nLevel, unsigned short nArea, const char* fmt, va_list arguments )
{
    char buf[4096] = "";
    int nSize = vsnprintf( buf, 4096, fmt, arguments );
    if( nSize > 4094 ) nSize = 4094;
    buf[nSize] = '\n';
    buf[nSize+1] = '\0';
    kDebugBackend( nLevel, nArea, buf);
}

void kDebugInfo( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_INFO, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugInfo( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_INFO, area, fmt, arguments  );
    va_end( arguments );
}

void kDebugInfo( bool cond, unsigned short area, const char* fmt, ... )
{
  if(cond)
    {
      va_list arguments;
      va_start( arguments, fmt );
      kDebugBackend2( KDEBUG_INFO, area, fmt, arguments );
      va_end( arguments );
    }
}

void kDebugWarning( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_WARN, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugWarning( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_WARN, area, fmt, arguments );
    va_end( arguments );
}

void kDebugWarning( bool cond, unsigned short area, const char* fmt, ... )
{
  if(cond)
    {
      va_list arguments;
      va_start( arguments, fmt );
      kDebugBackend2( KDEBUG_INFO, area, fmt, arguments );
      va_end( arguments );
    }
}

void kDebugError( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_ERROR, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugError( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_ERROR, area, fmt, arguments );
    va_end( arguments );
}

void kDebugError( bool cond, unsigned short area, const char* fmt, ... )
{
  if(cond)
    {
      va_list arguments;
      va_start( arguments, fmt );
      kDebugBackend2( KDEBUG_INFO, area, fmt, arguments );
      va_end( arguments );
    }
}

void kDebugFatal(const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_FATAL, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugFatal(unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_FATAL, area, fmt, arguments );
    va_end( arguments );
}

void kDebugFatal( bool cond, unsigned short area, const char* fmt, ... )
{
  if(cond)
    {
      va_list arguments;
      va_start( arguments, fmt );
      kDebugBackend2( KDEBUG_INFO, area, fmt, arguments );
      va_end( arguments );
    }
}

void kDebugPError( const char* fmt, ... )
{
    char buf[4096];
    va_list arguments;
    va_start( arguments, fmt );
    vsprintf( buf, fmt, arguments );
    kDebugError( "%s: %s", buf, strerror(errno) );
}

void kDebugPError( unsigned short area, const char* fmt, ... )
{
    char buf[4096];
    va_list arguments;
    va_start( arguments, fmt );
    vsprintf( buf, fmt, arguments );
    kDebugError( area, "%s: %s", buf, strerror(errno) );
}

kdbgstream &perror( kdbgstream &s) { return s << " " << strerror(errno); }
kdbgstream kdDebug(int area) { return kdbgstream(area, KDEBUG_INFO); }
kdbgstream kdDebug(bool cond, int area) { if (cond) return kdbgstream(area, KDEBUG_INFO); else return kdbgstream(0, 0, false); }

kdbgstream kdError(int area) { return kdbgstream(area, KDEBUG_ERROR); }

void kdbgstream::flush() {
    if (output.isEmpty() || !print)
	return;
    kDebugBackend( level, area, output.local8Bit().data() );
    output = QString::null;
}

kdbgstream::~kdbgstream() {
    if (!output.isEmpty()) {
	fprintf(stderr, "ASSERT: debug output not ended with \\n\n");
	*this << "\n";
    }
}
