/*
    This file is part of KNewStuff.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KNEWSTUFF_PROVIDER_H
#define KNEWSTUFF_PROVIDER_H

#include <qdom.h>
#include <QtCore/QObject>
#include <QtCore/QString>

#include <kurl.h>
#include <kdelibs_export.h>

class KJob;
namespace KIO { class Job; }

namespace KNS {

/**
 * @short KNewStuff provider container.
 *
 * This class provides accessors for the provider object.
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class KNEWSTUFF_EXPORT Provider
{
  public:
    typedef QList<Provider *> List;

    /**
     * Constructor.
     */
    Provider();

    /**
     * Constructor with XML feed.
     */
    Provider( const QDomElement & );

    /**
     * Destructor.
     */
    ~Provider();

    /**
     * Sets the common name of the provider.
     */
    void setName( const QString & );

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    QString name() const;

    /**
     * Sets the download URL.
     */
    void setDownloadUrl( const KUrl & );

    /**
     * Retrieves the download URL.
     *
     * @return download URL
     */
    KUrl downloadUrl() const;

    /**
     * Variant to retrieve 'tagged' download URLs.
     * Variant can be one of 'score', 'downloads', 'latest'.
     *
     * @return download specific URL
     */
    KUrl downloadUrlVariant( QString variant ) const;

    /**
     * Sets the upload URL.
     */
    void setUploadUrl( const KUrl & );

    /**
     * Retrieves the upload URL.
     *
     * @return upload URL
     */
    KUrl uploadUrl() const;

    /**
     * Sets the URL where a user is led if the provider does not support
     * uploads.
     *
     * @see setNoUpload
     */
    void setNoUploadUrl( const KUrl & );

    /**
     * Retrieves the URL where a user is led if the provider does not
     * support uploads.
     *
     * @return website URL
     */
    KUrl noUploadUrl() const;

    /**
     * Indicate whether provider supports uploads.
     */
    void setNoUpload( bool );

    /**
     * Query whether provider supports uploads.
     *
     * @return upload support status
     */
    bool noUpload() const;

    /**
     * Sets the URL for an icon for this provider.
     * The icon should be in 32x32 format. If not set, the default icon
     * of KDialog is used.
     */
    void setIcon( const KUrl & );

    /**
     * Retrieves the icon URL for this provider.
     *
     * @return icon URL
     */
    KUrl icon() const;

  protected:
    void parseDomElement( const QDomElement & );

    QDomElement createDomElement( QDomDocument &, QDomElement &parent );

  private:
    QString mName;
    KUrl mDownloadUrl;
    KUrl mUploadUrl;
    KUrl mNoUploadUrl;
    KUrl mIcon;
    bool mNoUpload;
};

/**
 * KNewStuff provider loader.
 * This class sets up a list of all possible providers by querying
 * the main provider database for this specific application.
 * It should probably not be used directly by the application.
 */
class KNEWSTUFF_EXPORT ProviderLoader : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructor.
     *
     * @param parentWidget the parent widget
     */
    ProviderLoader( QWidget *parentWidget );
	~ProviderLoader();
    /**
     * Starts asynchronously loading the list of providers of the
     * specified category.
     *
     * @param category data type such as 'kdesktop/wallpaper'.
     * @param providerList the URl to the list of providers; if empty
     *    we first try the ProvidersUrl from KGlobal::config, then we
     *    fall back to a hardcoded value.
     */
    void load( const QString &category, const QString &providerList = QString() );

  Q_SIGNALS:
    /**
     * Indicates that the list of providers has been successfully loaded.
     */
    void providersLoaded( Provider::List * );

  protected Q_SLOTS:
    void slotJobData( KIO::Job *, const QByteArray & );
    void slotJobResult( KJob * );

  private:
    QWidget *mParentWidget;

    QString mJobData;

    Provider::List mProviders;
};

}

#endif
