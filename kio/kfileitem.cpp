/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#include <assert.h>
#include <unistd.h>

#include "kfileitem.h"

#include <qdir.h>
#include <qfile.h>

#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmimetype.h>
#include <krun.h>

class KFileItem::KFileItemPrivate
{
public:
  KFileItemPrivate() {}
   // For special case like link to dirs over FTP
  QString m_guessedMimeType;
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
          break;

        case KIO::UDS_GUESSED_MIME_TYPE:
          d->m_guessedMimeType = (*it).m_str;
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
      struct stat buf;
      if ( lstat( QFile::encodeName(m_url.path( -1 )), &buf ) == 0 ) // set mode only if lstat succeeded! otherwise buf is
                                                  // undefined! (Simon)
      {
        mode = buf.st_mode;

        if ( S_ISLNK( mode ) )
        {
          m_bLink = true;
          stat( QFile::encodeName(m_url.path( -1 )), &buf ); // shouldn't we check if stat() succeeded or not? (before
                                          // taking buf.st_mode blindly ) (Simon)
          mode = buf.st_mode;
        }
      }
    }
    if ( m_fileMode == (mode_t) -1 )
      m_fileMode = mode & S_IFMT; // extract file type
    if ( m_permissions == (mode_t) -1 )
      m_permissions = mode & 07777; // extract permissions
  }

  // determine the mimetype
  if (!m_pMimeType && !_determineMimeTypeOnDemand )
      m_pMimeType = KMimeType::findByURL( m_url, m_fileMode, m_bIsLocalURL );

}

void KFileItem::refresh()
{
  m_fileMode = (mode_t)-1;
  m_permissions = (mode_t)-1;
  init( false );
}

void KFileItem::refreshMimeType()
{
  m_pMimeType = 0L;
  init( false ); // Will determine the mimetype
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
      return QString( buf );
    }
  }
  return QString::null;
}

long KFileItem::size() const
{
  // Extract it from the KIO::UDSEntry
  KIO::UDSEntry::ConstIterator it = m_entry.begin();
  for( ; it != m_entry.end(); it++ )
    if ( (*it).m_uds == KIO::UDS_SIZE )
      return (*it).m_long;
  // If not in the KIO::UDSEntry, or if UDSEntry empty, use stat() [if local URL]
  if ( m_bIsLocalURL )
  {
    struct stat buf;
    stat( QFile::encodeName(m_url.path( -1 )), &buf );
    return buf.st_size;
  }
  return 0L;
}

time_t KFileItem::time( unsigned int which ) const
{
  // Extract it from the KIO::UDSEntry
  KIO::UDSEntry::ConstIterator it = m_entry.begin();
  for( ; it != m_entry.end(); it++ )
    if ( (*it).m_uds == which )
      return static_cast<time_t>((*it).m_long);

  // If not in the KIO::UDSEntry, or if UDSEntry empty, use stat() [if local URL]
  if ( m_bIsLocalURL )
  {
    struct stat buf;
    stat( QFile::encodeName(m_url.path( -1 )), &buf );
    return (which == KIO::UDS_MODIFICATION_TIME) ? buf.st_mtime :
           (which == KIO::UDS_ACCESS_TIME) ? buf.st_atime :
        static_cast<time_t>(0); // We can't determine creation time for local files
  }
  return static_cast<time_t>(0);
}

QString KFileItem::mimetype() const
{
  KFileItem * that = const_cast<KFileItem *>(this);
  return that->determineMimeType()->name();
}

KMimeType::Ptr KFileItem::determineMimeType()
{
  if ( !m_pMimeType )
  {
    //kdDebug(1203) << "finding mimetype for " << m_url.url() << endl;
    m_pMimeType = KMimeType::findByURL( m_url, m_fileMode, m_bIsLocalURL );
  }

  return m_pMimeType;
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

  if ( !S_ISDIR( m_fileMode ) // Locked dirs have a special icon
       && !isReadable())
     _state |= KIcon::LockOverlay;

  KMimeType::Ptr mime;
  // Use guessed mimetype if the main one is clueless
  if ( m_pMimeType->name() == KMimeType::defaultMimeType()
      && !d->m_guessedMimeType.isEmpty() )
      mime = KMimeType::mimeType( d->m_guessedMimeType );
  else
      mime = m_pMimeType;
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

  // Or if we can't read it [using access()] - not network transparent
  else if ( m_bIsLocalURL && access( QFile::encodeName(m_url.path()), R_OK ) == -1 )
      return false;

  return true;
}


bool KFileItem::acceptsDrops()
{
  // Any directory : yes
  if ( S_ISDIR( mode() ) )
    return true;

  // But only local .desktop files and executables
  if ( !m_bIsLocalURL )
    return false;

  if ( mimetype() == "application/x-desktop")
    return true;

  // Executable, shell script ... ?
  if ( access( QFile::encodeName(m_url.path()), X_OK ) == 0 )
    return true;

  return false;
}

QString KFileItem::getStatusBarInfo()
{
  QString comment = determineMimeType()->comment( m_url, false );
  QString text = m_strText;
  // Extract from the KIO::UDSEntry the additional info we didn't get previously
  QString myLinkDest = linkDest();
  long mySize = size();

  QString text2 = text.copy();

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
      text = QString("%1 (%2)").arg(text2).arg( KIO::convertSize( mySize ) );
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
    *d = *item.d;
    // We had a mimetype previously (probably), so we need to re-determine it
    determineMimeType();
}
