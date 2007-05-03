/* This file is part of the KDE libraries
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 2001,2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#ifndef KSHORTCUTSEDITOR_H
#define KSHORTCUTSEDITOR_H

#include <QtGui/QWidget>
#include "kgesture.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QTreeWidgetItem;
class KActionCollection;
class KConfigBase;
class KGlobalAccel;
class KShortcut;
class KShortcutsEditorItem;
class KShortcutsEditorPrivate;

// KShortcutsEditor expects that the list of existing shortcuts is already
// free of conflicts. If it is not, nothing will crash, but your users
// won't like the resulting behavior.


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
 * @see KShortcutsDialog
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 * @author Hamish Rodda <rodda@kde.org> (KDE 4 porting)
 */
class KDEUI_EXPORT KShortcutsEditor : public QWidget
{
	Q_OBJECT

public:
	enum ActionType {
		/// Actions which are triggered by any keypress in a widget which has the action added to it
		WidgetAction      = Qt::WidgetShortcut      /*0*/,
		/// Actions which are triggered by any keypress in a window which has the action added to it or its child widget(s)
		WindowAction      = Qt::WindowShortcut      /*1*/,
		/// Actions which are triggered by any keypress in the application
		ApplicationAction = Qt::ApplicationShortcut /*2*/,
		/// Actions which are triggered by any keypress in the windowing system
		GlobalAction      = 4,
		/// All actions
		AllActions        = 0xffffffff
	};
	Q_DECLARE_FLAGS(ActionTypes, ActionType)

	enum LetterShortcuts {
		/// Shortcuts without a modifier are not allowed,
		/// so 'A' would not be valid, whereas 'Ctrl+A' would be.
		/// This only applies to printable characters, however.
		/// 'F1', 'Insert' etc. could still be used.
		LetterShortcutsDisallowed = 0,
		/// Letter shortcuts are allowed
		LetterShortcutsAllowed
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
	KShortcutsEditor(KActionCollection *collection, QWidget *parent, ActionTypes actionTypes = AllActions, LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed);

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
	explicit KShortcutsEditor( QWidget* parent, ActionTypes actionTypes = AllActions, LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed );

	/// Destructor
	virtual ~KShortcutsEditor();

	/**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * already associated with the KShortcutsEditor object.
	 * @param title subtree title of this collection of shortcut.
	 */
	void addCollection(KActionCollection *, const QString &title = QString());

	/**
	 * This function writes any shortcut changes back to the original
	 * action set(s).
	 */
	//void commitChanges(); //we do it the other way around

    /**
	 * This function reverts any shortcut changes to the original
	 * action set(s). Not implemented yet.
	 */
    void undoChanges();

	/**
	 * This saves the actions to disk.
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
	 * Set all shortcuts to their default values (bindings).
	 **/
	void allDefault();

protected:
	virtual void showEvent(QShowEvent* event);

private:
	Q_PRIVATE_SLOT(d, void capturedShortcut(QVariant, const QModelIndex &))

	Q_PRIVATE_SLOT(d, void globalSettingsChangedSystemwide(int))

private:
	friend class KShortcutsDialog;
	friend class KShortcutsEditorPrivate;
	KShortcutsEditorPrivate *const d;
	Q_DISABLE_COPY(KShortcutsEditor)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KShortcutsEditor::ActionTypes)

#endif // KSHORTCUTSEDITOR_H
