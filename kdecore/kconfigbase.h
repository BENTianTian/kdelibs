/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

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

#ifndef KCONFIGBASE_H
#define KCONFIGBASE_H

#include <qcolor.h>
#include <qvariant.h>
#include <kdelibs_export.h>
#include <kdebug.h>

// This include fixes linker errors under msvc:
// In qdbusmessage.h QList<QVariant> is instantiated because
// it is a base class of an exported class. This must be 
// known here to avoid an additional instantiation. 
#ifdef Q_CC_MSVC
#include <QtDBus/qdbusmessage.h>
#endif

template <typename KT, typename KV> class QMap;
class QString;
#ifdef KDE3_SUPPORT
class Q3StrList;
#endif

class KConfigBackEnd;
class KConfigGroup;
struct KEntry;
struct KEntryKey;
typedef QMap<KEntryKey, KEntry> KEntryMap;

/**
 * @short KDE Configuration Management abstract base class
 *
 * This class forms the base for all %KDE configuration. It is an
 * abstract base class, meaning that you cannot directly instantiate
 * objects of this class. Either use KConfig (for usual %KDE
 * configuration) or KSimpleConfig (for special needs as in ksamba), or
 * even KSharedConfig (stores values in shared memory).
 *
 * All configuration entries are key, value pairs.  Each entry also
 * belongs to a specific group of related entries.  All configuration
 * entries that do not explicitly specify which group they are in are
 * in a special group called the default group.
 *
 * If there is a $ character in an entry, KConfigBase tries to expand
 * environment variable and uses its value instead of its name. You
 * can avoid this feature by having two consecutive $ characters in
 * your config file which get expanded to one.
 *
 * \note the '=' char is not allowed in keys and the ']' char is not allowed in
 * a group name.
 *
 * @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
 * @see KGlobal#config()
 * @see KConfig
 * @see KSimpleConfig
 * @see KSharedConfig
 */
class KDECORE_EXPORT KConfigBase
{
  friend class KConfigBackEnd;
  friend class KConfigINIBackEnd;
  friend class KConfigGroup;

public:
  /**
   * Flags to control write entry
   */
  enum WriteConfigFlag
  {
    Persistent = 0x01,
    /**<
     * Save this entry when saving the config object.
     */
    Global = 0x02,
    /**<
     * Save the entry to the global %KDE config file instead of the
     * application specific config file.
     */
    Localized = 0x04,
    NLS = Localized,
    /**<
     * Add the locale tag to the key when writing it.
     */
    Normal=Persistent
    /**<
     * Save the entry to the application specific config file without
     * a locale tag. This is the default.
     */

  };
  Q_DECLARE_FLAGS(WriteConfigFlags, WriteConfigFlag)

  /**
   * Construct a KConfigBase object.
   */
  KConfigBase();

  /**
   * Destructs the KConfigBase object.
   */
  virtual ~KConfigBase();

  /**
   * Specifies the group in which keys will be read and written.
   *
   *  Subsequent
   * calls to readEntry() and writeEntry() will be applied only in the
   * activated group.
   *
   * Switch back to the default group by passing a null string.
   * @param group The name of the new group.
   */
  void setGroup( const QString& group );

  /**
   * Sets the group to the "Desktop Entry" group used for
   * desktop configuration files for applications, mime types, etc.
   */
  void setDesktopGroup();

  /**
   * Returns the name of the group in which we are
   *  searching for keys and from which we are retrieving entries.
   *
   * @return The current group.
   */
  QString group() const;

  /**
   * Returns true if the specified group is known about.
   *
   * @param group The group to search for.
   * @return true if the group exists.
   */
  bool hasGroup(const QString &group) const;

  /**
   * Returns a list of groups that are known about.
   *
   * @return The list of groups.
   **/
  virtual QStringList groupList() const = 0;

  /**
   * Returns a the current locale.
   *
   * @return A string representing the current locale.
   */
  QString locale() const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * If you want to read a path, please use readPathEntry().
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString() if aDefault is null.
   */
   QString readEntry(const char *pKey,
                     const QString& aDefault ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString() if aDefault is null.
   */
   QString readEntry(const char *pKey, const char *aDefault = 0 ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the type of the given default value.
   *
   * @note Only the following QVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, LongLong, ULongLong, DateTime and Date.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found or
   * if the read value cannot be converted to the QVariant::Type.
   * @return The value for the key or the default value if the key was not
   *         found.
   */
  QVariant readEntry( const char *pKey, const QVariant &aDefault) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key, or @p aDefault.
   */
  template <typename T>
  inline T readEntry( const char* pKey, const T& aDefault) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * @copydoc readEntry(const char*, const T&) const
   */
  template <typename T>
  T readEntry( const QString& pKey, const T& aDefault) const
    { return readEntry(pKey.toUtf8().constData(), aDefault); }

  /**
   * Reads the color of an entry specified by @p pKey in the current group.
   *
   */
  QColor readEntry(const char* pKey, Qt::GlobalColor aDefault) const
    { return readEntry(pKey, QColor(aDefault)); }

  /**
   * Reads the color of an entry specified by @p pKey in the current group.
   *
   */
  QColor readEntry(const QString& pKey, Qt::GlobalColor aDefault) const
    { return readEntry(pKey, QColor(aDefault)); }

  // these two are here temporarily for porting, remove before KDE4
  KDE_DEPRECATED QVariant readPropertyEntry( const QString& pKey, const QVariant& aDefault) const
    { return readEntry(pKey, aDefault); }
  KDE_DEPRECATED QVariant readPropertyEntry( const char *pKey, const QVariant& aDefault) const
    { return readEntry(pKey, aDefault); }

#ifdef KDE3_SUPPORT
  /**
   * Reads a list of strings.
   *
   * @deprecated Use readListEntry(const QString&, char) const instead.
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  KDE_DEPRECATED int readListEntry( const QString& pKey, Q3StrList &list, char sep = ',' ) const
    { return readListEntry( pKey.toUtf8().constData(), list, sep); }

  /**
   * Reads a list of strings.
   *
   * @deprecated Use readListEntry(const char*, char) const instead.
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  KDE_DEPRECATED int readListEntry( const char *pKey, Q3StrList &list, char sep = ',' ) const;
#endif

  /**
   * Reads a list of strings.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   * @deprecated use readEntry( const QString&, const QStringList&, char) const instead.
   */
  KDE_DEPRECATED QStringList readListEntry( const QString& pKey, char sep = ',' ) const
    { return readEntry(pKey, QStringList(), sep); }

  /**
   * Reads a list of strings.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   * @deprecated use readEntry(const char*, const QStringList&, char) const instead.
   */
  KDE_DEPRECATED QStringList readListEntry( const char *pKey, char sep = ',' ) const
    { return readEntry(pKey, QStringList(), sep); }

  /**
   * Reads a list of strings, but returns a default if the key
   * did not exist.
   * @param pKey The key to search for.
   * @param aDefault The default value to use if the key does not exist.
   * @param sep The list separator (default is ",").
   * @return The list. Contains @p aDefault if the Key does not exist.
   * @deprecated use readEntry(const char*, const QStringList&, char) const instead.
   */
  KDE_DEPRECATED QStringList readListEntry( const char* pKey,
                                            const QStringList& aDefault,
                                            char sep = ',' ) const
    { return readEntry(pKey, aDefault, sep); }

  /**
   * Reads a list from the config object.
   *
   * @note This function only works for those types that QVariant can convert
   * from QString.
   * @param pKey The key to search for.
   * @param aDefault The default value to use if the key does not exist.
   * @return The list.
   */
  template <typename T>
  inline QList<T> readEntry( const char* pKey, const QList<T>& aDefault ) const;

  /**
   * Reads a list from the config object.
   *
   * @copydoc readEntry(const char*, const QList<T>&) const
   *
   * @warning This function doesn't convert the items returned
   *          to any type. It's actually a list of QVariant::String's. If you
   *          want the items converted to a specific type use
   *          readEntry(const char*, const QList<T>&) const
   */
  QVariantList readEntry( const char* pKey, const QVariantList& aDefault ) const;

  /**
   * Reads a list of strings from the config object.
   * @param pKey The key to search for.
   * @param aDefault The default value to use if the key does not exist.
   * @param sep The list separator.
   * @return The list. Contains @p aDefault if @p pKey does not exist.
   */
  QStringList readEntry(const char* pKey, const QStringList& aDefault,
                        char sep=',') const;

  /**
   * Reads a list of strings, but returns a default if the key
   * did not exist.
   *
   * @copydoc readEntry(const char*, const QStringList&, char) const
   */
  QStringList readEntry(const QString& pKey, const QStringList& aDefault,
                        char sep=',') const
    { return readEntry(pKey.toUtf8().constData(), aDefault, sep); }

  /**
   * Reads a list of Integers.
   *
   * @param pKey The key to search for.
   * @return The list. Empty if the entry does not exist.
   * @deprecated use readEntry(const QString&, const QList<T>&) const instead.
   */
  KDE_DEPRECATED QList<int> readIntListEntry( const QString& pKey ) const
    { return readEntry( pKey, QList<int>() ); }

  /**
   * Reads a list of Integers.
   *
   * @param pKey The key to search for.
   * @return The list. Empty if the entry does not exist.
   * @deprecated use readEntry(const char*, const QList<T>&) const instead.
   */
  KDE_DEPRECATED QList<int> readIntListEntry( const char *pKey ) const
    { return readEntry( pKey, QList<int>() ); }

  /**
   * Reads a path.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a path. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString() if aDefault is null.
   */
  QString readPathEntry( const QString& pKey, const QString & aDefault = QString() ) const;

  /**
   * Reads a path.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a path. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString() if aDefault is null.
   */
  QString readPathEntry( const char *pKey, const QString & aDefault = QString() ) const;

  /**
   * Reads a list of string paths.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a list of paths. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   */
  QStringList readPathListEntry( const QString& pKey, char sep = ',' ) const;

  /**
   * Reads a list of string paths.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a list of paths. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   */
  QStringList readPathListEntry( const char *pKey, char sep = ',' ) const;


  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED int readNumEntry( const QString& pKey, int nDefault = 0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED int readNumEntry( const char *pKey, int nDefault = 0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Reads an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED unsigned int readUnsignedNumEntry( const QString& pKey, unsigned int nDefault = 0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Reads an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED unsigned int readUnsignedNumEntry( const char *pKey, unsigned int nDefault = 0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED long readLongNumEntry( const QString& pKey, long nDefault = 0 ) const
    { return readEntry(pKey, static_cast<int>(nDefault)); }

  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED long readLongNumEntry( const char *pKey, long nDefault = 0 ) const
    { return readEntry(pKey, static_cast<int>(nDefault)); }

  /**
   * Read an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED unsigned long readUnsignedLongNumEntry( const QString& pKey, unsigned long nDefault = 0 ) const
    { return readEntry(pKey, static_cast<unsigned int>(nDefault)); }

  /**
   * Read an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED unsigned long readUnsignedLongNumEntry( const char *pKey, unsigned long nDefault = 0 ) const
    { return readEntry(pKey, static_cast<unsigned int>(nDefault)); }

  /**
   * Reads a 64-bit numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED qint64 readNum64Entry( const QString& pKey, qint64 nDefault = 0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Reads a 64-bit numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED qint64 readNum64Entry( const char *pKey, qint64 nDefault = 0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Read an 64-bit unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED quint64 readUnsignedNum64Entry( const QString& pKey, quint64 nDefault = 0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Read an 64-bit unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED quint64 readUnsignedNum64Entry( const char *pKey, quint64 nDefault = 0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Reads a floating point value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED double readDoubleNumEntry( const QString& pKey, double nDefault = 0.0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Reads a floating point value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED double readDoubleNumEntry( const char *pKey, double nDefault = 0.0 ) const
    { return readEntry( pKey, nDefault ); }

  /**
   * Reads a QFont value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a font object.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null QFont by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QFont readFontEntry( const QString& pKey, const QFont* pDefault = 0L ) const;

  /**
   * Reads a QFont value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a font object.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null QFont by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QFont readFontEntry( const char *pKey, const QFont* pDefault = 0L ) const;

  /**
   * Reads a boolean entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a boolean value. Currently "on", "yes", "1" and
   * "true" are accepted as true, everything else if false.
   *
   * @param pKey The key to search for
   * @param bDefault A default value returned if the key was not found.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED bool readBoolEntry( const QString& pKey, bool bDefault = false ) const
    { return readEntry( pKey, bDefault ); }

  /**
   * Reads a boolean entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a boolean value. Currently "on", "yes", "1" and
   * "true" are accepted as true, everything else if false.
   *
   * @param pKey The key to search for
   * @param bDefault A default value returned if the key was not found.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED bool readBoolEntry( const char *pKey, bool bDefault = false ) const
    { return readEntry( pKey, bDefault ); }

  /**
   * Reads a QRect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QRect object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QRect by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QRect readRectEntry( const QString& pKey, const QRect* pDefault = 0L ) const;

  /**
   * Reads a QRect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QRect object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QRect by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QRect readRectEntry( const char *pKey, const QRect* pDefault = 0L ) const;

  /**
   * Reads a QPoint entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a QPoint object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QPoint by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QPoint readPointEntry( const QString& pKey, const QPoint* pDefault = 0L ) const;

  /**
   * Reads a QPoint entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a QPoint object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QPoint by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QPoint readPointEntry( const char *pKey, const QPoint* pDefault = 0L ) const;

  /**
   * Reads a QSize entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a QSize object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QSize by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QSize readSizeEntry( const QString& pKey, const QSize* pDefault = 0L ) const;

  /**
   * Reads a QSize entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a QSize object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QSize by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QSize readSizeEntry( const char *pKey, const QSize* pDefault = 0L ) const;


  /**
   * Reads a QColor entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a color.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null QColor by default) returned if the
   * key was not found or if the value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QColor readColorEntry( const QString& pKey, const QColor* pDefault = 0L ) const
    { return readEntry(pKey, (pDefault? *pDefault: QColor())); }

  /**
   * Reads a QColor entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a color.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null QColor by default) returned if the
   * key was not found or if the value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QColor readColorEntry( const char *pKey, const QColor* pDefault = 0L ) const
    { return readEntry(pKey, (pDefault? *pDefault: QColor())); }

  /**
   * Reads a QDateTime entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a date and time.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value ( currentDateTime() by default)
   * returned if the key was not found or if the read value cannot be
   * interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QDateTime readDateTimeEntry( const QString& pKey, const QDateTime* pDefault = 0L ) const;

  /**
   * Reads a QDateTime entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a date and time.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value ( currentDateTime() by default)
   * returned if the key was not found or if the read value cannot be
   * interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QDateTime readDateTimeEntry( const char *pKey, const QDateTime* pDefault = 0L ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The untranslated entry is returned, you normally do not need this.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
   QString readEntryUntranslated( const QString& pKey,
                     const QString& aDefault = QString() ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The untranslated entry is returned, you normally do not need this.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
   QString readEntryUntranslated( const char *pKey,
                     const QString& aDefault = QString() ) const;

  /**
   * Writes a key/value pair.
   *
   * This is stored in the most specific config file when destroying the
   * config object or when calling sync().
   *
   * If you want to write a path, please use writePathEntry().
   *
   * @param pKey         The key to write.
   * @param value       The value to write.
   * @param pFlags       The flags to use when writing this entry.
   */
  void writeEntry( const char *pKey, const QString& value,
                   WriteConfigFlags pFlags = Normal );

  /**
   * writeEntry() Overridden to accept a property.
   *
   * @param pKey The key to write
   * @param value The property to write
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const QVariant& value,
                   WriteConfigFlags pFlags = Normal );

  /**
   * @copydoc writeEntry( const char*, const QString&, WriteConfigFlags )
   */
  template <typename T>
  void writeEntry( const char* pKey, const T& value,
                   WriteConfigFlags pFlags = Normal );

  /**
   * @copydoc writeEntry( const char*, const QString&, WriteConfigFlags )
   */
  template <typename T>
  void writeEntry( const QString& pKey, const T& value,
                   WriteConfigFlags pFlags = Normal )
    { writeEntry( pKey.toUtf8().constData(), value, pFlags ); }

#ifdef KDE3_SUPPORT
  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  KDE_DEPRECATED void writeEntry( const QString& pKey, const Q3StrList &value,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false )
    { writeEntry(pKey.toUtf8().constData(), value, sep, bPersistent, bGlobal, bNLS); }

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  KDE_DEPRECATED void writeEntry( const char *pKey, const Q3StrList &value,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );
#endif

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writeEntry()
   */
  void writeEntry( const QString& pKey, const QStringList &value,
		   char sep = ',',
                   WriteConfigFlags pFlags = Normal )
    { writeEntry( pKey.toUtf8().constData(), value, sep, pFlags ); }

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const QStringList &value,
		   char sep = ',',
                   WriteConfigFlags pFlags = Normal );

  /**
   * writeEntry() overridden to accept a list.
   *
   * @param pKey The key to write
   * @param value The list to write.
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writeEntry()
   */
  template <typename T>
  void writeEntry( const char* pKey, const QList<T>& value,
                   WriteConfigFlags pFlags = Normal );

  /**
   * writeEntry() overridden to accept a list of variants.
   * @copydoc writeEntry(const char*, const QList<T>&, WriteConfigFlags)
   */
  void writeEntry( const char* pKey, const QVariantList& value,
                   WriteConfigFlags pFlags = Normal )
    { writeEntry( pKey, QVariant(value), pFlags ); }

  /**
   * Write a (key/value) pair where the value is a const char*.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling sync().
   *
   *  @param pKey               The key to write.
   *  @param value     The value to write; assumed to be in latin1 encoding.
   *  @param pFlags       The flags to use when writing this entry.
   */
  void writeEntry( const char *pKey, const char *value,
                   WriteConfigFlags pFlags = Normal )
    { writeEntry(pKey, QString::fromLatin1(value), pFlags); }

  /**
   * Write a (key/value) pair where the value is a QByteArray.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling sync().
   *
   *  @param pKey               The key to write.
   *  @param value     The value to write; assumed to be in latin1 encoding.
   *                    If it contains the null character between 0 and size()-1,
   *                    the string will be truncated at the null character.
   *
   * @param pFlags       The flags to use when writing this entry.
   */
  void writeEntry( const char *pKey, const QByteArray& value,
                   WriteConfigFlags pFlags = Normal )
    { writeEntry(pKey, QString::fromLatin1(value, value.size()), pFlags); }

  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param pFlags       The flags to use when writing this entry.
   */
  void writePathEntry( const QString& pKey, const QString & path,
                       WriteConfigFlags pFlags = Normal );

  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param pFlags       The flags to use when writing this entry.
   */
  void writePathEntry( const char *pKey, const QString & path,
                       WriteConfigFlags pFlags = Normal );

  /**
   * writePathEntry() overridden to accept a list of paths (strings).
   *
   * It is checked whether the paths are located under $HOME. If so each of
   * the paths are written out with the user's home-directory replaced with
   * $HOME. The paths should be read back with readPathListEntry()
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writePathEntry()
   * @see  readPathListEntry()
   */
  void writePathEntry( const QString& pKey, const QStringList &value,
                       char sep = ',', WriteConfigFlags pFlags = Normal );

  /**
   * writePathEntry() overridden to accept a list of paths (strings).
   *
   * It is checked whether the paths are located under $HOME. If so each of
   * the paths are written out with the user's home-directory replaced with
   * $HOME. The paths should be read back with readPathListEntry()
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writePathEntry()
   * @see  readPathListEntry()
   */
  void writePathEntry( const char *pKey, const QStringList &value,
		   char sep = ',', WriteConfigFlags pFlags = Normal );

  /**
   * Deletes the entry specified by @p pKey in the current group.
   *
   * @param pKey The key to delete.
   * @param pFlags       The flags to use when deleting this entry.
   */
   void deleteEntry( const QString& pKey, WriteConfigFlags pFlags = Normal );

  /**
   * Deletes the entry specified by @p pKey in the current group.
   *
   * @param pKey The key to delete.
   * @param pFlags       The flags to use when deleting this entry.
   */
   void deleteEntry( const char *pKey, WriteConfigFlags pFlags = Normal );

  /**
   * Deletes a configuration entry group
   *
   * If the group is not empty and Recursive is not set, nothing gets
   * deleted and false is returned.
   * If this group is the current group and it is deleted, the
   * current group is undefined and should be set with setGroup()
   * before the next operation on the configuration object.
   *
   * @param group The name of the group
   * @param pFlags       The flags to use when writing this entry.
   */
  void deleteGroup( const QString& group, WriteConfigFlags pFlags = Normal );

  /**
   * Turns on or off "dollar  expansion" (see KConfigBase introduction)
   *  when reading config entries.
   * Dollar sign expansion is initially OFF.
   *
   * @param _bExpand Tf true, dollar expansion is turned on.
   */
  void setDollarExpansion( bool _bExpand = true ) { bExpand = _bExpand; }

  /**
   * Returns whether dollar expansion is on or off.  It is initially OFF.
   *
   * @return true if dollar expansion is on.
   */
  bool isDollarExpansion() const { return bExpand; }

  /**
   * Mark the config object as "clean," i.e. don't write dirty entries
   * at destruction time. If @p bDeep is false, only the global dirty
   * flag of the KConfig object gets cleared. If you then call
   * writeEntry() again, the global dirty flag is set again and all
   * dirty entries will be written at a subsequent sync() call.
   *
   * Classes that derive from KConfigBase should override this
   * method and implement storage-specific behavior, as well as
   * calling the KConfigBase::rollback() explicitly in the initializer.
   *
   * @param bDeep If true, the dirty flags of all entries are cleared,
   *        as well as the global dirty flag.
   */
  virtual void rollback( bool bDeep = true );

  /**
   * Flushes all changes that currently reside only in memory
   * back to disk / permanent storage. Dirty configuration entries are
   * written to the most specific file available.
   *
   * Asks the back end to flush out all pending writes, and then calls
   * rollback().  No changes are made if the object has @p readOnly
   * status.
   *
   * You should call this from your destructor in derivative classes.
   *
   * @see rollback(), #isReadOnly()
   */
  virtual void sync();

  /**
   * Checks whether the config file has any dirty (modified) entries.
   * @return true if the config file has any dirty (modified) entries.
   */
  bool isDirty() const { return bDirty; }

  /**
   * Sets the config object's read-only status.
   *
   * @param _ro If true, the config object will not write out any
   *        changes to disk even if it is destroyed or sync() is called.
   *
   */
  virtual void setReadOnly(bool _ro) { bReadOnly = _ro; }

   /**
    * Returns the read-only status of the config object.
    *
    * @return The read-only status.
    */
  bool isReadOnly() const { return bReadOnly; }

  /**
   * Checks whether the key has an entry in the currently active group.
   * Use this to determine whether a key is not specified for the current
   * group (hasKey() returns false). Keys with null data are considered
   * nonexistent.
   *
   * @param key The key to search for.
   * @return If true, the key is available.
   */
  bool hasKey( const QString& key ) const;

  /**
   * Returns a map (tree) of entries for all entries in a particular
   * group.  Only the actual entry string is returned, none of the
   * other internal data should be included.
   *
   * @param group A group to get keys from.
   * @return A map of entries in the group specified, indexed by key.
   *         The returned map may be empty if the group is not found.
   * @see   QMap
   */
  virtual QMap<QString, QString> entryMap(const QString &group) const = 0;

  /**
   * Reparses all configuration files. This is useful for programs
   * that use stand alone graphical configuration tools. The base
   * method implemented here only clears the group list and then
   * appends the default group.
   *
   * Derivative classes should clear any internal data structures and
   * then simply call parseConfigFiles() when implementing this
   * method.
   *
   * @see  parseConfigFiles()
   */
  virtual void reparseConfiguration() = 0;

  /**
   * Checks whether this configuration file can be modified.
   * @return whether changes may be made to this configuration file.
   */
  bool isImmutable() const;

  /**
   * Checks whether it is possible to change the given group.
   * @param group the group to check
   * @return whether changes may be made to @p group in this configuration
   * file.
   */
  bool groupIsImmutable(const QString &group) const;

  /**
   * Checks whether it is possible to change the given entry.
   * @param key the key to check
   * @return whether the entry @p key may be changed in the current group
   * in this configuration file.
   */
  bool entryIsImmutable(const QString &key) const;

  /**
   * Possible return values for getConfigState().
   *
   * @see  getConfigState()
   */
  enum ConfigState { NoAccess, ReadOnly, ReadWrite };

  /**
   * Returns the state of the app-config object.
   *
   * Possible return values
   * are NoAccess (the application-specific config file could not be
   * opened neither read-write nor read-only), ReadOnly (the
   * application-specific config file is opened read-only, but not
   * read-write) and ReadWrite (the application-specific config
   * file is opened read-write).
   *
   * @see  ConfigState()
   * @return the state of the app-config object
   */
  ConfigState getConfigState() const;

  /**
   * Check whether the config files are writable.
   * @param warnUser Warn the user if the configuration files are not writable.
   * @return Indicates that all of the configuration files used are writable.
   */
  bool checkConfigFilesWritable(bool warnUser);

  /**
   * When set, all readEntry and readXXXEntry calls return the system
   * wide (default) values instead of the user's preference.
   * This is off by default.
   */
  void setReadDefaults(bool b);

  /**
   * @returns true if all readEntry and readXXXEntry calls return the system
   * wide (default) values instead of the user's preference.
   */
  bool readDefaults() const;

  /**
   * Reverts the entry with key @p key in the current group in the
   * application specific config file to either the system wide (default)
   * value or the value specified in the global KDE config file.
   *
   * To revert entries in the global KDE config file, the global KDE config
   * file should be opened explicitly in a separate config object.
   *
   * @param key The key of the entry to revert.
   */
  void revertToDefault(const QString &key);

  /**
   * Returns whether a default is specified for an entry in either the
   * system wide configuration file or the global KDE config file.
   *
   * If an application computes a default value at runtime for
   * a certain entry, e.g. like:
   * \code
   * QColor computedDefault = kapp->palette().color(QPalette::Active, QPalette::Text)
   * QColor color = config->readEntry(key, computedDefault);
   * \encode
   *
   * Then it may wish to make the following check before
   * writing back changes:
   * \code
   * if ( (value == computedDefault) && !config->hasDefault(key) )
   *    config->revertToDefault(key)
   * else
   *    config->writeEntry(key, value)
   * \endcode
   *
   * This ensures that as long as the entry is not modified to differ from
   * the computed default, the application will keep using the computed default
   * and will follow changes the computed default makes over time.
   * @param key The key of the entry to check.
   */
  bool hasDefault(const QString &key) const;

protected:
  /**
   * Reads the locale and put in the configuration data struct.
   * Note that this should be done in the constructor, but this is not
   * possible due to some mutual dependencies in KApplication::init()
   */
  void setLocale();

  /**
   * Sets the global dirty flag of the config object
   *
   * @param _bDirty How to mark the object's dirty status
   */
  virtual void setDirty(bool _bDirty = true) { bDirty = _bDirty; }

  /**
   * Parses all configuration files for a configuration object.
   *
   * The actual parsing is done by the associated KConfigBackEnd.
   */
  virtual void parseConfigFiles();

  /**
   * Returns a map (tree) of the entries in the specified group.
   * This may or may not return all entries that belong to the
   * config object.  The only guarantee that you are given is that
   * any entries that are dirty (i.e. modified and not yet written back
   * to the disk) will be contained in the map.  Some derivative
   * classes may choose to return everything.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param pGroup The group to provide a KEntryMap for.
   * @return The map of the entries in the group.
   * @internal
   */
  virtual KEntryMap internalEntryMap( const QString& pGroup ) const = 0;

  /**
   * Returns a map (tree) of the entries in the tree.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @return A map of the entries in the tree.
   *
   * @internal
   *
   */
  virtual KEntryMap internalEntryMap() const = 0;

  /**
   * Inserts a (key/value) pair into the internal storage mechanism of
   * the configuration object. Classes that derive from KConfigBase
   * will need to implement this method in a storage-specific manner.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param _key The key to insert.  It contains information both on
   *        the group of the key and the key itself. If the key already
   *        exists, the old value will be replaced.
   * @param _data the KEntry that is to be stored.
   * @param _checkGroup When false, assume that the group already exists.
   * @internal
   */
  virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup = true) = 0;

  /**
   * Looks up an entry in the config object's internal structure.
   * Classes that derive from KConfigBase will need to implement this
   * method in a storage-specific manner.
   *
   * Do not use this function, the implementation and return type are
   * subject to change.
   *
   * @param _key The key to look up  It contains information both on
   *        the group of the key and the entry's key itself.
   * @return The KEntry value (data) found for the key.  @p KEntry.aValue
   * will be the null string if nothing was located.
   * @internal
   */
  virtual KEntry lookupData(const KEntryKey &_key) const = 0;

  virtual bool internalHasGroup(const QByteArray &group) const = 0;

  /**
   * A back end for loading/saving to disk in a particular format.
   */
  KConfigBackEnd *backEnd;
public:
  /**
   * Overloaded public methods:
   */
  void setGroup( const QByteArray &pGroup );
  void setGroup( const char *pGroup );
  bool hasGroup(const QByteArray &_pGroup) const;
  bool hasGroup(const char *_pGroup) const;
  bool hasKey( const char *pKey ) const;

protected:
  QByteArray readEntryUtf8( const char *pKey) const;

  /**
   * The currently selected group. */
  QByteArray mGroup;

  /**
   * The locale to retrieve keys under if possible, i.e en_US or fr.  */
  QByteArray aLocaleString;

  /**
   * Indicates whether there are any dirty entries in the config object
   * that need to be written back to disk. */
  bool bDirty;

  bool bLocaleInitialized;
  bool bReadOnly;           // currently only used by KSimpleConfig
  mutable bool bExpand;     // whether dollar expansion is used

protected:
  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );
private:
  class Private;
  Private *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KConfigBase::WriteConfigFlags )

#define KCONFIG_DECLARE_ENUM_QOBJECT(Class, Enum)                         \
template <>                                                               \
inline Class::Enum KConfigBase::readEntry(const char* pKey, const Class::Enum& value) const\
{                                                                         \
const QMetaObject* Mobj = &Class::staticMetaObject;                       \
const QMetaEnum Menum = Mobj->enumerator(Mobj->indexOfEnumerator(#Enum)); \
int tmp = value;                                                          \
if (Menum.isFlag()) {                                                     \
  const QString str = readEntry(pKey, Menum.valueToKeys(tmp));            \
  tmp = Menum.keysToValue(str.toLatin1().constData());                    \
} else {                                                                  \
  const QString str = readEntry(pKey, Menum.valueToKey(tmp));             \
  tmp = Menum.keyToValue(str.toLatin1().constData());                     \
}                                                                         \
return static_cast<Class::Enum>(tmp);                                     \
}                                                                         \
template <>                                                               \
void KConfigBase::writeEntry(const char* pKey, const Class::Enum& value, WriteConfigFlags flags)\
{                                                                         \
const QMetaObject* Mobj = &Class::staticMetaObject;                       \
const QMetaEnum Menum = Mobj->enumerator(Mobj->indexOfEnumerator(#Enum)); \
if (Menum.isFlag()) writeEntry(pKey, Menum.valueToKeys(value), flags);    \
else writeEntry(pKey, Menum.valueToKey(value), flags);                    \
}

#define KCONFIG_QVARIANT_CHECK 1
#if KCONFIG_QVARIANT_CHECK
#include <conversion_check.h>
#endif

/*
 * just output error, or any wrong config file will
 * let the app exit, which is IMHO bad
 */
#define kcbError kWarning

template <typename T>
inline QList<T> KConfigBase::readEntry( const char* pKey, const QList<T>& aDefault) const
{
  QVariant::Type wanted = QVariant(T()).type();
#if KCONFIG_QVARIANT_CHECK
  ConversionCheck::to_QVariant<T>();
  ConversionCheck::to_QString<T>();
#else
  kcbError(!QVariant(QVariant::String).canConvert(wanted))
    << "QString cannot convert to \"" << QVariant::typeToName(wanted)
    << "\" information will be lost" << endl;
#endif

  if (!hasKey(pKey))
    return aDefault;

  QList<QVariant> vList;

  if (!aDefault.isEmpty()) {
    foreach (T aValue, aDefault)
      vList.append( aValue );
  }
  vList = readEntry( pKey, vList );

  QList<T> list;
  if (!vList.isEmpty()) {
    foreach (QVariant aValue, vList) {
      kcbError(!aValue.convert(wanted)) << "conversion to "
        << QVariant::typeToName(wanted) << " information has been lost" << endl;
      list.append( qvariant_cast<T>(aValue) );
    }
  }

  return list;
}

template <typename T>
inline T KConfigBase::readEntry( const char* pKey, const T& aDefault) const
{
#if KCONFIG_QVARIANT_CHECK
  ConversionCheck::to_QVariant<T>();
#endif
  return qvariant_cast<T>(readEntry(pKey, QVariant(aDefault)));
}

template <typename T>
void KConfigBase::writeEntry( const char* pKey, const QList<T>& value,
                              WriteConfigFlags pFlags )
{
#if KCONFIG_QVARIANT_CHECK
  ConversionCheck::to_QVariant<T>();
  ConversionCheck::to_QString<T>();
#else
  QVariant dummy QVariant(T());
  kcbError(!dummy.canConvert(QVariant::String))
    << QVariant::typeToName(dummy.type())
    << " cannot convert to QString information will be lost" << endl;
#endif

  QVariantList vList;
  foreach(T aValue, value)
    vList.append(aValue);

  writeEntry( pKey, QVariant(vList), pFlags );
}

template <typename T>
inline void KConfigBase::writeEntry( const char* pKey, const T& value,
                              WriteConfigFlags pFlags )
{
#if KCONFIG_QVARIANT_CHECK
  ConversionCheck::to_QVariant<T>();
#endif
  writeEntry( pKey, QVariant(value), pFlags );
}

#ifdef KDE3_SUPPORT
/**
  * Helper class to facilitate working with KConfig / KSimpleConfig
  * groups.
  *
  * Careful programmers always set the group of a
  * KConfig KSimpleConfig object to the group they want to read from
  * and set it back to the old one of afterwards. This is usually
  * written as:
  * \code
  *
  * QString oldgroup config->group();
  * config->setGroup( "TheGroupThatIWant" );
  * ...
  * config->writeEntry( "Blah", "Blubb" );
  *
  * config->setGroup( oldgroup );
  * \endcode
  *
  * In order to facilitate this task, you can use
  * KConfigGroupSaver. Simply construct such an object ON THE STACK
  * when you want to switch to a new group. Then, when the object goes
  * out of scope, the group will automatically be restored. If you
  * want to use several different groups within a function or method,
  * you can still use KConfigGroupSaver: Simply enclose all work with
  * one group (including the creation of the KConfigGroupSaver object)
  * in one block.
  *
  * @deprecated This class is deprecated and will be removed in KDE 4.
  * KConfigGroup provides similar functionality in a more object oriented
  * way.
  *
  * @author Matthias Kalle Dalheimer <kalle@kde.org>
  * @see KConfigBase, KConfig, KSimpleConfig, KConfigGroup
  * @short Helper class for easier use of KConfig/KSimpleConfig groups
  */
class KDECORE_EXPORT KConfigGroupSaver // KDE4 remove
{
public:
  /**
   * Constructor. You pass a pointer to the KConfigBase-derived
   * object you want to work with and a string indicating the _new_
   * group.
   *
   * @param pConfig The KConfigBase-derived object this
   *               KConfigGroupSaver works on.
   * @param group  The new group that the config object should switch to.
   */
  KConfigGroupSaver( KConfigBase* pConfig, const QString &group )
      : _config(pConfig), _oldgroup(pConfig->group())
        { _config->setGroup( group ); }

  KConfigGroupSaver( KConfigBase* pConfig, const char *group )
      : _config(pConfig), _oldgroup(pConfig->group())
        { _config->setGroup( group ); }

  KConfigGroupSaver( KConfigBase* pConfig, const QByteArray &group )
      : _config(pConfig), _oldgroup(pConfig->group())
        { _config->setGroup( group ); }

  ~KConfigGroupSaver() { _config->setGroup( _oldgroup ); }

    KConfigBase* config() { return _config; };

private:
  KConfigBase* _config;
  QString _oldgroup;

  KConfigGroupSaver(const KConfigGroupSaver&);
  KConfigGroupSaver& operator=(const KConfigGroupSaver&);

  class Private;
  Private *d;
};
#endif

/**
 * A KConfigBase derived class for one specific group in a KConfig object.
 */
class KDECORE_EXPORT KConfigGroup: public KConfigBase
{
public:
   /**
    * Construct a config group corresponding to @p group in @p master.
    * @p group is the group name encoded in UTF-8.
    */
   KConfigGroup(KConfigBase *master, const QByteArray &group);
   /**
    * This is an overloaded constructor provided for convenience.
    * It behaves essentially like the above function.
    *
    * Construct a config group corresponding to @p group in @p master
    */
   KConfigGroup(KConfigBase *master, const QString &group);
   /**
    * This is an overloaded constructor provided for convenience.
    * It behaves essentially like the above function.
    *
    * Construct a config group corresponding to @p group in @p master
    * @p group is the group name encoded in UTF-8.
    */
   KConfigGroup(KConfigBase *master, const char * group);

   /**
    * Delete all entries in the entire group
    * @param bFlags flags passed to KConfigBase::deleteGroup
    */
   void deleteGroup(WriteConfigFlags pFlags=Normal);

   /**
   * Checks whether it is possible to change this group.
   * @return whether changes may be made to this group in this configuration
   * file.
   */
  bool groupIsImmutable() const;

   // The following functions are reimplemented:
   virtual void setDirty(bool _bDirty);
   virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup = true);
   virtual KEntry lookupData(const KEntryKey &_key) const;
   virtual void sync();

private:
   // Hide the following members:
   void setGroup() { }
   void setDesktopGroup() { }
   void group() { }
   void hasGroup() { }
   void setReadOnly(bool) { }
   void isDirty() { }

   // The following members are not used.
   virtual QStringList groupList() const;
   virtual void rollback(bool) { }
   virtual void reparseConfiguration() { }
   virtual QMap<QString, QString> entryMap(const QString &) const
    { return QMap<QString,QString>(); }
   virtual KEntryMap internalEntryMap( const QString&) const;
   virtual KEntryMap internalEntryMap() const;
   virtual bool internalHasGroup(const QByteArray &) const
    { return false; }

   void getConfigState() { }

   KConfigBase *mMaster;
protected:
   /** Virtual hook, used to add new "virtual" functions while maintaining
       binary compatibility. Unused in this class.
   */
   virtual void virtual_hook( int id, void* data );
private:
   Q_DISABLE_COPY(KConfigGroup)
   class Private;
   Private* d;
};

#endif
