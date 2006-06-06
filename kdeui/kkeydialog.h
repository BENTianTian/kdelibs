/* This file is part of the KDE libraries
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 2001,2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KKEYDIALOG_H
#define KKEYDIALOG_H

#include <kdialog.h>
#include <kshortcut.h>

class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QTreeWidgetItem;
class KActionCollection;
class KConfigBase;
class KGlobalAccel;
class KShortcut;
class KKeyChooserItem;
class KAction;

/**
 * @short Widget for configuration of KAccel and KGlobalAccel.
 *
 * Configure dictionaries of key/action associations for KActions,
 * including global shortcuts.
 *
 * The class takes care of all aspects of configuration, including
 * handling key conflicts internally. Connect to the allDefault()
 * slot if you want to set all configurable shortcuts to their
 * default values.
 *
 * @see KKeyDialog
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 * @author Hamish Rodda <rodda@kde.org> (KDE 4 porting)
 */
class KDEUI_EXPORT KKeyChooser : public QWidget
{
	Q_OBJECT

public:
	enum ActionType {
		/// Actions which are triggered by any keypress in the application
		ApplicationAction = 0x1,
		/// Actions which are triggered by any keypress in a window which has the action added to it or its child widget(s)
		WindowAction      = 0x2,
		/// Actions which are triggered by any keypress in a widget which has the action added to it
		WidgetAction      = 0x4,
		/// Actions which are triggered by any keypress in the windowing system
		GlobalAction      = 0x8,
		/// All actions
		AllActions        = 0xffffffff
	};
	Q_DECLARE_FLAGS(ActionTypes, ActionType)
	
	enum LetterShortcuts {
		LetterShortcutsAllowed,
		LetterShortcutsDisallowed
	};

	/**
	 * Constructor.
	 *
	 * @param collection the KActionCollection to configure
	 * @param parent parent widget
	 * @param actionTypes types of actions to display in this widget.
	 * @param allowLetterShortcuts set to LetterShortcutsDisallowed if unmodified alphanumeric
	 *  keys ('A', '1', etc.) are not permissible shortcuts.
	 */
	KKeyChooser( KActionCollection* collection, QWidget* parent, ActionTypes actionTypes = AllActions, LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed );

	/**
	 * \overload
	 *
	 * Creates a key chooser without a starting action collection.
	 *
	 * @param parent parent widget
	 * @param actionTypes types of actions to display in this widget.
	 * @param allowLetterShortcuts set to LetterShortcutsDisallowed if unmodified alphanumeric
	 *  keys ('A', '1', etc.) are not permissible shortcuts.
	 */
	KKeyChooser( QWidget* parent, ActionTypes actionTypes = AllActions, LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed );

	/// Destructor
	virtual ~KKeyChooser();

	/**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * already associated with the KKeyChooser object.
	 * @param title subtree title of this collection of shortcut.
	 */
	bool insert( KActionCollection *, const QString &title = QString());

	/**
	 * This function writes any shortcut changes back to the original
	 * action set(s).
	 */
	void commitChanges();

	/**
	 * This commits and then saves the actions to disk.
	 * Any KActionCollection objects with the xmlFile() value set
	 * will be written to an XML file.  All other will be written
	 * to the application's rc file.
	 */
	void save();

	/**
	 * Checks whether the given shortcut conflicts with global keyboard shortcuts.
	 * If yes, and the warnUser argument is true, warns the user and gives them a chance
	 * to reassign the shortcut from the global shortcut.
	 *
	 * @return true if there was conflict (and the user didn't reassign the shortcut)
	 * @param cut the shortcut that will be checked for conflicts
	 * @param warnUser if true, the user will be warned about a conflict and given a chance
	 *        to reassign the shortcut
	 * @param parent parent widget for the warning dialog
	 */
	static bool checkGlobalShortcutsConflict( const KShortcut& cut, bool warnUser, QWidget* parent );

	/**
	 * Checks whether the given shortcut conflicts with standard keyboard shortcuts.
	 * If yes, and the warnUser argument is true, warns the user and gives them a chance
	 * to reassign the shortcut from the standard shortcut.
	 *
	 * @return true if there was conflict (and the user didn't reassign the shortcut)
	 * @param cut the shortcut that will be checked for conflicts
	 * @param warnUser if true, the user will be warned about a conflict and given a chance
	 *        to reassign the shortcut
	 * @param parent parent widget for the warning dialog
	 */
	static bool checkStandardShortcutsConflict( const KShortcut& cut, bool warnUser, QWidget* parent );

Q_SIGNALS:
	/**
	 * Emitted when an action's shortcut has been changed.
	 **/
	void keyChange();

public Q_SLOTS:
	/**
	 * Resize colums to width required
	 */
	void resizeColumns();

	/**
	 * Set all keys to their default values (bindings).
	 **/
	void allDefault();

// KDE4 a lot of stuff in this class should be probably private:
protected:
	virtual void showEvent(QShowEvent* event);
 
	enum { NoKey = 1, DefaultKey, CustomKey };

	void initGUI( ActionTypes type, LetterShortcuts allowLetterShortcuts );
	void buildListView( uint iList, const QString &title = QString() );

	void updateButtons();
	void setLocalShortcut( const KShortcut& cut );
	void setGlobalShortcut( const KShortcut& cut );
	bool isKeyPresent( const KShortcut& cut, bool warnuser = true );

protected Q_SLOTS:
	void slotLocalNoKey();
	void slotLocalDefaultKey();
	void slotLocalCustomKey();
	void slotLocalCapturedShortcut( const KShortcut& cut );
	
	void slotGlobalNoKey();
	void slotGlobalDefaultKey();
	void slotGlobalCustomKey();
	void slotGlobalCapturedShortcut( const KShortcut& cut );

	void slotListItemSelected( QTreeWidgetItem *item );
	void slotSettingsChanged( int );

private:
	bool isKeyPresentLocally( const KShortcut& cut, KKeyChooserItem* ignoreItem, bool bWarnUser );
	static bool promptForReassign( const QKeySequence& cut, const QString& sAction, ActionType action, QWidget* parent );
	// Remove the key sequences contained in cut from the standard shortcut @p name
	// which currently has @p origCut as shortcut.
	static void removeStandardShortcut( const QString& name, KKeyChooser* chooser, const KShortcut &cut );
	// Remove the key sequences contained in cut from the global shortcut @p name
	// which currently has @p origCut as shortcut.
	static void removeGlobalShortcut( KAction* action, KKeyChooser* chooser, const KShortcut &cut );
	static void readGlobalKeys( QMap< QString, KShortcut >& map );
	static bool checkGlobalShortcutsConflictInternal( const KShortcut& cut, bool bWarnUser, QWidget* parent, const QString& ignoreAction = QString() );
	// Remove the key sequences contained in cut from this item
	bool removeShortcut( const QString& name, const KShortcut &cut );

private Q_SLOTS:
	void captureCurrentItem();


private:
	class KKeyChooserPrivate* const d;
	friend class KKeyDialog;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KKeyChooser::ActionTypes)

/**
 * @short Dialog for configuration of KActionCollection and KGlobalAccel.
 *
 * The KKeyDialog class is used for configuring dictionaries of key/action
 * associations for KActionCollection and KGlobalAccel. It uses the KKeyChooser widget
 * and offers buttons to set all keys to defaults and invoke on-line help.
 *
 * Several static methods are supplied which provide the most convenient interface
 * to the dialog. The most common and most encouraged use is with KActionCollection.
 *
 * \code
 * KKeyDialog::configure( actionCollection() );
 * \endcode
 *
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 * @author Hamish Rodda <rodda@kde.org> (KDE 4 porting)
 */
class KDEUI_EXPORT KKeyDialog : public KDialog
{
	Q_OBJECT

public:
	/**
	 * Constructs a KKeyDialog as a child of @p parent.
	 * Set @p bAllowLetterShortcuts to false if unmodified alphanumeric
	 * keys ('A', '1', etc.) are not permissible shortcuts.
	 */
	KKeyDialog( KKeyChooser::ActionTypes types = KKeyChooser::AllActions, KKeyChooser::LetterShortcuts allowLetterShortcuts = KKeyChooser::LetterShortcutsAllowed, QWidget* parent = 0 );

	/**
	 * Destructor. Deletes all resources used by a KKeyDialog object.
	 */
	virtual ~KKeyDialog();

	/**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * displayed by the dialog.
	 * Simply call insert with the action collections of each one in turn.
	 *
	 * @param title the title associated with the collection (if null, the
	 * KAboutData::progName() of the collection's instance is used)
	 */
	void insert(KActionCollection *, const QString &title = QString());

	/**
	 * Run the dialog and call commitChanges() if @p bSaveSettings
	 * is true.
	 */
	bool configure( bool bSaveSettings = true );

	/**
	 * Commit key setting changes so that changed settings actually become active.
	 * This method is implicitly called from configure(bool) if
	 * @p bSaveSettings is true.
	 */
	void commitChanges();

	/**
	 * Pops up a modal dialog for configuring key settings. The new
	 * shortcut settings will be active if the user presses OK.  If
	 * @p bSaveSettings if true, the settings will also be saved back to
	 * the *uirc file which they were intially read from.
	 *
	 * @param coll the KActionCollection to configure
	 * @param allowLetterShortcuts set to KKeyChooser::LetterShortcutsDisallowed if unmodified alphanumeric
	 *  keys ('A', '1', etc.) are not permissible shortcuts.
	 * @param parent the parent widget to attach to
	 * @param bSaveSettings if true, the settings will also be saved back to
	 * the *uirc file which they were intially read from.
	 *
	 * @return Accept if the dialog was closed with OK, Reject otherwise.
	 */
	static int configure( KActionCollection* coll, KKeyChooser::LetterShortcuts allowLetterShortcuts = KKeyChooser::LetterShortcutsAllowed, QWidget* parent = 0, bool bSaveSettings = true );

private:
	class KKeyDialogPrivate* d;
	KKeyChooser* m_keyChooser;
};

#endif // KKEYDIALOG_H
