// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

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
#ifndef __kio_global_h__
#define __kio_global_h__

#include <kio/kio_export.h>

#include <QtCore/QCharRef>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QList>

#include <kicontheme.h> // K3Icon (for pixmapForUrl)
#include <QtGui/QPixmap> // for pixmapForUrl

#include <sys/stat.h> // S_ISDIR

#include <kjob.h>

#ifdef Q_OS_WIN
template class KDE_EXPORT QMap<QString, QString>;
#endif

class KUrl;
class KUiServerJobTracker;

class QTime;

/**
 * @short A namespace for KIO globals
 *
 */
namespace KIO
{
  /// 64-bit file offset
  typedef qlonglong fileoffset_t;
  /// 64-bit file size
  typedef qulonglong filesize_t;

  /**
   * Converts @p size from bytes to the string representation.
   *
   * @param  size  size in bytes
   * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
   */
  KIO_EXPORT QString convertSize( KIO::filesize_t size );

  /**
   * Converts a size to a string representation
   * Not unlike QString::number(...)
   *
   * @param size size in bytes
   * @return  converted size as a string - e.g. 123456789
   */
  KIO_EXPORT QString number( KIO::filesize_t size );

  /**
   * Converts size from kibi-bytes (2^10) to the string representation.
   *
   * @param  kibSize  size in kibi-bytes (2^10)
   * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
   */
   KIO_EXPORT QString convertSizeFromKiB( KIO::filesize_t kibSize );

  /**
   * Calculates remaining time in seconds from total size, processed size and speed.
   *
   * @param  totalSize      total size in bytes
   * @param  processedSize  processed size in bytes
   * @param  speed          speed in bytes per second
   * @return calculated remaining time in seconds
   */
  KIO_EXPORT unsigned int calculateRemainingSeconds( KIO::filesize_t totalSize,
                                                     KIO::filesize_t processedSize, KIO::filesize_t speed );

  /**
   * Convert @p seconds to a string representing number of days, hours, minutes and seconds
   *
   * @param  seconds number of seconds to convert
   * @return string representation in a locale depending format
   */
  KIO_EXPORT QString convertSeconds( unsigned int seconds );

  /**
   * Calculates remaining time from total size, processed size and speed.
   * Warning: As QTime is limited to 23:59:59, use calculateRemainingSeconds() instead
   *
   * @param  totalSize      total size in bytes
   * @param  processedSize  processed size in bytes
   * @param  speed          speed in bytes per second
   * @return calculated remaining time
   */
  KIO_EXPORT_DEPRECATED QTime calculateRemaining( KIO::filesize_t totalSize, KIO::filesize_t processedSize, KIO::filesize_t speed );

  /**
   * Helper for showing information about a set of files and directories
   * @param items the number of items (= @p files + @p dirs + number of symlinks :)
   * @param files the number of files
   * @param dirs the number of dirs
   * @param size the sum of the size of the @p files
   * @param showSize whether to show the size in the result
   * @return the summary string
   */
  KIO_EXPORT QString itemsSummaryString(uint items, uint files, uint dirs, KIO::filesize_t size, bool showSize);

  /**
   * Encodes (from the text displayed to the real filename)
   * This translates % into %% and / into %2f
   * Used by KIO::link, for instance.
   * @param str the file name to encode
   * @return the encoded file name
   */
  KIO_EXPORT QString encodeFileName( const QString & str );
  /**
   * Decodes (from the filename to the text displayed)
   * This translates %2[fF] into / and %% into %
   * @param str the file name to decode
   * @return the decoded file name
   */
  KIO_EXPORT QString decodeFileName( const QString & str );

  /**
   * Commands that can be invoked by a job.
   */
  enum Command {
    CMD_HOST = '0', // 48
    CMD_CONNECT = '1', // 49
    CMD_DISCONNECT = '2', // 50
    CMD_SLAVE_STATUS = '3', // 51
    CMD_SLAVE_CONNECT = '4', // 52
    CMD_SLAVE_HOLD = '5', // 53
    CMD_NONE = 'A', // 65
    CMD_TESTDIR = 'B', // 66
    CMD_GET = 'C', // 67
    CMD_PUT = 'D', // 68
    CMD_STAT = 'E', // 69
    CMD_MIMETYPE = 'F', // 70
    CMD_LISTDIR = 'G', // 71
    CMD_MKDIR = 'H', // 72
    CMD_RENAME = 'I', // 73
    CMD_COPY = 'J', // 74
    CMD_DEL = 'K', // 75
    CMD_CHMOD = 'L', // 76
    CMD_SPECIAL = 'M', // 77
    // 'N', // 78  - currently unused
    CMD_REPARSECONFIGURATION = 'O', // 79
    CMD_META_DATA = 'P', // 80
    CMD_SYMLINK = 'Q', // 81
    CMD_SUBURL = 'R', // 82  Inform the slave about the url it is streaming on.
    CMD_MESSAGEBOXANSWER = 'S', // 83
    CMD_RESUMEANSWER = 'T', // 84
    CMD_CONFIG = 'U', // 85
    CMD_MULTI_GET = 'V', // 86
    CMD_SETLINKDEST = 'W', // 87
    CMD_OPEN = 'X' // 88
    // Add new ones here once a release is done, to avoid breaking binary compatibility.
    // Note that protocol-specific commands shouldn't be added here, but should use special.
  };

  /**
   * Commands that can be invoked on a slave-file.
   */
  enum FileCommand {
    CMD_READ  = 90,
    CMD_WRITE = 91,
    CMD_SEEK  = 92,
    CMD_CLOSE = 93
  };

  /**
   * Error codes that can be emitted by KIO.
   */
  enum Error {
    ERR_CANNOT_OPEN_FOR_READING = KJob::UserDefinedError + 1,
    ERR_CANNOT_OPEN_FOR_WRITING = KJob::UserDefinedError + 2,
    ERR_CANNOT_LAUNCH_PROCESS = KJob::UserDefinedError + 3,
    ERR_INTERNAL = KJob::UserDefinedError + 4,
    ERR_MALFORMED_URL = KJob::UserDefinedError + 5,
    ERR_UNSUPPORTED_PROTOCOL = KJob::UserDefinedError + 6,
    ERR_NO_SOURCE_PROTOCOL = KJob::UserDefinedError + 7,
    ERR_UNSUPPORTED_ACTION = KJob::UserDefinedError + 8,
    ERR_IS_DIRECTORY = KJob::UserDefinedError + 9, // ... where a file was expected
    ERR_IS_FILE = KJob::UserDefinedError + 10, // ... where a directory was expected (e.g. listing)
    ERR_DOES_NOT_EXIST = KJob::UserDefinedError + 11,
    ERR_FILE_ALREADY_EXIST = KJob::UserDefinedError + 12,
    ERR_DIR_ALREADY_EXIST = KJob::UserDefinedError + 13,
    ERR_UNKNOWN_HOST = KJob::UserDefinedError + 14,
    ERR_ACCESS_DENIED = KJob::UserDefinedError + 15,
    ERR_WRITE_ACCESS_DENIED = KJob::UserDefinedError + 16,
    ERR_CANNOT_ENTER_DIRECTORY = KJob::UserDefinedError + 17,
    ERR_PROTOCOL_IS_NOT_A_FILESYSTEM = KJob::UserDefinedError + 18,
    ERR_CYCLIC_LINK = KJob::UserDefinedError + 19,
    ERR_USER_CANCELED = KJob::KilledJobError,
    ERR_CYCLIC_COPY = KJob::UserDefinedError + 21,
    ERR_COULD_NOT_CREATE_SOCKET = KJob::UserDefinedError + 22, // KDE4: s/COULD_NOT/CANNOT/ or the other way round
    ERR_COULD_NOT_CONNECT = KJob::UserDefinedError + 23,
    ERR_CONNECTION_BROKEN = KJob::UserDefinedError + 24,
    ERR_NOT_FILTER_PROTOCOL = KJob::UserDefinedError + 25,
    ERR_COULD_NOT_MOUNT = KJob::UserDefinedError + 26,
    ERR_COULD_NOT_UNMOUNT = KJob::UserDefinedError + 27,
    ERR_COULD_NOT_READ = KJob::UserDefinedError + 28,
    ERR_COULD_NOT_WRITE = KJob::UserDefinedError + 29,
    ERR_COULD_NOT_BIND = KJob::UserDefinedError + 30,
    ERR_COULD_NOT_LISTEN = KJob::UserDefinedError + 31,
    ERR_COULD_NOT_ACCEPT = KJob::UserDefinedError + 32,
    ERR_COULD_NOT_LOGIN = KJob::UserDefinedError + 33,
    ERR_COULD_NOT_STAT = KJob::UserDefinedError + 34,
    ERR_COULD_NOT_CLOSEDIR = KJob::UserDefinedError + 35,
    ERR_COULD_NOT_MKDIR = KJob::UserDefinedError + 37,
    ERR_COULD_NOT_RMDIR = KJob::UserDefinedError + 38,
    ERR_CANNOT_RESUME = KJob::UserDefinedError + 39,
    ERR_CANNOT_RENAME = KJob::UserDefinedError + 40,
    ERR_CANNOT_CHMOD = KJob::UserDefinedError + 41,
    ERR_CANNOT_DELETE = KJob::UserDefinedError + 42,
    // The text argument is the protocol that the dead slave supported.
    // This means for example: file, ftp, http, ...
    ERR_SLAVE_DIED = KJob::UserDefinedError + 43,
    ERR_OUT_OF_MEMORY = KJob::UserDefinedError + 44,
    ERR_UNKNOWN_PROXY_HOST = KJob::UserDefinedError + 45,
    ERR_COULD_NOT_AUTHENTICATE = KJob::UserDefinedError + 46,
    ERR_ABORTED = KJob::UserDefinedError + 47, // Action got aborted from application side
    ERR_INTERNAL_SERVER = KJob::UserDefinedError + 48,
    ERR_SERVER_TIMEOUT = KJob::UserDefinedError + 49,
    ERR_SERVICE_NOT_AVAILABLE = KJob::UserDefinedError + 50,
    ERR_UNKNOWN = KJob::UserDefinedError + 51,
    // (was a warning) ERR_CHECKSUM_MISMATCH = 52,
    ERR_UNKNOWN_INTERRUPT = KJob::UserDefinedError + 53,
    ERR_CANNOT_DELETE_ORIGINAL = KJob::UserDefinedError + 54,
    ERR_CANNOT_DELETE_PARTIAL = KJob::UserDefinedError + 55,
    ERR_CANNOT_RENAME_ORIGINAL = KJob::UserDefinedError + 56,
    ERR_CANNOT_RENAME_PARTIAL = KJob::UserDefinedError + 57,
    ERR_NEED_PASSWD = KJob::UserDefinedError + 58,
    ERR_CANNOT_SYMLINK = KJob::UserDefinedError + 59,
    ERR_NO_CONTENT = KJob::UserDefinedError + 60, // Action succeeded but no content will follow.
    ERR_DISK_FULL = KJob::UserDefinedError + 61,
    ERR_IDENTICAL_FILES = KJob::UserDefinedError + 62, // src==dest when moving/copying
    ERR_SLAVE_DEFINED = KJob::UserDefinedError + 63, // for slave specified errors that can be
                                                     // rich text.  Email links will be handled
                                                     // by the standard email app and all hrefs
                                                     // will be handled by the standard browser.
                                                     // <a href="exec:/khelpcenter ?" will be
                                                     // forked.
    ERR_UPGRADE_REQUIRED = KJob::UserDefinedError + 64, // A transport upgrade is required to access this
                                                        // object.  For instance, TLS is demanded by
                                                        // the server in order to continue.
    ERR_POST_DENIED = KJob::UserDefinedError + 65, // Issued when trying to POST data to a certain Ports
                                                  // see job.cpp
    ERR_COULD_NOT_SEEK = KJob::UserDefinedError + 66
  };

  /**
   * Returns a translated error message for @p errorCode using the
   * additional error information provided by @p errorText.
   * @param errorCode the error code
   * @param errorText the additional error text
   * @return the created error string
   */
  KIO_EXPORT QString buildErrorString(int errorCode, const QString &errorText);

  /**
   * Returns a translated html error message for @p errorCode using the
   * additional error information provided by @p errorText , @p reqUrl
   * (the request URL), and the ioslave @p method .
   * @param errorCode the error code
   * @param errorText the additional error text
   * @param reqUrl the request URL
   * @param method the ioslave method
   * @return the created error string
   */
  KIO_EXPORT QString buildHTMLErrorString(int errorCode, const QString &errorText,
                                const KUrl *reqUrl = 0L, int method = -1 );

  /**
   * Returns translated error details for @p errorCode using the
   * additional error information provided by @p errorText , @p reqUrl
   * (the request URL), and the ioslave @p method .
   *
   * @param errorCode the error code
   * @param errorText the additional error text
   * @param reqUrl the request URL
   * @param method the ioslave method
   * @return the following data:
   * @li QString errorName - the name of the error
   * @li QString techName - if not null, the more technical name of the error
   * @li QString description - a description of the error
   * @li QStringList causes - a list of possible causes of the error
   * @li QStringList solutions - a liso of solutions for the error
   */
  KIO_EXPORT QByteArray rawErrorDetail(int errorCode, const QString &errorText,
                                const KUrl *reqUrl = 0L, int method = -1 );

  /**
   * Returns an appropriate error message if the given command @p cmd
   * is an unsupported action (ERR_UNSUPPORTED_ACTION).
   * @param protocol name of the protocol
   * @param cmd given command
   * @see enum Command
   */
  KIO_EXPORT QString unsupportedActionErrorString(const QString &protocol, int cmd);

  /**
   * Specifies how to use the cache.
   * @see parseCacheControl()
   * @see getCacheControlString()
   */
  enum CacheControl
  {
      CC_CacheOnly, ///< Fail request if not in cache
      CC_Cache,     ///< Use cached entry if available
      CC_Verify,    ///< Validate cached entry with remote site if expired
      CC_Refresh,   ///< Always validate cached entry with remote site
      CC_Reload     ///< Always fetch from remote site.
  };

  /**
   * Parses the string representation of the cache control option.
   *
   * @param cacheControl the string representation
   * @return the cache control value
   * @see getCacheControlString()
   */
  KIO_EXPORT KIO::CacheControl parseCacheControl(const QString &cacheControl);

  /**
   * Returns a string representation of the given cache control method.
   *
   * @param cacheControl the cache control method
   * @return the string representation
   * @see parseCacheControl()
   */
  KIO_EXPORT QString getCacheControlString(KIO::CacheControl cacheControl);

  /**
   * Returns the mount point where @p device is mounted
   * right now. This means, it has to be mounted, not just
   * defined in fstab.
   *
   * @deprecated use KMountPoint::currentMountPoints() and findByDevice().
   *
   */
  KIO_EXPORT_DEPRECATED QString findDeviceMountPoint( const QString& device );

  /**
   * Returns the mount point on which resides @p filename.
   * For instance if /home is a separate partition, findPathMountPoint("/home/user/blah")
   * will return /home
   * @param filename the file name to check
   * @return the mount point of the given @p filename
   *
   * @deprecated use KMountPoint::currentMountPoints() and findByPath().
   */
  KIO_EXPORT_DEPRECATED QString findPathMountPoint( const QString & filename );

  /**
   * Checks if the path belongs to a filesystem that is probably
   * slow. It checks for NFS or for paths belonging to automounted
   * paths not yet mounted
   * @param filename the file name to check
   * @return true if the filesystem is probably slow
   */
  KIO_EXPORT bool probably_slow_mounted(const QString& filename);

  /**
   * Checks if the path belongs to a filesystem that is manually
   * mounted.
   * @param filename the file name to check
   * @return true if the filesystem is manually mounted
   */
  KIO_EXPORT bool manually_mounted(const QString& filename);

  enum FileSystemFlag { SupportsChmod, SupportsChown, SupportsUTime,
                        SupportsSymlinks, CaseInsensitive };
  /**
   * Checks the capabilities of the filesystem to which a given file belongs.
   * given feature (e.g. chmod).
   * @param filename the file name to check
   * @param flag the flag to check
   * @return true if the filesystem has that flag, false if not (or some error occurred)
   *
   * The availables flags are:
   * @li SupportsChmod: returns true if the filesystem supports chmod
   * (e.g. msdos filesystems return false)
   * @li SupportsChown: returns true if the filesystem supports chown
   * (e.g. msdos filesystems return false)
   * @li SupportsUtime: returns true if the filesystems supports utime
   * (e.g. msdos filesystems return false)
   * @li SupportsSymlinks: returns true if the filesystems supports symlinks
   * (e.g. msdos filesystems return false)
   * @li CaseInsensitive: returns true if the filesystem treats
   * "foo" and "FOO" as being the same file (true for msdos systems)
   *
   */
  KIO_EXPORT bool testFileSystemFlag(const QString& filename, FileSystemFlag flag);

  /**
   * Convenience method to find the pixmap for a URL.
   *
   * Call this one when you don't know the mimetype.
   *
   * @param _url URL for the file.
   * @param _mode the mode of the file. The mode may modify the icon
   *              with overlays that show special properties of the
   *              icon. Use 0 for default
   * @param _group The icon group where the icon is going to be used.
   * @param _force_size Override globally configured icon size.
   *        Use 0 for the default size
   * @param _state The icon state, one of: K3Icon::DefaultState,
   * K3Icon::ActiveState or K3Icon::DisabledState.
   * @param _path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the URL, can be a default icon if not found
   */
  KIO_EXPORT QPixmap pixmapForUrl( const KUrl & _url, mode_t _mode = 0, K3Icon::Group _group = K3Icon::Desktop,
                                   int _force_size = 0, int _state = 0, QString * _path = 0 );

  KIO_EXPORT KUiServerJobTracker *getJobTracker();

  /**
   * Constants used to specify the type of a UDSField.
   */
  enum UDSFieldTypes {

    // First let's define the item types: bit field

    /// Indicates that the field is a QString
    UDS_STRING = 0x01000000,
    /// Indicates that the field is a number (long long)
    UDS_NUMBER   = 0x02000000,
    /// Indicates that the field represents a time, which is modelled by a long long
    UDS_TIME   = 0x04000000 | UDS_NUMBER,

    // The rest isn't a bit field

    /// Size of the file
    UDS_SIZE = 1 | UDS_NUMBER,
    /// @internal
    UDS_SIZE_LARGE = 2 | UDS_NUMBER,
    /// User ID of the file owner
    UDS_USER = 3 | UDS_STRING,
    /// Name of the icon, that should be used for displaying.
    /// It overrides all other detection mechanisms
    UDS_ICON_NAME = 4 | UDS_STRING,
    /// Group ID of the file owner
    UDS_GROUP =	5 | UDS_STRING,
    /// Filename - as displayed in directory listings etc.
    /// "." has the usual special meaning of "current directory"
    UDS_NAME = 6 | UDS_STRING,
    /// A local file path if the ioslave display files sitting
    /// on the local filesystem (but in another hierarchy, e.g. media:/)
    UDS_LOCAL_PATH = 7 | UDS_STRING,
    /// Treat the file as a hidden file (if set to 1) or as a normal file (if set to 0).
    /// This field overrides the default behavior (the check for a leading dot in the filename).
    UDS_HIDDEN = 8 | UDS_NUMBER,
    /// Access permissions (part of the mode returned by stat)
    UDS_ACCESS = 9 | UDS_NUMBER,
    /// The last time the file was modified
    UDS_MODIFICATION_TIME = 10 | UDS_TIME,
    /// The last time the file was opened
    UDS_ACCESS_TIME = 11 | UDS_TIME,
    /// The time the file was created
    UDS_CREATION_TIME = 12 | UDS_TIME,
    /// File type, part of the mode returned by stat
    /// (for a link, this returns the file type of the pointed item)
    /// check UDS_LINK_DEST to know if this is a link
    UDS_FILE_TYPE = 13 | UDS_NUMBER,
    /// Name of the file where the link points to
    /// Allows to check for a symlink (don't use S_ISLNK !)
    UDS_LINK_DEST = 14 | UDS_STRING,
    /// An alternative URL (If different from the caption)
    UDS_URL = 15 | UDS_STRING,
    /// A mime type; the slave should set it if it's known.
    UDS_MIME_TYPE = 16 | UDS_STRING,
    /// A mime type to be used for displaying only.
    /// But when 'running' the file, the mimetype is re-determined
    /// This is for special cases like symlinks in FTP; you probably don't want to use this one.
    UDS_GUESSED_MIME_TYPE = 17 | UDS_STRING,
    /// XML properties, e.g. for WebDAV
    UDS_XML_PROPERTIES = 18 | UDS_STRING,

    /// Indicates that the entry has extended ACL entries
    UDS_EXTENDED_ACL = 19 | UDS_NUMBER,
    /// The access control list serialized into a single string.
    UDS_ACL_STRING = 20 | UDS_STRING,
    /// The default access control list serialized into a single string.
    /// Only available for directories.
    UDS_DEFAULT_ACL_STRING = 21 | UDS_STRING,

    /// Extra data (used only if you specified Columns/ColumnsTypes)
    /// KDE 4.0 change: you cannot repeat this entry anymore, use UDS_EXTRA + i
    /// until UDS_EXTRA_END.
    UDS_EXTRA = 100 | UDS_STRING,
    /// Extra data (used only if you specified Columns/ColumnsTypes)
    /// KDE 4.0 change: you cannot repeat this entry anymore, use UDS_EXTRA + i
    /// until UDS_EXTRA_END.
    UDS_EXTRA_END = 140 | UDS_STRING
  };

/**
 * A field inside a UDSEntry, to store information about a file or URL.
 * Each field is for instance the name, the size, the modification time of the file, etc.
 * You never need to use this class directly, see UDSEntry for the public API.
 */
class KIO_EXPORT UDSField // KDE4: naming: I would get rid of "UDS" everywhere, but what about the UDS_X constants?
{
public:
  /// @internal needed by QHash
  UDSField() {}
  /**
   * Create a field holding a string.
   * This constructor is called implicitly when doing
   * \code
   * entry.insert( KIO::UDS_XXX, myString )
   * \endcode
   */
  UDSField( const QString& s ) : m_str( s ) {}
  /**
   * Create a field holding a long long.
   * This constructor is called implicitly when doing
   * \code
   * entry.insert( KIO::UDS_XXX, myNumber )
   * \endcode
   */
  UDSField( long long l ) : m_long( l ) {}

  /**
   * @return the string value held by this UDSField
   */
  QString toString() const { return m_str; }

  /**
   * @return the number value held by this UDSField
   */
  long long toNumber() const { return m_long; }

private:
  /// Whether 'm_str' or 'm_long' is used depends on the field type, stored as the key in UDSEntry.
  QString m_str;
  /// Whether 'm_str' or 'm_long' is used depends on the field type, stored as the key in UDSEntry.
  long long m_long;
};

/**
 * Universal Directory Service
 *
 * UDS entry is the data structure representing all the fields about a given URL
 * (file or directory).
 *
 * The KIO::listDir() and KIO:stat() operations use this data structure.
 *
 * KIO defines a number of fields, see the UDS_XXX enums (see UDSFieldTypes).
 *
 * UDSEntry is a hashtable, so you can retrieve fields directly.
 * For instance, to retrieve the name of the entry, use:
 * \code
 * QString displayName = entry.stringValue( KIO::UDS_NAME );
 * \endcode
 *
 * To know the modification time of the file/url:
 * \code
 *  time_t mtime = entry.numberValue( KIO::UDS_MODIFICATION_TIME, -1 );
 *  if ( mtime != -1 )
 *      ...
 * \endcode
 */
class UDSEntry : public QHash<uint, UDSField>
{
public:
  QString stringValue( uint field ) const {
    return value( field ).toString();
  }
  long long numberValue( uint field, long long defaultValue = 0 ) const {
    const_iterator it = find( field );
    return it != constEnd() ? it.value().toNumber() : defaultValue;
  }

  // Convenience methods.
  // Let's not add one method per field, only methods that have some more logic
  // than just calling stringValue(field) or numberValue(field).

  /// @return true if this entry is a directory (or a link to a directory)
  bool isDir() const { return S_ISDIR( numberValue( KIO::UDS_FILE_TYPE ) ); }
  /// @return true if this entry is a link
  bool isLink() const { return !stringValue( KIO::UDS_LINK_DEST ).isEmpty(); }
};

/**
 * A directory listing is a list of UDSEntry instances.
 *
 * To list the name and size of all the files in a directory listing you would do:
 * \code
 *   KIO::UDSEntryList::ConstIterator it = entries.begin();
 *   const KIO::UDSEntryList::ConstIterator end = entries.end();
 *   for (; it != end; ++it) {
 *     const KIO::UDSEntry& entry = *it;
 *     QString name = entry.stringValue( KIO::UDS_NAME );
 *     bool isDir = entry.isDir();
 *     KIO::filesize_t size = entry.numberValue( KIO::UDS_SIZE, -1 );
 *     ...
 *   }
 * \endcode
 */
typedef QList<UDSEntry> UDSEntryList;


/**
 * MetaData is a simple map of key/value strings.
 */
class KIO_EXPORT MetaData : public QMap<QString, QString>
{
public:
  /**
   * Creates an empty meta data map.
   */
   MetaData() : QMap<QString, QString>() { }
  /**
   * Copy constructor.
   */
   MetaData(const QMap<QString, QString>&metaData) :
     QMap<QString, QString>(metaData) { }

   /**
    * Adds the given meta data map to this map.
    * @param metaData the map to add
    * @return this map
    */
   MetaData & operator+= ( const QMap<QString,QString> &metaData )
   {
      QMap<QString,QString>::ConstIterator it;
      for( it = metaData.begin();
           it !=  metaData.end();
           ++it)
      {
         insert(it.key(), it.value());
      }
      return *this;
   }
};

}
#endif
