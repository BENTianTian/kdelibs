/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __kmimetype_h__
#define __kmimetype_h__

#include <sys/types.h>
#include <sys/stat.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qdict.h>
#include <qmap.h>

#include <kurl.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>

#include "ksycocatype.h"
#include "kservicetype.h"

/**
 * Represents a mime type, like "text/plain".
 * The starting point you need is often the static methods.
 * See also KServiceType.
 */
class KMimeType : public KServiceType
{
  K_SYCOCATYPE( KST_KMimeType, KServiceType )

public:
  typedef KSharedPtr<KMimeType> Ptr;
  typedef QValueList<Ptr> List;
  /**
   * Constructor.  You may pass in arguments to create a mimetype with
   * specific properties
   */
  KMimeType( const QString & _fullpath, const QString& _type, const QString& _icon, 
	     const QString& _comment, const QStringList& _patterns );

  /**
   * Construct a mimetype and take all informations from a config file.
   */
  KMimeType( const QString & _fullpath );

  /**
   * Construct a mimetype and take all informations from a desktop file.
   */
  KMimeType( KDesktopFile *config );

  /**
   * @internal construct a service from a stream. 
   * The stream must already be positionned at the correct offset
   */
  KMimeType( QDataStream& _str, int offset );

  virtual ~KMimeType();
  
  /**
   * Return the filename of the icon associated with the mimetype.
   * The arguments are unused, but provided so that KMimeType derived classes
   * can use them (e.g. KFolderType uses the URL to return one out of 2 icons)
   * @return the path to the icon associated with this MIME type.
   */
  virtual QString icon( const QString& , bool ) const { return m_strIcon; }
  /**
   * This function differs from the above only in that a KURL may be
   * provided instead of a QString for convenience.
   */
  virtual QString icon( const KURL& , bool ) const { return m_strIcon; }
  /**
   * Use this function only if you don't have a special URL
   * for which you search a pixmap.  This function is useful to find
   * out, which icon is usually chosen for a certain mime type. Since
   * no URL is passed, it is impossible to obey icon hints in desktop
   * entries for example.
   * @param _size Size requested for the pixmap, see KIconLoader
   * @param _path Output parameter to get the full path. Seldom needed.
   */
  virtual QPixmap pixmap( KIconLoader::Size _size, QString * _path = 0L ) const;
  /**
   * Find the pixmap for a given file of this mimetype
   * Convenience method that uses icon(), but also locates and load the pixmap
   * @param _url URL for the file
   * @param _size Size requested for the pixmap, see KIconLoader
   * @param _path Output parameter to get the full path. Seldom needed.
   */
  virtual QPixmap pixmap( const KURL& _url, KIconLoader::Size _size, QString * _path = 0L ) const;

  /**
   * Convenience method to find the pixmap for a URL
   * Call this one when you don't know the mimetype.
   */
  static QPixmap pixmapForURL( const KURL & _url, mode_t _mode = 0, 
                               KIconLoader::Size _size = KIconLoader::Small, QString * _path = 0L );

  /**
   * The arguments are unused, but provided so that KMimeType derived classes
   * can use them.
   * @return the descriptive comment associated with the MIME type, if any.
   */
  virtual QString comment( const QString&, bool ) const { return m_strComment; }
  /**
   * This function differs from the above only in that a KURL may be
   * provided instead of a QString for convenience.
   */
  virtual QString comment( const KURL&, bool ) const { return m_strComment; }
  /**
   * @deprecated
   * Use @ref KServiceType::name instead.
   */
  virtual QString mimeType() const { return m_strName; }
  
  /**
   * @return the list of patterns associated to the MIME Type
   */
  virtual const QStringList& patterns() const { return m_lstPatterns; }
  
  /**
   * Load the mimetype from a stream. 
   * @param _parentLoaded internal (set by the constructor)
   */
  virtual void load( QDataStream&, bool _parentLoaded = false );
  /**
   * Save the mimetype to a stream
   */
  virtual void save( QDataStream& );

  virtual PropertyPtr property( const QString& _name ) const;
  virtual QStringList propertyNames() const;
  
  /**
   * @return a pointer to the mime type '_name' or a pointer to the default
   *         mime type "application/octet-stream". 0L is NEVER returned.
   *
   * VERY IMPORTANT : don't store the result in a KMimeType * !
   * @see KServiceType::serviceType
   */
  static Ptr mimeType( const QString& _name );
  /** 
   * This function looks at mode_t first. If that does not help it
   * looks at the extension.  This is ok for FTP, FILE, TAR and
   * friends, but is not for HTTP ( cgi scripts! ). You should use
   * @ref KRun instead, but this function returns immediately while
   * @ref KRun is async. If no extension matches, then @ref
   * KMimeMagic is used if the URL a local file or
   * "application/octet-stream" is returned otherwise.
   *
   * @param _url is the right most URL with a filesystem protocol. It
   *        is up to you to find out about that if you have a nested
   *        URL.  For example
   *        "http://localhost/mist.gz#gzip:/decompress" would have to
   *        pass the "http://..." URL part, while
   *        "file:/tmp/x.tar#tar:/src/test.gz#gzip:/decompress" would
   *        have to pass the "tar:/..." part of the URL, since gzip is
   *        a filter protocol and not a filesystem protocol.
   *
   * @param _fast_mode If set to true no disk access is allowed to
   *        find out the mimetype. The result may be suboptimal, but
   *        it is * FAST. 
   * @return a pointer to the matching mimetype. 0L is NEVER returned. 
   * VERY IMPORTANT : don't store the result in a KMimeType * !
   */
  static Ptr findByURL( const KURL& _url, mode_t _mode = 0,
			       bool _is_local_file = false, bool _fast_mode = false );

  /** 
   * Get all the mimetypes. Useful for showing the list of
   * available mimetypes.
   * Very memory consuming, don't use unless really necessary.
   */
  static List allMimeTypes();
  
protected:
  /**
   * Signal a missing mime type
   */
  static void errorMissingMimeType( const QString& _type );

  /**
   * This function makes sure that the default mime type exists
   */
  static void buildDefaultType();
  /**
   * This function makes sure that vital mime types are installed.
   */
  static void checkEssentialMimeTypes();
  /**
   * True if check for vital mime types has been done
   */
  static bool s_bChecked;

  QStringList m_lstPatterns;
  
  static Ptr s_pDefaultType;
};

/**
 * @short Mimetype for a folder (inode/directory)
 * Handles locked folders, for instance.
 */
class KFolderType : public KMimeType
{
  K_SYCOCATYPE( KST_KFolderType, KMimeType )

public:
//  KFolderType( const QString & _fullpath, const QString& _type, const QString& _icon, const QString& _comment,
//  	       const QStringList& _patterns );
//  KFolderType( const QString & _fullpath ) : KMimeType( _fullpath ) { }
  KFolderType( KDesktopFile *config) : KMimeType( config ) { }
  KFolderType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }

  virtual QString icon( const QString& _url, bool _is_local ) const;
  virtual QString icon( const KURL& _url, bool _is_local ) const;
  virtual QString comment( const QString& _url, bool _is_local ) const;
  virtual QString comment( const KURL& _url, bool _is_local ) const;
};

/**
 * @short Mimetype for a .desktop file
 * Handles mount/umount icon, and user-defined properties
 */
class KDEDesktopMimeType : public KMimeType
{
  K_SYCOCATYPE( KST_KDEDesktopMimeType, KMimeType )

public:
  enum ServiceType { ST_MOUNT, ST_UNMOUNT, /* ST_PROPERTIES, */ ST_USER_DEFINED };
		     
  struct Service
  {
    QString m_strName;
    QString m_strIcon;
    QString m_strExec;
    ServiceType m_type;
  };
  
//  KDEDesktopMimeType( const QString & _fullpath, const QString& _type, const QString& _icon, 
//                      const QString& _comment, const QStringList& _patterns );
//  KDEDesktopMimeType( const QString & _fullpath ) : KMimeType( _fullpath ) { }
  KDEDesktopMimeType( KDesktopFile *config) : KMimeType( config ) { }
  KDEDesktopMimeType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }

  virtual QString icon( const QString& _url, bool _is_local ) const;
  virtual QString icon( const KURL& _url, bool _is_local ) const;
  virtual QPixmap pixmap( const KURL& _url, KIconLoader::Size _size, QString * _path = 0L ) const;
  virtual QString comment( const QString& _url, bool _is_local ) const;
  virtual QString comment( const KURL& _url, bool _is_local ) const;

  /**
   * @return a list of services for the given .desktop file that are handled
   * by kio itself. Namely mount/unmount for FSDevice files.
   */
  static QValueList<Service> builtinServices( const KURL& _url );
  /**
   * @return a list of services defined by the user as possible actions
   * on the given .desktop file.
   */
  static QValueList<Service> userDefinedServices( const KURL& _url );

  /** 
   * @param _url is the URL of the desktop entry. The URL must be
   *        local, otherwise nothing will happen.  
   */
  static void executeService( const QString& _url, KDEDesktopMimeType::Service& _service );

  /** 
   * Invokes the default action for the desktop entry. If the desktop
   * entry is not local, then only false is returned. Otherwise we
   * would create a security problem. Only types Link and Mimetype
   * could be followed.
   *
   * @return true on success and false on failure.
   *
   * @see KRun::runURL */

  static bool run( const QString& _url, bool _is_local );

protected:
  static bool runFSDevice( const QString& _url, KSimpleConfig &cfg );
  static bool runApplication( const QString& _url, const QString & _serviceFile );
  static bool runLink( const QString& _url, KSimpleConfig &cfg );
  static bool runMimeType( const QString& _url, KSimpleConfig &cfg );
};

/**
 * @short MimeType for any executable, like /bin/ls
 */
class KExecMimeType : public KMimeType
{
  K_SYCOCATYPE( KST_KExecMimeType, KMimeType )

public:
//  KExecMimeType( const QString & _fullpath, const QString& _type, const QString& _icon, 
//                 const QString& _comment, const QStringList& _patterns );
//  KExecMimeType( const QString & _fullpath ) : KMimeType( _fullpath ) { }
  KExecMimeType( KDesktopFile *config) : KMimeType( config ) { }
  KExecMimeType( QDataStream& _str, int offset ) : KMimeType( _str, offset ) { }
};

#endif
