/* This file is part of the KDE libraries
   Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KPOPUP_H
#define _KPOPUP_H

#define INCLUDE_MENUITEM_DEF

#include <Q3PopupMenu>
#include <kpixmapeffect.h>
#include <kpixmap.h>
#include <kdelibs_export.h>

/**
 * @short A menu with keyboard searching and convenience methods for title items.
 *
 * KPopupMenu is a class for menus with standard title items and keyboard
 * accessibility for popups with many options and/or varying options. It acts
 * identically to QMenu, with the addition of insertTitle(),
 * changeTitle(), setKeyboardShortcutsEnabled() and
 * setKeyboardShortcutsExecute() methods.
 *
 * The titles support a text string and an icon.
 *
 * The keyboard search algorithm is incremental with additional underlining
 * for user feedback.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 * @author Hamish Rodda <rodda@kde.org>
 * FIXME KDE4 rename this file to kmenu.h
 */
class KDEUI_EXPORT KPopupMenu : public Q3PopupMenu {
    Q_OBJECT
public:
    /**
     * Constructs a KPopupMenu.
     */
    KPopupMenu(QWidget *parent=0);

    /**
     * Destructs the object
     */
    ~KPopupMenu();

    /**
     * Inserts a title item with no icon.
     */
    QAction* addTitle(const QString &text, QAction* before = 0L);
    /**
     * Inserts a title item with the given icon and title.
     */
    QAction* addTitle(const QIcon &icon, const QString &text, QAction* before = 0L);

    /**
     * Enables keyboard navigation by searching for the entered key sequence.
     * Also underlines the currently selected item, providing feedback on the search.
     *
     * Defaults to off.
     *
     * WARNING: calls to text() of currently keyboard-selected items will
     * contain additional ampersand characters.
     *
     * WARNING: though pre-existing keyboard shortcuts will not interfere with the
     * operation of this feature, they may be confusing to the user as the existing
     * shortcuts will not work.
     * @since 3.1
     */
    void setKeyboardShortcutsEnabled(bool enable);

    /**
     * Enables execution of the menu item once it is uniquely specified.
     * Defaults to off.
     * @since 3.1
     */
    void setKeyboardShortcutsExecute(bool enable);

    /**
     * Returns the context menu associated with this menu
     */
    Q3PopupMenu* contextMenu();

    /**
     * Returns the context menu associated with this menu
     */
    const Q3PopupMenu* contextMenu() const;

    /**
     * Hides the context menu if shown
     * @since 3.2
     */
    void hideContextMenu();

    /**
     * Returns the KPopupMenu associated with the current context menu
     * @since 3.2
     */
    static KPopupMenu* contextMenuFocus();

    /**
     * returns the QAction associated with the current context menu
     * @since 3.2
     */
    static QAction* contextMenuFocusAction();

    /**
     * Return the state of the mouse button and keyboard modifiers
     * when the last menuitem was activated.
     * @since 3.4
     */
    Qt::ButtonState state() const;

    // BEGIN compat methods
    /**
     * Constructs a KPopupMenu.
     */
    KPopupMenu(QWidget *parent, const char *name) KDE_DEPRECATED;

    /**
     * Inserts a title item with no icon.
     */
    int insertTitle(const QString &text, int id=-1, int index=-1) KDE_DEPRECATED;
    /**
     * Inserts a title item with the given icon and title.
     */
    int insertTitle(const QPixmap &icon, const QString &text, int id=-1,
                    int index=-1) KDE_DEPRECATED;
    /**
     * Changes the title of the item at the specified id. If a icon was
     * previously set it is cleared.
     */
    void changeTitle(int id, const QString &text) KDE_DEPRECATED;
    /**
     * Changes the title and icon of the title item at the specified id.
     */
    void changeTitle(int id, const QPixmap &icon, const QString &text) KDE_DEPRECATED;
    /**
     * Returns the title of the title item at the specified id. The default
     * id of -1 is for backwards compatibility only, you should always specify
     * the id.
     */
    QString title(int id=-1) const KDE_DEPRECATED;
    /**
     * Returns the icon of the title item at the specified id.
     */
    QPixmap titlePixmap(int id) const KDE_DEPRECATED;

    /**
     * @deprecated
     * Obsolete method provided for backwards compatibility only. Use the
     * normal constructor and insertTitle instead.
     */
    KPopupMenu(const QString &title, QWidget *parent=0, const char *name=0) KDE_DEPRECATED;

    /**
     * @deprecated
     * Obsolete method provided for backwards compatibility only. Use
     * insertTitle and changeTitle instead.
     */
    void setTitle(const QString &title) KDE_DEPRECATED;

    /**
     * returns the ID of the menuitem associated with the current context menu
     * @since 3.2
     */
    static int contextMenuFocusItem() KDE_DEPRECATED;

    /**
     * Reimplemented for internal purposes
     * @since 3.4
     */
    virtual void activateItemAt(int index) KDE_DEPRECATED;
    // END compat methods

signals:
    /**
     * connect to this signal to be notified when a context menu is about to be shown
     * @param menu The menu that the context menu is about to be shown for
     * @param menuAction The action that the context menu is currently on
     * @param ctxMenu The context menu itself
     * @since 3.2
     */
    void aboutToShowContextMenu(KPopupMenu* menu, QAction* menuAction, QMenu* ctxMenu);
    /// compat
    void aboutToShowContextMenu(KPopupMenu* menu, int menuItem, Q3PopupMenu* ctxMenu);

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void keyPressEvent(QKeyEvent* e);
    /// @since 3.4
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual bool focusNextPrevChild( bool next );
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void hideEvent(QHideEvent*);

    virtual void virtual_hook( int id, void* data );

protected slots:
    /// @since 3.1
    QString underlineText(const QString& text, uint length);
    /// @since 3.1
    void resetKeyboardVars(bool noMatches = false);
    void actionHovered(QAction* action);
    void showCtxMenu(QPoint pos);
    void ctxMenuHiding();
    void ctxMenuHideShowingMenu();

private:
    class KPopupMenuPrivate;
    KPopupMenuPrivate *d;
};

#endif
