/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
                 1999 Waldo Bastian <bastian@kde.org>
 
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

#ifndef __k_build_service_factory_h__
#define __k_build_service_factory_h__

#include <kservicefactory.h>

/**
 * @internal
 * Service factory for building ksycoca
 */
class KBuildServiceFactory : public KServiceFactory
{
public:
  /**
   * Create factory
   */
  KBuildServiceFactory( KSycocaFactory *serviceTypeFactory );
  
  virtual ~KBuildServiceFactory() { }

  /**
   * Write out service type specific index files.
   */
  virtual void save(QDataStream &str);

  /**
   * Write out header information
   *
   * Don't forget to call the parent first when you override 
   * this function.
   */
  virtual void saveHeader(QDataStream &str);
private:
  void saveOfferList(QDataStream &str);

  KSycocaFactory *m_serviceTypeFactory;
};

#endif
