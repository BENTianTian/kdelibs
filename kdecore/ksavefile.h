/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

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

#ifndef KSAVEFILE_H
#define KSAVEFILE_H

#include "kdelibs_export.h"

#include <stdio.h>
#include <QFile>
class QString;
class QTextStream;
class QDataStream;

/**
 * The KSaveFile class has been made to write out changes to an existing
 * file atomically.
 * This means that EITHER:
 * a)
 *   All changes have been written successfully to the file.
 *
 * b)
 *   Some error occurred, no changes have been written whatsoever and the
 *   old file is still in place.
 */
class KDECORE_EXPORT KSaveFile
{
public:
   /**
    * Creates a new KSaveFile with the given file name.
    * @param filename the path of the file
    * @param mode the mode of the file (see chmod(1))
    */
   KSaveFile(const QString &filename, int mode = 0666 );

   /**
    * The destructor closes the file.
    * You might want to call close() explicitly though, to test whether it worked.
    **/
   ~KSaveFile();

   /**
    * Returns the status of the file based on errno. (see errno.h)
    * 0 means OK.
    *
    * You should check the status after object creation to check
    * whether a file could be created in the first place.
    *
    * You may check the status after closing the file to verify that
    * the file has indeed been written correctly.
    * @return the errno status, 0 means ok
    **/
   int status() const;

   /**
    * The name of the file as passed to the constructor.
    * @return The name of the file, or QString() if opening the
    *         file has failed
    **/
   QString name() const;

   /**
    * An integer file descriptor open for writing to the file.
    * @return The file descriptor, or a negative number if opening
    *         the temporary file failed
    **/
   int handle()	const;

   /**
    * A FILE* stream open for writing to the file.
    * @return FILE* stream open for writing to the file, or 0
    *         if opening the temporary file failed
    **/
   FILE *fstream();

   /**
    * A QFile* open for writing to the file.
    * @return A QFile open for writing to the file, or 0 if
    *         opening the temporary file failed.
    **/
   QFile *file();

   /**
    * A QTextStream* open for writing to the file.
    * @return A QTextStream that is open for writing to the file, or 0
    *         if opening the temporary file failed
    **/
   QTextStream *textStream();

   /**
    * A QDataStream* open for writing to the file.
    * @return A QDataStream that is open for writing to the file, or 0
    *         if opening the file failed
    **/
   QDataStream *dataStream();

   /**
    * Aborts the write operation and removes any intermediate files
    * This implies a close.
    **/
   void abort();

   /**
    * Closes the file and makes the changes definitive.
    * Returns 'true' is successful, or 'false' if an error has occurred.
    * See status() for details about errors.
    * @return true if successful, or false if an error has occurred.
    **/
   bool close();

    /**
     * Static method to create a backup file before saving.
     * You can use this method even if you don't use KSaveFile.
     * @param filename the file to backup
     * @param backupDir optional directory where to save the backup file in.
     * If empty (the default), the backup will be in the same directory as @p filename.
     * The backup type (simple, rcs, or numbered), extension string, and maximum
     * number of backup files are read from the user's global configuration.
     * Use simpleBackupFile() or numberedBackupFile() to force one of these
     * specific backup styles.
     */
   static bool backupFile( const QString& filename,
                           const QString& backupDir = QString() );

    /**
     * Static method to create a backup file before saving.
     * You can use this method even if you don't use KSaveFile.
     * @param filename the file to backup
     * @param backupDir optional directory where to save the backup file in.
     * If empty (the default), the backup will be in the same directory as @p filename.
     * @param backupExtension the extension to append to @p filename, "~" by default.
     */
   static bool simpleBackupFile( const QString& filename,
                                 const QString& backupDir = QString(),
                                 const QString& backupExtension = QLatin1String( "~" ) );

    /**
     * Static method to create a backup file before saving.
     * You can use this method even if you don't use KSaveFile.
     * The backup file names will be of the form:
     *     \<name\>.\<number\>\<extension\>
     * for instance
     *     \verbatim chat.3.log\endverbatim
     *
     * The new backup file will be have the backup number 1.
     * Each existing backup file will have its number incremented by 1.
     * Any backup files with numbers greater than the maximum number
     * permitted (@p maxBackups) will be removed.
     *
     * @param filename the file to backup
     * @param backupDir optional directory where to save the backup file in.
     * If empty (the default), the backup will be in the same directory as
     * @p filename.
     * @param backupExtension the extension to append to @p filename,
     * which is "~" by default.  Do not use an extension containing digits.
     * @param maxBackups the maximum number of backup files permitted.
     * For best performance a small number (10) is recommended.
     */
    static bool numberedBackupFile( const QString& filename,
                                    const QString& backupDir = QString(),
                                    const QString& backupExtension = QString::fromLatin1( "~" ),
                               const uint maxBackups = 10
        );


    /**
     * Static method to create a backup file before saving.
     * You can use this method even if you don't use KSaveFile.
     * The backup file names will be of the form:
     *     \<name\>,v
     * for instance
     *     \verbatim photo.jpg,v\endverbatim
     *
     * The new backup file will be in RCS format.
     * Each existing backup file will be committed as a new revision.
     *
     * @param filename the file to backup
     * @param backupDir optional directory where to save the backup file in.
     * If empty (the default), the backup will be in the same directory as
     * @p filename.
     * @param backupMessage is the RCS commit message for this revision.
     */
    static bool rcsBackupFile( const QString& filename,
                               const QString& backupDir = QString(),
                               const QString& backupMessage = QString()
        );

private:

   Q_DISABLE_COPY(KSaveFile)

   class Private;
   Private *const d;
};

#endif
