/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997-2000 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>

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

#ifndef _KACCEL_H
#define _KACCEL_H

#include <qdict.h>
#include <qstring.h>
#include <qaccel.h>
#include <kstdaccel.h>

class QPopupMenu;
class KConfig;
class QObject;
class QWidget;
class KAccelPrivate;

/**
 * Accelerator information, similar to an action.
 *
 * It is used internally by @ref KAccel.
 * @internal
 * @deprecated
 */
struct KKeyEntry {
    int aCurrentKeyCode;
    int aDefaultKeyCode;
    int aConfigKeyCode;
    bool bConfigurable;
    bool bEnabled;
    int aAccelId;
    const QObject *receiver;
    const char *member;
    QString descr;
    int menuId;
    QPopupMenu *menu;
};

/**
 * Handle keyboard accelerators.
 *
 * Allow an user to configure
 * key bindings through application configuration files or through the
 * @ref KKeyChooser GUI.
 *
 * A @ref KAccel contains a list of accelerator items. Each accelerator item
 * consists of an action name and a keyboard code combined with modifiers
 * (Shift, Ctrl and Alt.)
 *
 * For example, "Ctrl+P" could be a shortcut for printing a document. The key
 * codes are listed in ckey.h. "Print" could be the action name for printing.
 * The action name identifies the key binding in configuration files and the
 * @ref KKeyChooser GUI.
 *
 * When pressed, an accelerator key calls the slot to which it has been
 * connected. Accelerator items can be connected so that a key will activate
 * two different slots.
 *
 * A KAccel object handles key events sent to its parent widget and to all
 * children of this parent widget.
 *
 * Key binding reconfiguration during run time can be prevented by specifying
 * that an accelerator item is not configurable when it is inserted. A special
 * group of non-configurable key bindings are known as the
 * standard accelerators.
 *
 * The standard accelerators appear repeatedly in applications for
 * standard document actions such as printing and saving. Convenience methods are
 * available to insert and connect these accelerators which are configurable on
 * a desktop-wide basis.
 *
 * It is possible for a user to choose to have no key associated with
 * an action.
 *
 * The translated first argument for @ref insertItem() is used only
 * in the configuration dialog.
 *<pre>
 * KAccel *a = new KAccel( myWindow );
 * // Insert an action "Scroll Up" which is associated with the "Up" key:
 * a->insertItem( i18n("Scroll up"), "Scroll Up", "Up" );
 * // Insert an action "Scroll Down" which is not associated with any key:
 * a->insertItem( i18n("Scroll down"), "Scroll Down", 0);
 * a->connectItem( "Scroll up", myWindow, SLOT( scrollUp() ) );
 * // a->insertStdItem( KStdAccel::Print ); //not necessary, since it
 *	// is done automatially with the
 *	// connect below!
 * a->connectItem(KStdAccel::Print, myWindow, SLOT( printDoc() ) );
 *
 * a->readSettings();
 *</pre>
 *
 * If a shortcut has a menu entry as well, you could insert them like
 * this. The example is again the @ref KStdAccel::Print from above.
 *
 * <pre>
 * int id;
 * id = popup->insertItem("&Print",this, SLOT(printDoc()));
 * a->changeMenuAccel(popup, id, KStdAccel::Print );
 * </pre>
 *
 * If you want a somewhat "exotic" name for your standard print action, like
 *   id = popup->insertItem(i18n("Print &Document"),this, SLOT(printDoc()));
 * it might be a good idea to insert the standard action before as
 *          a->insertStdItem( KStdAccel::Print, i18n("Print Document") )
 * as well, so that the user can easily find the corresponding function.
 *
 * This technique works for other actions as well.  Your "scroll up" function
 * in a menu could be done with
 *
 * <pre>
 *    id = popup->insertItem(i18n"Scroll &up",this, SLOT(scrollUp()));
 *    a->changeMenuAccel(popup, id, "Scroll Up" );
 * </pre>
 *
 * Please keep the order right:  First insert all functions in the
 * acceleratior, then call a -> @ref readSettings() and @em then build your
 * menu structure.
 *
 * @short Configurable key binding support.
 * @version $Id$
 */
class KAccel : public QAccel
{
 Q_OBJECT	
	
 public:
	
	/**
	 * Create a KAccel object with a parent widget and a name.
	 */
	KAccel( QWidget * parent, const char *name = 0 );
	
	/**
	 * Remove all accelerator items.
	 */
	void clear();
	
	/**
	 * Connect an accelerator item to a slot/signal in another object.
	 *
	 * Arguments:
	 *
	 *  @param action The accelerator item action name.
	 *  @param receiver The object to receive a signal.
	 *  @param member A slot or signal in the receiver.
	 *  @param activate Indicates whether the accelerator item should be
	 *  enabled immediately.
	 */
	void connectItem( const QString& action,
					 const QObject* receiver, const char *member,
					 bool activate = true );

    
	/**
	 * Same as the preceding @ref connectItem(), but used for standard
	 * accelerators.
	 *
	 * If the standard accelerator was not inserted so far, it
	 * will be inserted automatically.
	 */
	void connectItem( KStdAccel::StdAccel accel,
					 const QObject* receiver, const char *member,
					 bool activate = true );

	/**
	* Retrieve the number of accelerator items.
	*/					
	uint count() const;
	
	/**
	* Retrieve the key code of the accelerator item with the action name
	* @p action, or zero if either the action name cannot be
	* found or the current key is set to no key.
	*/
	int currentKey( const QString& action ) const;

	/**
	 * Retrieve the description  of the accelerator item with the
	 * action name @p action, or @ref QString::null if the action name cannot
	 * be found. Useful for menus.
	 */
    QString description( const QString& action ) const;
	void setDescription(const QString &action, const QString &description);

	/**
	* Retrieve the default key code of the accelerator item with
	* the action name
	* @p action, or zero if the action name cannot be found.
	*/
	int defaultKey( const QString& action) const;
	
	/**
	 * Disconnect an accelerator item from a function in another object.
	 */
	void disconnectItem( const QString& action,
						const QObject* receiver, const char *member );
	
	/**
	 * Rerieve the identifier of the accelerator item with the keycode @p key,
	 * or @ref QString::null if the item cannot be found.
	 */
	QString findKey( int key ) const;
	
	/**
	 * Insert an accelerator item.
	 *
	 * If an action already exists the old association and connections
	 * will be removed.
	 *
	 *  @param descr The localized name of the action, useful in
	 *  menus or the keybinding editor.
	 *  @param action The internal accelerator item action name. It
	 *  is supposed to be the same for all languages.
	 *  @param defaultKeyCode A key code to be used as the default
	 *  for the action.  Set it to 0 for no default key (It still
	 *  may be configured later.)
	 *  @param configurable Indicates whether a user can configure
	 *  the key binding using the @ref KKeyChooser GUI and whether the
	 *  key will be written back to configuration files when
	 *  @ref writeSettings() is called.
	 *  @return @p true if successful.
	 */
	bool insertItem( const QString& descr, const QString& action, 
					int defaultKeyCode, bool configurable = true );
	
	/**
	 * Insert an accelerator item.
	 *
	 * If an action already exists the old association and connections
	 * will be removed.
	 *
	 *  @param descr The localized name of the action, useful in
	 *  menus or the keybinding editor.
	 *  @param action The internal accelerator item action name. It
	 *  is supposed to be the same for all languages.
	 *  @param defaultKeyCode A key code to be used as the default
	 *  for the action.  Set it to 0 for no default key (It still
	 *  may be configured later.)
	 *  @param id Menu index of menu items associated with this action.
	 *  @param qmenu Menu containing items associated with this action.
	 *  @param configurable Indicates whether a user can configure
	 *  the key binding using the @ref KKeyChooser GUI and whether the
	 *  key will be written back to configuration files when
	 *  @ref writeSettings() is called.
	 *  @return @p true if successful.
	 *
	 */
	bool insertItem( const QString& descr, const QString& action, 
					int defaultKeyCode, int id, QPopupMenu *qmenu, 
					bool configurable = true );
	
	 /**
	 * Insert an accelerator item.
	 *
	 * If an action already exists the old association and connections
	 * will be removed.
	 *	
	 *  @param descr The localized name of the action, useful in
	 *  menus or the keybinding editor.
	 *  @param action The internal accelerator item action name. It
	 *  is supposed to be the same for all languages.
	 *  @param defaultKeyCode A key plus a combination of Shift, Ctrl
	 *	and Alt to be used as the default for the action.
	 *  @param id Menu index of menu items associated with this action.
	 *  @param qmenu Menu containing items associated with this action.
	 *  @param configurable Indicates whether a user can configure
	 *  the key binding using the @ref KKeyChooser GUI and whether the
	 *  key will be written back to configuration files when
	 *  @ref writeSettings() is called.
	 *  @return @p true if successful.
	 *
	 */
	bool insertItem( const QString& descr, const QString& action,
					const QString& defaultKeyCode,
					bool configurable = true );
	
	 /**
	 * Insert an accelerator item.
	 *	
	 * If an action already exists the old association and connections
	 * will be removed..
	 *
	 *  @param descr The localized name of the action, useful in
	 *  menus or the keybinding editor.
	 *  @param action The internal accelerator item action name. It
	 *  is supposed to be the same for all languages.
	 *  @param defaultKeyCode A key plus a combination of Shift, Ctrl
	 *	and Alt to be used as the default for the action.
	 *  @param configurable Indicates whether a user can configure
	 *  the key binding using the @ref KKeyChooser GUI and whether the
	 *  key will be written back to configuration files when
	 *  @ref writeSettings() is called.
	 *  @return @p true if successful.
	 *
	 */
	bool insertItem( const QString& descr, const QString& action,
					const QString& defaultKeyCode,
					int id, QPopupMenu *qmenu, bool configurable = true );
				
	/**
	 * Insert a standard accelerator item.
	 *
	 * If an action already exists the old association and connections
	 * will be removed.
	 * param id One of the following: @tt Open,
	 *	@tt New, @tt Close, @tt Save, @tt Print, @tt Quit, @tt Cut, @tt Copy, @tt Paste, @tt Undo, @tt Redo,
	 *	@tt Find, @tt Replace, @tt Insert, @tt Home, @tt End, @tt Prior, @tt Next, or @ttHelp.
	 * param descr You can optionally also assign a description to
	 * the standard item which may be used a in a popup menu.
	 */
	bool insertStdItem( KStdAccel::StdAccel id, const QString& descr = QString::null );


	/**
	 * Convenience function form of @ref insertItem() 
	 * without the need to specify a localized
	 * function name for the user.
	 *
	 * This is useful if the accelerator
	 * is used internally only, without appearing in a menu or a
	 * keybinding editor.
	 */
	bool insertItem( const QString& action, int defaultKeyCode,
				 bool configurable = true );

	/**
	 * Convenience function for of @ref insertItem() without the need
	 * to specify a localized
	 * function name for the user.
	 *
	 * This is useful if the accelerator
	 * is only used internally, without appearing in a menu or a
	 * keybinding editor.
	 */
	bool insertItem( const QString& action, int defaultKeyCode,
				 int id, QPopupMenu *qmenu, 
				 bool configurable = true );

 	/**
	 * Remove the accelerator item with the action name action.
	 */
	void removeItem( const QString& action );

	/**
	 * Shortcuts should be visible in the menu
	 * structure of an application.
	 *
	 * Use this function for that
	 * purpose.  Note that the action must have been inserted
	 * before!
	 */

	void changeMenuAccel ( QPopupMenu *menu, int id,
			       const QString& action );
	/**
	 * Same as @ref changeMenuAccel() but for standard accelerators.
	 */
	void changeMenuAccel ( QPopupMenu *menu, int id,
						  KStdAccel::StdAccel accel );

	/**
	 * Set the dictionary of accelerator action names and @ref KKeyEntry
	 * objects to @p nKeyDict.
	 *
	 * Note that only a shallow copy is made so
	 * that items will be lost when the @ref KKeyEntry objects are deleted.
	 */	
	bool setKeyDict( QDict<KKeyEntry> nKeyDict );
	
	/**
	 * Retrieve the dictionary of accelerator action names and 
	 * @ref KKeyEntry
	 * objects. Note that only a shallow copy is returned so that
	 * items will be lost when the @ref KKeyEntry objects are deleted.
	 */
	QDict<KKeyEntry> keyDict();
				
	/**
	 * Read all key associations from @p config, or (if @p config
	 * is zero) from the application's configuration file
	 * @ref KGlobal::config().
	 *
	 * The group in which the configuration is stored can be
	 * set with @ref setConfigGroup().
	 */	
	void readSettings(KConfig* config = 0);

	/**
	 * Write the current configurable associations to @p config,
         * or (if @p config is zero) to the application's
	 * configuration file.
	 */	
	void writeSettings(KConfig* config = 0);
	
	/**
	 * Set the group in the configuration file in which the
	 * accelerator settings are stored.
	 *
	 * By default, this is "Keys".
	 */
	void setConfigGroup( const QString& group );

	/**
	 * Retrieve the name of the group in which accelerator
	 * settings are stored.
	 **/
	QString configGroup() const;

	/**
	 * If @global is @true, KAccel writes to the global
	 *  configurtion file, instead of the application configuration file.
	 **/
	void setConfigGlobal( bool global );
	/**
	 * Will KAccel write to the global configuration file (instead of
	 *  the application configuration file)?
	 **/
	bool configGlobal() const;
	
	/**
	 * Enable all accelerators if activate is @p true, or disable it if
	 * activate is @p false.
	 *
	 * Individual keys can also be enabled or disabled.
	 */
	void setEnabled( bool activate );
	/**
	 * Are accelerators enabled?
	 **/
	bool isEnabled() const;
	
	/**
	 * Enable or disable an accelerator item.
	 *
	 * @param action The accelerator item action name.
	 * @param activate Specifies whether the item should be enabled or
	 *	disabled.
	 */
	void setItemEnabled( const QString& action, bool activate );
	/**
	 * Check whether a specific accelerator, @p action, is enabled.
	 **/
	bool isItemEnabled( const QString& action ) const;

	/**
	 * Returns @p true if keyentry can be modified.
	 */
	bool configurable( const QString &action ) const;

	/**
	 *  Change the keycode for an accelerator.
	 */
	bool updateItem( const QString &action, int keyCode);

	/**
	 *  Remove the keycode for an accelerator.
	 **/
	void clearItem(const QString &action);

	/**
	 *  Clear any pointers to a menu.
	 **/
	void removeDeletedMenu(QPopupMenu *menu);
	
	/**
	 * Retrieve the key code corresponding to the string @p sKey or
	 * zero if the string is not recognized.
	 *
	 * The string must be something like "Shift+A",
	 * "F1+Ctrl+Alt" or "Backspace" for example. That is, the string
	 * must consist of a key name plus a combination of
	 * the modifiers Shift, Ctrl and Alt.
	 *
	 * N.B.: @p sKey must @em not be @ref i18n()'d!
	 */	
	static int stringToKey( const QString& sKey );
	
	/**
	 * Retrieve a string corresponding to the key code @p keyCode,
	  * which is empty if
	 * @p keyCode is not recognized or zero.
	 */
	static QString keyToString( int keyCode, bool i18_n = false );

 signals:
	void keycodeChanged();
	
 protected:
	int aAvailableId;
	QDict<KKeyEntry> aKeyDict;
	bool bEnabled;
	bool bGlobal;
	QString aGroup;
 private:
    KAccelPrivate *d;
};
	
#endif
