/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

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

#include "kmimetypetrader.h"

#include "kservicetypeprofile.h"
#include "kservicetype.h"
#include "kservicetypetrader.h"
#include "kmimetype.h"
#include "kservicefactory.h"
#include "kmimetypefactory.h"

#include <kstaticdeleter.h>
#include <kdebug.h>

KMimeTypeTrader* KMimeTypeTrader::s_self = 0;
static KStaticDeleter<KMimeTypeTrader> kmimetypetradersd;

KMimeTypeTrader* KMimeTypeTrader::self()
{
    if ( !s_self )
	kmimetypetradersd.setObject( s_self, new KMimeTypeTrader );
    return s_self;
}

KMimeTypeTrader::~KMimeTypeTrader()
{
}

KMimeTypeTrader::KMimeTypeTrader()
{
}

#if 0
// slow - but only used for all/all and all/allfiles. TODO: write them into ksycoca, in kbuildservicefactory.cpp
static void addUnique( KServiceOfferList &lst, const KServiceOfferList &newLst, bool lowPrio )
{
    KServiceOfferList::const_iterator it = newLst.begin();
    const KServiceOfferList::const_iterator end = newLst.end();
    for( ; it != end; ++it )
    {
        KServiceOffer offer = *it;
        KService::Ptr service = offer.service();
        KServiceOfferList::const_iterator exisit = lst.begin();
        const KServiceOfferList::const_iterator exisend = lst.end();
        bool alreadyThere = false;
        for( ; exisit != exisend && !alreadyThere; ++exisit )
        {
            if ( (*exisit).service() == service )
                alreadyThere = true;
        }
        if ( alreadyThere )
            continue;
        if ( lowPrio )
            offer.setPreference( 0 ); // if you remove this code before kde4, remove KServiceOffer::setPreference too.
        lst.append( offer );
    }
}
#endif

// helper method for weightedOffers
static KServiceOfferList mimeTypeSycocaOffers( const QString& mimeType )
{
    KServiceOfferList lst;

    // Services associated directly with this mimetype (the normal case)
    KMimeType::Ptr mime = KMimeTypeFactory::self()->findMimeTypeByName( mimeType, KMimeType::ResolveAliases );
    if ( !mime ) {
        kWarning(7014) << "KMimeTypeTrader: mimeType " << mimeType << " not found" << endl;
        return lst; // empty
    }
    if ( mime->serviceOffersOffset() > -1 ) {
        lst = KServiceFactory::self()->offers( mime->offset(), mime->serviceOffersOffset() );
    }

    //debug
    //foreach( KService::Ptr serv, lst )
    //    kDebug() << serv.data() << " " << serv->name() << endl;


    // With xdg-shared-mime this is automatically done via the implicit inheritance from application/octet-stream
#if 0
    // Support for all/* is deactivated by KServiceTypeProfile::configurationMode()
    // (and makes no sense when querying for an "all" servicetype itself
    // nor for non-mimetypes service types)
    if ( !KServiceTypeProfile::configurationMode()
         && !mimeType.startsWith( QLatin1String( "all/" ) ) )
    {
        // Support for services associated with "all"
        const KMimeType::Ptr mimeAll = KMimeTypeFactory::self()->findMimeTypeByName( "all/all" );
        if ( mimeAll ) {
            if ( mimeAll->serviceOffersOffset() > -1 )
                addUnique(lst, KServiceFactory::self()->offers( mimeAll->offset(), mimeAll->serviceOffersOffset() ), true);
        }
        else
            kWarning(7014) << "KMimeTypeTrader : mimetype all/all not found" << endl;

        // Support for services associated with "allfiles"
        if ( mimeType != "inode/directory" && mimeType != "inode/directory-locked" )
        {
            const KMimeType::Ptr mimeAllFiles = KMimeTypeFactory::self()->findMimeTypeByName( "all/allfiles" );
            if ( mimeAllFiles ) {
                if ( mimeAllFiles->serviceOffersOffset() > -1 )
                    addUnique(lst, KServiceFactory::self()->offers( mimeAllFiles->offset(), mimeAllFiles->serviceOffersOffset() ), true);
            }
            else
                kWarning(7014) << "KMimeTypeTrader : mimetype all/allfiles not found" << endl;
        }
    }
#endif

    return lst;
}

// Defined in kdecore
namespace KServiceTypeProfile {
    KDECORE_EXPORT KServiceOfferList sortMimeTypeOffers( const KServiceOfferList& list, const QString& mimeType, const QString & genericServiceType );
}

KServiceOfferList KMimeTypeTrader::weightedOffers( const QString& mimeType,
                                                   const QString& genericServiceType ) const
{
    kDebug(7014) << "KMimeTypeTrader::weightedOffers( " << mimeType << ", " << genericServiceType << " )" << endl;

    Q_ASSERT( !genericServiceType.isEmpty() );
    // First, get all offers known to ksycoca.
    const KServiceOfferList offers = mimeTypeSycocaOffers( mimeType );

    // Assign preferences from the profile to those offers - and filter for genericServiceType
    return KServiceTypeProfile::sortMimeTypeOffers( offers, mimeType, genericServiceType );
}

KService::List KMimeTypeTrader::query( const QString& mimeType,
                                       const QString& genericServiceType,
                                       const QString& constraint ) const
{
    // Get all services of this mime type.
    const KServiceOfferList offers = weightedOffers( mimeType, genericServiceType );

    // Now extract only the services; the weighting was only used for sorting.
    KService::List lst;
    KServiceOfferList::const_iterator itOff = offers.begin();
    for( ; itOff != offers.end(); ++itOff )
        lst.append( (*itOff).service() );

    KServiceTypeTrader::applyConstraints( lst, constraint );

    kDebug(7014) << "query for mimeType " << mimeType << ", " << genericServiceType
                 << " : returning " << lst.count() << " offers" << endl;
    return lst;
}

KService::Ptr KMimeTypeTrader::preferredService( const QString & mimeType, const QString & genericServiceType )
{
    const KServiceOfferList offers = weightedOffers( mimeType, genericServiceType );

    KServiceOfferList::const_iterator itOff = offers.begin();
    // Look for the first one that is allowed as default.
    // Since the allowed-as-default are first anyway, we only have
    // to look at the first one to know.
    if( itOff != offers.end() && (*itOff).allowAsDefault() )
        return (*itOff).service();

    //kDebug(7014) << "No offers, or none allowed as default" << endl;
    return KService::Ptr();
}
