/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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

#ifndef __kservicefactory_h__
#define __kservicefactory_h__

#include <qstringlist.h>

#include "kservice.h"
#include "ksycocafactory.h"

class KSycoca;
class KSycocaDict;

/**
 * @internal
 * A sycoca factory for services (e.g. applications)
 * It loads the services from parsing directories (e.g. applnk/)
 * but can also create service from data streams or single config files
 */
class KServiceFactory : public KSycocaFactory
{
  K_SYCOCAFACTORY( KST_KServiceFactory )
public:
  /**
   * Create factory
   */
  KServiceFactory();
  virtual ~KServiceFactory();
  
  /**
   * Construct a KService from a config file.
   */
  virtual KSycocaEntry *createEntry(const QString &file);

  /**
   * Find a service
   */
  KService * findServiceByName( const QString &_name );

  /**
   * @return the services supporting the given service type
   */
  KService::List offers( int serviceTypeOffset );

  /**
   * @return all services. Very memory consuming, avoid using.
   */
  KService::List allServices();

  /**
   * @return the unique service factory, creating it if necessary
   */
  static KServiceFactory * self();

protected:
  KService * createService(int offset);
  int m_offerListOffset;

private:
  static KServiceFactory *_self;
};

#endif
