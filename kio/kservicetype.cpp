/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kservice.h"
#include "ksycoca.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"

#include <assert.h>
#include <kdebug.h>
#include <qsmartptr.h>

template QDataStream& operator>> <QString, QVariant>(QDataStream&, QMap<QString, QVariant>&);
template QDataStream& operator<< <QString, QVariant>(QDataStream&, const QMap<QString, QVariant>&);
template QDataStream& operator>> <QString, QVariant::Type>(QDataStream&, QMap<QString, QVariant::Type>&);
template QDataStream& operator<< <QString, QVariant::Type>(QDataStream&, const QMap<QString, QVariant::Type>&);

KServiceType::KServiceType( const QString & _fullpath)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KServiceType::KServiceType( KDesktopFile *config )
{
  init(config);
}

void
KServiceType::init( KDesktopFile *config)
{
  m_strDesktopEntryPath = config->filename();

  // Is it a mimetype ?
  m_strName = config->readEntry( "MimeType" );

  // Or is it a servicetype ?
  if ( m_strName.isEmpty() )
  {
    m_strName = config->readEntry( "X-KDE-ServiceType" );
  }

  m_strComment = config->readComment();
  m_bDeleted = config->readBoolEntry( "Hidden", false );
  m_strIcon = config->readIcon();

  QStringList tmpList = config->groupList();
  QStringList::Iterator gIt = tmpList.begin();

  for( ; gIt != tmpList.end(); ++gIt )
  {
    if ( (*gIt).find( "Property::" ) == 0 )
    {
      config->setGroup( *gIt );
      m_mapProps.insert( (*gIt).mid( 10 ),
			 config->readPropertyEntry( "Value",
			 QVariant::nameToType( config->readEntry( "Type" ) ) ) );
    }
  }

  gIt = tmpList.begin();
  for( ; gIt != tmpList.end(); ++gIt )
  {
    if( (*gIt).find( "PropertyDef::" ) == 0 )
    {
      config->setGroup( *gIt );
      m_mapPropDefs.insert( (*gIt).mid( 13 ),
			    QVariant::nameToType( config->readEntry( "Type" ) ) );
    }
  }

  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( const QString & _fullpath, const QString& _type,
                            const QString& _icon, const QString& _comment )
{
  m_strDesktopEntryPath = _fullpath;
  m_strName = _type;
  m_strIcon = _icon;
  m_strComment = _comment;
  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( QDataStream& _str, int offset ) : KSycocaEntry( _str, offset )
{
  load( _str);
}

void
KServiceType::load( QDataStream& _str )
{
  Q_INT8 b;
  _str >> m_strName >> m_strIcon >> m_strComment >> m_mapProps >> m_mapPropDefs
       >> b >> m_strDesktopEntryPath;
  m_bValid = b;
}

void
KServiceType::save( QDataStream& _str )
{
  KSycocaEntry::save( _str );
  // Warning adding/removing fields here involves a binary incompatible change - update version
  // number in ksycoca.h
  _str << m_strName << m_strIcon << m_strComment << m_mapProps << m_mapPropDefs
       << (Q_INT8)m_bValid << m_strDesktopEntryPath;
}

KServiceType::~KServiceType()
{
}

QVariant
KServiceType::property( const QString& _name ) const
{
  QVariant v;

  if ( _name == "Name" )
    v = QVariant( m_strName );
  else if ( _name == "Icon" )
    v = QVariant( m_strIcon );
  else if ( _name == "Comment" )
    v = QVariant( m_strComment );
  else {
    QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( _name );
    if ( it != m_mapProps.end() )
      v = it.data();
  }

  return v;
}

QStringList
KServiceType::propertyNames() const
{
  QStringList res;

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.begin();
  for( ; it != m_mapProps.end(); ++it )
    res.append( it.key() );

  res.append( "Name" );
  res.append( "Comment" );
  res.append( "Icon" );

  return res;
}

QVariant::Type
KServiceType::propertyDef( const QString& _name ) const
{
  QMap<QString,QVariant::Type>::ConstIterator it = m_mapPropDefs.find( _name );
  if ( it == m_mapPropDefs.end() )
    return QVariant::Invalid;
  return it.data();
}

QStringList
KServiceType::propertyDefNames() const
{
  QStringList l;

  QMap<QString,QVariant::Type>::ConstIterator it = m_mapPropDefs.begin();
  for( ; it != m_mapPropDefs.end(); ++it )
    l.append( it.key() );

  return l;
}

KServiceType::Ptr KServiceType::serviceType( const QString& _name )
{
  KServiceType * p = KServiceTypeFactory::self()->findServiceTypeByName( _name );
  return KServiceType::Ptr( p );
}

KService::List KServiceType::offers( const QString& _servicetype )
{
  KServiceType * serv = KServiceTypeFactory::self()->findServiceTypeByName( _servicetype );
  if ( serv )
    return KServiceFactory::self()->offers( serv->offset() );
  else
  {
    kdWarning(7009) << QString("KServiceType::offers : servicetype %1 not found").arg( _servicetype ) << endl;
    KService::List l;
    return l;
  }
}

KServiceType::List KServiceType::allServiceTypes()
{
  return KServiceTypeFactory::self()->allServiceTypes();
}

