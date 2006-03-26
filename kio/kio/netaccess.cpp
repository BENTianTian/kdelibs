/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999 David Faure (faure@kde.org)

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

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <cstring>

#include <qstring.h>
#include <qapplication.h>
#include <qfile.h>
#include <qmetaobject.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/scheduler.h>

#include "kio/netaccess.h"

using namespace KIO;

QString * NetAccess::lastErrorMsg;
int NetAccess::lastErrorCode = 0;
QStringList* NetAccess::tmpfiles;

bool NetAccess::download(const KUrl& u, QString & target, QWidget* window)
{
  if (u.isLocalFile()) {
    // file protocol. We do not need the network
    target = u.path();
    bool accessible = checkAccess(target, R_OK);
    if(!accessible)
    {
        if(!lastErrorMsg)
            lastErrorMsg = new QString;
        *lastErrorMsg = i18n("File '%1' is not readable").arg(target);
        lastErrorCode = ERR_COULD_NOT_READ;
    }
    return accessible;
  }

  if (target.isEmpty())
  {
      KTempFile tmpFile;
      target = tmpFile.name();
      if (!tmpfiles)
          tmpfiles = new QStringList;
      tmpfiles->append(target);
  }

  NetAccess kioNet;
  KUrl dest;
  dest.setPath( target );
  return kioNet.filecopyInternal( u, dest, -1, true /*overwrite*/,
                                  false, window, false /*copy*/);
}

bool NetAccess::upload(const QString& src, const KUrl& target, QWidget* window)
{
  if (target.isEmpty())
    return false;

  // If target is local... well, just copy. This can be useful
  // when the client code uses a temp file no matter what.
  // Let's make sure it's not the exact same file though
  if (target.isLocalFile() && target.path() == src)
    return true;

  NetAccess kioNet;
  KUrl s;
  s.setPath(src);
  return kioNet.filecopyInternal( s, target, -1, true /*overwrite*/,
                                  false, window, false /*copy*/ );
}

bool NetAccess::copy( const KUrl & src, const KUrl & target, QWidget* window )
{
  return NetAccess::file_copy( src, target, -1, false /*not overwrite*/, false, window );
}

bool NetAccess::file_copy( const KUrl& src, const KUrl& target, int permissions,
                           bool overwrite, bool resume, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.filecopyInternal( src, target, permissions, overwrite, resume,
                                  window, false /*copy*/ );
}


bool NetAccess::file_move( const KUrl& src, const KUrl& target, int permissions,
                           bool overwrite, bool resume, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.filecopyInternal( src, target, permissions, overwrite, resume,
                                  window, true /*move*/ );
}

bool NetAccess::dircopy( const KUrl & src, const KUrl & target, QWidget* window )
{
  KUrl::List srcList;
  srcList.append( src );
  return NetAccess::dircopy( srcList, target, window );
}

bool NetAccess::dircopy( const KUrl::List & srcList, const KUrl & target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, false /*copy*/ );
}

bool NetAccess::move( const KUrl& src, const KUrl& target, QWidget* window )
{
  KUrl::List srcList;
  srcList.append( src );
  return NetAccess::move( srcList, target, window );
}

bool NetAccess::move( const KUrl::List& srcList, const KUrl& target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, true /*move*/ );
}

bool NetAccess::exists( const KUrl & url, bool source, QWidget* window )
{
  if ( url.isLocalFile() )
    return QFile::exists( url.path() );
  NetAccess kioNet;
  return kioNet.statInternal( url, 0 /*no details*/, source, window );
}

bool NetAccess::stat( const KUrl & url, KIO::UDSEntry & entry, QWidget* window )
{
  NetAccess kioNet;
  bool ret = kioNet.statInternal( url, 2 /*all details*/, true /*source*/, window );
  if (ret)
    entry = kioNet.m_entry;
  return ret;
}

KUrl NetAccess::mostLocalURL(const KUrl & url, QWidget* window)
{
  if ( url.isLocalFile() )
  {
    return url;
  }

  KIO::UDSEntry entry;
  if (!stat(url, entry, window))
  {
    return url;
  }

  const QString path = entry.stringValue( KIO::UDS_LOCAL_PATH );
  if ( !path.isEmpty() )
  {
    KUrl new_url;
    new_url.setPath(path);
    return new_url;
  }

  return url;
}

bool NetAccess::del( const KUrl & url, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.delInternal( url, window );
}

bool NetAccess::mkdir( const KUrl & url, QWidget* window, int permissions )
{
  NetAccess kioNet;
  return kioNet.mkdirInternal( url, permissions, window );
}

QString NetAccess::fish_execute( const KUrl & url, const QString command, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.fish_executeInternal( url, command, window );
}

bool NetAccess::synchronousRun( Job* job, QWidget* window, QByteArray* data,
                                KUrl* finalURL, QMap<QString, QString>* metaData )
{
  NetAccess kioNet;
  return kioNet.synchronousRunInternal( job, window, data, finalURL, metaData );
}

QString NetAccess::mimetype( const KUrl& url, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.mimetypeInternal( url, window );
}

void NetAccess::removeTempFile(const QString& name)
{
  if (!tmpfiles)
    return;
  if (tmpfiles->contains(name))
  {
    unlink(QFile::encodeName(name));
    tmpfiles->removeAll(name);
  }
}

bool NetAccess::filecopyInternal(const KUrl& src, const KUrl& target, int permissions,
                                 bool overwrite, bool resume, QWidget* window, bool move)
{
  bJobOK = true; // success unless further error occurs

  KIO::Scheduler::checkSlaveOnHold(true);
  KIO::Job * job = move
                   ? KIO::file_move( src, target, permissions, overwrite, resume )
                   : KIO::file_copy( src, target, permissions, overwrite, resume );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::dircopyInternal(const KUrl::List& src, const KUrl& target,
                                QWidget* window, bool move)
{
  bJobOK = true; // success unless further error occurs

  KIO::Job * job = move
                   ? KIO::move( src, target )
                   : KIO::copy( src, target );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::statInternal( const KUrl & url, int details, bool source,
                              QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::StatJob * job = KIO::stat( url, !url.isLocalFile() );
  job->setWindow (window);
  job->setDetails( details );
  job->setSide( source );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::delInternal( const KUrl & url, QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::del( url );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::mkdirInternal( const KUrl & url, int permissions,
                               QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::mkdir( url, permissions );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

QString NetAccess::mimetypeInternal( const KUrl & url, QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  m_mimetype = QLatin1String("unknown");
  KIO::Job * job = KIO::mimetype( url );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  connect( job, SIGNAL( mimetype (KIO::Job *, const QString &) ),
           this, SLOT( slotMimetype (KIO::Job *, const QString &) ) );
  enter_loop();
  return m_mimetype;
}

void NetAccess::slotMimetype( KIO::Job *, const QString & type  )
{
  m_mimetype = type;
}

QString NetAccess::fish_executeInternal(const KUrl & url, const QString command, QWidget* window)
{
  QString target, remoteTempFileName, resultData;
  KUrl tempPathUrl;
  KTempFile tmpFile;
  tmpFile.setAutoDelete( true );

  if( url.protocol() == "fish" )
  {
    // construct remote temp filename
    tempPathUrl = url;
    remoteTempFileName = tmpFile.name();
    // only need the filename KTempFile adds some KDE specific dirs
    // that probably does not exist on the remote side
    int pos = remoteTempFileName.lastIndexOf('/');
    remoteTempFileName = "/tmp/fishexec_" + remoteTempFileName.mid(pos + 1);
    tempPathUrl.setPath( remoteTempFileName );
    bJobOK = true; // success unless further error occurs
    QByteArray packedArgs;
    QDataStream stream( &packedArgs, QIODevice::WriteOnly );

    stream << int('X') << tempPathUrl << command;

    KIO::Job * job = KIO::special( tempPathUrl, packedArgs, true );
    job->setWindow( window );
    connect( job, SIGNAL( result (KIO::Job *) ),
             this, SLOT( slotResult (KIO::Job *) ) );
    enter_loop();

    // since the KIO::special does not provide feedback we need to download the result
    if( NetAccess::download( tempPathUrl, target, window ) )
    {
      QFile resultFile( target );

      if (resultFile.open( QIODevice::ReadOnly ))
      {
        QTextStream ts( &resultFile ); // default encoding is Locale
        resultData = ts.readAll();
        resultFile.close();
        NetAccess::del( tempPathUrl, window );
      }
    }
  }
  else
  {
    resultData = QString( "ERROR: Unknown protocol '%1'" ).arg( url.protocol() );
  }
  return resultData;
}

bool NetAccess::synchronousRunInternal( Job* job, QWidget* window, QByteArray* data,
                                        KUrl* finalURL, QMap<QString,QString>* metaData )
{
  job->setWindow( window );

  m_metaData = metaData;
  if ( m_metaData ) {
      for ( QMap<QString, QString>::iterator it = m_metaData->begin(); it != m_metaData->end(); ++it ) {
          job->addMetaData( it.key(), it.value() );
      }
  }

  if ( finalURL ) {
      SimpleJob *sj = qobject_cast<SimpleJob*>( job );
      if ( sj ) {
          m_url = sj->url();
      }
  }

  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  const QMetaObject* meta = job->metaObject();

  static const char dataSignal[] = "data(KIO::Job*,QByteArray)";
  if ( meta->indexOfSignal( dataSignal ) != -1 ) {
      connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
               this, SLOT(slotData(KIO::Job*,const QByteArray&)) );
  }

  static const char redirSignal[] = "redirection(KIO::Job*,KUrl)";
  if ( meta->indexOfSignal( redirSignal ) != -1 ) {
      connect( job, SIGNAL(redirection(KIO::Job*,const KUrl&)),
               this, SLOT(slotRedirection(KIO::Job*, const KUrl&)) );
  }

  enter_loop();

  if ( finalURL )
      *finalURL = m_url;
  if ( data )
      *data = m_data;

  return bJobOK;
}

void NetAccess::enter_loop()
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(leaveModality()),
            &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void NetAccess::slotResult( KIO::Job * job )
{
  lastErrorCode = job->error();
  bJobOK = !job->error();
  if ( !bJobOK )
  {
    if ( !lastErrorMsg )
      lastErrorMsg = new QString;
    *lastErrorMsg = job->errorString();
  }
  KIO::StatJob* statJob = qobject_cast<KIO::StatJob *>( job );
  if ( statJob )
    m_entry = statJob->statResult();

  if ( m_metaData )
    *m_metaData = job->metaData();

  emit leaveModality();
}

void NetAccess::slotData( KIO::Job*, const QByteArray& data )
{
  if ( data.isEmpty() )
    return;

  unsigned offset = m_data.size();
  m_data.resize( offset + data.size() );
  std::memcpy( m_data.data() + offset, data.data(), data.size() );
}

void NetAccess::slotRedirection( KIO::Job*, const KUrl& url )
{
  m_url = url;
}

#include "netaccess.moc"
