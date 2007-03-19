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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef __kmimetype_h__
#define __kmimetype_h__

#include <sys/types.h>
#include <sys/stat.h>

#include <qstringlist.h>
#include <QtCore/QList>

#include <kurl.h>
#include <ksycocatype.h>
#include <kservicetype.h>

/**
 * Represent a mime type, like "text/plain", and the data that is associated
 * with it.
 *
 * The starting point you need is often the static methods.
 *
 * KMimeType inherits KServiceType because "text/plain" can be used to find
 * services (apps and components) "which can open text/plain".
 *
 * @see KServiceType
 */
class KIO_EXPORT KMimeType : public KServiceType
{
    K_SYCOCATYPE( KST_KMimeType, KServiceType )

public:
    typedef KSharedPtr<KMimeType> Ptr;
    typedef QList<Ptr> List;

    virtual ~KMimeType();

    /**
     * Returns the icon associated with this mime type. See also icon(url)
     * which takes an URL and returns a special icon for this URL.
     *
     * For instance for a folder you can get the folder-specific icon instead
     * of the generic folder icon.
     *
     * @return the name of the icon, can be QString().
     */
    QString iconName() const;

    /**
     * Return the filename of the icon associated with the mimetype.
     * Use KIconLoader::loadMimeTypeIcon to load the icon.
     *
     * The url argument is unused, but is provided so that KMimeType-derived classes
     * can use it (e.g. KFolderType uses the URL to return one out of 2 icons)
     *
     * @return The path to the icon associated with this MIME type.
     */
    virtual QString iconName( const KUrl& ) const;

    /**
     * Return the filename of the icon associated with the mimetype, for a given url.
     * Use KIconLoader::loadMimeTypeIcon to load the icon.
     * @param url URL for the file
     * @param mode the mode of the file. The mode may modify the icon
     *              with overlays that show special properties of the
     *              icon. Use 0 for default
     * @return the name of the icon. The name of a default icon if there is no icon
     *         for the mime type
     */
    static QString iconNameForUrl( const KUrl & url, mode_t mode = 0 );

    /**
     * Return the "favicon" (see http://www.favicon.com) for the given @p url,
     * if available. Does NOT attempt to download the favicon, it only returns
     * one that is already available.
     *
     * If unavailable, returns QString().
     * @param url the URL of the favicon
     * @return the name of the favicon, or QString()
     */
    static QString favIconForUrl( const KUrl& url );

    /**
     * Returns the descriptive comment associated with the MIME type.
     * The url argument is unused, but provided so that KMimeType derived classes
     * can use it.
     *
     * @return The descriptive comment associated with the MIME type, if any.
     */
    virtual QString comment( const KUrl& = KUrl() ) const { return KServiceType::comment(); }

    /**
     * Retrieve the list of patterns associated with the MIME Type.
     * @return a list of file globs that describe the file names
     *         (or, usually, the extensions) of files with this mime type
     */
    const QStringList& patterns() const;

    /**
     * Load the mimetype from a stream.
     * @param qs the stream to load from
     */
    virtual void load( QDataStream &qs );

    /**
     * Save the mimetype to a stream.
     * @param qs the stream to save to
     */
    virtual void save( QDataStream &qs );

    /**
     * Returns the property with the given @p name.
     * @param name the name of the property
     * @return the value of the property
     * @see propertyNames()
     */
    virtual QVariant property( const QString& name ) const;

    /**
     * Retrieves a list of all properties associated with this
     * KMimeType.
     * @return a list of all property names
     * @see property()
     */
    virtual QStringList propertyNames() const;

    enum FindByNameOption { DontResolveAlias, ResolveAliases = 1 };

    /**
     * Retrieve a pointer to the mime type @p name .
     *
     * @em Very @em important: Don't store the result in a KMimeType* !
     *
     * Also note that you get a new KMimeType pointer every time you call this.
     * Don't ever write code that compares mimetype pointers, compare names instead.
     *
     * @param name the name of the mime type
     * @return the pointer to the KMimeType with the given @p name, or
     *         0 if not found
     * @see KServiceType::serviceType
     */
    static Ptr mimeType( const QString& name, FindByNameOption options = DontResolveAlias );

    /**
     * Finds a KMimeType with the given @p url.
     * This function looks at mode_t first.
     * If that does not help it
     * looks at the extension.  This is fine for FTP, FILE, TAR and
     * friends, but is not for HTTP ( cgi scripts! ). You should use
     * KRun instead, but this function returns immediately while
     * KRun is async. If no extension matches, then
     * the file contents will be examined if the URL is a local file, or
     * "application/octet-stream" is returned otherwise.
     *
     * @param url Is the right most URL with a filesystem protocol. It
     *        is up to you to find out about that if you have a nested
     *        URL.  For example
     *        "http://localhost/mist.gz#gzip:/decompress" would have to
     *        pass the "http://..." URL part, while
     *        "file:/tmp/x.tar#tar:/src/test.gz#gzip:/decompress" would
     *        have to pass the "tar:/..." part of the URL, since gzip is
     *        a filter protocol and not a filesystem protocol.
     * @param mode the mode of the file (used, for example, to identify
     *              executables)
     * @param is_local_file true if the file is local; false if not, or if you don't know.
     * @param fast_mode If set to true no disk access is allowed to
     *        find out the mimetype. The result may be suboptimal, but
     *        it is @em fast.
     * @param accuracy if set, the accuracy of the result, between 0 and 100.
     *        For instance, when the extension was used to determine the mimetype,
     *        the accuracy is set to 80, as per the shared-mime spec.
     *        Some 'magic' rules (used when !fast_mode) have an accuracy > 80
     *        (and have priority over the filename, others are < 80).
     *
     * @return A pointer to the matching mimetype. 0 is never returned.
     * @em Very @em Important: Don't store the result in a KMimeType* !
     */
    static Ptr findByUrl( const KUrl& url, mode_t mode = 0,
                          bool is_local_file = false, bool fast_mode = false,
                          int *accuracy = 0 );
    /**
     * Finds a KMimeType with the given @p url.
     * This function looks at mode_t first.
     * If that does not help it
     * looks at the extension.  This is fine for FTP, FILE, TAR and
     * friends, but is not for HTTP ( cgi scripts! ). You should use
     * KRun instead, but this function returns immediately while
     * KRun is async. If no extension matches, then
     * the file contents will be examined if the URL is a local file, or
     * "application/octet-stream" is returned otherwise.
     *
     * Equivalent to
     * \code
     * KUrl u;
     * u.setPath(path);
     * return findByUrl( u, mode, true, fast_mode );
     * \endcode
     *
     * @param path the path to the file
     * @param mode the mode of the file (used, for example, to identify
     *              executables)
     * @param fast_mode If set to true no disk access is allowed to
     *        find out the mimetype. The result may be suboptimal, but
     *        it is @em fast.
     * @param accuracy If not a null pointer, *accuracy is set to the
     *          accuracy of the match (which is in the range 0..100)
     * @return A pointer to the matching mimetype. 0 is never returned.
     */
    static Ptr findByPath( const QString& path, mode_t mode = 0,
                           bool fast_mode = false, int* accuracy = 0 );

    /**
     * Tries to find out the MIME type of a data chunk by looking for
     * certain magic numbers and characteristic strings in it.
     *
     * @param data the data to examine
     * @param accuracy If not a null pointer, *accuracy is set to the
     *          accuracy of the match (which is in the range 0..100)
     * @return a pointer to the KMimeType. application/octet-stream's KMimeType of the
     *         type can not be found this way.
     */
    static Ptr findByContent( const QByteArray &data, int *accuracy=0 );

    /**
     * Tries to find out the MIME type of filename/url and a data chunk.
     * Whether to trust the extension or the data depends on the results of both approaches,
     * and is determined automatically.
     *
     * This method is useful for instance in the get() method of kioslaves, and anywhere else
     * where a filename is associated with some data which is available immediately.
     *
     * @param name the filename or url representing this data.
     * Only used for the extension, not used as a local filename.
     * @param data the data to examine when the extension isn't conclusive in itself
     * @param mode the mode of the file (used, for example, to identify executables)
     * @param accuracy If not a null pointer, *accuracy is set to the
     *          accuracy of the match (which is in the range 0..100)
     */
    static Ptr findByNameAndContent( const QString& name, const QByteArray& data,
                                     mode_t mode = 0, int *accuracy=0 );

    /**
     * Tries to find out the MIME type of a file by looking for
     * certain magic numbers and characteristic strings in it.
     * This function is similar to the previous one. Note that the
     * file name is not used for determining the file type, it is just
     * used for loading the file's contents.
     *
     * @param fileName the path to the file
     * @param accuracy If not a null pointer, *accuracy is set to the
     *          accuracy of the match (which is in the range 0..100)
     * @return a pointer to the KMimeType. application/octet-stream's KMimeType of the
     *         type can not be found this way.
     */
    static Ptr findByFileContent( const QString &fileName, int *accuracy=0 );

    /**
     * Returns whether a file has an internal format that is not human readable.
     * This is much more generic than "not mime->is(text/plain)".
     * Many application file formats (like rtf and postscript) are based on text,
     * but text that the user should rarely ever see.
     */
    static bool isBinaryData( const QString &fileName );

    /**
     * Get all the mimetypes.
     *
     * Useful for showing the list of
     * available mimetypes.
     * More memory consuming than the ones above, don't use unless
     * really necessary.
     * @return the list of all existing KMimeTypes
     */
    static List allMimeTypes();

    /**
     * Returns the name of the default mimetype.
     * Always application/octet-stream, but this method exists
     * for performance purposes.
     * @return the name of the default mime type, always
     *         "application/octet-stream"
     */
    static const QString & defaultMimeType();

    /**
     * Returns the default mimetype.
     * Always application/octet-stream.
     * This can be used to check the result of mimeType(name).
     * @return the "application/octet-stream" mimetype pointer.
     */
    static KMimeType::Ptr defaultMimeTypePtr();

    /// Return true if this mimetype is the default mimetype
    bool isDefault() const { return this == defaultMimeTypePtr().data(); }

    /**
     * If this mimetype inherits from ("is also") another mimetype,
     * return the name of the parent.
     *
     * For instance a text/x-log is a special kind of text/plain,
     * so the definition of text/x-log can say
     *      sub-class-of type="text/plain"
     * Or an smb-workgroup is a special kind of inode/directory, etc.
     * This mechanism can also be used to rename mimetypes and preserve compat.
     *
     * Note that this notion doesn't map to the servicetype inheritance mechanism,
     * since an application that handles the specific type doesn't necessarily handle
     * the base type. The opposite is true though.
     *
     * @return the parent mime type, or QString() if not set
     */
    QString parentMimeType() const;

    /**
     * Do not use name()=="somename" anymore, to check for a given mimetype.
     * For mimetype inheritance to work, use is("somename") instead.
     * Warning, do not use inherits(), that's the servicetype inheritance concept!
     * is() also supports mimetype aliases.
     */
    bool is( const QString& mimeTypeName ) const;

    /**
     * Determines the extension from a filename (or full path) using the mimetype database.
     * This allows to extract "tar.bz2" for foo.tar.bz2
     * but still return "txt" for my.doc.with.dots.txt
     */
    static QString extractKnownExtension( const QString &fileName );

protected:

    friend class KMimeTypeFactory; // for KMimeType(QDataStream&,int)
    friend class KBuildMimeTypeFactory; // for KMimeType(QDataStream&,int)
    friend class KMimeFileParser; // for addPattern and addMagicRule
    /**
     * @internal Construct a service from a stream.
     *
     * The stream must already be positionned at the correct offset
     */
    KMimeType( QDataStream& str, int offset );

    /**
     * Construct a mimetype and take all information from an XML file.
     * @param fullpath the path to the xml that describes the mime type
     * @param the name of the mimetype (usually the end of the path)
     * @param comment the comment associated with the mimetype
     */
    KMimeType( const QString& fullpath, const QString& name, const QString& comment );

    /// @internal for kbuildsycoca
    void addPattern(const QString& pattern);
    /// @internal for kbuildsycoca
    void setParentMimeType(const QString& parent);

private:

    void loadInternal( QDataStream& );
    static void buildDefaultType();
    static void checkEssentialMimeTypes();
    static KMimeType::Ptr findByUrlHelper( const KUrl& url, mode_t mode,
                                           bool is_local_file, QIODevice* device, int* accuracy );

    class Private;
    Private* const d;

protected:
    virtual void virtual_hook( int id, void* data );
};

/**
 * Folder mime type. Handles locked folders, for instance.
 * @short Mimetype for a folder (inode/directory)
 */
class KIO_EXPORT KFolderType : public KMimeType
{
    K_SYCOCATYPE( KST_KFolderType, KMimeType )

public:
    /** \internal */
    KFolderType( const QString& fullpath, const QString& name, const QString& comment )
        : KMimeType( fullpath, name, comment ) { }
    /** \internal */
    KFolderType( QDataStream& str, int offset ) : KMimeType( str, offset ) { }

    virtual QString icon( const KUrl& url ) const;
    virtual QString comment( const KUrl& url ) const;
protected:
    virtual void virtual_hook( int id, void* data );
};

#endif
