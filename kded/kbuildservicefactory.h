/* This file is part of the KDE project
   Copyright (C) 1999, 2007 David Faure <faure@kde.org>
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __k_build_service_factory_h__
#define __k_build_service_factory_h__

#include <QtCore/QStringList>

#include <kservicefactory.h>
// We export the services to the service group factory!
#include <QSet>
#include <kserviceoffer.h>
class KBuildServiceGroupFactory;
class KBuildMimeTypeFactory;

/**
 * Service factory for building ksycoca
 * @internal
 */
class KBuildServiceFactory : public KServiceFactory
{
public:
    /**
     * Create factory
     */
    KBuildServiceFactory( KSycocaFactory *serviceTypeFactory,
                          KBuildMimeTypeFactory *mimeTypeFactory,
                          KBuildServiceGroupFactory *serviceGroupFactory );

    virtual ~KBuildServiceFactory();

    /// Reimplemented from KServiceFactory
    virtual KService::Ptr findServiceByName(const QString &_name);

    /**
     * Construct a KService from a config file.
     */
    virtual KSycocaEntry * createEntry(const QString &file, const char *resource) const;

    virtual KService * createEntry( int ) const { assert(0); return 0; }

    /**
     * Add a new entry.
     */
    virtual void addEntry(const KSycocaEntry::Ptr& newEntry);

    /**
     * Write out service specific index files.
     */
    virtual void save(QDataStream &str);

    /**
     * Write out header information
     *
     * Don't forget to call the parent first when you override
     * this function.
     */
    virtual void saveHeader(QDataStream &str);

    /**
     * Returns all resource types for this service factory
     */
    static QStringList resourceTypes();

    void populateServiceTypes();

private:
    void saveOfferList(QDataStream &str);
    void addServiceOffer( const QString& serviceType, const KServiceOffer& offer );

    QHash<QString, KService::Ptr> m_serviceDict;
    QSet<KSycocaEntry::Ptr> m_dupeDict;

    struct ServiceTypeOffersData {
        QList<KServiceOffer> offers; // service + initial preference + allow as default
        QSet<KService::Ptr> offerSet; // for quick contains() check
    };
    QHash<QString, ServiceTypeOffersData> m_serviceTypeData;

    KSycocaFactory *m_serviceTypeFactory;
    KBuildMimeTypeFactory *m_mimeTypeFactory;
    KBuildServiceGroupFactory *m_serviceGroupFactory;
};

#endif
