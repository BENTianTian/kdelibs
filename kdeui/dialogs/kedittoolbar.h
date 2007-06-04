// -*- mode: c++; c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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
#ifndef KEDITTOOLBAR_H
#define KEDITTOOLBAR_H

#include <kxmlguiclient.h>
#include <kdialog.h>

class K3Process;
class KActionCollection;
class QTreeWidget;

class KEditToolBarWidget;
class KEditToolBarPrivate;
class KEditToolBarWidgetPrivate;

/**
 * @short A dialog used to customize or configure toolbars.
 *
 * This dialog only works if your application uses the XML UI
 * framework for creating menus and toolbars.  It depends on the XML
 * files to describe the toolbar layouts and it requires the actions
 * to determine which buttons are active.
 *
 * Typically, you would include the KStdAction::configureToolBars()
 * standard action in your application.  In your slot to this action,
 * you would have something like so:
 *
 * \code
 * KEditToolBar dlg(actionCollection());
 * if (dlg.exec())
 * {
 *   createGUI();
 * }
 * \endcode
 *
 * That code snippet also takes care of redrawing the menu and
 * toolbars if you have made any changes.
 *
 * If you are using KMainWindow's settings methods (either save/apply manually
 * or autoSaveSettings), you should write something like:
 * \code
 * void MyClass::slotConfigureToolBars()
 * {
 *   saveMainWindowSettings( KGlobal::config(), "MainWindow" );
 *   KEditToolBar dlg(actionCollection());
 *   connect(&dlg,SIGNAL(newToolBarConfig()),this,SLOT(slotNewToolBarConfig()));
 *   dlg.exec();
 * }
 *
 * void MyClass::slotNewToolBarConfig() // This is called when OK, Apply or Defaults is clicked
 * {
 *    ...if you use any action list, use plugActionList on each here...
 *    createGUI();
 *    applyMainWindowSettings( KGlobal::config(), "MainWindow" );
 * }
 * \endcode
 *
 * Note that the procedure is a bit different for KParts applications.
 * In this case, you need only pass along a pointer to your
 * application's KXMLGUIFactory object.  The editor will take care of
 * finding all of the action collections and XML files.  The editor
 * aims to be semi-intelligent about where it assigns any
 * modifications.  In other words, it will not write out part specific
 * changes to your shell's XML file.
 *
 * An example would be:
 *
 * \code
 * saveMainWindowSettings( KGlobal::config(), "MainWindow" );
 * KEditToolBar dlg(factory());
 * connect(&dlg,SIGNAL(newToolBarConfig()),this,SLOT(slotNewToolBarConfig()));
 * dlg.exec();
 *
 * void MyClass::slotNewToolBarConfig() // This is called when OK, Apply or Defaults is clicked
 * {
 *    ...if you use any action list, use plugActionList on each here...
 *    // Do NOT call createGUI()!
 *    applyMainWindowSettings( KGlobal::config(), "MainWindow" );
 * }
 * \endcode
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
class KDEUI_EXPORT KEditToolBar : public KDialog
{
    Q_OBJECT
public:
  /**
   * Constructor for apps that do not use components.
   *
   * This is the
   * only entry point to this class.  You @em must pass along your
   * collection of actions (some of which appear in your toolbars).
   * The other two parameters are optional.
   *
   * @param collection The collection of actions to work on.
   * @param defaultToolBar The toolbar with this name will appear for editing.
   *                       Pass in QString() for the default behaviour,
   *                       generallyd desired for apps that do not use
   *                       components.
   * @param parent The parent of the dialog.
   */
  explicit KEditToolBar(KActionCollection *collection,
                        QWidget* parent = 0);

  /**
   * Constructor for KParts based apps.
   *
   * The main parameter, factory(), is a pointer to the
   * XML GUI factory object for your application.  It contains a list
   * of all of the GUI clients (along with the action collections and
   * xml files) and the toolbar editor uses that.
   *
   * Use this like so:
   * \code
   * KEditToolBar edit(factory());
   * if ( edit.exec() )
   * ...
   * \endcode
   *
   * @param factory Your application's factory object
   * @param defaultToolBar The toolbar with this name will appear for editing.
   *                       Pass in QString() for default behavior.
   * @param parent The usual parent for the dialog.
   */
  explicit KEditToolBar( KXMLGUIFactory* factory,
                         QWidget* parent = 0 );

  /// destructor
  ~KEditToolBar();

  /**
   * Sets the default toolbar that will be selected when the dialog is shown.
   * If not set, or QString() is passed in, the global default tool bar name
   * will be used.
   * @param toolBarName the name of the tool bar
   * @see setGlobalDefaultToolBar
   */
  void setDefaultToolBar( const QString& toolBarName );

  /**
   * The name (absolute or relative) of your application's UI resource file
   * is assumed to be share/apps/appname/appnameui.rc though this can be
   * overridden by calling this method.
   *
   * The global parameter controls whether or not the
   * global resource file is used.  If this is @p true, then you may
   * edit all of the actions in your toolbars -- global ones and
   * local one.  If it is @p false, then you may edit only your
   * application's entries.  The only time you should set this to
   * false is if your application does not use the global resource
   * file at all (very rare).
   *
   * @param xmlfile The application's local resource file.
   * @param global If @p true, then the global resource file will also
   *               be parsed.
   */
  void setResourceFile( const QString& file, bool global = true );

  /**
   * Sets the default toolbar which will be auto-selected for all
   * KEditToolBar instances. Can be overridden on a per-dialog basis
   * by calling setDefaultToolBar( const QString& ) on the dialog.
   *   @param  toolbarName  the name of the tool bar
   */
  static void setGlobalDefaultToolBar(const char *toolBarName);

protected Q_SLOTS:
  /**
   * Overridden in order to save any changes made to the toolbars
   */
  virtual void slotOk();
  /**
   * idem
   */
  virtual void slotApply();

  /** should OK really save?
  * @internal
  **/
  void acceptOK(bool b);

  /**
   * Set toolbars to default value
  **/
  void slotDefault();

Q_SIGNALS:
  /**
   * Signal emitted when 'apply' or 'ok' is clicked or toolbars were reset.
   * Connect to it, to plug action lists and to call applyMainWindowSettings
   * (see sample code in this class's documentation)
   */
  void newToolBarConfig();

  QT_MOC_COMPAT void newToolbarConfig();

protected:
  virtual void showEvent(QShowEvent* event);
  virtual void hideEvent(QHideEvent* event);

private:
  void init();

  friend class KEditToolBarPrivate;
  KEditToolBarPrivate *const d;

  Q_DISABLE_COPY(KEditToolBar)
};

/**
 * @short A widget used to customize or configure toolbars
 *
 * This is the widget that does all of the work for the
 * KEditToolBar dialog.  In most cases, you will want to use the
 * dialog instead of this widget directly.
 *
 * Typically, you would use this widget only if you wanted to embed
 * the toolbar editing directly into your existing configure or
 * preferences dialog.
 *
 * This widget only works if your application uses the XML UI
 * framework for creating menus and toolbars.  It depends on the XML
 * files to describe the toolbar layouts and it requires the actions
 * to determine which buttons are active.
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
class KDEUI_EXPORT KEditToolBarWidget : public QWidget, virtual public KXMLGUIClient
{
  Q_OBJECT
public:
  /**
   * Constructor.  This is the only entry point to this class.  You
   * @p must pass along your collection of actions (some of which
   * appear in your toolbars).  The other three parameters are
   * optional.
   *
   * The second parameter, xmlfile, is the name (absolute or
   * relative) of your application's UI resource file.  If it is
   * left blank, then the resource file: share/apps/appname/appnameui.rc
   * is used.  This is the same resource file that is used by the
   * default createGUI function in KMainWindow so you're usually
   * pretty safe in leaving it blank.
   *
   * The third parameter, global, controls whether or not the
   * global resource file is used.  If this is true, then you may
   * edit all of the actions in your toolbars -- global ones and
   * local one.  If it is false, then you may edit only your
   * application's entries.  The only time you should set this to
   * false is if your application does not use the global resource
   * file at all (very rare)
   *
   * The last parameter, parent, is the standard parent stuff.
   *
   * @param collection The collection of actions to work on
   * @param xmlfile The application's local resource file
   * @param global If true, then the global resource file will also
   *               be parsed
   * @param parent This widget's parent
   */
  explicit KEditToolBarWidget( KActionCollection *collection,
                               QWidget *parent = 0L);

  /**
   * Constructor for KParts based apps.
   *
   * The first parameter, factory, is a pointer to the XML GUI
   * factory object for your application.  It contains a list of all
   * of the GUI clients (along with the action collections and xml
   * files) and the toolbar editor uses that.
   *
   * The second parameter, parent, is the standard parent
   *
   * Use this like so:
   * \code
   * KEditToolBar edit(factory());
   * if ( edit.exec() )
   * ...
   * \endcode
   *
   * @param factory Your application's factory object
   * @param parent This widget's parent
   */
  explicit KEditToolBarWidget( QWidget *parent = 0L );

  /**
   * Destructor.  Note that any changes done in this widget will
   * @p NOT be saved in the destructor.  You @p must call save()
   * to do that.
   */
  virtual ~KEditToolBarWidget();

  /**
   * Loads the toolbar configuration into the widget. Should be called before being shown.
   * @see KEditToolBar
   */
  void load( const QString& resourceFile,
             bool global = true,
             const QString& defaultToolBar = QString() );

  /**
   * Loads the toolbar configuration into the widget. Should be called before being shown.
   * @see KEditToolBar
   */
  void load( KXMLGUIFactory* factory,
             const QString& defaultToolBar = QString() );

  /**
   * @internal Reimplemented for internal purposes.
   */
  virtual KActionCollection *actionCollection() const;

  /**
   * Save any changes the user made.  The file will be in the user's
   * local directory (usually $HOME/.kde/share/apps/\<appname\>).  The
   * filename will be the one specified in the constructor.. or the
   * made up one if the filename was NULL.
   *
   * @return The status of whether or not the save succeeded.
   */
  bool save();

  /**
   * Remove and readd all KMXLGUIClients to update the GUI
   */
  void rebuildKXMLGUIClients();

Q_SIGNALS:
  /**
   * Emitted whenever any modifications are made by the user.
   */
  void enableOk(bool);

private:
  Q_PRIVATE_SLOT(d, void slotToolBarSelected(const QString& text))

  Q_PRIVATE_SLOT(d, void slotInactiveSelectionChanged())
  Q_PRIVATE_SLOT(d, void slotActiveSelectionChanged())

  Q_PRIVATE_SLOT(d, void slotInsertButton())
  Q_PRIVATE_SLOT(d, void slotRemoveButton())
  Q_PRIVATE_SLOT(d, void slotUpButton())
  Q_PRIVATE_SLOT(d, void slotDownButton())

  Q_PRIVATE_SLOT(d, void slotChangeIcon())

  Q_PRIVATE_SLOT(d, void slotProcessExited( K3Process* ))

private:
  friend class KEditToolBarWidgetPrivate;
  KEditToolBarWidgetPrivate *const d;
  
  Q_DISABLE_COPY(KEditToolBarWidget)
};

#endif // _KEDITTOOLBAR_H
