/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 - 2001 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 1999 - 2005 David Faure   <faure@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kservice.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

#include <QtCore/QCharRef>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QMap>

#include <kauthorized.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>

#include "kservicefactory.h"
#include "kservicetypefactory.h"

class KService::Private
{
public:
    Private()
        : m_bValid(true)
    {
    }
    void init(const KDesktopFile *config, KService* q);
    void load( QDataStream& );
    void save( QDataStream& );

    QStringList categories;
    QString menuId;
    QString m_strType;
    QString m_strName;
    QString m_strExec;
    QString m_strIcon;
    QString m_strTerminalOptions;
    QString m_strPath;
    QString m_strComment;
    QString m_strLibrary;
    QStringList m_lstServiceTypes;
    int m_initialPreference;
    QString m_strDesktopEntryName;
    DBUSStartupType_t m_DBUSStartusType;
    QMap<QString,QVariant> m_mapProps;
    QStringList m_lstKeywords;
    QString m_strGenName;
    bool m_bAllowAsDefault;
    bool m_bTerminal;
    bool m_bValid;
};

void KService::Private::init( const KDesktopFile *config, KService* q )
{
    const QString entryPath = q->entryPath();
    bool absPath = !QDir::isRelativePath(entryPath);

    // TODO: it makes sense to have a KConstConfigGroup I guess
    KConfigGroup desktopGroup = const_cast<KDesktopFile*>(config)->desktopGroup();
    QMap<QString, QString> entryMap = desktopGroup.entryMap();

    entryMap.remove("Encoding"); // reserved as part of Desktop Entry Standard
    entryMap.remove("Version");  // reserved as part of Desktop Entry Standard

    q->setDeleted( desktopGroup.readEntry("Hidden", false) );
    entryMap.remove("Hidden");
    if ( q->isDeleted() ) {
        m_bValid = false;
        return;
    }

    m_strName = config->readName();
    entryMap.remove("Name");
    if ( m_strName.isEmpty() )
    {
        if (desktopGroup.readEntry( "Exec" ).isEmpty())
        {
            m_bValid = false;
            return;
        }
        // Try to make up a name.
        m_strName = entryPath;
        int i = m_strName.lastIndexOf('/');
        m_strName = m_strName.mid(i+1);
        i = m_strName.lastIndexOf('.');
        if (i != -1)
            m_strName = m_strName.left(i);
    }

    m_strType = desktopGroup.readEntry( "Type" );
    entryMap.remove("Type");
    if ( m_strType.isEmpty() )
    {
        /*kWarning(7012) << "The desktop entry file " << entryPath
          << " has no Type=... entry."
          << " It should be \"Application\" or \"Service\"" << endl;
          m_bValid = false;
          return;*/
        m_strType = "Application";
    } else if ( m_strType != "Application" && m_strType != "Service" )
    {
        kWarning(7012) << "The desktop entry file " << entryPath
                       << " has Type=" << m_strType
                       << " instead of \"Application\" or \"Service\"" << endl;
        m_bValid = false;
        return;
    }

    // In case Try Exec is set, check if the application is available
    if (!config->tryExec()) {
        q->setDeleted( true );
        m_bValid = false;
        return;
    }

    QString resource = config->resource();

    if ( (m_strType == "Application") &&
         (!resource.isEmpty()) &&
         (resource != "apps") &&
         !absPath)
    {
        kWarning(7012) << "The desktop entry file " << entryPath
                       << " has Type=" << m_strType << " but is located under \"" << resource
                       << "\" instead of \"apps\"" << endl;
        m_bValid = false;
        return;
    }

    if ( (m_strType == "Service") &&
         (!resource.isEmpty()) &&
         (resource != "services") &&
         !absPath)
    {
        kWarning(7012) << "The desktop entry file " << entryPath
                       << " has Type=" << m_strType << " but is located under \"" << resource
                       << "\" instead of \"services\"" << endl;
        m_bValid = false;
        return;
    }

    QString _name = entryPath;
    int pos = _name.lastIndexOf('/');
    if (pos != -1)
        _name = _name.mid(pos+1);
    pos = _name.indexOf('.');
    if (pos != -1)
        _name = _name.left(pos);

    m_strExec = desktopGroup.readPathEntry( "Exec" );
    entryMap.remove("Exec");
    if (m_strType == "Application" && m_strExec.isEmpty())
    {
        kWarning(7012) << "The desktop entry file " << entryPath
                       << " has Type=" << m_strType << " but has no Exec field." << endl;
    }

    m_strIcon = desktopGroup.readEntry( "Icon" );
    entryMap.remove("Icon");
    m_bTerminal = desktopGroup.readEntry( "Terminal", false); // should be a property IMHO
    entryMap.remove("Terminal");
    m_strTerminalOptions = desktopGroup.readEntry( "TerminalOptions" ); // should be a property IMHO
    entryMap.remove("TerminalOptions");
    m_strPath = desktopGroup.readPathEntry( "Path" );
    entryMap.remove("Path");
    m_strComment = config->readComment();
    entryMap.remove("Comment");
    m_strGenName = config->readGenericName();
    entryMap.remove("GenericName");
    QString _untranslatedGenericName = desktopGroup.readEntryUntranslated( "GenericName" );
    entryMap.insert("UntranslatedGenericName", _untranslatedGenericName);

    m_lstKeywords = desktopGroup.readEntry("Keywords", QStringList());
    entryMap.remove("Keywords");
    categories = desktopGroup.readEntry("Categories", QStringList(), ';');
    entryMap.remove("Categories");
    m_strLibrary = desktopGroup.readEntry( "X-KDE-Library" );
    entryMap.remove("X-KDE-Library");

    m_lstServiceTypes = desktopGroup.readEntry( "ServiceTypes", QStringList() );
    entryMap.remove("ServiceTypes");
    m_lstServiceTypes += desktopGroup.readEntry( "MimeType", QStringList(), ';' ); // freedesktop.org standard
    entryMap.remove("MimeType");

    if ( m_strType == "Application" && !m_lstServiceTypes.contains("Application") )
        // Applications implement the service type "Application" ;-)
        m_lstServiceTypes += "Application";

    QString dbusStartupType = desktopGroup.readEntry("X-DBUS-StartupType").toLower();
    //Compatibility
    if( dbusStartupType.isEmpty() && desktopGroup.hasKey("X-DCOP-ServiceType"))
    {
        dbusStartupType = desktopGroup.readEntry("X-DCOP-ServiceType").toLower();
        entryMap.remove("X-DCOP-ServiceType");
    }
    entryMap.remove("X-DBUS-StartupType");
    if (dbusStartupType == "unique")
        m_DBUSStartusType = DBUS_Unique;
    else if (dbusStartupType == "multi")
        m_DBUSStartusType = DBUS_Multi;
    else if (dbusStartupType == "wait")
        m_DBUSStartusType = DBUS_Wait;
    else
        m_DBUSStartusType = DBUS_None;

    m_strDesktopEntryName = _name.toLower();

    m_bAllowAsDefault = desktopGroup.readEntry("AllowDefault", true);
    entryMap.remove("AllowDefault");

    m_initialPreference = desktopGroup.readEntry( "InitialPreference", 1 );
    entryMap.remove("InitialPreference");

    // Store all additional entries in the property map.
    // A QMap<QString,QString> would be easier for this but we can't
    // break BC, so we have to store it in m_mapProps.
//  qDebug("Path = %s", entryPath.toLatin1().constData());
    QMap<QString,QString>::ConstIterator it = entryMap.begin();
    for( ; it != entryMap.end();++it)
    {
//     qDebug("   Key = %s Data = %s", it.key().toLatin1().data(), it->toLatin1().data());
        m_mapProps.insert( it.key(), QVariant( *it));
    }
}

void KService::Private::load(QDataStream& s)
{
    // dummies are here because of fields that were removed, to keep bin compat.
    // Feel free to re-use, but fields for Applications only (not generic services)
    // should rather be added to application.desktop
    qint8 def, term;
    qint8 dst, initpref;

    // WARNING: THIS NEEDS TO REMAIN COMPATIBLE WITH PREVIOUS KDE 4.x VERSIONS!
    // !! This data structure should remain binary compatible at all times !!
    // You may add new fields at the end. Make sure to update the version
    // number in ksycoca.h
    s >> m_strType >> m_strName >> m_strExec >> m_strIcon
      >> term >> m_strTerminalOptions
      >> m_strPath >> m_strComment >> m_lstServiceTypes >> def >> m_mapProps
      >> m_strLibrary
      >> dst
      >> m_strDesktopEntryName
      >> initpref
      >> m_lstKeywords >> m_strGenName
      >> categories >> menuId;

    m_bAllowAsDefault = def;
    m_bTerminal = term;
    m_DBUSStartusType = (DBUSStartupType_t) dst;
    m_initialPreference = initpref;

    m_bValid = true;
}

void KService::Private::save(QDataStream& s)
{
    qint8 def = m_bAllowAsDefault, initpref = m_initialPreference;
    qint8 term = m_bTerminal;
    qint8 dst = (qint8) m_DBUSStartusType;

    // WARNING: THIS NEEDS TO REMAIN COMPATIBLE WITH PREVIOUS KDE 4.x VERSIONS!
    // !! This data structure should remain binary compatible at all times !!
    // You may add new fields at the end. Make sure to update the version
    // number in ksycoca.h
    s << m_strType << m_strName << m_strExec << m_strIcon
      << term << m_strTerminalOptions
      << m_strPath << m_strComment << m_lstServiceTypes << def << m_mapProps
      << m_strLibrary
      << dst
      << m_strDesktopEntryName
      << initpref
      << m_lstKeywords << m_strGenName
      << categories << menuId;
}

////

KService::KService( const QString & _name, const QString &_exec, const QString &_icon)
    : KSycocaEntry(QString()), d(new Private)
{
    d->m_strType = "Application";
    d->m_strName = _name;
    d->m_strExec = _exec;
    d->m_strIcon = _icon;
    d->m_bTerminal = false;
    d->m_bAllowAsDefault = true;
    d->m_initialPreference = 10;
}


KService::KService( const QString & _fullpath )
    : KSycocaEntry(_fullpath), d(new Private)
{
    KDesktopFile config( _fullpath );
    d->init(&config, this);
}

KService::KService( const KDesktopFile *config )
    : KSycocaEntry(config->fileName()), d(new Private)
{
    d->init(config, this);
}

KService::KService( QDataStream& _str, int _offset ) : KSycocaEntry( _str, _offset ), d(new Private)
{
    d->load( _str );
}

KService::~KService()
{
    delete d;
}

void KService::load( QDataStream& s )
{
    d->load(s);
}

void KService::save( QDataStream& s )
{
    KSycocaEntry::save( s );
    d->save(s);
}

bool KService::hasServiceType( const QString& serviceType ) const
{
    if (!d->m_bValid) return false; // (useless) safety test
    const KServiceType::Ptr ptr = KServiceType::serviceType( serviceType );
    // share the implementation, at least as long as
    // we don't have special code for mimetype inheritance
    return hasMimeType( ptr.data() );
}

bool KService::hasMimeType( const KServiceType* ptr ) const
{
    if (!ptr) return false;
    int serviceOffset = offset();
    // doesn't seem to work:
    //if ( serviceOffset == 0 )
    //    serviceOffset = serviceByStorageId( storageId() );
    if ( serviceOffset )
        return KServiceFactory::self()->hasOffer( ptr->offset(), ptr->serviceOffersOffset(), serviceOffset );

    // fall-back code for services that are from ksycoca
    bool isNumber;
    // For each service type we are associated with, if it doesn't
    // match then we try its parent service types.
    QStringList::ConstIterator it = d->m_lstServiceTypes.begin();
    for( ; it != d->m_lstServiceTypes.end(); ++it )
    {
        it->toInt(&isNumber);
        if (isNumber)
            continue;
        //kDebug(7012) << "    has " << (*it);
        if ( *it == ptr->name() )
            return true;
        // also the case of parent servicetypes
        KServiceType::Ptr p = KServiceType::serviceType( *it );
        if ( p && p->inherits( ptr->name() ) )
            return true;
        // #### but we can't handle inherited mimetypes here,
        // since KMimeType is in kio... One solution would be a kdecore
        // factory for accessing the mimetypes as servicetypes, plus
        // the notion of the parent mimetype; ouch.
        // Or a reverse API, KMimeType::isHandledBy( KService::Ptr )
        // and KServiceType::isHandledBy( KService::Ptr )...
        // Anyway this should be a very rare case, most code gets KServices from ksycoca.
    }
    return false;
}

//TODO: make this a KConfigBackend instead
class KServiceReadProperty : public KConfigBase
{
public:
   KServiceReadProperty(const QString &_key, const QByteArray &_value)
	: key(_key), value(_value) { }

   bool internalHasGroup(const QByteArray &) const { /*qDebug("hasGroup(const QByteArray &)");*/ return false; }

   QStringList groupList() const { return QStringList(); }

   QMap<QString,QString> entryMap(const QString &) const
      { return QMap<QString,QString>(); }

   void reparseConfiguration() { }

   KEntryMap internalEntryMap( const QString &) const { return KEntryMap(); }

   KEntryMap internalEntryMap() const { return KEntryMap(); }

   void putData(const KEntryKey &, const KEntry&, bool) { }

   KEntry lookupData(const KEntryKey &) const
   { KEntry entry; entry.mValue = value; return entry; }

   template <typename T>
      T readEntry( const QString& pKey, const T& aDefault) const
   { return internalGroup().readEntry(pKey.toUtf8().constData(), aDefault); }

protected:
   QString key;
   QByteArray value;
};

QVariant KService::property( const QString& _name) const
{
    return property( _name, QVariant::Invalid);
}

// Return a string QVariant if string isn't null, and invalid variant otherwise
// (the variant must be invalid if the field isn't in the .desktop file)
// This allows trader queries like "exist Library" to work.
static QVariant makeStringVariant( const QString& string )
{
    // Using isEmpty here would be wrong.
    // Empty is "specified but empty", null is "not specified" (in the .desktop file)
    return string.isNull() ? QVariant() : QVariant( string );
}

QVariant KService::property( const QString& _name, QVariant::Type t ) const
{
    if ( _name == "Type" )
        return QVariant( d->m_strType ); // can't be null
    else if ( _name == "Name" )
        return QVariant( d->m_strName ); // can't be null
    else if ( _name == "Exec" )
        return makeStringVariant( d->m_strExec );
    else if ( _name == "Icon" )
        return makeStringVariant( d->m_strIcon );
    else if ( _name == "Terminal" )
        return QVariant( static_cast<int>(d->m_bTerminal) );
    else if ( _name == "TerminalOptions" )
        return makeStringVariant( d->m_strTerminalOptions );
    else if ( _name == "Path" )
        return makeStringVariant( d->m_strPath );
    else if ( _name == "Comment" )
        return makeStringVariant( d->m_strComment );
    else if ( _name == "GenericName" )
        return makeStringVariant( d->m_strGenName );
    else if ( _name == "ServiceTypes" )
        return QVariant( d->m_lstServiceTypes );
    else if ( _name == "AllowAsDefault" )
        return QVariant( static_cast<int>(d->m_bAllowAsDefault) );
    else if ( _name == "InitialPreference" )
        return QVariant( d->m_initialPreference );
    else if ( _name == "Library" )
        return makeStringVariant( d->m_strLibrary );
    else if ( _name == "DesktopEntryPath" ) // can't be null
        return QVariant( entryPath() );
    else if ( _name == "DesktopEntryName")
        return QVariant( d->m_strDesktopEntryName ); // can't be null
    else if ( _name == "Categories")
        return QVariant( d->categories );
    else if ( _name == "Keywords")
        return QVariant( d->m_lstKeywords );

    // Ok we need to convert the property from a QString to its real type.
    // Maybe the caller helped us.
    if (t == QVariant::Invalid)
    {
        // No luck, let's ask KServiceTypeFactory what the type of this property
        // is supposed to be.
        t = KServiceTypeFactory::self()->findPropertyTypeByName(_name);
        if (t == QVariant::Invalid)
        {
            kDebug(7012) << "Request for unknown property '" << _name << "'\n";
            return QVariant(); // Unknown property: Invalid variant.
        }
    }

    // Then we use a homebuild class based on KConfigBase to convert the QString.
    // For some often used property types we do the conversion ourselves.
    QMap<QString,QVariant>::ConstIterator it = d->m_mapProps.find( _name );
    if ( (it == d->m_mapProps.end()) || (!it->isValid()))
    {
        //kDebug(7012) << "Property not found " << _name;
        return QVariant(); // No property set.
    }

    switch(t)
    {
    case QVariant::String:
        return *it;
    case QVariant::Bool:
    case QVariant::Int:
    {
        QString aValue = it->toString().toLower();
        int val = 0;
        if (aValue == "true" || aValue == "on" || aValue == "yes")
            val = 1;
        else
        {
            bool bOK;
            val = aValue.toInt( &bOK );
            if( !bOK )
                val = 0;
        }
        if (t == QVariant::Bool)
        {
            return QVariant(bool(val));
        }
        return QVariant(val);
    }
    default:
        // All others
        // For instance properties defined as StringList, like MimeTypes.
        KServiceReadProperty ksrp(_name, it->toString().toUtf8());
        return ksrp.readEntry(_name, QVariant(t));
    }
}

QStringList KService::propertyNames() const
{
    QStringList res;

    QMap<QString,QVariant>::ConstIterator it = d->m_mapProps.begin();
    for( ; it != d->m_mapProps.end(); ++it )
        res.append( it.key() );

    res.append( "Type" );
    res.append( "Name" );
    res.append( "Comment" );
    res.append( "GenericName" );
    res.append( "Icon" );
    res.append( "Exec" );
    res.append( "Terminal" );
    res.append( "TerminalOptions" );
    res.append( "Path" );
    res.append( "ServiceTypes" );
    res.append( "AllowAsDefault" );
    res.append( "InitialPreference" );
    res.append( "Library" );
    res.append( "DesktopEntryPath" );
    res.append( "DesktopEntryName" );
    res.append( "Keywords" );
    res.append( "Categories" );

    return res;
}

KService::List KService::allServices()
{
    return KServiceFactory::self()->allServices();
}

KService::Ptr KService::serviceByName( const QString& _name )
{
    return KServiceFactory::self()->findServiceByName( _name );
}

KService::Ptr KService::serviceByDesktopPath( const QString& _name )
{
    return KServiceFactory::self()->findServiceByDesktopPath( _name );
}

KService::Ptr KService::serviceByDesktopName( const QString& _name )
{
    KService::Ptr s = KServiceFactory::self()->findServiceByDesktopName( _name.toLower() );
    if (!s && !_name.startsWith("kde4-"))
        s = KServiceFactory::self()->findServiceByDesktopName( "kde4-"+_name.toLower() );
    return s;
}

KService::Ptr KService::serviceByMenuId( const QString& _name )
{
    return KServiceFactory::self()->findServiceByMenuId( _name );
}

KService::Ptr KService::serviceByStorageId( const QString& _storageId )
{
    KService::Ptr service = KService::serviceByMenuId( _storageId );
    if (service)
        return service;

    service = KService::serviceByDesktopPath(_storageId);
    if (service)
        return service;

    if (!QDir::isRelativePath(_storageId) && QFile::exists(_storageId))
        return KService::Ptr(new KService(_storageId));

    QString tmp = _storageId;
    tmp = tmp.mid(tmp.lastIndexOf('/')+1); // Strip dir

    if (tmp.endsWith(".desktop"))
        tmp.truncate(tmp.length()-8);

    if (tmp.endsWith(".kdelnk"))
        tmp.truncate(tmp.length()-7);

    service = KService::serviceByDesktopName(tmp);

    return service;
}

bool KService::substituteUid() const {
    QVariant v = property("X-KDE-SubstituteUID", QVariant::Bool);
    return v.isValid() && v.toBool();
}

QString KService::username() const {
    // See also KDesktopFile::tryExec()
    QString user;
    QVariant v = property("X-KDE-Username", QVariant::String);
    user = v.isValid() ? v.toString() : QString();
    if (user.isEmpty())
        user = ::getenv("ADMIN_ACCOUNT");
    if (user.isEmpty())
        user = "root";
    return user;
}

bool KService::noDisplay() const {
    QMap<QString,QVariant>::ConstIterator it = d->m_mapProps.find( "NoDisplay" );
    if ( (it != d->m_mapProps.end()) && (it->isValid()))
    {
        QString aValue = it->toString().toLower();
        if (aValue == "true" || aValue == "on" || aValue == "yes")
            return true;
    }

    it = d->m_mapProps.find( "OnlyShowIn" );
    if ( (it != d->m_mapProps.end()) && (it->isValid()))
    {
        QString aValue = it->toString();
        QStringList aList = aValue.split(';');
        if (!aList.contains("KDE"))
            return true;
    }

    it = d->m_mapProps.find( "NotShowIn" );
    if ( (it != d->m_mapProps.end()) && (it->isValid()))
    {
        QString aValue = it->toString();
        QStringList aList = aValue.split(';');
        if (aList.contains("KDE"))
            return true;
    }

    if (!KAuthorized::authorizeControlModule(d->menuId))
        return true;

    return false;
}

QString KService::untranslatedGenericName() const {
    QVariant v = property("UntranslatedGenericName", QVariant::String);
    return v.isValid() ? v.toString() : QString();
}

QString KService::parentApp() const {
    QMap<QString,QVariant>::ConstIterator it = d->m_mapProps.find( "X-KDE-ParentApp" );
    if ( (it == d->m_mapProps.end()) || (!it->isValid()))
    {
        return QString();
    }

    return it->toString();
}

bool KService::allowMultipleFiles() const {
    // Can we pass multiple files on the command line or do we have to start the application for every single file ?
    return (d->m_strExec.contains( "%F" ) || d->m_strExec.contains( "%U" ) ||
            d->m_strExec.contains( "%N" ) || d->m_strExec.contains( "%D" ));
}

QStringList KService::categories() const
{
    return d->categories;
}

QString KService::menuId() const
{
    return d->menuId;
}

void KService::setMenuId(const QString &_menuId)
{
    d->menuId = _menuId;
}

QString KService::storageId() const
{
    if (!d->menuId.isEmpty())
        return d->menuId;
    return entryPath();
}

QString KService::locateLocal() const
{
    if (d->menuId.isEmpty() || desktopEntryPath().startsWith(".hidden") ||
        (QDir::isRelativePath(desktopEntryPath()) && d->categories.isEmpty()))
        return KDesktopFile::locateLocal(desktopEntryPath());

    return KStandardDirs::locateLocal("xdgdata-apps", d->menuId);
}

QString KService::newServicePath(bool showInMenu, const QString &suggestedName,
                                 QString *menuId, const QStringList *reservedMenuIds)
{
    QString base = suggestedName;
    if (!showInMenu)
        base.prepend("kde4-");

    QString result;
    for(int i = 1; true; i++)
    {
        if (i == 1)
            result = base + ".desktop";
        else
            result = base + QString("-%1.desktop").arg(i);

        if (reservedMenuIds && reservedMenuIds->contains(result))
            continue;

        // Lookup service by menu-id
        KService::Ptr s = serviceByMenuId(result);
        if (s)
            continue;

        if (showInMenu)
        {
            if (!KStandardDirs::locate("xdgdata-apps", result).isEmpty())
                continue;
        }
        else
        {
            QString file = result.mid(5); // Strip "kde4-"
            if (!KStandardDirs::locate("apps", ".hidden/"+file).isEmpty())
                continue;
        }

        break;
    }
    if (menuId)
        *menuId = result;

    if (showInMenu)
    {
        return KStandardDirs::locateLocal("xdgdata-apps", result);
    }

    QString file = result.mid(5); // Strip "kde4-"
    return KStandardDirs::locateLocal("apps", ".hidden/"+file);
}

bool KService::isApplication() const
{
    return d->m_strType == "Application";
}

QString KService::type() const
{
    return d->m_strType;
}

void KService::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }

QString KService::name() const
{
    return d->m_strName;
}

QString KService::exec() const
{
    return d->m_strExec;
}

QString KService::library() const
{
    return d->m_strLibrary;
}

QString KService::icon() const
{
    return d->m_strIcon;
}

QString KService::terminalOptions() const
{
    return d->m_strTerminalOptions;
}

bool KService::terminal() const
{
    return d->m_bTerminal;
}

// KDE4: remove and port code to entryPath?
QString KService::desktopEntryPath() const
{
    return entryPath();
}

QString KService::desktopEntryName() const
{
    return d->m_strDesktopEntryName;
}

KService::DBUSStartupType_t KService::DBUSStartupType() const
{
    return d->m_DBUSStartusType;
}

QString KService::path() const
{
    return d->m_strPath;
}

QString KService::comment() const
{
    return d->m_strComment;
}

QString KService::genericName() const
{
    return d->m_strGenName;
}

QStringList KService::keywords() const
{
    return d->m_lstKeywords;
}

QStringList KService::serviceTypes() const
{
    return d->m_lstServiceTypes;
}

bool KService::allowAsDefault() const
{
    return d->m_bAllowAsDefault;
}

int KService::initialPreference() const
{
    return d->m_initialPreference;
}

// unused now, it seems
/*void KService::setInitialPreference( int i )
  {
  d->m_initialPreference = i;
  }
*/

bool KService::isValid() const
{
    return d->m_bValid;
}

void KService::setTerminal(bool b)
{
    d->m_bTerminal = b;
}

void KService::setTerminalOptions(const QString &options)
{
    d->m_strTerminalOptions = options;
}

QStringList & KService::accessServiceTypes()
{
    return d->m_lstServiceTypes;
}
