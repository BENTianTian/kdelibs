/*  $Id$

    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999 David Faure (faure@kde.org)

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

#ifndef __kio_netaccess_h
#define __kio_netaccess_h

#include <qobject.h>
#include <qstring.h>
#include <kio/global.h>

class QStringList;
class QWidget;
class KURL;

namespace KIO {

  class Job;

  /**
   * Net Transparency.
   *
   * NetAccess allows you to do simple file operation (load, save,
   * copy, delete..) without working with @ref KIO::Job directly.
   * Whereas a @ref KIO::Job is asynchronous, meaning that the
   * developer has to connect slots for it, KIO::NetAccess provides
   * synchronous downloads and uploads, as well as temporary file
   * creation and removal. The functions appear to be blocking,
   * but the Qt event loop continues running while the operations
   * are handled. This means that the GUI will not freeze.
   *
   * This class isn't meant to be used as a class but only as a simple
   * namespace for static functions, though an instance of the class
   * is built for internal purposes.
   *
   * Port to kio done by David Faure, faure@kde.org
   *
   * @short Provides an easy, synchronous interface to KIO file operations.
   */
class NetAccess : public QObject
{
  Q_OBJECT

public:
    /**
     * Downloads a file from an arbitrary URL (@p src) to a
     * temporary file on the local filesystem (@p target).
     *
     * If the argument
     * for @p target is an empty string, download will generate a
     * unique temporary filename in /tmp. Since @p target is a reference
     * to @ref QString you can access this filename easily. Download will
     * return true if the download was successful, otherwise false.
     *
     * Special case:
     * If the URL is of kind file:, then no downloading is
     * processed but the full filename is returned in @p target.
     * That means you @em have to take care about the @p target argument.
     * (This is very easy to do, please see the example below.)
     *
     * Download is synchronous. That means you can use it like
     * this, (assuming @p u is a string which represents a URL and your
     * application has a @ref loadFile() function):
     *
     * <pre>
     * QString tmpFile;
     * if( KIO::NetAccess::download( u, tmpFile ) )
     * {
     *   loadFile( tmpFile );
     *   KIO::NetAccess::removeTempFile( tmpFile );
     *   }
     * </pre>
     *
     * Of course, your user interface will still process exposure/repaint
     * events during the download.
     *
     * @param src URL Reference to the file to download.
     * @param target String containing the final local location of the
     *               file.  If you insert an empty string, it will
     *               return a location in a temporary spot.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if successful, false for failure
     */
    static bool download(const KURL& src, QString & target, QWidget* window);

    /**
     * @deprecated. Use the function above instead.
     */
    static bool download(const KURL& src, QString & target);

    /**
     * Removes the specified file if and only if it was created
     * by KIO::NetAccess as a temporary file for a former download.
     *
     * Note: This means that if you created your temporary with @ref KTempFile,
     * use @ref KTempFile::unlink() or @ref KTempFile::setAutoDelete() to have
     * it removed.
     *
     * @param name Path to temporary file to remove.  May not be
     *             empty.
     */
    static void removeTempFile(const QString& name);

    /**
     * Uploads file @p src to URL @p target.
     *
     * Both must be specified, unlike @ref #download.
     * Note that this is assumed to be used for saving a file over
     * the network, so overwriting is set to true. This is not the
     * case with @ref #copy.
     *
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     *
     * @return true if successful, false for failure
     */
    static bool upload(const QString& src, const KURL& target, QWidget* window);

    /**
     * @deprecated. Use the function above instead.
     */
    static bool upload(const QString& src, const KURL& target);

    /**
     * Alternative to @ref upload for copying over the network.
     * Overwrite is false, so this will fail if @p target exists.
     *
     * This one takes two URLs and is a direct equivalent
     * of @ref KIO::file_copy (not KIO::copy!).
     *
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     *
     * @return true if successful, false for failure
     */
    static bool copy( const KURL& src, const KURL& target, QWidget* window );

    /**
     * @deprecated. Use the function above instead.
     */
    static bool copy( const KURL& src, const KURL& target );

    /**
     * Full-fledged equivalent of KIO::file_copy
     */
    static bool file_copy( const KURL& src, const KURL& dest, int permissions=-1,
                            bool overwrite=false, bool resume=false, QWidget* window = 0L );

    /**
     * Alternative method for copying over the network.
     * Overwrite is false, so this will fail if @p target exists.
     *
     * This one takes two URLs and is a direct equivalent
     * of @ref KIO::copy!.
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the
     *               file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     * @return true if successful, false for failure
     */
    static bool dircopy( const KURL& src, const KURL& target, QWidget* window );

    /**
     * @deprecated. Use the function above instead.
     */
    static bool dircopy( const KURL& src, const KURL& target );

    /**
     * Tests whether a URL exists.
     *
     * This is a convenience function for @ref KIO::stat
     * (it saves creating a slot and testing for the job result).
     *
     * @param url the url we are testing
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if the URL exists, false otherwise
     */
    static bool exists(const KURL& url, QWidget* window);

    /**
     * @deprecated. Use the function above instead.
     */
    static bool exists(const KURL& url);

    /**
     * Overloaded version of @ref exists().
     * A stat() can have two meanings. Either we want to read from this URL,
     * or to check if we can write to it. First case is "source", second is "dest".
     * It is necessary to know what the StatJob is for, to tune the kioslave's behaviour
     * (e.g. with FTP)
     * Tests whether a URL exists.
     *
     * @param url the url we are testing
     * @param source if true, we want to read from that URL.
     *               If false, we want to write to it.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if the URL exists and we can do the operation specified by
     *              @p source, false otherwise
     */
    static bool exists(const KURL& url, bool source, QWidget* window);

    /**
     * @deprecated. Use the function above instead.
     */
    static bool exists(const KURL& url, bool source);

    /**
     * Tests whether a URL exists and return information on it.
     *
     * This is a convenience function for @ref KIO::stat
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The URL we are testing.
     * @param entry The result of the stat. Iterate over the list
     * of atoms to get hold of name, type, size, etc., or use @ref KFileItem.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if successful, false for failure
     */
    static bool stat(const KURL& url, KIO::UDSEntry & entry, QWidget* window);

    /**
     * @deprecated. Use the function above instead.
     */
    static bool stat(const KURL& url, KIO::UDSEntry & entry);

    /**
     * Deletes a file or a directory in an synchronous way.
     *
     * This is a convenience function for @ref KIO::del
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The file or directory to delete.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true on success, false on failure.
     */
    static bool del( const KURL & url, QWidget* window );

    /**
     * @deprecated. Use the function above instead.
     */
    static bool del( const KURL & url );

    /**
     * Creates a directory in a synchronous way.
     *
     * This is a convenience function for @p KIO::mkdir
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The directory to create.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true on success, false on failure.
     */
    static bool mkdir( const KURL & url, QWidget* window, int permissions = -1 );

    /**
     * @deprecated. Use the function above instead.
     */
    static bool mkdir( const KURL & url, int permissions = -1 );

    /**
     * Executes a remote process via the fish ioslave in a synchrounous way.
     *
     * @param url The remote machine where the command should be executed.
     *            e.g. fish://someuser@somehost:sshport/
     *            some special cases exist.
     *            fish://someuser@localhost/
     *            will use su instead of ssh to connect and execute the command.
     *            fish://someuser@localhost:port/
     *            will use ssh to connect and execute the command.
     * @param command The command to be executed.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return The resulting output of the @p command that is executed.
     */
    static QString fish_execute( const KURL & url, const QString command, QWidget* window );

    /**
     * @internal
     * This function is not implemented!?
     * (only mimetypeInternal)
     *
     * Determines the mimetype of a given URL.
     *
     * This is a convenience function for @ref KIO::mimetype.  You
     * should call this only when really necessary.  @ref
     * KMimeType::findByURL can determine extension a lot faster, but
     * less reliably for remote files. Only when @ref findByURL() returns
     * unknown (application/octet-stream) then this one should be
     * used.
     *
     * @param url The URL whose mimetype we are interested in.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return The mimetype name.
     */
    static QString mimetype( const KURL & url, QWidget* window );

    /**
     * @deprecated. Use the function above instead.
     */
    static QString mimetype( const KURL & url );

    /**
     * Returns the error string for the last job, in case it failed.
     * @return the last error string, or QString::null
     */
    static QString lastErrorString() { return lastErrorMsg ? *lastErrorMsg : QString::null; }

private:
    /**
     * Private constructor
     */
    NetAccess() {}

    /**
     * Private destructor
     */
    ~NetAccess() {}

    /**
     * Internal methods
     */
    bool copyInternal(const KURL& src, const KURL& target, int permissions,
                      bool overwrite, bool resume, QWidget* window);
    bool dircopyInternal(const KURL& src, const KURL& target, QWidget* window = 0);
    bool statInternal(const KURL & url, int details, bool source, QWidget* window = 0);
    UDSEntry m_entry;
    bool delInternal(const KURL & url, QWidget* window = 0);
    bool mkdirInternal(const KURL & url, int permissions, QWidget* window = 0);
    QString fish_executeInternal(const KURL & url, const QString command, QWidget* window = 0);

    QString mimetypeInternal(const KURL & url, QWidget* window = 0);
    void enter_loop();
    QString m_mimetype;

    /**
     * List of temporary files
     */
    static QStringList* tmpfiles;

    static QString* lastErrorMsg;
    /**
     * Whether the download succeeded or not
     */
    bool bJobOK;
    friend class I_like_this_class;

private slots:
    void slotResult( KIO::Job * job );
    void slotMimetype( KIO::Job * job, const QString & type );
private:
    class NetAccessPrivate* d;
};

};

#endif
