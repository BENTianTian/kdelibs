// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * settings.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 * Copyright (C)  2006  Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "settings.h"

#include "loader.h"

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>

#include <qmap.h>
#include <qstringlist.h>
#include <kconfiggroup.h>

namespace KSpell2
{
class Settings::Private
{
public:
    Loader*  loader; //can't be a Ptr since we don't want to hold a ref on it
    KSharedConfig::Ptr config;
    bool     modified;

    QString defaultLanguage;
    QString defaultClient;

    bool checkUppercase;
    bool skipRunTogether;
    bool backgroundCheckerEnabled;

    int disablePercentage;
    int disableWordCount;

    QMap<QString, bool> ignore;
};

Settings::Settings( Loader *loader, KSharedConfig::Ptr config )
	:d(new Private)
{
    d->loader = loader;

    Q_ASSERT( config );
    d->config = config;

    d->modified = false;
    loadConfig();
}

Settings::~Settings()
{
    delete d;
}

KSharedConfig *Settings::sharedConfig() const
{
    return d->config.data();
}

void Settings::setDefaultLanguage( const QString& lang )
{
    QStringList cs = d->loader->languages();
    if ( cs.indexOf( lang ) != -1 &&
         d->defaultLanguage != lang ) {
        d->defaultLanguage = lang;
        readIgnoreList();
        d->modified = true;
        d->loader->changed();
    }
}

QString Settings::defaultLanguage() const
{
    return d->defaultLanguage;
}

void Settings::setDefaultClient( const QString& client )
{
    //Different from setDefaultLanguage because
    //the number of clients can't be even close
    //as big as the number of languages
    if ( d->loader->clients().contains( client ) ) {
        d->defaultClient = client;
        d->modified = true;
        d->loader->changed();
    }
}

QString Settings::defaultClient() const
{
    return d->defaultClient;
}

void Settings::setCheckUppercase( bool check )
{
    if ( d->checkUppercase != check ) {
        d->modified = true;
        d->checkUppercase = check;
    }
}

bool Settings::checkUppercase() const
{
    return d->checkUppercase;
}

void Settings::setSkipRunTogether( bool skip )
{
    if ( d->skipRunTogether != skip ) {
        d->modified = true;
        d->skipRunTogether = skip;
    }
}

bool Settings::skipRunTogether() const
{
    return d->skipRunTogether;
}

void Settings::setBackgroundCheckerEnabled( bool enable )
{
    if ( d->backgroundCheckerEnabled != enable ) {
        d->modified = true;
        d->backgroundCheckerEnabled = enable;
    }
}

bool Settings::backgroundCheckerEnabled() const
{
    return d->backgroundCheckerEnabled;
}

void Settings::setCurrentIgnoreList( const QStringList& ignores )
{
    setQuietIgnoreList( ignores );
    d->modified = true;
}

void Settings::setQuietIgnoreList( const QStringList& ignores )
{
    d->ignore = QMap<QString, bool>();//clear out
    for ( QStringList::const_iterator itr = ignores.begin();
          itr != ignores.end(); ++itr ) {
        d->ignore.insert( *itr, true );
    }
}

QStringList Settings::currentIgnoreList() const
{
    return d->ignore.keys();
}

void Settings::addWordToIgnore( const QString& word )
{
    if ( !d->ignore.contains( word ) ) {
        d->modified = true;
        d->ignore.insert( word, true );
    }
}

bool Settings::ignore( const QString& word )
{
    return d->ignore.contains( word );
}

void Settings::readIgnoreList()
{
    KConfigGroup conf( d->config.data(), "Spelling" );
    QString ignoreEntry = QString( "ignore_%1" ).arg( d->defaultLanguage );
    QStringList ignores = conf.readEntry( ignoreEntry, QStringList() );
    setQuietIgnoreList( ignores );
}

int Settings::disablePercentageWordError() const
{
    return d->disablePercentage;
}

int Settings::disableWordErrorCount() const
{
    return d->disableWordCount;
}

void Settings::save()
{
    if ( d->modified ) {
        KConfigGroup conf( d->config.data(), "Spelling" );
        conf.writeEntry( "defaultClient", d->defaultClient );
        conf.writeEntry( "defaultLanguage", d->defaultLanguage );
        conf.writeEntry( "checkUppercase", d->checkUppercase );
        conf.writeEntry( "skipRunTogether", d->skipRunTogether );
        conf.writeEntry( "backgroundCheckerEnabled", d->backgroundCheckerEnabled );
        conf.writeEntry( QString( "ignore_%1" ).arg( d->defaultLanguage ),
                         d->ignore.keys() );

        conf.sync();
    }
}

void Settings::loadConfig()
{
    KConfigGroup conf( d->config.data(), "Spelling" );
    d->defaultClient = conf.readEntry( "defaultClient",
                                        QString() );
    d->defaultLanguage = conf.readEntry(
        "defaultLanguage", KGlobal::locale()->language() );

    //same defaults are in the default filter (filter.cpp)
    d->checkUppercase = conf.readEntry(
        "checkUppercase", true );

    d->skipRunTogether = conf.readEntry(
        "skipRunTogether", true );

    d->backgroundCheckerEnabled = conf.readEntry(
        "backgroundCheckerEnabled", true);

    d->disablePercentage = conf.readEntry( "KSpell_AsYouTypeDisablePercentage", 42 );
    d->disableWordCount = conf.readEntry( "KSpell_AsYouTypeDisableWordCount", 100 );

    readIgnoreList();
}


}
