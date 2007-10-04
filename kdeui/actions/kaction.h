/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KACTION_H
#define KACTION_H

#include <kdeui_export.h>
#include <kguiitem.h>
#include <kshortcut.h>

#include <QtGui/QWidgetAction>

class KIcon;
class KShapeGesture;
class KRockerGesture;

//TODO Reduce the word count. This is not very focused and takes too long to read.
//Keep in mind that QAction also has documentation that we don't need to repeat here.
/**
 * @short Class to encapsulate user-driven action or event
 *
 * The KAction class (and derived and super classes) extends QAction,
 * which provides a way to easily encapsulate a "real" user-selected
 * action or event in your program.
 *
 * For instance, a user may want to @p paste the contents of
 * the clipboard, @p scroll @p down a document, or @p quit the
 * application.  These are all \b actions -- events that the
 * user causes to happen.  The KAction class allows the developer to
 * deal with these actions in an easy and intuitive manner, and conforms
 * to KDE's extended functionality requirements - including supporting
 * multiple user-configurable shortcuts, and KDE named icons.  Actions
 * also improve accessibility.
 *
 * Specifically, QAction (and thus KAction) encapsulates the various attributes
 * of an event/action.  For instance, an action might have an icon()
 * that provides a visual representation (a clipboard for a "paste" action or
 * scissors for a "cut" action).  The action should also be described by some text().
 * It will certainly be connected to a method that actually @p executes the action!
 * All these attributes are contained within the action object.
 *
 * The advantage of dealing with actions is that you can manipulate
 * the Action without regard to the GUI representation of it.  For
 * instance, in the "normal" way of dealing with actions like "cut",
 * you would manually insert a item for Cut into a menu and a button
 * into a toolbar.  If you want to disable the cut action for a moment
 * (maybe nothing is selected), you would have to hunt down the pointer
 * to the menu item and the toolbar button and disable both
 * individually.  Setting the menu item and toolbar item up uses very
 * similar code - but has to be done twice!
 *
 * With the action concept, you simply add the action to whatever
 * GUI element you want.  The KAction class will then take care of
 * correctly defining the menu item (with icons, accelerators, text,
 * etc), toolbar button, or other.  From then on, if you
 * manipulate the action at all, the effect will propogate through all
 * GUI representations of it.  Back to the "cut" example: if you want
 * to disable the Cut Action, you would simply call
 * 'cutAction->setEnabled(false)' and both the menuitem and button would
 * instantly be disabled!
 *
 * This is the biggest advantage to the action concept -- there is a
 * one-to-one relationship between the "real" action and @p all
 * GUI representations of it.
 *
 * KAction emits the hovered() signal on mouseover, and the triggered(bool checked)
 * signal on activation of a corresponding GUI element ( menu item, toolbar button, etc. )
 *
 * If you are in the situation of wanting to map the triggered()
 * signal of multiple action objects to one slot, with a special
 * argument bound to each action, you have several options:
 *
 * Using QActionGroup:
 * \li Create a QActionGroup and assign it to each of the actions with setActionGroup(), then
 * \li Connect the QActionGroup::triggered(QAction*) signal to your slot.
 *
 * Using QSignalMapper:
 * \code
 * QSignalMapper *desktopNumberMapper = new QSignalMapper( this );
 * connect( desktopNumberMapper, SIGNAL( mapped( int ) ),
 *          this, SLOT( moveWindowToDesktop( int ) ) );
 *
 * for ( uint i = 0; i < numberOfDesktops; ++i ) {
 *     KAction *desktopAction = new KAction( i18n( "Move Window to Desktop %i" ).arg( i ), ... );
 *     connect( desktopAction, SIGNAL( triggered(bool) ), desktopNumberMapper, SLOT( map() ) );
 *     desktopNumberMapper->setMapping( desktopAction, i );
 * }
 * \endcode
 *
 * \section kaction_general General Usage
 *
 * The steps to using actions are roughly as follows:
 *
 * @li Decide which attributes you want to associate with a given
 *     action (icons, text, keyboard shortcut, etc)
 * @li Create the action using KAction (or derived or super class).
 * @li Add the action into whatever GUI element you want.  Typically,
 *      this will be a menu or toolbar.
 *
 * \section kaction_general The kinds of shortcuts
 *
 * Local shortcuts are active
 * in their context, global shortcus are active everywhere, usually even if
 * another program has focus.
 *
 * @li Active shortcuts trigger a KAction if activated.
 * @li Default shortcuts are what the active shortcuts revert to if the user chooses
 * to reset shortcuts to default.
 *
 * \section kaction_example Detailed Example
 *
 * Here is an example of enabling a "New [document]" action
 * \code
 * KAction *newAct = new KAction("filenew", i18n("&New"), actionCollection(), "new");
 * newAct->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::New));
 * connect(newAct, SIGNAL(triggered(bool)), SLOT(fileNew()));
 * \endcode
 *
 * This section creates our action.  It says that wherever this action is
 * displayed, it will use "&New" as the text, the standard icon, and
 * the standard shortcut.  It further says that whenever this action
 * is invoked, it will use the fileNew() slot to execute it.
 *
 * \code
 * QMenu *file = new QMenu;
 * file->addAction(newAct);
 * \endcode
 * That just inserted the action into the File menu.  The point is, it's not
 * important in which menu it is: all manipulation of the item is
 * done through the newAct object.
 *
 * \code
 * toolBar()->addAction(newAct);
 * \endcode
 * And this added the action into the main toolbar as a button.
 *
 * That's it!
 *
 * If you want to disable that action sometime later, you can do so
 * with
 * \code
 * newAct->setEnabled(false)
 * \endcode
 * and both the menuitem in File and the toolbar button will instantly
 * be disabled.
 *
 * Unlike with previous versions of KDE, the action can simply be deleted
 * when you have finished with it - the destructor takes care of all
 * of the cleanup.
 *
 * \note calling QAction::setShortcut() on a KAction may lead to unexpected
 * behavior. There is nothing we can do about it because QAction::setShortcut()
*  is not virtual.
 *
 * \note if you are using a "standard" action like "new", "paste",
 * "quit", or any other action described in the KDE UI Standards,
 * please use the methods in the KStdAction class rather than
 * defining your own.
 *
 * \section kaction_xmlgui Usage Within the XML Framework
 *
 * If you are using KAction within the context of the XML menu and
 * toolbar building framework, you do not ever
 * have to add your actions to containers manually.  The framework
 * does that for you.
 *
 * @see KStdAction
 */
class KDEUI_EXPORT KAction : public QWidgetAction
{
  Q_OBJECT

  Q_PROPERTY( KShortcut shortcut READ shortcut WRITE setShortcut )
  Q_PROPERTY( bool shortcutConfigurable READ isShortcutConfigurable WRITE setShortcutConfigurable )
  Q_PROPERTY( KShortcut globalShortcut READ globalShortcut WRITE setGlobalShortcut )
  Q_PROPERTY( bool globalShortcutAllowed READ globalShortcutAllowed WRITE setGlobalShortcutAllowed )
  Q_FLAGS( ShortcutType )

public:
    /**
     * An enumeration about the two types of shortcuts in a KAction
     */
    enum ShortcutType {
      /// The shortcut will immediately become active but may be reset to "default".
      ActiveShortcut = 0x1,
      /// The shortcut is a default shortcut - it becomes active when somebody decides to
      /// reset shortcuts to default.
      DefaultShortcut = 0x2
    };
    Q_DECLARE_FLAGS(ShortcutTypes, ShortcutType)

    /**
     * An enum about global shortcut setter semantics
     */
    //This enum will be ORed with ShortcutType in calls to KGlobalAccel, so it must not contain
    //any value equal to a value in ShortcutType.
    enum GlobalShortcutLoading {
      /// Look up the action in global settings (using its main component's name and text())
      /// and set the shortcut as saved there.
      /// @see setGlobalShortcut()
      Autoloading = 0x0,
      /// Prevent autoloading of saved global shortcut for action
      NoAutoloading = 0x4
    };
    /**
     * Constructs an action.
     */
    explicit KAction(QObject *parent);

    /**
     * Constructs an action with the specified parent and visible text.
     *
     * @param text The visible text for this action.
     * @param parent The parent for this action.
     */
    KAction(const QString& text, QObject *parent);

    /**
     * Constructs an action with text and icon; a shortcut may be specified by
     * the ampersand character (e.g. \"&amp;Option\" creates a shortcut with key \e O )
     *
     * This is the other common KAction constructor used.  Use it when you
     * \e do have a corresponding icon.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent The parent for this action.
     */
    KAction(const KIcon& icon, const QString& text, QObject *parent);

    /**
     * Standard destructor
     */
    virtual ~KAction();

    /**
     * Get the shortcut for this action.
     *
     * This is preferred over QAction::shortcut(), as it allows for multiple shortcuts
     * per action. The first and second shortcut as reported by shortcuts() will be the
     * primary and alternate shortcut of the shortcut returned.
     *
     * \param types the type of shortcut to return. Should both be specified, only the
     *             active shortcut will be returned. Defaults to the active shortcut, if one exists.
     * \sa shortcuts()
     */
    KShortcut shortcut(ShortcutTypes types = ActiveShortcut) const;

    /**
     * Set the shortcut for this action.
     *
     * This is preferred over QAction::setShortcut(), as it allows for multiple shortcuts
     * per action.
     *
     * \param shortcut shortcut(s) to use for this action in its specified shortcutContext()
     * \param type type of shortcut to be set: active shortcut,
     *  default shortcut, or both (the default).
     */
    void setShortcut(const KShortcut& shortcut, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));

    /**
     * \overload void setShortcut(const KShortcut& shortcut)
     *
     * Set the primary shortcut only for this action.
     *
     * This function is there to explicitly override QAction::setShortcut(const QKeySequence&).
     * QAction::setShortcut() will bypass everything in KAction and may lead to unexpected behavior.
     *
     * \param shortcut shortcut(s) to use for this action in its specified shortcutContext()
     * \param type type of shortcut to be set: active shortcut,
     *  default shortcut, or both (default argument value).
     */
    void setShortcut(const QKeySequence& shortcut, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));

    /**
     * Returns true if this action's shortcut is configurable.
     */
    bool isShortcutConfigurable() const;

    /**
     * Indicate whether the user may configure the action's shortcut.
     *
     * \param configurable set to \e true if this shortcut may be configured by the user, otherwise \e false.
     */
    void setShortcutConfigurable(bool configurable);

    /**
     * Get the global shortcut for this action, if one exists. Global shortcuts
     * allow your actions to respond to accellerators independently of the focused window.
     * Unlike regular shortcuts, the application's window does not need focus
     * for them to be activated.
     *
     * \param type the type of shortcut to be returned. Should both be specified, only the
     *             active shortcut will be returned.  Defaults to the active shortcut,
     *             if one exists.
     *
     * \sa KGlobalAccel
     * \sa setGlobalShortcut()
     */
    const KShortcut& globalShortcut(ShortcutTypes type = ActiveShortcut) const;

    /**
     * Assign a global shortcut for this action. Global shortcuts
     * allow your actions to respond to keys independently of the focused window.
     * Unlike regular shortcuts, the application's window does not need focus
     * for them to be activated.
     *
     * When an action, identified by main component name and text(), is assigned
     * a global shortcut for the first time on a KDE installation the assignment will
     * be saved and restored every time the action's globalShortcutAllowed flag
     * becomes true. \e This \e includes \e calling \e setGlobalShortcut()!
     * If you actually want to change the global shortcut you have to set
     * @p loading to NoAutoloading. The new shortcut will be saved again.
     * The only way to forget the action's global shortcut is to do
     * \code
     * setGlobalShortcut(KShortcut(), KAction::ActiveShortcut | KAction::DefaultShortcut,
     *                   KAction::NoAutoloading)
     * \endcode
     * \param shortcut global shortcut(s) to assign
     * \param type the type of shortcut to be set, whether the active shortcut, the default shortcut,
     *             or both (the default).
     * \param loading load the previous shortcut (Autoloading, the default) or really set a new
     *                shortcut (NoAutoloading).
     *
     * \sa KGlobalAccel
     * \sa globalShortcut()
     */
    void setGlobalShortcut(const KShortcut& shortcut, ShortcutTypes type =
                           ShortcutTypes(ActiveShortcut | DefaultShortcut),
                           GlobalShortcutLoading loading = Autoloading);

    /**
     * Returns true if this action is permitted to have a global shortcut.
     * Defaults to false.
     */
    bool globalShortcutAllowed() const;

    /**
     * Indicate whether the programmer and/or user may define a global shortcut for this action.
     * Defaults to false. Note that calling setGlobalShortcut() turns this on automatically.
     *
     * \param allowed set to \e true if this action may have a global shortcut, otherwise \e false.
     */
    void setGlobalShortcutAllowed(bool allowed, GlobalShortcutLoading loading = Autoloading);
    //^ TODO: document autoloading

    KShapeGesture shapeGesture(ShortcutTypes type = ActiveShortcut) const;
    KRockerGesture rockerGesture(ShortcutTypes type = ActiveShortcut) const;

    void setShapeGesture(const KShapeGesture& gest, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));
    void setRockerGesture(const KRockerGesture& gest, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));

Q_SIGNALS:
#ifdef KDE3_SUPPORT
    /**
     * Emitted when this action is activated
     *
     * \deprecated use triggered(bool checked) instead.
     */
    QT_MOC_COMPAT void activated();
#endif

    /**
     * Emitted when the action is triggered. Also provides the state of the
     * keyboard modifiers and mouse buttons at the time.
     */
    void triggered(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

private:
    friend class KGlobalAccel;
    Q_PRIVATE_SLOT(d, void slotTriggered())
    class KActionPrivate* const d;
    friend class KActionPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAction::ShortcutTypes)

#endif
