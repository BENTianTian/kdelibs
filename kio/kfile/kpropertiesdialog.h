/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (c) 1999, 2000 Preston Brown <pbrown@kde.org>
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (c) 2000 David Faure <faure@kde.org>

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

/*
 * This file holds the definitions for all classes used to
 * display a properties dialog.
 */

#ifndef __propsdialog_h
#define __propsdialog_h

#include <QtCore/QString>

#include <kdemacros.h>
#include <kurl.h>
#include <kfileitem.h>
#include <kpagedialog.h>

class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class KLineEdit;
class KUrlRequester;
class QPushButton;
class KIconButton;
class KPropertiesDialogPlugin;
class QComboBox;

class KJob;
namespace KIO { class Job; }

/**
 * The main properties dialog class.
 * A Properties Dialog is a dialog which displays various information
 * about a particular file or URL, or several files or URLs.
 * This main class holds various related classes, which are instantiated in
 * the form of tab entries in the tabbed dialog that this class provides.
 * The various tabs themselves will let the user view, and sometimes change,
 * information about the file or URL.
 *
 * \image html kpropertiesdialog.png "Typical KProperties Dialog"
 *
 * The best way to display the properties dialog is to use showDialog().
 * Otherwise, you should use (void)new KPropertiesDialog(...)
 * It will take care of deleting itself when closed.
 *
 * If you are looking for more flexibility, see KFileMetaInfo and
 * KFileMetaInfoWidget.
 */
class KIO_EXPORT KPropertiesDialog : public KPageDialog
{
  Q_OBJECT

public:

  /**
   * Determine whether there are any property pages available for the
   * given file items.
   * @param _items the list of items to check.
   * @return true if there are any property pages, otherwise false.
   */
  static bool canDisplay( const KFileItemList& _items );

  /**
   * Brings up a Properties dialog, as shown above.
   * This is the normal constructor for
   * file-manager type applications, where you have a KFileItem instance
   * to work with.  Normally you will use this
   * method rather than the one below.
   *
   * @param item file item whose properties should be displayed.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   */
  explicit KPropertiesDialog( KFileItem * item,
                              QWidget* parent = 0 );

  /**
   * \overload
   *
   * You use this constructor for cases where you have a number of items,
   * rather than a single item. Be careful which methods you use
   * when passing a list of files or URLs, since some of them will only
   * work on the first item in a list.
   *
   * @param _items list of file items whose properties should be displayed.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   */
  explicit KPropertiesDialog( const KFileItemList& _items,
                              QWidget *parent = 0 );

  /**
   * Brings up a Properties dialog. Convenience constructor for
   * non-file-manager applications, where you have a KUrl rather than a
   * KFileItem or KFileItemList.
   *
   * @param _url the URL whose properties should be displayed
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   *
   * IMPORTANT: This constructor, together with exec(), leads to a grave
   * display bug (due to KIO::stat() being run before the dialog has all the
   * necessary information). Do not use this combination for now.
   * TODO: Check if the above is still true with Qt4.
   * For local files with a known mimetype, simply create a KFileItem and pass
   * it to the other constructor.
   */
  explicit KPropertiesDialog( const KUrl& _url,
                              QWidget* parent = 0 );

  /**
   * Creates a properties dialog for a new .desktop file (whose name
   * is not known yet), based on a template. Special constructor for
   * "File / New" in file-manager type applications.
   *
   * @param _tempUrl template used for reading only
   * @param _currentDir directory where the file will be written to
   * @param _defaultName something to put in the name field,
   * like mimetype.desktop
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   */
  KPropertiesDialog( const KUrl& _tempUrl, const KUrl& _currentDir,
                     const QString& _defaultName,
                     QWidget* parent = 0 );

  /**
   * Creates an empty properties dialog (for applications that want use
   * a standard dialog, but for things not doable via the plugin-mechanism).
   *
   * @param title is the string display as the "filename" in the caption of the dialog.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   */
  explicit KPropertiesDialog(const QString& title,
                             QWidget* parent = 0);

  /**
   * Cleans up the properties dialog and frees any associated resources,
   * including the dialog itself. Note that when a properties dialog is
   * closed it cleans up and deletes itself.
   */
  virtual ~KPropertiesDialog();

  /**
   * Immediately displays a Properties dialog using constructor with
   * the same parameters.
   * On MS Windows, if @p item points to a local file, native (non modal) property
   * dialog is displayed (@p parent and @p modal are ignored in this case).
   *
   * @return true on successful dialog displaying (can be false on win32).
   */
  static bool showDialog(KFileItem* item, QWidget* parent = 0,
                         bool modal = true);

  /**
   * Immediately displays a Properties dialog using constructor with
   * the same parameters.
   * On MS Windows, if @p _url points to a local file, native (non modal) property
   * dialog is displayed (@p parent and @p modal are ignored in this case).
   *
   * @return true on successful dialog displaying (can be false on win32).
   */
  static bool showDialog(const KUrl& _url, QWidget* parent = 0,
                         bool modal = true);

  /**
   * Immediately displays a Properties dialog using constructor with
   * the same parameters.
   * On MS Windows, if @p _items has one element and this element points
   * to a local file, native (non modal) property dialog is displayed
   * (@p parent and @p modal are ignored in this case).
   *
   * @return true on successful dialog displaying (can be false on win32).
   */
  static bool showDialog(const KFileItemList& _items, QWidget* parent = 0,
                         bool modal = true);

  /**
   * Adds a "3rd party" properties plugin to the dialog.  Useful
   * for extending the properties mechanism.
   *
   * To create a new plugin type, inherit from the base class KPropertiesDialogPlugin
   * and implement all the methods. If you define a service .desktop file
   * for your plugin, you do not need to call insertPlugin().
   *
   * @param plugin is a pointer to the KPropertiesDialogPlugin. The Properties
   *        dialog will do destruction for you. The KPropertiesDialogPlugin \b must
   *        have been created with the KPropertiesDialog as its parent.
   * @see KPropertiesDialogPlugin
   */
  void insertPlugin (KPropertiesDialogPlugin *plugin);

  /**
   * The URL of the file that has its properties being displayed.
   * This is only valid if the KPropertiesDialog was created/shown
   * for one file or URL.
   *
   * @return a parsed URL.
   */
  KUrl kurl() const;

  /**
   * @return the file item for which the dialog is shown
   *
   * Warning: this method returns the first item of the list.
   * This means that you should use this only if you are sure the dialog is used
   * for a single item. Otherwise, you probably want items() instead.
   */
  KFileItem *item();

  /**
   * @return the items for which the dialog is shown
   */
  KFileItemList items() const;

  /**
   * If the dialog is being built from a template, this method
   * returns the current directory. If no template, it returns QString().
   * See the template form of the constructor.
   *
   * @return the current directory or QString()
   */
  KUrl currentDir() const;

  /**
   * If the dialog is being built from a template, this method
   * returns the default name. If no template, it returns QString().
   * See the template form of the constructor.
   * @return the default name or QString()
   */
  QString defaultName() const;

  /**
   * Updates the item URL (either called by rename or because
   * a global apps/mimelnk desktop file is being saved)
   * Can only be called if the dialog applies to a single file or URL.
   * @param _newUrl the new URL
   */
  void updateUrl( const KUrl& _newUrl );

  /**
   * Renames the item to the specified name. This can only be called if
   * the dialog applies to a single file or URL.
   * @param _name new filename, encoded.
   * \see FilePropsDialogPlugin::applyChanges
   */
  void rename( const QString& _name );

  /**
   * To abort applying changes.
   */
  void abortApplying();

  /**
   * Shows the page that was previously set by
   * setFileSharingPage(), or does nothing if no page
   * was set yet.
   * \see setFileSharingPage
   */
  void showFileSharingPage();

  /**
   * Sets the file sharing page.
   * This page is shown when calling showFileSharingPage().
   *
   * @param page the page to set
   * \see showFileSharingPage
   */
  void setFileSharingPage(QWidget* page);

   /**
    * Call this to make the filename lineedit readonly, to prevent the user
    * from renaming the file.
    * \param ro true if the lineedit should be read only
    */
  void setFileNameReadOnly( bool ro );

public Q_SLOTS:
  /**
   * Called when the user presses 'Ok'.
   */
  virtual void slotOk();      // Deletes the PropertiesDialog instance
  /**
   * Called when the user presses 'Cancel'.
   */
  virtual void slotCancel();     // Deletes the PropertiesDialog instance

Q_SIGNALS:
  /**
   * This signal is emitted when the Properties Dialog is closed (for
   * example, with OK or Cancel buttons)
   */
  void propertiesClosed();

  /**
   * This signal is emitted when the properties changes are applied (for
   * example, with the OK button)
   */
  void applied();

  /**
   * This signal is emitted when the properties changes are aborted (for
   * example, with the Cancel button)
   */

  void canceled();
  /**
   * Emitted before changes to @p oldUrl are saved as @p newUrl.
   * The receiver may change @p newUrl to point to an alternative
   * save location.
   */
  void saveAs(const KUrl &oldUrl, KUrl &newUrl);

Q_SIGNALS:
  void leaveModality();
private:
  class KPropertiesDialogPrivate;
  KPropertiesDialogPrivate* const d;

    Q_DISABLE_COPY(KPropertiesDialog)
};

/**
 * A Plugin in the Properties dialog
 * This is an abstract class. You must inherit from this class
 * to build a new kind of tabbed page for the KPropertiesDialog.
 * A plugin in itself is just a library containing code, not a dialog's page.
 * It's up to the plugin to insert pages into the parent dialog.
 *
 * To make a plugin available, define a service that implements the KPropertiesDialog/Plugin
 * servicetype, as well as the mimetypes for which the plugin should be created.
 * For instance, ServiceTypes=KPropertiesDialog/Plugin,text/html,application/x-mymimetype.
 *
 * You can also include X-KDE-Protocol=file if you want that plugin
 * to be loaded only for local files, for instance.
 */
class KIO_EXPORT KPropertiesDialogPlugin : public QObject
{
  Q_OBJECT
public:
  /**
   * Constructor
   * To insert tabs into the properties dialog, use the add methods provided by
   * KPageDialog (the properties dialog is a KPageDialog).
   */
  KPropertiesDialogPlugin( KPropertiesDialog *_props );
  virtual ~KPropertiesDialogPlugin();

  /**
   * Applies all changes to the file.
   * This function is called when the user presses 'Ok'. The last plugin inserted
   * is called first.
   */
  virtual void applyChanges();

  /**
   * Convenience method for most ::supports methods
   * @return true if the file is a local, regular, readable, desktop file
   */
  static bool isDesktopFile( KFileItem * _item );

  void setDirty( bool b );
  bool isDirty() const;

public Q_SLOTS:
  void setDirty(); // same as setDirty( true )

Q_SIGNALS:
  /**
   * Emit this signal when the user changed anything in the plugin's tabs.
   * The hosting PropertiesDialog will call applyChanges only if the
   * PropsPlugin has emitted this signal or if you have called setDirty() before.
   */
  void changed();

protected:
  /**
   * Pointer to the dialog
   */
  KPropertiesDialog *properties;

  int fontHeight;
private:
  class KPropertiesDialogPluginPrivate;
  KPropertiesDialogPluginPrivate* const d;
};

/**
 * 'General' plugin
 *  This plugin displays the name of the file, its size and access times.
 * @internal
 */
class KIO_EXPORT KFilePropsPlugin : public KPropertiesDialogPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KFilePropsPlugin( KPropertiesDialog *_props );
  virtual ~KFilePropsPlugin();

  /**
   * Applies all changes made.  This plugin must be always the first
   * plugin in the dialog, since this function may rename the file which
   * may confuse other applyChanges functions.
   */
  virtual void applyChanges();

  /**
   * Tests whether the files specified by _items need a 'General' plugin.
   */
  static bool supports( const KFileItemList& _items );

  /**
   * Called after all plugins applied their changes
   */
  void postApplyChanges();

  void setFileNameReadOnly( bool ro );

protected Q_SLOTS:
  void slotEditFileType();
  void slotCopyFinished( KIO::Job * );
  void slotFileRenamed( KIO::Job *, const KUrl &, const KUrl & );
  void slotDirSizeUpdate();
  void slotDirSizeFinished( KJob * );
  void slotFoundMountPoint( const QString& mp, quint64 kBSize,
			    quint64 kBUsed, quint64 kBAvail );
  void slotSizeStop();
  void slotSizeDetermine();

Q_SIGNALS:
  void leaveModality();
private Q_SLOTS:
  // workaround for compiler bug
  void slotFoundMountPoint( const quint64& kBSize, const quint64&
			  kBUsed, const quint64& kBAvail, const QString& mp );
  void nameFileChanged(const QString &text );
  void slotIconChanged();

private:
  void determineRelativePath( const QString & path );
  void applyIconChanges();

  QWidget *iconArea;
  QWidget *nameArea;

  QLabel *m_sizeLabel;
  QPushButton *m_sizeDetermineButton;
  QPushButton *m_sizeStopButton;

  QString m_sRelativePath;
  bool m_bFromTemplate;

  /**
   * The initial filename
   */
  QString oldName;

  class KFilePropsPluginPrivate;
  KFilePropsPluginPrivate* const d;
};

/**
 * 'Permissions' plugin
 * In this plugin you can modify permissions and change
 * the owner of a file.
 * @internal
 */
class KIO_EXPORT KFilePermissionsPropsPlugin : public KPropertiesDialogPlugin
{
  Q_OBJECT
public:
  enum PermissionsMode {
    PermissionsOnlyFiles = 0,
    PermissionsOnlyDirs = 1,
    PermissionsOnlyLinks = 2,
    PermissionsMixed = 3
  };

  enum PermissionsTarget {
    PermissionsOwner  = 0,
    PermissionsGroup  = 1,
    PermissionsOthers = 2
  };

  /**
   * Constructor
   */
  KFilePermissionsPropsPlugin( KPropertiesDialog *_props );
  virtual ~KFilePermissionsPropsPlugin();

  virtual void applyChanges();

  /**
   * Tests whether the file specified by _items needs a 'Permissions' plugin.
   */
  static bool supports( const KFileItemList& _items );

private Q_SLOTS:

  void slotChmodResult( KJob * );
  void slotShowAdvancedPermissions();

Q_SIGNALS:
  void leaveModality();

private:
  void setComboContent(QComboBox *combo, PermissionsTarget target,
		       mode_t permissions, mode_t partial);
  bool isIrregular(mode_t permissions, bool isDir, bool isLink);
  void enableAccessControls(bool enable);
  void updateAccessControls();
  void getPermissionMasks(mode_t &andFilePermissions,
			  mode_t &andDirPermissions,
			  mode_t &orFilePermissions,
			  mode_t &orDirPermissions);

  static const mode_t permissionsMasks[3];
  static const mode_t standardPermissions[4];
  static const char *permissionsTexts[4][4];

  // unused, for binary compatibility!
  QCheckBox *permBox[3][4];

  QComboBox *grpCombo;

  KLineEdit *usrEdit, *grpEdit;

  /**
   * Old permissions
   */
  mode_t permissions;
  /**
   * Old group
   */
  QString strGroup;
  /**
   * Old owner
   */
  QString strOwner;

  // unused, for compatibility
  static mode_t fperm[3][4];

  class KFilePermissionsPropsPluginPrivate;
  KFilePermissionsPropsPluginPrivate* const d;
};


/**
 * Used to edit the files containing
 * [Desktop Entry]
 * URL=....
 *
 * Such files are used to represent a program in kicker and konqueror.
 * @internal
 */
class KIO_EXPORT KUrlPropsPlugin : public KPropertiesDialogPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KUrlPropsPlugin( KPropertiesDialog *_props );
  virtual ~KUrlPropsPlugin();

  virtual void applyChanges();

  static bool supports( const KFileItemList& _items );

private:
  KUrlRequester *URLEdit;
  KIconButton *iconBox;

  QString URLStr;
  QString iconStr;

  QPixmap pixmap;
  QString pixmapFile;
private:
  class KUrlPropsPluginPrivate;
  KUrlPropsPluginPrivate* const d;
};


/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=MimeType
 * @internal
 */
class KIO_EXPORT KBindingPropsPlugin : public KPropertiesDialogPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KBindingPropsPlugin( KPropertiesDialog *_props );
  virtual ~KBindingPropsPlugin();

  virtual void applyChanges();
  static bool supports( const KFileItemList& _items );

private:

  QLineEdit *commentEdit;
  QLineEdit *patternEdit;
  QLineEdit *mimeEdit;
  QString m_sMimeStr;

  QCheckBox * cbAutoEmbed;

  class KBindingPropsPluginPrivate;
  KBindingPropsPluginPrivate* const d;
};

/**
 * Properties plugin for device .desktop files
 * @internal
 */
class KIO_EXPORT KDevicePropsPlugin : public KPropertiesDialogPlugin
{
  Q_OBJECT
public:
  KDevicePropsPlugin( KPropertiesDialog *_props );
  virtual ~KDevicePropsPlugin();

  virtual void applyChanges();

  static bool supports( const KFileItemList& _items );

private Q_SLOTS:
  void slotActivated( int );
  void slotDeviceChanged();
  void slotFoundMountPoint( const quint64& kBSize,
                            const quint64& kBUsed,
                            const quint64& kBAvail,
                            const QString& );

private:
  void updateInfo();

private:
  QComboBox* device;
  QLabel* mountpoint;
  QCheckBox* readonly;
  void* unused;
  //KIconButton* mounted;
  KIconButton* unmounted;

  QStringList m_devicelist;
  int indexDevice;
  int indexMountPoint;
  int indexFSType;

  QPixmap pixmap;
  QString pixmapFile;

  class KDevicePropsPluginPrivate;
  KDevicePropsPluginPrivate* const d;
};

class Ui_KPropertiesDesktopBase;

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=Application
 *
 * Such files are used to represent a program in kicker and konqueror.
 * @internal
 */
class KIO_EXPORT KDesktopPropsPlugin : public KPropertiesDialogPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KDesktopPropsPlugin( KPropertiesDialog *_props );
  virtual ~KDesktopPropsPlugin();

  virtual void applyChanges();

  static bool supports( const KFileItemList& _items );

public Q_SLOTS:
  void slotAddFiletype();
  void slotDelFiletype();
  void slotBrowseExec();
  void slotAdvanced();

private:
  void checkCommandChanged();

private:
  Ui_KPropertiesDesktopBase* w;
  QWidget *m_frame;

  QString m_origCommandStr;
  QString m_terminalOptionStr;
  QString m_suidUserStr;
  QString m_dbusStartusType;
  bool m_terminalBool;
  bool m_terminalCloseBool;
  bool m_suidBool;
  bool m_startupBool;
  bool m_systrayBool;

  class KDesktopPropsPluginPrivate;
  KDesktopPropsPluginPrivate* d;
};

#endif

