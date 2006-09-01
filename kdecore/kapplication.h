/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

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

#ifndef KAPP_H
#define KAPP_H

// Version macros. Never put this further down.
#include "kdeversion.h"
#include "kdelibs_export.h"

class KConfig;
class KConfigBase;
class KCharsets;

#ifdef KDE3_SUPPORT
#include <krandom.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <qpixmap.h>
#include <qicon.h>
#endif

typedef unsigned long Atom;
#if !defined(Q_WS_X11)
typedef void Display;
#endif

#include <qapplication.h>
#include <kinstance.h>

#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#endif
struct _IceConn;
class QPixmap;
class KSessionManaged;
class KStyle;
class KUrl;

#define kapp KApplication::kApplication()

/**
* Controls and provides information to all KDE applications.
*
* Only one object of this class can be instantiated in a single app.
* This instance is always accessible via the 'kapp' global variable.
*
* This class provides the following services to all KDE applications.
*
* @li It controls the event queue (see QApplication ).
* @li It provides the application with KDE resources such as
* accelerators, common menu entries, a KConfig object. session
* management events, help invocation etc.
* @li Installs an empty signal handler for the SIGPIPE signal.
* If you want to catch this signal
* yourself, you have set a new signal handler after KApplication's
* constructor has run.
* @li It can start new services
*
*
* The way a service gets started depends on the 'X-DCOP-ServiceType'
* entry in the desktop file of the service:
*
* There are three possibilities:
* @li X-DCOP-ServiceType=None (default)
*    Always start a new service,
*    don't wait till the service registers with dcop.
* @li X-DCOP-ServiceType=Multi
*    Always start a new service,
*    wait until the service has registered with dcop.
* @li X-DCOP-ServiceType=Unique
*    Only start the service if it isn't already running,
*    wait until the service has registered with dcop.
*
* @short Controls and provides information to all KDE applications.
* @author Matthias Kalle Dalheimer <kalle@kde.org>
*/
class KDECORE_EXPORT KApplication : public QApplication, public KInstance
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.KApplication")
public:
  /**
   * This constructor is the one you should use.
   * It takes aboutData and command line arguments from KCmdLineArgs.
   *
   * @param GUIenabled Set to false to disable all GUI stuff.
   */
  explicit KApplication(bool GUIenabled = true);

#ifdef Q_WS_X11
  /**
   * Constructor. Parses command-line arguments. Use this constructor when you
   * you need to use a non-default visual or colormap.
   *
   * @param display Will be passed to Qt as the X display. The display must be
   * valid and already opened.
   *
   * @param visual A pointer to the X11 visual that should be used by the
   * application. Note that only TrueColor visuals are supported on depths
   * greater than 8 bpp. If this parameter is NULL, the default visual will
   * be used instead.
   *
   * @param colormap The colormap that should be used by the application. If
   * this parameter is 0, the default colormap will be used instead.
   */
  explicit KApplication(Display *display, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);

  /**
   * Constructor. Parses command-line arguments. Use this constructor to use KApplication
   * in a Motif or Xt program.
   *
   * @param display Will be passed to Qt as the X display. The display must be valid and already
   * opened.
   *
   * @param argc command line argument count
   *
   * @param argv command line argument value(s)
   *
   * @param rAppName application name. Will be used for finding the
   * associated message files and icon files, and as the default
   * registration name for DCOP. This is a mandatory parameter.
   *
   * @param GUIenabled Set to false to disable all GUI stuff.
   */
  KApplication(Display *display, int& argc, char** argv, const QByteArray& rAppName,
               bool GUIenabled=true);
#endif

  virtual ~KApplication();

  /**
   * Returns the current application object.
   *
   * This is similar to the global QApplication pointer qApp. It
   * allows access to the single global KApplication object, since
   * more than one cannot be created in the same application. It
   * saves you the trouble of having to pass the pointer explicitly
   * to every function that may require it.
   * @return the current application object
   */
  static KApplication* kApplication() { return KApp; }

  /**
   * Returns the application session config object.
   *
   * @return A pointer to the application's instance specific
   * KConfig object.
   * @see KConfig
   */
  KConfig* sessionConfig();

#ifdef KDE3_SUPPORT
 /**
   * Is the application restored from the session manager?
   *
   * @return If true, this application was restored by the session manager.
   *    Note that this may mean the config object returned by
   * sessionConfig() contains data saved by a session closedown.
   * @see sessionConfig()
   * @deprecated use qApp->isSessionRestored()
   */
  inline KDE_DEPRECATED bool isRestored() const { return QApplication::isSessionRestored(); }
#endif

  /**
   * Disables session management for this application.
   *
   * Useful in case  your application is started by the
   * initial "startkde" script.
   */
  void disableSessionManagement();

  /**
   * Enables session management for this application, formerly
   * disabled by calling disableSessionManagement(). You usually
   * shouldn't call this function, as session management is enabled
   * by default.
   */
  void enableSessionManagement();

    /**
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManaged.
     * @internal
     */
  void commitData( QSessionManager& sm );

    /**
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManaged.
     * @internal
     */
  void saveState( QSessionManager& sm );

  /**
   * Returns true if the application is currently saving its session
   * data (most probably before KDE logout). This is intended for use
   * mainly in KMainWindow::queryClose() and KMainWindow::queryExit().
   *
   * @see KMainWindow::queryClose
   * @see KMainWindow::queryExit
   */
  bool sessionSaving() const;

#ifdef KDE3_SUPPORT
  /**
   * Returns a QPixmap with the application icon.
   * @return the application icon
   * @deprecated Use QApplication::windowIcon()
   */
  inline KDE_DEPRECATED QPixmap icon() const {
      int size = IconSize(K3Icon::Desktop);
      return windowIcon().pixmap(size,size);
  };

  /**
   * Returns the mini-icon for the application as a QPixmap.
   * @return the application's mini icon
   * @deprecated Use QApplication::windowIcon()
   */
  inline KDE_DEPRECATED QPixmap miniIcon() const {
      int size = IconSize(K3Icon::Small);
      return windowIcon().pixmap(size,size);
  };
#endif

  /**
   *  Sets the top widget of the application.
   *  This means basically applying the right window caption.
   *  An application may have several top widgets. You don't
   *  need to call this function manually when using KMainWindow.
   *
   *  @param topWidget A top widget of the application.
   *
   *  @see icon(), caption()
   **/
  void setTopWidget( QWidget *topWidget );

public:
  /**
   * Get a file name in order to make a temporary copy of your document.
   *
   * @param pFilename The full path to the current file of your
   * document.
   * @return A new filename for auto-saving.
   */
  static QString tempSaveName( const QString& pFilename );

  /**
   * Check whether  an auto-save file exists for the document you want to
   * open.
   *
   * @param pFilename The full path to the document you want to open.
   * @param bRecover  This gets set to true if there was a recover
   * file.
   * @return The full path of the file to open.
   */
  static QString checkRecoverFile( const QString& pFilename, bool& bRecover );

#ifdef KDE3_SUPPORT
#ifdef Q_WS_X11
  /**
   * Get the X11 display
   * @return the X11 Display
   * @deprecated use QX11Info::display()
   */
  static inline KDE_DEPRECATED Display *getDisplay() { return QX11Info::display(); }
#endif
#endif

  /**
   *  Installs widget filter as global X11 event filter.
   *
   * The widget
   *  filter receives XEvents in its standard QWidget::x11Event() function.
   *
   *  Warning: Only do this when absolutely necessary. An installed X11 filter
   *  can slow things down.
   **/
  void installX11EventFilter( QWidget* filter );

  /**
   * Removes global X11 event filter previously installed by
   * installX11EventFilter().
   */
  void removeX11EventFilter( const QWidget* filter );

#ifdef KDE3_SUPPORT
  /**
   * Generates a uniform random number.
   * @return A truly unpredictable number in the range [0, RAND_MAX)
   * @deprecated Use KRandom::random()
   */
  static inline KDE_DEPRECATED int random() { return KRandom::random(); }

  /**
   * Generates a random string.  It operates in the range [A-Za-z0-9]
   * @param length Generate a string of this length.
   * @return the random string
   * @deprecated use KRandom::randomString() instead.
   */
  static inline KDE_DEPRECATED QString randomString(int length) { return KRandom::randomString(length); }
#endif

  /**
   * Adds a message type to the KIPC event mask. You can only add "system
   * messages" to the event mask. These are the messages with id < 32.
   * Messages with id >= 32 are user messages.
   * @param id The message id. See KIPC::Message.
   * @see KIPC
   * @see removeKipcEventMask()
   * @see kipcMessage()
   */
  void addKipcEventMask(int id);

  /**
   * Removes a message type from the KIPC event mask. This message will
   * not be handled anymore.
   * @param id The message id.
   * @see KIPC
   * @see addKipcEventMask()
   * @see kipcMessage()
   */
  void removeKipcEventMask(int id);

  /**
   * Returns the app startup notification identifier for this running
   * application.
   * @return the startup notification identifier
   */
  QByteArray startupId() const;

  /**
   * @internal
   * Sets a new value for the application startup notification window property for newly
   * created toplevel windows.
   * @param startup_id the startup notification identifier
   * @see KStartupInfo::setNewStartupId
   */
  void setStartupId( const QByteArray& startup_id );

public Q_SLOTS:
  /**
   * Updates the last user action timestamp to the given time, or to the current time,
   * if 0 is given. Do not use unless you're really sure what you're doing.
   * Consult focus stealing prevention section in kdebase/kwin/README.
   */
  Q_SCRIPTABLE void updateUserTimestamp( int time = 0 );

  // D-Bus slots:
  Q_SCRIPTABLE void reparseConfiguration();
  Q_SCRIPTABLE void quit();

public:
  /**
   * Returns the last user action timestamp or 0 if no user activity has taken place yet.
   * @see updateuserTimestamp
   */
  unsigned long userTimestamp() const;

  /**
   * Updates the last user action timestamp in the application registered to DCOP with dcopId
   * to the given time, or to this application's user time, if 0 is given.
   * Use before causing user interaction in the remote application, e.g. invoking a dialog
   * in the application using a DCOP call.
   * Consult focus stealing prevention section in kdebase/kwin/README.
   */
  void updateRemoteUserTimestamp( const QString& service, int time = 0 );

#ifdef KDE3_SUPPORT
    /**
    * Returns the argument to --geometry if any, so the geometry can be set
    * wherever necessary
    * @return the geometry argument, or QString() if there is none
    * @deprecated please use the following code instead:
    *
    * <code>
    * QString geometry;
    * KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    * if (args && args->isSet("geometry"))
    *     geometry = args->getOption("geometry");
    *
    * </code>
    */
  static inline KDE_DEPRECATED QString geometryArgument() {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    return args->isSet("geometry") ? QString::fromLatin1( args->getOption("geometry") ) : QString();
  };
#endif

protected:
  /**
   * @internal Used by KUniqueApplication
   */
  KApplication( bool GUIenabled, KInstance* _instance );

#ifdef Q_WS_X11
  /**
   * @internal Used by KUniqueApplication
   */
  KApplication( Display *display, Qt::HANDLE visual, Qt::HANDLE colormap,
		KInstance* _instance );

  /**
   * Used to catch X11 events
   */
  bool x11EventFilter( XEvent * );
#endif

  Atom kipcCommAtom;
  int kipcEventMask;

  /// Current application object.
  static KApplication *KApp;

private Q_SLOTS:
  void x11FilterDestroyed();
  void checkAppStartedSlot();

private:
  QString sessionConfigName() const;

  KConfig* pSessionConfig; //instance specific application config object
  bool bSessionManagement;
  QWidget *smw;

  void init();
  void parseCommandLine( ); // Handle KDE arguments (Using KCmdLineArgs)
  void read_app_startup_id();

public:
  /**
   * @internal
   */
  bool notify(QObject *receiver, QEvent *event);

  /**
      @internal
    */
  int xErrhandler( Display*, void* );

  /**
      @internal
    */
  int xioErrhandler( Display* );

  /**
   * @internal
   */
  void iceIOErrorHandler( _IceConn *conn );

  /**
   * @internal
   */
  static bool loadedByKdeinit;

  /**
   * @internal
   */
  static void startKdeinit();

  /**
   * Valid values for the settingsChanged signal
   */
  enum SettingsCategory { SETTINGS_MOUSE, SETTINGS_COMPLETION, SETTINGS_PATHS,
         SETTINGS_POPUPMENU, SETTINGS_QT, SETTINGS_SHORTCUTS };

  /**
   * Used to obtain the QPalette that will be used to set the application palette.
   *
   * This is only useful for configuration modules such as krdb and should not be
   * used in normal circumstances.
   * @return the QPalette
   */
  static QPalette createApplicationPalette();

  /**
   * @internal
   * Raw access for use by KDM.
   * note: expects config to be in the correct group already.
   */
  static QPalette createApplicationPalette( KConfigBase *config, int contrast );

Q_SIGNALS:
  /**
   * Emitted when KApplication has changed its palette due to a KControl request.
   *
   * Normally, widgets will update their palette automatically, but you
   * should connect to this to program special behavior.
   */
  void kdisplayPaletteChanged();

  /**
   * Emitted when KApplication has changed its GUI style in response to a KControl request.
   *
   * Normally, widgets will update their styles automatically (as they would
   * respond to an explicit setGUIStyle() call), but you should connect to
   * this to program special behavior.
   */
  void kdisplayStyleChanged();

  /**
   * Emitted when KApplication has changed its font in response to a KControl request.
   *
   * Normally widgets will update their fonts automatically, but you should
   * connect to this to monitor global font changes, especially if you are
   * using explicit fonts.
   *
   * Note: If you derive from a QWidget-based class, a faster method is to
   *       reimplement QWidget::fontChange(). This is the preferred way
   *       to get informed about font updates.
   */
  void kdisplayFontChanged();

  /**
   * Emitted when KApplication has changed either its GUI style, its font or its palette
   * in response to a kdisplay request. Normally, widgets will update their styles
   * automatically, but you should connect to this to program special
   * behavior. */
  void appearanceChanged();

  /**
   * Emitted when the settings for toolbars have been changed. KToolBar will know what to do.
   */
  void toolbarAppearanceChanged(int);

  /**
   * Emitted when the desktop background has been changed by @p kcmdisplay.
   *
   * @param desk The desktop whose background has changed.
   */
  void backgroundChanged(int desk);

  /**
   * Emitted when the global settings have been changed - see KGlobalSettings
   * KApplication takes care of calling reparseConfiguration on KGlobal::config()
   * so that applications/classes using this only have to re-read the configuration
   * @param category the category among the enum above
   */
  void settingsChanged(int category);

  /**
   * Emitted when the global icon settings have been changed.
   * @param group the new group
   */
  void iconChanged(int group);

  /**
   * Emitted when a KIPC user message has been received.
   * @param id the message id
   * @param data the data
   * @see KIPC
   * @see KIPC::Message
   * @see addKipcEventMask
   * @see removeKipcEventMask
   */
  void kipcMessage(int id, int data);

  /**
      Session management asks you to save the state of your application.

     This signal is provided for compatibility only. For new
     applications, simply use KMainWindow. By reimplementing
     KMainWindow::queryClose(), KMainWindow::saveProperties() and
 KMainWindow::readProperties() you can simply handle session
     management for applications with multiple toplevel windows.

     For purposes without KMainWindow, create an instance of
     KSessionManaged and reimplement the functions
     KSessionManaged::commitData() and/or
     KSessionManaged::saveState()

     If you still want to use this signal, here is what you should do:

     Connect to this signal in order to save your data. Do NOT
     manipulate the UI in that slot, it is blocked by the session
     manager.

     Use the sessionConfig() KConfig object to store all your
     instance specific data.

     Do not do any closing at this point! The user may still select
     Cancel  wanting to continue working with your
     application. Cleanups could be done after aboutToQuit().
  */
  void saveYourself();

private:
#ifndef KDE3_SUPPORT
  KConfig *config() { return KInstance::config(); }
#endif
  void propagateSettings(SettingsCategory category);
  void kdisplaySetPalette();
  void kdisplaySetStyle();
  void kdisplaySetFont();
  void applyGUIStyle();

  KApplication(const KApplication&);
  KApplication& operator=(const KApplication&);
protected:
  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );
private:
  //### KDE4: This is to catch invalid implicit conversions, may want to reconsider
  KApplication(bool, bool);

  class Private;
  Private* const d;
};


/**
   Provides highlevel access to session management on a per-object
   base.

   KSessionManaged makes it possible to provide implementations for
 QApplication::commitData() and QApplication::saveState(), without
   subclassing KApplication. KMainWindow internally makes use of this.

   You don't need to do anything with this class when using
   KMainWindow. Instead, use KMainWindow::saveProperties(),
 KMainWindow::readProperties(), KMainWindow::queryClose(),
 KMainWindow::queryExit() and friends.

  @short Highlevel access to session management.
  @author Matthias Ettrich <ettrich@kde.org>
 */
class KDECORE_EXPORT KSessionManaged
{
public:
  KSessionManaged();
  virtual ~KSessionManaged();

    /**
       See QApplication::saveState() for documentation.

       This function is just a convenience version to avoid subclassing KApplication.

       Return true to indicate a successful state save or false to
       indicate a problem and to halt the shutdown process (will
       implicitly call sm.cancel() ).
     */
  virtual bool saveState( QSessionManager& sm );
    /**
       See QApplication::commitData() for documentation.

       This function is just a convenience version to avoid subclassing KApplication.

       Return true to indicate a successful commit of data or false to
       indicate a problem and to halt the shutdown process (will
       implicitly call sm.cancel() ).
     */
  virtual bool commitData( QSessionManager& sm );

protected:
  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );
private:
  class Private;
  Private* d;
};

#endif

