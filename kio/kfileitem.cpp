/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
                 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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
// $Id$

#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include <assert.h>
#include <unistd.h>

#include "kfileitem.h"

#include <qdir.h>
#include <qfile.h>
#include <qmap.h>
#include <qstylesheet.h>

#include <klargefile.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmimetype.h>
#include <krun.h>
#include <kfilemetainfo.h>

class KFileItem::KFileItemPrivate
{
public:
  KFileItemPrivate() {
    bMimeTypeKnown = false;
    metaInfo = 0L;
    refresh();
  }

  void operator=( const KFileItemPrivate& d ) {
      guessedMimeType   = d.guessedMimeType;
      access            = d.access;
      bMimeTypeKnown    = d.bMimeTypeKnown;
      for ( int i = 0; i < NumFlags; i++ )
          time[i] = d.time[i];
      size              = d.size;
      // note: extra is NOT copied, as we'd have no control over who is
      // deleting the data or not.
  }

  void refresh() {
    access = QString::null;
    size = (KIO::filesize_t) -1;
    for ( int i = 0; i < NumFlags; i++ )
      time[i] = (time_t) -1;
  }

   // For special case like link to dirs over FTP
  QString guessedMimeType;
  QString access;
  bool bMimeTypeKnown;
  QMap<const void*, void*> extra;
  KFileMetaInfo *metaInfo;

  enum { Modification = 0, Access = 1, Creation = 2, NumFlags = 3 };
  time_t time[3];
  KIO::filesize_t size;
};

KFileItem::KFileItem( const KIO::UDSEntry& _entry, const KURL& _url,
                      bool _determineMimeTypeOnDemand, bool _urlIsDirectory ) :
  m_entry( _entry ),
  m_url( _url ),
  m_bIsLocalURL( _url.isLocalFile() ),
  m_fileMode( (mode_t)-1 ),
  m_permissions( (mode_t)-1 ),
  m_bLink( false ),
  m_pMimeType( 0 ),
  m_bMarked( false ),
  d(new KFileItemPrivate)
{
  bool UDS_URL_seen = false;
  // extract the mode and the filename from the KIO::UDS Entry
  KIO::UDSEntry::ConstIterator it = m_entry.begin();
  for( ; it != m_entry.end(); it++ ) {
    switch ((*it).m_uds) {

        case KIO::UDS_FILE_TYPE:
          m_fileMode = (mode_t)((*it).m_long);
          break;

        case KIO::UDS_ACCESS:
          m_permissions = (mode_t)((*it).m_long);
          break;

        case KIO::UDS_USER:
          m_user = ((*it).m_str);
          break;

        case KIO::UDS_GROUP:
          m_group = ((*it).m_str);
          break;

        case KIO::UDS_NAME:
          m_strName = (*it).m_str;
          m_strText = KIO::decodeFileName( m_strName );
          break;

        case KIO::UDS_URL:
          UDS_URL_seen = true;
          m_url = KURL((*it).m_str);
          break;

        case KIO::UDS_MIME_TYPE:
          m_pMimeType = KMimeType::mimeType((*it).m_str);
          d->bMimeTypeKnown = true;
          break;

        case KIO::UDS_GUESSED_MIME_TYPE:
          d->guessedMimeType = (*it).m_str;
          break;

        case KIO::UDS_LINK_DEST:
          m_bLink = !(*it).m_str.isEmpty(); // we don't store the link dest
          break;
    }
  }
  // avoid creating these QStrings again and again
  static const QString& dot = KGlobal::staticQString(".");
  if ( _urlIsDirectory && !UDS_URL_seen && !m_strName.isEmpty() && m_strName != dot )
    m_url.addPath( m_strName );
  init( _determineMimeTypeOnDemand );
}

KFileItem::KFileItem( mode_t _mode, mode_t _permissions, const KURL& _url, bool _determineMimeTypeOnDemand ) :
  m_entry(), // warning !
  m_url( _url ),
  m_bIsLocalURL( _url.isLocalFile() ),
  m_strName( _url.fileName() ),
  m_strText( KIO::decodeFileName( m_strName ) ),
  m_fileMode ( _mode ),
  m_permissions( _permissions ),
  m_bLink( false ),
  m_bMarked( false ),
  d(new KFileItemPrivate)
{
  init( _determineMimeTypeOnDemand );
}

KFileItem::KFileItem( const KURL &url, const QString &mimeType, mode_t mode )
:  m_url( url ),
  m_bIsLocalURL( url.isLocalFile() ),
  m_strName( url.fileName() ),
  m_strText( KIO::decodeFileName( m_strName ) ),
  m_fileMode( mode ),
  m_permissions( 0 ),
  m_bLink( false ),
  m_bMarked( false ),
  d(new KFileItemPrivate)
{
  m_pMimeType = KMimeType::mimeType( mimeType );
  init( false );
}

KFileItem::KFileItem( const KFileItem & item ) :
  d(new KFileItemPrivate)
{
    assign( item );
}

KFileItem::~KFileItem()
{
  delete d;
}

void KFileItem::init( bool _determineMimeTypeOnDemand )
{
  // determine mode and/or permissions if unknown
  if ( m_fileMode == (mode_t) -1 || m_permissions == (mode_t) -1 )
  {
    mode_t mode = 0;
    if ( m_url.isLocalFile() )
    {
      /* directories may not have a slash at the end if
       * we want to stat() them; it requires that we
       * change into it .. which may not be allowed
       * stat("/is/unaccessible")  -> rwx------
       * stat("/is/unaccessible/") -> EPERM            H.Z.
       * This is the reason for the -1
       */
      KDE_struct_stat buf;
      QCString path = QFile::encodeName(m_url.path( -1 ));
      if ( KDE_lstat( path.data(), &buf ) == 0 )
      {
        mode = buf.st_mode;
        if ( S_ISLNK( mode ) )
        {
          m_bLink = true;
          if ( KDE_stat( path.data(), &buf ) == 0 )
              mode = buf.st_mode;
          else // link pointing to nowhere (see kio/file/file.cc)
              mode = (S_IFMT-1) | S_IRWXU | S_IRWXG | S_IRWXO;
        }
      }
    }
    if ( m_fileMode == (mode_t) -1 )
      m_fileMode = mode & S_IFMT; // extract file type
    if ( m_permissions == (mode_t) -1 )
      m_permissions = mode & 07777; // extract permissions
  }

  // determine the mimetype
  if (!m_pMimeType )
  {
      m_pMimeType = KMimeType::findByURL( m_url, m_fileMode, m_bIsLocalURL,
                                          // use fast mode if not mimetype on demand
                                          _determineMimeTypeOnDemand );
      // if we didn't use fast mode, or if we got a result, then this is the mimetype
      // otherwise, determineMimeType will be able to do better.
      d->bMimeTypeKnown = (!_determineMimeTypeOnDemand) || (m_pMimeType->name() != KMimeType::defaultMimeType());
  }

}

void KFileItem::refresh()
{
  m_fileMode = (mode_t)-1;
  m_permissions = (mode_t)-1;
  m_user = QString::null;
  m_group = QString::null;
  d->refresh();
  // Basically, we can't trust any information we got while listing.
  // Everything could have changed...
  // Clearing m_entry makes it possible to detect changes in the size of the file,
  // the time information, etc.
  m_entry = KIO::UDSEntry();
  init( false );
}

void KFileItem::refreshMimeType()
{
  m_pMimeType = 0L;
  init( false ); // Will determine the mimetype
}

void KFileItem::setURL( const KURL &url )
{
  m_url = url;
  m_strName = url.fileName();
  m_strText = KIO::decodeFileName( m_strName );
}

QString KFileItem::linkDest() const
{
  // Extract it from the KIO::UDSEntry
  KIO::UDSEntry::ConstIterator it = m_entry.begin();
  for( ; it != m_entry.end(); it++ )
    if ( (*it).m_uds == KIO::UDS_LINK_DEST )
      return (*it).m_str;
  // If not in the KIO::UDSEntry, or if UDSEntry empty, use readlink() [if local URL]
  if ( m_bIsLocalURL )
  {
    char buf[1000];
    int n = readlink( QFile::encodeName(m_url.path( -1 )), buf, 1000 );
    if ( n != -1 )
    {
      buf[ n ] = 0;
      return QString::fromLocal8Bit( buf );
    }
  }
  return QString::null;
}

KIO::filesize_t KFileItem::size() const
{
  if ( d->size != (KIO::filesize_t) -1 )
    return d->size;

  // Extract it from the KIO::UDSEntry
  KIO::UDSEntry::ConstIterator it = m_entry.begin();
  for( ; it != m_entry.end(); it++ )
    if ( (*it).m_uds == KIO::UDS_SIZE ) {
      d->size = (*it).m_long;
      return d->size;
    }
  // If not in the KIO::UDSEntry, or if UDSEntry empty, use stat() [if local URL]
  if ( m_bIsLocalURL )
  {
    KDE_struct_stat buf;
    KDE_stat( QFile::encodeName(m_url.path( -1 )), &buf );
    return buf.st_size;
  }
  return 0L;
}

time_t KFileItem::time( unsigned int which ) const
{
  unsigned int mappedWhich = 0;

  switch( which ) {
    case KIO::UDS_MODIFICATION_TIME:
      mappedWhich = KFileItemPrivate::Modification;
      break;
    case KIO::UDS_ACCESS_TIME:
      mappedWhich = KFileItemPrivate::Access;
      break;
    case KIO::UDS_CREATION_TIME:
      mappedWhich = KFileItemPrivate::Creation;
      break;
  }

  if ( d->time[mappedWhich] != (time_t) -1 )
    return d->time[mappedWhich];

  // Extract it from the KIO::UDSEntry
  KIO::UDSEntry::ConstIterator it = m_entry.begin();
  for( ; it != m_entry.end(); it++ )
    if ( (*it).m_uds == which ) {
      d->time[mappedWhich] = static_cast<time_t>((*it).m_long);
      return d->time[mappedWhich];
    }

  // If not in the KIO::UDSEntry, or if UDSEntry empty, use stat() [if local URL]
  if ( m_bIsLocalURL )
  {
    KDE_struct_stat buf;
    KDE_stat( QFile::encodeName(m_url.path( -1 )), &buf );
    d->time[mappedWhich] = (which == KIO::UDS_MODIFICATION_TIME) ?
                           buf.st_mtime :
                           (which == KIO::UDS_ACCESS_TIME) ? buf.st_atime :
                           static_cast<time_t>(0); // We can't determine creation time for local files
    return d->time[mappedWhich];
  }
  return static_cast<time_t>(0);
}


QString KFileItem::user() const
{
  if ( m_user.isEmpty() && m_bIsLocalURL )
  {
    KDE_struct_stat buff;
    if ( KDE_lstat( QFile::encodeName(m_url.path( -1 )), &buff ) == 0) // get uid/gid of the link, if it's a link
    {
      struct passwd *user = getpwuid( buff.st_uid );
      if ( user != 0L )
        m_user = QString::fromLocal8Bit(user->pw_name);
    }
  }
  return m_user;
}

QString KFileItem::group() const
{
  if (m_group.isEmpty() && m_bIsLocalURL )
  {
    KDE_struct_stat buff;
    if ( KDE_lstat( QFile::encodeName(m_url.path( -1 )), &buff ) == 0) // get uid/gid of the link, if it's a link
    {
      struct group *ge = getgrgid( buff.st_gid );
      if ( ge != 0L ) {
        m_group = QString::fromLocal8Bit(ge->gr_name);
        if (m_group.isEmpty())
          m_group.sprintf("%d",ge->gr_gid);
      } else
        m_group.sprintf("%d",buff.st_gid);
    }
  }
  return m_group;
}

QString KFileItem::mimetype() const
{
  KFileItem * that = const_cast<KFileItem *>(this);
  return that->determineMimeType()->name();
}

KMimeType::Ptr KFileItem::determineMimeType()
{
  if ( !m_pMimeType || !d->bMimeTypeKnown )
  {
    //kdDebug(1203) << "finding mimetype for " << m_url.url() << endl;
    m_pMimeType = KMimeType::findByURL( m_url, m_fileMode, m_bIsLocalURL );
    d->bMimeTypeKnown = true;
  }

  return m_pMimeType;
}

bool KFileItem::isMimeTypeKnown() const
{
  // The mimetype isn't known if determineMimeType was never called (on-demand determination)
  // or if this fileitem has a guessed mimetype (e.g. ftp symlink) - in which case
  // it always remains "not fully determined"
  return d->bMimeTypeKnown && d->guessedMimeType.isEmpty();
}

QString KFileItem::mimeComment()
{
 KMimeType::Ptr mType = determineMimeType();
 QString comment = mType->comment( m_url, false );
  if (!comment.isEmpty())
    return comment;
  else
    return mType->name();
}

QString KFileItem::iconName()
{
  return determineMimeType()->icon(m_url, false);
}

QPixmap KFileItem::pixmap( int _size, int _state ) const
{
  if ( !m_pMimeType )
  {
    static const QString & defaultFolderIcon =
       KGlobal::staticQString(KMimeType::mimeType( "inode/directory" )->KServiceType::icon());
    if ( S_ISDIR( m_fileMode ) )
     return DesktopIcon( defaultFolderIcon, _size, _state );

    return DesktopIcon( "unknown", _size, _state );
  }

  if ( m_bLink )
      _state |= KIcon::LinkOverlay;

  if ( S_ISDIR( m_fileMode ) // Locked dirs have a special icon
       && !isReadable())
     _state |= KIcon::LockOverlay;

  if ( m_strName[0] == '.' )
     _state |= KIcon::HiddenOverlay;

  KMimeType::Ptr mime;
  // Use guessed mimetype if the main one hasn't been determined for sure
  if ( !d->bMimeTypeKnown && !d->guessedMimeType.isEmpty() )
      mime = KMimeType::mimeType( d->guessedMimeType );
  else
      mime = m_pMimeType;

  // Support for gzipped files
  if ( mime->name() == "application/x-gzip" && m_url.fileName().right(3) == ".gz" )
  {
      QString subFileName = m_url.path().left( m_url.path().length() - 3 );
      //kdDebug() << "KFileItem::pixmap subFileName=" << subFileName << endl;
      mime = KMimeType::findByURL( subFileName, 0, m_bIsLocalURL );
      _state |= KIcon::ZipOverlay;
  }

  QPixmap p = mime->pixmap( m_url, KIcon::Desktop, _size, _state );
  if (p.isNull())
      kdWarning() << "Pixmap not found for mimetype " << m_pMimeType->name() << endl;

  return p;
}

bool KFileItem::isReadable() const
{
  /*
  struct passwd * user = getpwuid( geteuid() );
  bool isMyFile = (QString::fromLocal8Bit(user->pw_name) == m_user);
  // This gets ugly for the group....
  // Maybe we want a static QString for the user and a static QStringList
  // for the groups... then we need to handle the deletion properly...
  */

  // No read permission at all
  if ( !(S_IRUSR & m_permissions) && !(S_IRGRP & m_permissions) && !(S_IROTH & m_permissions) )
      return false;

  // Or if we can't read it [using ::access()] - not network transparent
  else if ( m_bIsLocalURL && ::access( QFile::encodeName(m_url.path()), R_OK ) == -1 )
      return false;

  return true;
}

bool KFileItem::isDir() const
{
  if ( !d->bMimeTypeKnown && !d->guessedMimeType.isEmpty() )
  {
    kdDebug() << " KFileItem::isDir can't say -> false " << endl;
    return false; // can't say for sure, so no
  }
  return S_ISDIR(m_fileMode);
}

bool KFileItem::acceptsDrops()
{
  // A directory ?
  if ( S_ISDIR( mode() ) )
  {
    if ( m_bIsLocalURL ) // local -> check if we can enter it
       return (::access( QFile::encodeName(m_url.path()), X_OK ) == 0);
    else
       return true; // assume ok for remote urls
  }

  // But only local .desktop files and executables
  if ( !m_bIsLocalURL )
    return false;

  if ( mimetype() == "application/x-desktop")
    return true;

  // Executable, shell script ... ?
  if ( ::access( QFile::encodeName(m_url.path()), X_OK ) == 0 )
    return true;

  return false;
}

QString KFileItem::getStatusBarInfo()
{
  QString comment = determineMimeType()->comment( m_url, false );
  QString text = m_strText;
  // Extract from the KIO::UDSEntry the additional info we didn't get previously
  QString myLinkDest = linkDest();
  KIO::filesize_t mySize = size();

  if ( m_bLink )
  {
      QString tmp;
      if ( comment.isEmpty() )
        tmp = i18n ( "Symbolic Link" );
      else
        tmp = i18n("%1 (Link)").arg(comment);
      text += "->";
      text += myLinkDest;
      text += "  ";
      text += tmp;
  }
  else if ( S_ISREG( m_fileMode ) )
  {
      text += QString(" (%1)").arg( KIO::convertSize( mySize ) );
      text += "  ";
      text += comment;
  }
  else if ( S_ISDIR ( m_fileMode ) )
  {
      text += "/  ";
      text += comment;
    }
    else
    {
      text += "  ";
      text += comment;
    }
    return text;
}

QString KFileItem::getToolTipText()
{
  // we can return QString::null if no tool tip should be shown
#ifdef _GNUC  
#waring move that tool tip maxcout elsewhere (make it configurable?)
#endif
  const int maxcount = 6;

  QString tip;
  KFileMetaInfo* info = 0L;
  
  if ( m_url.isLocalFile() ) 
  {
    KFileMetaInfoProvider *prov = KFileMetaInfoProvider::self();
    if ( !d->metaInfo )
    {
      info = prov->metaInfo( m_url.path() );
      d->metaInfo = info;
    }
    else
      info = d->metaInfo;
  }

  QStringList keys;
  
  // if we got no or empty info, show a default tip
  if ( !info || (keys = info->preferredKeys()) .isEmpty() )
  {
    kdDebug() << "Found no meta info" << endl;
    tip  = "<nobr><b><center>"+ m_url.fileName() + "</center></b>";
    tip += i18n("Type: ");
        
    KMimeType::Ptr mimetype = determineMimeType();
    if ( m_bLink )
      tip += i18n("Link to %1<br>").arg(mimetype->comment());
    else
      tip += mimetype->comment() + "<br>";

    if ( !S_ISDIR ( m_fileMode ) )
      tip += i18n("Size: ") + KIO::convertSize( size() ) + "<br>";

    tip += i18n("Modified: ") + 
                timeString( KIO::UDS_MODIFICATION_TIME) + "<br>" +
           i18n("Permissions: ") + parsePermissions(m_permissions) +"</nobr>";
  }
  else
  {
    // first the title in bold and centered
    KFileMetaInfoItem *item;
    
    // if we don't find a title, show the file name instead
    if (! (item = info->item("Title")) && ! (item = info->item("Name")))
      tip = "<nobr><b><center>"+ QStyleSheet::escape(m_url.fileName()) +
                "</center></b>";
    else
      tip = "<nobr><b><center>" + QStyleSheet::escape(item->value().toString())+
            "</center></b>";
      
    // now the rest
    
    QStringList::Iterator it = keys.begin();
    for (int count = 0; count<=maxcount && it!=keys.end() ; ++it)
    {
      item = info->item( *it );
      if ( item && (item->key() != "Title") && (item->key() != "Name") )
      {
        QString s;
        if ( item->value().type() == QVariant::Bool )
          s = item->value().toBool() ? i18n("Yes") : i18n("No");
        else
          s = item->value().toString();
      
        if ( (s != QString::null) && ! s.isEmpty() )
        {
          count++;
          tip += QStyleSheet::escape( item->translatedKey() ) + ": " +
                 item->prefix() +
                 QStyleSheet::escape( s ) +
                 item->postfix();
          
          if ( (item->key()!=keys.last()) && (count<=maxcount) ) tip += "<br>";
        }

      }
    }
    tip += "</nobr>";
  }
  return tip;
}

void KFileItem::run()
{
  KURL url( m_url );
  if ( m_bLink )
    url = KURL( m_url, linkDest() );
  (void) new KRun( url, m_fileMode, m_bIsLocalURL );
}

bool KFileItem::cmp( const KFileItem & item )
{
    return ( m_strName == item.m_strName
             && m_bIsLocalURL == item.m_bIsLocalURL
             && m_fileMode == item.m_fileMode
             && m_permissions == item.m_permissions
             && m_user == item.m_user
             && m_group == item.m_group
             && m_bLink == item.m_bLink
             && size() == item.size()
             && time(KIO::UDS_MODIFICATION_TIME) == item.time(KIO::UDS_MODIFICATION_TIME) );
}

void KFileItem::assign( const KFileItem & item )
{
    m_entry = item.m_entry;
    m_url = item.m_url;
    m_bIsLocalURL = item.m_bIsLocalURL;
    m_strName = item.m_strName;
    m_strText = item.m_strText;
    m_fileMode = item.m_fileMode;
    m_permissions = item.m_permissions;
    m_user = item.m_user;
    m_group = item.m_group;
    m_bLink = item.m_bLink;
    m_pMimeType = item.m_pMimeType;
    m_strLowerCaseName = item.m_strLowerCaseName;
    *d = *item.d; // Note: d->extra is just a shallow copy...
    // We had a mimetype previously (probably), so we need to re-determine it
    determineMimeType();
}

void KFileItem::setExtraData( const void *key, void *value )
{
    if ( !key )
        return;

    d->extra.replace( key, value );
}

const void * KFileItem::extraData( const void *key ) const
{
    QMapConstIterator<const void*,void*> it = d->extra.find( key );
    if ( it != d->extra.end() )
        return it.data();
    return 0L;
}

void * KFileItem::extraData( const void *key )
{
    QMapIterator<const void*,void*> it = d->extra.find( key );
    if ( it != d->extra.end() )
        return it.data();
    return 0L;
}

void KFileItem::removeExtraData( const void *key )
{
    d->extra.remove( key );
}

QString KFileItem::permissionsString() const
{
    if (d->access.isNull())
      d->access = parsePermissions( m_permissions );

    return d->access;
}

QString KFileItem::parsePermissions(mode_t perm) const
{
    char p[] = "----------";

    if (isDir())
	p[0]='d';
    else if (isLink())
	p[0]='l';

    if (perm & QFileInfo::ReadUser)
	p[1]='r';
    if (perm & QFileInfo::WriteUser)
	p[2]='w';
    if (perm & QFileInfo::ExeUser)
	p[3]='x';

    if (perm & QFileInfo::ReadGroup)
	p[4]='r';
    if (perm & QFileInfo::WriteGroup)
	p[5]='w';
    if (perm & QFileInfo::ExeGroup)
	p[6]='x';

    if (perm & QFileInfo::ReadOther)
	p[7]='r';
    if (perm & QFileInfo::WriteOther)
	p[8]='w';
    if (perm & QFileInfo::ExeOther)
	p[9]='x';

    return QString::fromLatin1(p);
}

// check if we need to cache this
QString KFileItem::timeString( unsigned int which ) const
{
    QDateTime t;
    t.setTime_t( time(which) );
    return KGlobal::locale()->formatDateTime( t );
}

void KFileItem::setMetaInfo( KFileMetaInfo *info )
{
    d->metaInfo = info;
}

KFileMetaInfo * KFileItem::metaInfo() const
{
    return d->metaInfo;
}

