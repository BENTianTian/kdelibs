/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Portions copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#ifndef KCONFIGBACKEND_H
#define KCONFIGBACKEND_H

#include <kdecore_export.h>

#include <kconfigbase.h>
#include <klockfile.h>

#include <QtCore/QStack>

class QFile;
class QByteArray;

/**
 * Abstract base class for KDE configuration file loading/saving.
 *
 * This class forms the base for all classes that implement some
 * manner of loading/saving to configuration files.  It is an
 * abstract base class, meaning that you cannot directly instantiate
 * objects of this class. As of right now, the only back end available
 * is one to read/write to INI-style files, but in the future, other
 * formats may be available, such as XML or a database.
 *
 * @author Preston Brown <pbrown@kde.org>,
 *         Matthias Kalle Dalheimer <kalle@kde.org>
 * @short KDE Configuration file loading/saving abstract base class
 */
class KDECORE_EXPORT KConfigBackEnd
{
  friend class KConfig;
  friend class KSharedConfig;
public:
  /**
   * Constructs a configuration back end.
   *
   * @param _config Specifies the configuration object which values
   *        will be passed to as they are read, or from where values
   *        to be written to will be obtained from.
   * @param _fileName The name of the file in which config
   *        data is stored.  All registered configuration directories
   *        will be looked in in order of decreasing relevance.
   * @param _resType the resource type of the fileName specified, _if_
   *        it is not an absolute path (otherwise this parameter is ignored).
   * @param _useKDEGlobals If true, the user's system-wide kdeglobals file
   *        will be imported into the config object.  If false, only
   *        the filename specified will be dealt with.
   */
  KConfigBackEnd(KConfigBase *_config, const QString &_fileName,
		 const char * _resType, bool _useKDEGlobals);

  /**
   * Destructs the configuration backend.
   */
  virtual ~KConfigBackEnd();

  /**
   * Parses all configuration files for a configuration object.  This
   * method must be reimplemented by the derived classes.
   *
   * @returns Whether or not parsing was successful.
   */
  virtual bool parseConfigFiles() = 0;

  /**
   * Writes configuration data to file(s).  This method must be
   * reimplemented by the derived classes.
   *
   * @param bMerge Specifies whether the old config file already
   *        on disk should be merged in with the data in memory.  If true,
   *        data is read off the disk and merged.  If false, the on-disk
   *        file is removed and only in-memory data is written out.
   */
  virtual void sync(bool bMerge = true) = 0;

  /**
   * Changes the filenames associated with this back end.  You should
   * probably reparse your config info after doing this.
   *
   * @param _fileName the new filename to use
   * @param _resType the resource type of the fileName specified, _if_
   *        it is not an absolute path (otherwise this parameter is ignored).
   * @param _useKDEGlobals specifies whether or not to also parse the
   *        global KDE configuration files.
   */
  void changeFileName(const QString &_fileName, const char * _resType,
		      bool _useKDEGlobals);

  /**
   * Returns the state of the app-config object.
   *
   * @see KConfig::getConfigState
   */
  virtual KConfigBase::ConfigState getConfigState() const;

  /**
   * Returns the filename as passed to the constructor.
   * @return the filename as passed to the constructor.
   */
  QString fileName() const;

  /**
   * Returns the resource type as passed to the constructor.
   * @return the resource type as passed to the constructor.
   */
  const char * resource() const;

  /**
   * Set the locale string that defines the current language.
   * @param _localeString the identifier of the language
   * @see KLocale
   */
  void setLocaleString(const QByteArray &_localeString);

  /**
   * Set the file mode for newly created files.
   * @param mode the filemode (as in chmod)
   */
  void setFileWriteMode(int mode);

  /**
   * Check whether the config files are writable.
   * @param warnUser Warn the user if the configuration files are not writable.
   * @return Indicates that all of the configuration files used are writable.
   */
  bool checkConfigFilesWritable(bool warnUser);

  /**
   * Gets the extraConfigFiles in the merge stack.
   */
  QStringList extraConfigFiles() const;

  /**
   * Sets the merge stack to the list of files. The stack is last in first out with
   * the top of the stack being the most specific config file.
   * @param files A list of extra config files containing the full path of the
   * local config file to remove from the stack.
   */
  void setExtraConfigFiles( const QStringList &files );

  /**
   * Remove all files from merge stack. This does not include the local file that
   * was specified in the constructor.
   */
  void removeAllExtraConfigFiles();

  /**
   * Returns a lock file object for the configuration file
   * @param bGlobal If true, returns a lock file object for kdeglobals
   */
  KLockFile::Ptr lockFile( bool bGlobal = false );

protected:
  KConfigBase *pConfig;

  QString mfileName;
  QByteArray resType;
  bool useKDEGlobals : 1;
  bool bFileImmutable : 1;
  QByteArray localeString;
  QString mLocalFileName;
  QString mGlobalFileName;

  QStack<QString> mMergeStack;

  KConfigBase::ConfigState mConfigState;
  int mFileMode;

protected:
  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );
protected:
  class Private;
  Private *const d;
};

#endif
