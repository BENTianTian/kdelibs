/* This file is part of the KDE libraries
   Copyright (c) 1999 Pietro Iglio <iglio@kde.org>

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

#ifndef _KDESKTOPFILE_H
#define _KDESKTOPFILE_H

#include "kconfig.h"

/** 
 * KDE Desktop File Management.
 *
 * @author Pietro Iglio <iglio@kde.org>
 * @version $Id$
 * @see  KConfigBase  KConfig
 * @short KDE Desktop File Management class
 */
class KDesktopFile : public KConfig
{
  Q_OBJECT 

public:
  /** 
   * Construct a KDesktopFile object and make it either read-write
   * or read-only.  
   *
   * @param pFileName The file used for saving the data. The
   *                  full path must be specified.
   * @param bReadOnly Whether the object should be read-only.
   * @param resType   Allows you to change what sort of resource
   *                  to search for if @p pFileName is not absolute.  For
   *                  instance, you might want to specify "config".
   */
  KDesktopFile( const QString &pFileName, bool bReadOnly = false,
		const QString &resType = QString::fromLatin1("apps"));

  /** 
   * Destructor. 
   *
   * Write back any dirty configuration entries.
   */
  virtual ~KDesktopFile();

  /**
   * Check to see whether this is a desktop file.
   * 
   * The check is performed looking at the file extension (the file is not
   * opened).
   * Currently, valid extensions are ".kdelnk" and ".desktop".
   * @eturn @p true if the file appears to be a desktop file.
   */
  static bool isDesktopFile(const QString& path);

  /**
   * Retrieve the value of the "Type=" entry.
   */
  QString readType();

  /**
   * Retrieve the value of the "Icon=" entry.
   */
  QString readIcon();

  /**
   * Retrieve the value of the "Name=" entry.
   */
  QString readName();

  /**
   * Retrieve the value of the "Comment=" entry.
   */
  QString readComment();

  /**
   * Retrieve the value of the "Path=" entry.
   */
  QString readPath();

  /**
   * Retrieve the value of the "URL=" entry.
   */
  QString readURL();

  /**
   * Check to see if there is a "Type=Link" entry.
   *
   * The link points to the "URL=" entry.
   */
  bool hasLinkType();

  /**
   * Check to see if there is an entry "Type=Application".
   */
  bool hasApplicationType();

  /**
   * Check to see if there is an entry "Type=MimeType".
   */
  bool hasMimeTypeType(); // funny name :)

  /**
   * Check to see if there is an entry "Type=FSDev".
   */
  bool hasDeviceType();

  /**
   * Check to see if the TryExec field contains a binary
   * which is found on the local system.
   */
  bool tryExec();

  /**
   * @return The filename as passed to the constructor.
   */
  QString filename() const;
  
  /**
   * @return The resource type as passed to the constructor.
   */
  QString resource() const;

private:

  // copy-construction and assignment are not allowed
  KDesktopFile( const KDesktopFile& );
  KDesktopFile& operator= ( const KDesktopFile& );

};

  
#endif

