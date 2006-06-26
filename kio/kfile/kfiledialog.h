// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Frerich Raabe <raabe@kde.org>

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

#ifndef KFILEDIALOG_H
#define KFILEDIALOG_H

#include <kdialog.h>
#include <kfile.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kio/jobclasses.h>

class QCheckBox;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QMenu;
class QVBoxLayout;

class KActionCollection;
class KDirOperator;
class KUrlBar;
class KUrlComboBox;
class KFileFilterCombo;
class KFileView;
class KFileItem;
class KPushButton;
class KToolBar;
class KPreviewWidgetBase;

struct KFileDialogPrivate;

/**
 * Provides a user (and developer) friendly way to
 * select files and directories.
 *
 * The widget can be used as a drop in replacement for the
 * QFileDialog widget, but has greater functionality and a nicer GUI.
 *
 * You will usually want to use one of the static methods
 * getOpenFileName(), getSaveFileName(), getOpenURL()
 * or for multiple files getOpenFileNames() or getOpenURLs().
 *
 * The dialog has been designed to allow applications to customize it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children that will be incorporated into the layout.
 *
 * \image html kfiledialog.png "KDE File Dialog"
 *
 * @short A file selection dialog.
 *
 * @author Richard J. Moore <rich@kde.org>, Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KIO_EXPORT KFileDialog : public KDialog
{
    Q_OBJECT

public:

    /**
     * Defines some default behavior of the filedialog.
     * E.g. in mode @p Opening and @p Saving, the selected files/urls will
     * be added to the "recent documents" list. The Saving mode also implies
     * setKeepLocation() being set.
     *
     * @p Other means that no default actions are performed.
     *
     * @see setOperationMode
     * @see operationMode
     */
    enum OperationMode { Other = 0, Opening, Saving };

    /**
      * Constructs a file dialog.
      *
      * @param startDir This can either be
      *         @li The URL of the directory to start in.
      *         @li A KUrl() to start in the current working
      *             directory, or the last directory where a file has been
      *             selected.
      *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
      *             directory last used by a filedialog in the same application that specified
      *             the same keyword.
      *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
      *             in the directory last used by a filedialog in any application that specified the
      *             same keyword.
      *
      * @param filter A shell glob or a mime-type-filter that specifies
      *               which files to display.
      * See setFilter() for details on how to use this argument.
      *
      * @param widget A widget, or a widget of widgets, for displaying custom
      *               data in the dialog. This can be used, for example, to
      *               display a check box with the caption "Open as read-only".
      *               When creating this widget, you don't need to specify a parent,
      *               since the widget's parent will be set automatically by KFileDialog.
      * @param parent The parent widget of this dialog
      */
    KFileDialog( const KUrl& startDir, const QString& filter,
                 QWidget *parent, QWidget* widget = 0 );


    /**
     * Destructs the file dialog.
     */
    ~KFileDialog();

    /**
     * @returns The selected fully qualified filename.
     */
    KUrl selectedURL() const;

    /**
     * @returns The list of selected URLs.
     */
    KUrl::List selectedURLs() const;

    /**
     * @returns the currently shown directory.
     */
    KUrl baseURL() const;

    /**
     * Returns the full path of the selected file in the local filesystem.
     * (Local files only)
     */
    QString selectedFile() const;

    /**
     * Returns a list of all selected local files.
     */
    QStringList selectedFiles() const;

    /**
     * Sets the directory to view.
     *
     * @param url URL to show.
     * @param clearforward Indicates whether the forward queue
     * should be cleared.
     */
    void setURL(const KUrl &url, bool clearforward = true);

    /**
     * Sets the file name to preselect to @p name
     *
     * This takes absolute URLs and relative file names.
     */
    void setSelection(const QString& name);

    /**
     * Sets the operational mode of the filedialog to @p Saving, @p Opening
     * or @p Other. This will set some flags that are specific to loading
     * or saving files. E.g. setKeepLocation() makes mostly sense for
     * a save-as dialog. So setOperationMode( KFileDialog::Saving ); sets
     * setKeepLocation for example.
     *
     * The mode @p Saving, together with a default filter set via
     * setMimeFilter() will make the filter combobox read-only.
     *
     * The default mode is @p Opening.
     *
     * Call this method right after instantiating KFileDialog.
     *
     * @see operationMode
     * @see KFileDialog::OperationMode
     */
    void setOperationMode( KFileDialog::OperationMode );

    /**
     * @returns the current operation mode, Opening, Saving or Other. Default
     * is Other.
     *
     * @see operationMode
     * @see KFileDialog::OperationMode
     */
    OperationMode operationMode() const;

    /**
     * Sets whether the filename/url should be kept when changing directories.
     * This is for example useful when having a predefined filename where
     * the full path for that file is searched.
     *
     * This is implicitly set when operationMode() is KFileDialog::Saving
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
     * Sets the filter to be used to @p filter.
     *
     * You can set more
     * filters for the user to select separated by '\n'. Every
     * filter entry is defined through namefilter|text to diplay.
     * If no | is found in the expression, just the namefilter is
     * shown. Examples:
     *
     * \code
     * kfile->setFilter("*.cpp|C++ Source Files\n*.h|Header files");
     * kfile->setFilter("*.cpp");
     * kfile->setFilter("*.cpp|Sources (*.cpp)");
     * kfile->setFilter("*.cpp|" + i18n("Sources (*.cpp)"));
     * kfile->setFilter("*.cpp *.cc *.C|C++ Source Files\n*.h *.H|Header files");
     * \endcode
     *
     * Note: The text to display is not parsed in any way. So, if you
     * want to show the suffix to select by a specific filter, you must
     * repeat it.
     *
     * If the filter contains an unescaped '/', a mimetype-filter is assumed.
     * If you would like a '/' visible in your filter it can be escaped with
     * a '\'. You can specify multiple mimetypes like this (separated with
     * space):
     *
     * \code
     * kfile->setFilter( "image/png text/html text/plain" );
     * kfile->setFilter( "*.cue|CUE\\/BIN Files (*.cue)" );
     * \endcode
     *
     * @see filterChanged
     * @see setMimeFilter
     */
    void setFilter(const QString& filter);

    /**
     * Returns the current filter as entered by the user or one of the
     * predefined set via setFilter().
     *
     * @see setFilter()
     * @see filterChanged()
     */
    QString currentFilter() const;

    /**
     * Returns the mimetype for the desired output format.
     *
     * This is only valid if setFilterMimeType() has been called
     * previously.
     *
     * @see setFilterMimeType()
     */
    KMimeType::Ptr currentFilterMimeType();

    /**
     * Sets the filter up to specify the output type.
     *
     * @param types a list of mimetypes that can be used as output format
     * @param defaultType the default mimetype to use as output format, if any.
     * If @p defaultType is set, it will be set as the current item.
     * Otherwise, a first item showing all the mimetypes will be created.
     * Typically, @p defaultType should be empty for loading and set for saving.
     *
     * Do not use in conjunction with setFilter()
     */
    void setMimeFilter( const QStringList& types,
                        const QString& defaultType = QString() );

    /**
     * The mimetype for the desired output format.
     *
     * This is only valid if setMimeFilter() has been called
     * previously.
     *
     * @see setMimeFilter()
     */
    QString currentMimeFilter() const;

    /**
     *  Clears any mime- or namefilter. Does not reload the directory.
     */
    void clearFilter();

    /**
     * Adds a preview widget and enters the preview mode.
     *
     * In this mode the dialog is split and the right part contains your
     * preview widget.
     *
     * Ownership is transferred to KFileDialog. You need to create the
     * preview-widget with "new", i.e. on the heap.
     *
     * @param w The widget to be used for the preview.
     */
   void setPreviewWidget(const KPreviewWidgetBase *w);

    /**
     * Creates a modal file dialog and return the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getOpenFileName( const KUrl& startDir= KUrl(),
                                    const QString& filter= QString(),
                                    QWidget *parent= 0,
                                    const QString& caption = QString() );


   /**
     * Use this version only if you have no QWidget available as
     * parent widget. This can be the case if the parent widget is
     * a widget in another process or if the parent widget is a
     * non-Qt widget. For example, in a GTK program.
    */
   static QString getOpenFileNameWId( const KUrl& startDir,
                                      const QString& filter,
                                      WId parent_id, const QString& caption );

    /**
     * Creates a modal file dialog and returns the selected
     * filenames or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QStringList getOpenFileNames( const KUrl& startDir= KUrl(),
                                         const QString& filter = QString(),
                                         QWidget *parent = 0,
                                         const QString& caption= QString() );



    /**
     * Creates a modal file dialog and returns the selected
     * URL or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing URL.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl getOpenURL( const KUrl& startDir = KUrl(),
                            const QString& filter = QString(),
                            QWidget *parent= 0,
                            const QString& caption = QString() );



    /**
     * Creates a modal file dialog and returns the selected
     * URLs or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl::List getOpenURLs( const KUrl& startDir = KUrl(),
                                   const QString& filter = QString(),
                                   QWidget *parent = 0,
                                   const QString& caption = QString() );



    /**
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getSaveFileName( const KUrl& startDir = KUrl(),
                                    const QString& filter = QString(),
                                    QWidget *parent = 0,
                                    const QString& caption = QString() );


    /**
     * This function accepts the window id of the parent window, instead
     * of QWidget*. It should be used only when necessary.
     */
    static QString getSaveFileNameWId( const KUrl &startDir, const QString& filter,
                                       WId parent_id,
                                       const QString& caption );

    /**
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl getSaveURL( const KUrl& startDir = KUrl(),
                            const QString& filter = QString(),
                            QWidget *parent = 0,
                            const QString& caption = QString() );


    /**
     * Creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getExistingDirectory( const KUrl& startDir = KUrl(),
                                         QWidget * parent = 0,
                                         const QString& caption= QString() );

    /**
     * Creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * Contrary to getExistingDirectory(), this method allows the
     * selection of a remote directory.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl getExistingURL( const KUrl& startDir = KUrl(),
                                QWidget * parent = 0,
                                const QString& caption= QString() );

    /**
     * Creates a modal file dialog with an image previewer and returns the
     * selected url or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl getImageOpenURL( const KUrl& startDir = KUrl(),
                                 QWidget *parent = 0,
                                 const QString& caption = QString() );

    virtual void show();

    /**
     * Convenient overload of the other setMode(unsigned int) method.
     */
    void setMode( KFile::Mode m );

    /**
     * Sets the mode of the dialog.
     *
     * The mode is defined as (in kfile.h):
     * \code
     *    enum Mode {
     *         File         = 1,
     *         Directory    = 2,
     *         Files        = 4,
     *         ExistingOnly = 8,
     *         LocalOnly    = 16
     *    };
     * \endcode
     * You can OR the values, e.g.
     * \code
     * KFile::Mode mode = static_cast<KFile::Mode>( KFile::Files |
     *                                              KFile::ExistingOnly |
     *                                              KFile::LocalOnly );
     * setMode( mode );
     * \endcode
     */
    void setMode( unsigned int m );

    /**
     * Returns the mode of the filedialog.
     * @see setMode()
     */
    KFile::Mode mode() const;

    /**
     * Sets the text to be displayed in front of the selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    void setLocationLabel(const QString& text);

    /**
     * Returns a pointer to the toolbar.
     *
     * You can use this to insert custom
     * items into it, e.g.:
     * \code
     *      yourAction = new KAction( i18n("Your Action"), 0,
     *                                this, SLOT( yourSlot() ),
     *                                this, "action name" );
     *      yourAction->plug( kfileDialog->toolBar() );
     * \endcode
     */
    KToolBar *toolBar() const { return toolbar; }

    /**
     * @returns a pointer to the OK-Button in the filedialog. You may use it
     * e.g. to set a custom text to it.
     */
    KPushButton *okButton() const;

    /**
     * @returns a pointer to the Cancel-Button in the filedialog. You may use
     * it e.g. to set a custom text to it.
     */
    KPushButton *cancelButton() const;

    /**
     * @returns the KUrlBar object used as the "speed bar". You can add custom
     * entries to it like that:
     * \code
     * KUrlBar *urlBar = fileDialog->speedBar();
     * if ( urlBar )
     *     urlBar->insertDynamicItem( someURL, i18n("The URL's description") );
     * \endcode
     *
     * Note that this method may return a null-pointer if the user configured
     * to not use the speed-bar.
     * @see KUrlBar
     * @see KUrlBar::insertDynamicItem
     */
    KUrlBar *speedBar();

    /**
     * @returns a pointer to the action collection, holding all the used
     * KActions.
     */
    KActionCollection *actionCollection() const;

    /**
     * @returns the action before the path combobox so subclasses may insert widgets into
     * the dialog
     */
    QAction* pathComboIndex();

    /**
     * This method implements the logic to determine the user's default directory
     * to be listed. E.g. the documents directory, home directory or a recently
     * used directory.
     * @param startDir A url, to be used. May use the 'kfiledialog:///keyword' and
     *                 'kfiledialog:///keyword?global' syntax
     *                 as documented in the KFileDialog() constructor.
     * @param recentDirClass If the 'kfiledialog:///' syntax is used, recentDirClass
     *        will contain the string to be used later for KRecentDir::dir()
     * @return The URL that should be listed by default (e.g. by KFileDialog or
     *         KDirSelectDialog).
     */
    static KUrl getStartURL( const KUrl& startDir, QString& recentDirClass );

    /**
     * @internal
     * Used by KDirSelectDialog to share the dialog's start directory.
     */
    static void setStartDir( const KUrl& directory );

Q_SIGNALS:
    /**
      * Emitted when the user selects a file. It is only emitted in single-
      * selection mode. The best way to get notified about selected file(s)
      * is to connect to the okClicked() signal inherited from KDialog
      * and call selectedFile(), selectedFiles(),
      * selectedURL() or selectedURLs().
      */
    void fileSelected(const QString&);

    /**
      * Emitted when the user highlights a file.
      */
    void fileHighlighted(const QString&);

    /**
     * Emitted when the user hilights one or more files in multiselection mode.
     *
     * Note: fileHighlighted() or fileSelected() are @em not
     * emitted in multiselection mode. You may use selectedItems() to
     * ask for the current highlighted items.
     * @see fileSelected
     */
    void selectionChanged();

    /**
     * Emitted when the filter changed, i.e. the user entered an own filter
     * or chose one of the predefined set via setFilter().
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

    static KUrl *lastDirectory;

    KUrlComboBox *locationEdit;

    KFileFilterCombo *filterWidget;

    /**
     * Reimplemented to animate the cancel button.
     */
    virtual void keyPressEvent( QKeyEvent *e );

    /**
      * Perform basic initialization tasks. Called by constructors.
      */
    void init(const KUrl& startDir, const QString& filter, QWidget* widget);

    /**
      * rebuild geometry management.
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
     * Recommended usage together with KConfigGroup
     */
    virtual void readConfig( KConfigGroup * configGroup);

    /**
     * Saves the current configuration
     * Recommended usage together with KConfigGroup
     */
    virtual void writeConfig( KConfigGroup *configGroup);

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
    KUrl::List tokenize(const QString& line) const;

    /**
     * Returns the absolute version of the URL specified in locationEdit.
     */
    KUrl getCompleteURL(const QString&);

    /**
     * Returns the filename extension associated with the currentFilter().
     * QString() is returned if an extension is not available or if
     * operationMode() != Saving.
     */
    QString currentFilterExtension();

    /**
     * Updates the currentFilterExtension and the availability of the
     * Automatically Select Extension Checkbox (visible if operationMode()
     * == Saving and enabled if an extension _will_ be associated with the
     * currentFilter(), _after_ this call).  You should call this after
     * filterWidget->setCurrentItem().
     */
    void updateAutoSelectExtension();


protected Q_SLOTS:
    void urlEntered( const KUrl& );
    void enterURL( const KUrl& url );
    void enterURL( const QString& url );
    void locationActivated( const QString& url );


    void slotFilterChanged();
    void fileHighlighted(const KFileItem *i);
    void fileSelected(const KFileItem *i);
    void slotStatResult(KJob* job);
    void slotLoadingFinished();
    void fileCompletion( const QString& );
    void toggleSpeedbar( bool );
    void toggleBookmarks(bool show);

    virtual void slotOk();
    virtual void accept();
    virtual void slotCancel();

    void slotAutoSelectExtClicked();
    void addToRecentDocuments();
    void initSpeedbar();

private Q_SLOTS:
    void slotLocationChanged( const QString& text );

private:
    KFileDialog(const KFileDialog&);
    KFileDialog operator=(const KFileDialog&);

    void setLocationText( const QString& text );
    void updateLocationWhatsThis();

    void appendExtension(KUrl &url);
    void updateLocationEditExtension(const QString &);
    void updateFilter();

    static void initStatic();

    void setNonExtSelection();

protected:
    KDirOperator *ops;
    bool autoDirectoryFollowing;

    KUrl::List& parseSelectedURLs() const;

private:
    KFileDialogPrivate *d;
};

#endif
