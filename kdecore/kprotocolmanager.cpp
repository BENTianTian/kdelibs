/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kprotocolmanager.h"

#include <string.h>

#include <kstddirs.h>
#include <kglobal.h>
#include <kapp.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <qdir.h>
#include <qstrlist.h>
#include <kconfig.h>

#undef Unsorted // X11 headers

KProtocolManager* KProtocolManager::s_pManager = 0L;

KProtocolManager::KProtocolManager()
{
  ASSERT( !s_pManager );

  s_pManager = this;

  QStringList list = KGlobal::dirs()->findDirs("config", "protocols");
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
    scanConfig( *it );
}

void KProtocolManager::scanConfig( const QString& _dir )
{
  QDir dir( _dir, QString::null, QDir::Unsorted, QDir::Files | QDir::Readable );
  if (!dir.exists())
    return;
  QStringList p = dir.entryList();

  QStringList::Iterator e;
  for( e = p.begin(); e != p.end(); e++ )
  {
    if ( *e == "." || *e == ".." )
      continue;

    KSimpleConfig config( _dir + "/" + *e, true );
    config.setGroup( "Protocol" );

    Protocol p;
    p.exec = config.readEntry( "exec" );
    p.isSourceProtocol = config.readBoolEntry( "source", true );
    p.supportsReading = config.readBoolEntry( "reading", false );
    p.supportsWriting = config.readBoolEntry( "writing", false );
    p.supportsMakeDir = config.readBoolEntry( "makedir", false );
    p.supportsDeleting = config.readBoolEntry( "deleting", false );
    p.supportsLinking = config.readBoolEntry( "linking", false );
    p.supportsMoving = config.readBoolEntry( "moving", false );
    p.mimetypeFastMode = config.readBoolEntry( "mimetypefastmode", false );
    QStrList lst;
    config.readListEntry( "listing", lst );
    p.supportsListing = ( lst.count() > 0 );
    const char *l;
    for( l = lst.first(); l; l = lst.next() )
      p.listing.append( l );
    QString tmp = config.readEntry( "input" );
    if ( tmp == "filesystem" )
      p.inputType = T_FILESYSTEM;
    else if ( tmp == "stream" )
      p.inputType = T_STREAM;
    else
      p.inputType = T_NONE;
    tmp = config.readEntry( "output" );
    if ( tmp == "filesystem" )
      p.outputType = T_FILESYSTEM;
    else if ( tmp == "stream" )
      p.outputType = T_STREAM;
    else
      p.outputType = T_NONE;

    QString name = config.readEntry( "protocol" );

    m_protocols.insert( name, p );
  }
}

bool KProtocolManager::isSourceProtocol( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().isSourceProtocol;
}

bool KProtocolManager::isFilterProtocol( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return !it.data().isSourceProtocol;
}

bool KProtocolManager::isKnownProtocol( const QString& _protocol ) const
{
  return m_protocols.contains( _protocol );
}

bool KProtocolManager::supportsListing( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsListing;
}

QStringList KProtocolManager::listing( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return QStringList();
  }

  return it.data().listing;
}

bool KProtocolManager::supportsReading( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsReading;
}

bool KProtocolManager::supportsWriting( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsWriting;
}

bool KProtocolManager::supportsMakeDir( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsMakeDir;
}

bool KProtocolManager::supportsDeleting( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsDeleting;
}

bool KProtocolManager::supportsLinking( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsLinking;
}

bool KProtocolManager::supportsMoving( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsMoving;
}

bool KProtocolManager::mimetypeFastMode( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().mimetypeFastMode;
}

QStringList KProtocolManager::protocols() const
{
  QStringList res;

  ConstIterator it = m_protocols.begin();
  ConstIterator end = m_protocols.end();
  for (; it != end; ++it )
    res.append( it.key() );

  return res;
}

/*
QString KProtocolManager::library( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return QString::null;
  }

  return it.data().library;
}
*/

QString KProtocolManager::exec( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return QString::null;
  }

  // This is where we insert a small hack for FTP-Proxy.
  // If we want to use FTP-Proxy, then it's kio_http we want.
  // Something more flexible would require the possibility of having
  // a proxy for any protocol, but we probably don't need that (David).
  if ( _protocol == QString::fromLatin1("ftp") && useProxy() && !ftpProxy().isEmpty() )
    return "kio_http";

  return it.data().exec;
}

KProtocolManager::Type KProtocolManager::inputType( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return T_NONE;
  }

  return it.data().inputType;
}

KProtocolManager::Type KProtocolManager::outputType( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(7008) << "Protocol " << _protocol << " not found" << endl;
    return T_NONE;
  }

  return it.data().outputType;
}

int KProtocolManager::readTimeout()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readNumEntry( "ReadTimeout", 15 ); // 15 seconds
}

bool KProtocolManager::markPartial()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "MarkPartial", true );
}

int KProtocolManager::minimumKeepSize()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readNumEntry( "MinimumKeepSize", 5000 ); // 5000 byte
}

bool KProtocolManager::autoResume()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "AutoResume", false );
}

bool KProtocolManager::persistentConnections()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "PersistentConnections", true );
}

bool KProtocolManager::useProxy()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readBoolEntry( "UseProxy", false );
}

bool KProtocolManager::useCache()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  return config.readBoolEntry( "UseCache", true );
}

int KProtocolManager::maxCacheAge()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  return config.readNumEntry( "MaxCacheAge", 60*60*24*14 ); // 14 days
}

int KProtocolManager::maxCacheSize()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  return config.readNumEntry( "MaxCacheSize", 5000 );
}

QString KProtocolManager::ftpProxy()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readEntry( "FtpProxy" );
}

QString KProtocolManager::httpProxy()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readEntry( "HttpProxy" );
}

QString KProtocolManager::noProxyFor()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readEntry( "NoProxyFor" );
}

QString KProtocolManager::remoteFileProtocol()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readEntry( "RemoteFileProtocol" );
}

void KProtocolManager::setReadTimeout( int _timeout )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "ReadTimeout", _timeout );
  config.sync();
}


void KProtocolManager::setMarkPartial( bool _mode )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "MarkPartial", _mode );
  config.sync();
}


void KProtocolManager::setMinimumKeepSize( int _size )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "MinimumKeepSize", _size );
  config.sync();
}


void KProtocolManager::setAutoResume( bool _mode )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "AutoResume", _mode );
  config.sync();
}


void KProtocolManager::setPersistentConnections( bool _mode )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "PersistentConnections", _mode );
  config.sync();
}


void KProtocolManager::setUseProxy( bool _mode )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "UseProxy", _mode );
  config.sync();
}


void KProtocolManager::setFtpProxy( const QString& _proxy )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "FtpProxy", _proxy );
  config.sync();
}


void KProtocolManager::setHttpProxy( const QString& _proxy )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "HttpProxy", _proxy );
  config.sync();
}


void KProtocolManager::setNoProxyFor( const QString& _noproxy )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "NoProxyFor", _noproxy );
  config.sync();
}

void KProtocolManager::setRemoteFileProtocol(const QString &remoteFileProtocol)
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "RemoteFileProtocol", remoteFileProtocol );
  config.sync();
}
