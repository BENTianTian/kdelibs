// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef __KFILEDIALOG_H__
#define __KFILEDIALOG_H__

#include <qstring.h>

#include <kdialogbase.h>

#include <kfile.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kio/jobclasses.h>

class QCheckBox;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QPopupMenu;
class QVBoxLayout;

class KDirOperator;
class KFileBookmark;
class KFileBookmarkManager;
class KURLComboBox;
class KFileFilter;
class KFileView;
class KFileViewItem;
class KToolBar;

struct KFileDialogPrivate;

/**
 * Provide a user (and developer) friendly way to
 * select files and directories.
 *
 * The widget can be used as a drop in replacement for the
 * @ref  QFileDialog widget, but has greater functionality and a nicer GUI.
 *
 * You will usually want to use one of the static methods
 * @ref getOpenFileName(), @ref getSaveFileName(), @ref getOpenURL()
 * or for multiple files @ref getOpenFileNames() or @ref getOpenURLs().
 *
 * The dialog has been designed to allow applications to customise it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children that will be incorporated into the layout.
 *
 * @image kfiledialog.png KFileDialog
 *
 * @short A file selection dialog.
 *
 * @author Richard J. Moore <rich@kde.org>, Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KFileDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
      * Construct a file dialog.
      *
      * @param urlName The URL of the directory to start in. Use
      *		@ref QString::null to start in the current working
      *		directory, or the last directory where a file has been
      *		selected.
      *
      * @param filter A shell glob that specifies which files to display.
      * See @ref setFilter() for details on how to use this argument.
      *
      * @param acceptURLs If set to @p false, @ref KFileDialog will
      * just accept files on the local filesystem.
      */
    KFileDialog(const QString& urlName, const QString& filter,
		QWidget *parent, const char *name,
		bool modal);

    /**
     * Destruct the file dialog.
     */
    ~KFileDialog();

    /**
     * @returns The selected fully qualified filename.
     */
    KURL selectedURL() const;

    /**
     * @returns The list of selected URLs.
     */
    KURL::List selectedURLs() const;

    /**
     * @returns the currently shown directory.
     */
    KURL baseURL() const;

    /**
     * Retrieve the full path of the selected file in the local filesystem.
     * (Local files only)
     */
    QString selectedFile() const;

    /**
     * Retrieve a list of all selected local files.
     */
    QStringList selectedFiles() const;

    /**
     * Set the directory to view.
     *
     * @param url URL to show.
     * @param clearforward Indicates whether the forward queue
     * should be cleared.
     */
    void setURL(const KURL &url, bool clearforward = true);

    /**
     * Set the file name to preselect to @p name
     *
     * This takes absolute URLs and relative file names.
     */
    void setSelection(const QString& name);

    /**
     * Sets whether the filename/url should be kept when changing directories.
     * This is for example useful when having a predefined filename where
     * the full path for that file is searched.
     *
     * getSaveFileName() and getSaveURL() set this to true by default, so that
     * you can type in the filename and change the directory without having
     * to type the name again.
     */
    void setKeepLocation( bool keep );

    /**
     * @returns whether the contents of the location edit are kept when
     * changing directories.
     */
    bool keepsLocation() const;

    /**
     * Set the filter to be used to @p filter.
     *
     * You can set more
     * filters for the user to select seperated by '\n'. Every
     * filter entry is defined through namefilter|text to diplay.
     * If no | is found in the expression, just the namefilter is
     * shown. Examples:
     *
     * <pre>
     * kfile->setFilter("*.cpp|C++ Source Files\n*.h|Header files");
     * kfile->setFilter("*.cpp");
     * kfile->setFilter("*.cpp|Sources (*.cpp)");
     * kfile->setFilter("*.cpp *.cc *.C|C++ Source Files\n*.h *.H|Header files");
     * </pre>
     *
     * Note: The text to display is not parsed in any way. So, if you
     * want to show the suffix to select by a specific filter, you must
     * repeat it.
     * @see filterChanged()
     */
    void setFilter(const QString& filter);

    /**
     * Retrieve the current filter as entered by the user or one of the
     * predefined set via @ref setFilter().
     *
     * @param filter Contains the new filter (only the extension part,
     * not the explanation), for example, "*.cpp" or "*.cpp *.cc".
     *
     * @see setFilter()
     * @see filterChanged()
     */
    QString currentFilter() const;

    /**
     * Set the filter up to specify the output type.
     *
     * @param label the label to use instead of "Filter:"
     * @param types a list of mimetypes that can be used as output format
     * @param defaultType the default mimetype to use as output format.
     *
     * Do not use in conjunction with @ref setFilter()
     */
    void setFilterMimeType(const QString &label, const KMimeType::List &types, const KMimeType::Ptr &defaultType);

    /**
     * The mimetype for the desired output format.
     *
     * This is only valid if @ref setFilterMimeType() has been called
     * previously.
     *
     * @see setFilterMimeType()
     */
    KMimeType::Ptr currentFilterMimeType();

    /**
     * Add a preview widget and enter the preview mode.
     *
     * In this mode
     * the dialog is split and the right part contains your widget.
     * This widget has to inherit @ref QWidget and it has to implement
     * a slot @ref showPreview(const KURL &); which is called
     * every time the file changes. You may want to look at
     * koffice/lib/kofficecore/koFilterManager.cc for some hints :)
     */
    void setPreviewWidget(const QWidget *w);

    /**
     * Create a modal file dialog and return the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param dir This specifies the path the dialog will start in.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getOpenFileName(const QString& dir= QString::null,
				   const QString& filter= QString::null,
				   QWidget *parent= 0,
				   const QString& caption = QString::null);


    /**
     * Create a modal file dialog and return the selected
     * filenames or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param dir This specifies the path the dialog will start in.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QStringList getOpenFileNames(const QString& dir= QString::null,
					const QString& filter= QString::null,
					QWidget *parent = 0,
					const QString& caption= QString::null);

    /**
     * Create a modal file dialog and return the selected
     * URL or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing URL.
     *
     * @param url This specifies the path the dialog will start in.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getOpenURL(const QString& url = QString::null,
			   const QString& filter= QString::null,
			   QWidget *parent= 0,
			   const QString& caption = QString::null);


    /**
     * Create a modal file dialog and return the selected
     * URLs or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param url This specifies the path the dialog will start in.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL::List getOpenURLs(const QString& url= QString::null,
				  const QString& filter= QString::null,
				  QWidget *parent = 0,
				  const QString& caption= QString::null);

    /**
     * Create a modal file dialog and return the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param dir This specifies the path the dialog will start in.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getSaveFileName(const QString& dir= QString::null,
				   const QString& filter= QString::null,
				   QWidget *parent= 0,
				   const QString& caption = QString::null);

    /**
     * Create a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param url This specifies the path the dialog will start in.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getSaveURL(const QString& url= QString::null,
			   const QString& filter= QString::null,
			   QWidget *parent= 0,
			   const QString& caption = QString::null);
    /**
     * Create a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing directory.
     *
     * @param url The directory the dialog will start in.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getExistingDirectory(const QString & url = QString::null,
					QWidget * parent = 0,
					const QString& caption= QString::null);

    /**
     * @reimplemented
     **/
    virtual void show();

    /**
     * Set the mode of the dialog.
     *
     * The mode is defined as (in kfile.h):
     * <pre>    enum Mode {
     *                      File         = 1,
     *                      Directory    = 2,
     *                      Files        = 4,
     *                      ExistingOnly = 8,
     *                      LocalOnly    = 16
     *                    };
     * </pre>
     * You can OR the values, e.g.
     * <pre>
     * KFile::Mode mode = static_cast<KFile::Mode>( KFile::Files |
     *                                              KFile::ExistingOnly |
     *                                              KFile::LocalOnly );
     * setMode( mode );
     * </pre>
     * You need an explicit cast, which looks a little ugly, but is unavoidable
     * without using (even uglier) #defines. Please not that's at the moment
     * not possible to select multiple files and directories <em>simultaneously</em>.
     * It's possible to select one file or directory at once, though.
     * @see mode()
     */
    void setMode( KFile::Mode m );

    /**
     * Retrieve the mode of the filedialog.
     * @see #setMode()
     */
    KFile::Mode mode() const;

    /**
     * Set the text to be displayed in front of the
     * selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    void setLocationLabel(const QString& text);

    /**
     * Retrieve a pointer to the toolbar.
     *
     * You can use this to insert custom
     * items into it, e.g.:
     * <pre>
     *      yourAction = new KAction( i18n("Your Action"), 0,
     *                                this, SLOT( yourSlot() ),
     *                                this, "action name" );
     *      yourAction->plug( kfileDialog->toolBar() );
     * </pre>
     */
    KToolBar *toolBar() const { return toolbar; }

    /**
     * @returns a pointer to the OK-Button in the filedialog. You may use it
     * e.g. to set a custom text to it.
     */
    QPushButton *okButton() const;

    /**
     * @returns a pointer to the Cancel-Button in the filedialog. You may use
     * it e.g. to set a custom text to it.
     */
    QPushButton *cancelButton() const;

signals:
    /**
      * Emitted when the user selects a file.
      */
    void fileSelected(const QString&);

    /**
      * Emitted when the user highlights a file.
      */
    void fileHighlighted(const QString&);

    /**
     * Emitted when the user hilights one or more files in multiselection mode.
     *
     * Note: @ref fileHighlighted() or @ref fileSelected() are @em not
     * emitted in multiselection mode. You may use selectedItems() to
     * ask for the current highlighted items.
     */
    void selectionChanged();

    /**
     * Emitted when the filter changed, i.e. the user entered an own filter
     * or chose one of the predefined set via @ref setFilter().
     *
     * @param filter contains the new filter (only the extension part,
     * not the explanation), i.e. "*.cpp" or "*.cpp *.cc".
     *
     * @see setFilter()
     * @see currentFilter()
     */
    void filterChanged( const QString& filter );

protected:
    KToolBar *toolbar;

    static KURL *lastDirectory;

    QPopupMenu *bookmarksMenu;
    KURLComboBox *locationEdit;

    KFileFilter *filterWidget;

    KFileBookmarkManager *bookmarks;
    QStringList history; // FIXME: remove it, this is not used at all

    /**
     * adds a entry of the current directory. If disableUpdating is set
     * to true, it will care about clever updating
     **/
    void addDirEntry(KFileViewItem *entry, bool disableUpdating);

    /**
      * rebuild geometry managment.
      *
      */
    virtual void initGUI();

    /**
     * called when an item is highlighted/selected in multiselection mode.
     * handles setting the locationEdit.
     */
    void multiSelectionChanged();

    /**
     * Reads configuration and applies it (size, recent directories, ...)
     */
    virtual void readConfig( KConfig *, const QString& group = QString::null );

    /**
     * Saves the current configuration
     */
    virtual void saveConfig( KConfig *, const QString& group = QString::null );

    /**
     * Reads the recent used files and inserts them into the location combobox
     */
    virtual void readRecentFiles( KConfig * );

    /**
     * Saves the entries from the location combobox.
     */
    virtual void saveRecentFiles( KConfig * );

    /**
     * Parses the string "line" for files. If line doesn't contain any ", the
     * whole line will be interpreted as one file. If the number of " is odd,
     * an empty list will be returned. Otherwise, all items enclosed in " "
     * will be returned as correct urls.
     */
    KURL::List tokenize(const QString& line) const;

protected slots:
    void urlEntered( const KURL& );
    void pathComboActivated( const KURL& url );
    void pathComboReturnPressed( const QString& url );
    void locationActivated( const QString& url );
    void toolbarCallback(int);
    void toolbarPressedCallback(int);
    void slotFilterChanged();
    void pathComboChanged( const QString& );
    void fileHighlighted(const KFileViewItem *i);
    void fileSelected(const KFileViewItem *i);
    void slotStatResult(KIO::Job* job);
    void slotLoadingFinished();

    void dirCompletion( const QString& );
    void fileCompletion( const QString& );

    virtual void updateStatusLine(int dirs, int files);
    virtual void slotOk();
    virtual void accept();


    /**
      * Add the current location to the global bookmarks list
      */
    void addToBookmarks();
    void bookmarksChanged();
    void fillBookmarkMenu( KFileBookmark *parent, QPopupMenu *menu, int &id );

private:
    KFileDialog(const KFileDialog&);
    KFileDialog operator=(const KFileDialog&);


protected:
    KFileDialogPrivate *d;
    KDirOperator *ops;
    bool autoDirectoryFollowing;

    KURL::List& parseSelectedURLs() const;

};

#endif
