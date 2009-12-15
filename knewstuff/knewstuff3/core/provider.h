/*
    knewstuff3/provider.h
    This file is part of KNewStuff2.
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KNEWSTUFF3_PROVIDER_H
#define KNEWSTUFF3_PROVIDER_H

#include <QtCore/QList>
#include <QtCore/QString>

#include <kurl.h>

#include "entryinternal.h"


namespace KNS3
{
    class ProviderPrivate;

    /**
     * @short KNewStuff Base Provider class.
     *
     * This class provides accessors for the provider object.
     * It should not be used directly by the application.
     * This class is the base class and will be instantiated for
     * static website providers.
     *
     * @author Jeremy Whiting <jpwhiting@kde.org>
     *
     * @internal
     */
    class Provider: public QObject
    {
        Q_OBJECT
    public:
        typedef QList<Provider*> List;

        enum SortMode {
            Newest,
            Alphabetical,
            Rating,
            Downloads,
            Installed // only installed items
        };

        /**
         * Constructor.
         */
        Provider();
        Provider(const Provider &other);


        /**
         * Destructor.
         */
        virtual ~Provider();

        /**
         * A unique Id for this provider (the url in most cases)
         */
        virtual QString id() const = 0;

        /**
         * Set the provider data xml, to initialize the provider.
         * The Provider needs to have it's ID set in this function and cannot change it from there on.
         */
        virtual bool setProviderXML(QDomElement & xmldata) = 0;

        /**
         * get the xml for the provider
         */
        virtual QDomElement providerXML() const = 0;

        virtual bool isInitialized() const = 0;

        virtual void setCachedEntries(const KNS3::EntryInternal::List& cachedEntries) = 0;

        /**
         * Retrieves the common name of the provider.
         *
         * @return provider name
         */
        virtual QString name() const;

        /**
         * Retrieves the icon URL for this provider.
         *
         * @return icon URL
         */
        virtual KUrl icon() const; // FIXME use KIcon or pixmap?

        /**
         * load the given search and return given page
         * @param sortMode string to select the order in which the results are presented
         * @param searchstring string to search with
         * @param page         page number to load
         *
         * Note: the engine connects to loadingFinished() signal to get the result
         */
        virtual void loadEntries(SortMode sortMode = Rating, const QString & searchstring = QString(), const QStringList & categories = QStringList(), int page = 0, int pageSize = 20) = 0;
        virtual void loadPayloadLink(const EntryInternal& entry) = 0;

        virtual bool userCanVote() {return false;}
        virtual void vote(const EntryInternal& entry, bool positiveVote) { Q_UNUSED(entry) Q_UNUSED(positiveVote) }

        virtual bool userCanBecomeFan() {return false;}
        virtual void becomeFan(const EntryInternal& entry) { Q_UNUSED(entry) }

    signals:
        void providerInitialized(KNS3::Provider*);

        void loadingFinished(KNS3::Provider::SortMode sortMode, const QString& searchstring, int page, int pageSize, int totalpages, const KNS3::EntryInternal::List&) const;
        void loadingFailed(KNS3::Provider::SortMode sortMode, const QString& searchstring, int page);

        void payloadLinkLoaded(const KNS3::EntryInternal& entry);

    protected:
        QExplicitlySharedDataPointer<ProviderPrivate> const d_ptr;
        Provider(ProviderPrivate &dd);
    private:
        Q_DECLARE_PRIVATE(Provider)
    };

}

#endif
