/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              (C) 1999, 2000 Kurt Granroth (granroth@kde.org)

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
#ifndef _KTOOLBAR_H
#define _KTOOLBAR_H

#include <qframe.h>
#include <qcombobox.h>
#include <kiconloader.h>

class QSize;
class QPixmap;
class QPopupMenu;
class QStringList;

class KLineEdit;
class KToolBar;
class KToolBarItemList;
class KToolBoxManager;
class KAnimWidget;
class KPopupMenu;

class KToolBarPrivate;

 /**
  * A KDE-style toolbar.
  *
  * KToolBar can be "floated", dragged and docked from its parent window.
  *
  * A KToolBar can contain standard or toggle buttons, line edit widgets,
  * combo boxes, frames or any developer-defined custom widget, with
  * automatic full-width resize for each widget (except buttons). Buttons
  * can also be connected to popup menus with a delay option.
  *
  * KToolBar can be used as a standalone widget, but @ref KTMainWindow
  * provides easy factories and management of one or more toolbars.
  * Once you have a KToolBar object, you can insert items into it with the
  * insert... methods, or remove them with the @ref removeItem() method. This
  * can be done at any time; the toolbar will be automatically updated.
  * There are also many methods to set per-child properties like alignment
  * and toggle behaviour.
  *
  * KToolBar uses a global config group to load toolbar settings on
  * construction. It will reread this config group on a
  * @ref KApplication::appearanceChanged() signal.
  *
  * BUGS: Sometimes flickers on auto resize, no workaround has yet been
  * found for this.
  *
  * @short Floatable toolbar with auto resize.
  * @version $Id$
  * @author Stephan Kulow <coolo@kde.org>, Sven Radej <radej@kde.org>.
  */
class KToolBar : public QFrame
{
  Q_OBJECT

  friend class KToolBarButton;
  friend class KToolBarRadioGroup;

  Q_ENUMS( IconText BarPosition )
  Q_PROPERTY( IconText iconText READ iconText WRITE setIconText )
  Q_PROPERTY( BarPosition barPos READ barPos WRITE setBarPos )
  Q_PROPERTY( bool fullSize READ fullSize WRITE setFullSize )
  Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize )

public:
  /**
   * Possible positions for text.
   **/
  enum IconText{IconOnly = 0, IconTextRight, TextOnly, IconTextBottom};
  /**
   * The state of the status bar.
   **/
  enum BarStatus{Toggle, Show, Hide};
  /**
   * Possible bar positions.
   **/
  enum BarPosition{Top = 0, Left, Right, Bottom, Floating, Flat};

  /**
   * Constructor.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @ref #_honor_mode is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param parent      The standard toolbar parent (usually a
   *                    @ref KTMainWindow)
   * @param name        The standard internal name
   * @param _honor_mode If true, then global settings for IconSize
   *                    and IconText will be honored
   */
  KToolBar(QWidget *parent=0L, const char *name=0L, bool _honor_mode=false);

  /**
   * Destructor.
   *
   *  If toolbar is floating it will cleanup itself.
   */
  virtual ~KToolBar();

  /**
   * Insert a button (a @ref KToolbarButton) with a pixmap.  The
   * pixmap is loaded by the button itself based on the global icon
   * settings.
   *
   * You should connect to one or more signals in KToolBar:
   * @ref clicked() , @ref pressed() , @ref released() , or
   * @ref highlighted()  and if the button is a toggle button
   * (@ref setToggle() ) @ref toggled() .  Those signals have @p id
   * of a button that caused the signal.  If you want to bind a popup
   * to button, see @ref setButton().
   *
   * @param icon    The name of the icon to use as the active pixmap
   * @param id      The id of this button
   * @param enabled Enable or disable the button at startup
   * @param text    The tooltip or toolbar text (depending on state)
   * @param index The position of the button. (-1 = at end).
   *
   * @return The item index.
   */
  int insertButton(const QString& icon, int id, bool enabled = true,
                   const QString& text = QString::null, int index=-1 );

   /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected.
   *
   * You can add more signals with @ref addConnection().
   *
   * @param icon     The name of the icon to use as the active pixmap
   * @param id       The id of this button
   * @param signal   The signal to connect to
   * @param receiver The slot's parent
   * @param enabled  Enable or disable the button at startup
   * @param text     The tooltip or toolbar text (depending on state)
   * @param index    The position of the button. (-1 = at end).
   *
   * @return The item index.
   */
  int insertButton(const QString& icon, int id, const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true, const QString& text = QString::null,
                   int index=-1 );

  /**
   * Insert a button (a @ref KToolbarButton) with the specified
   * pixmap.  This pixmap will be used as the "active" one and the
   * disabled and default ones will be autogenerated.
   *
   * It is recommended that you use the insertButton function that
   * allows you to specify the icon name rather then the pixmap
   * itself.  Specifying the icon name is much more flexible.
   *
   * You should connect to one or more signals in KToolBar:
   * @ref clicked() , @ref pressed() , @ref released() , or
   * @ref highlighted()  and if the button is a toggle button
   * (@ref setToggle() ) @ref toggled() .  Those signals have @p id
   * of a button that caused the signal.  If you want to bind a popup
   * to button, see @ref setButton().
   *
   * @param pixmap  The active pixmap
   * @param id      The id of this button
   * @param enabled Enable or disable the button at startup
   * @param text    The tooltip or toolbar text (depending on state)
   * @param index The position of the button. (-1 = at end).
   *
   * @return The item index.
   */
  int insertButton(const QPixmap& pixmap, int id, bool enabled = true,
                   const QString& text = QString::null, int index=-1 );

  /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected.
   *
   * You can add more signals with @ref addConnection().
   *
   * @param icon     The name of the icon to use as the active pixmap
   * @param id       The id of this button
   * @param signal   The signal to connect to
   * @param receiver The slot's parent
   * @param enabled  Enable or disable the button at startup
   * @param text     The tooltip or toolbar text (depending on state)
   * @param index    The position of the button. (-1 = at end).
   *
   * @return The item index.
   */
  int insertButton(const QPixmap& pixmap, int id, const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true, const QString& text = QString::null,
                   int index=-1 );

  /**
   * Insert a button with popupmenu.
   *
   * Button will have small
   * triangle. You have to connect to popup's signals. The
   * signals @ref KButton::pressed(), @ref KButton::released(),
   * @ref KButton::clicked() or @ref KButton::doubleClicked() are @bf not
   * emmited by
   * this button (see @ref setDelayedPopup() for that).
   * You can add custom popups which inherit @ref QPopupMenu to get popups
   * with tables, drawings etc. Just don't fiddle with events there.
   */
  int insertButton(const QPixmap& pixmap, int id, QPopupMenu *popup,
                   bool enabled, const QString&_text, int index=-1);

  /**
   * Inserts a KLined. You have to specify signals and slots to
   * which KLineEdit will be connected. KLineEdit has all slots QLineEdit
   * has, plus signals @ref KLineEdit::completion and @ref KLineEdit::rotation
   * KLineEdit can be set to autoresize itself to full free width
   * in toolbar, that is to last right aligned item. For that,
   * toolbar must be set to full width (which it is by default).
   * @see setFullWidth()
   * @see setItemAutoSized()
   * @see KLineEdit
   * @return Item index.
   */
  int insertLined (const QString& text, int ID,
                   const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true,
                   const QString& toolTipText = QString::null,
                   int size = 70, int index =-1);

  /**
   * Insert @ref QComboBox with list.
   *
   * Can be writable, but cannot contain
   * pixmaps. By default inserting policy is AtBottom, i.e. typed items
   * are placed at the bottom of the list. Can be autosized. If the size 
   * argument is specified as -1, the width of the combobox is automatically
   * computed.
   *
   * @see setFullWidth()
   * @see setItemAutoSized()
   * @see QComboBox
   * @return Item index.
   */
  int insertCombo (QStrList *list, int id, bool writable,
                   const char *signal, const QObject *receiver,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   QComboBox::Policy policy = QComboBox::AtBottom);

  /**
   * Insert @ref QComboBox with list.
   *
   * Can be writable, but cannot contain
   * pixmaps. By default inserting policy is AtBottom, i.e. typed items
   * are placed at the bottom of the list. Can be autosized. If the size 
   * argument is specified as -1, the width of the combobox is automatically
   * computed.
   *
   * @see setFullWidth()
   * @see setItemAutoSized()
   * @see QComboBox
   * @return Item index.
   */
  int insertCombo (const QStringList &list, int id, bool writable,
                   const char *signal, const QObject *receiver,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   QComboBox::Policy policy = QComboBox::AtBottom);

  /**
   * Insert @ref QComboBox with text.
   *
   *  The rest is the same as above.
   * @see setItemAutoSized()
   *
   * @see QComboBox
   * @return Item index.
   */
  int insertCombo (const QString& text, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   QComboBox::Policy policy = QComboBox::AtBottom);
  /**
   * Insert separator.
   */
  int insertSeparator(int index=-1);

  /**
   * Insert line separator.
   */
  int insertLineSeparator(int index=-1);

  /**
   * Insert a user-defined widget.  The widget @bf must have this
   * toolbar as its parent.
   *
   * Widget must have a QWidget for base class.  Widget can be
   * autosized to full width. If you forget about it, you can get a
   * pointer to this widget with @ref getWidget().
   * @see setItemAutoSized()
   * @return Item index.
   */
  int insertWidget(int id, int width, QWidget *_widget, int index=-1);

  /**
   * Insert an animated widget.  A @ref KAnimWidget will be created
   * internally using the icon list you provide.
   * This will emit a signal (clicked()) whenever the
   * animation widget is clicked.
   *
   * @see animatedWidget()
   *
   * @param id       The id for this toolbar item
   * @param receiver The parent of your slot
   * @param slot     The slot to receive the clicked() signal
   * @param icons    A list of all icon names to use for the animation
   * @param index    The item index
   *
   * @return The item index
   */
  int insertAnimatedWidget(int id, QObject *receiver, const char *slot,
                           const QStringList& icons, int index = -1);

  /**
   * This will return a pointer to the given animated widget, if it
   * exists.
   *
   * @see insertAnimatedWidget
   *
   * @param id The id for the widget you want to get a pointer to
   *
   * @return A pointer to the current animated widget or 0L
   */
  KAnimWidget *animatedWidget( int id );

  /**
   * Adds connections to items.
   *
   * It is important that you
   * know the @p id of particular item. Nothing happens if you forget @p id.
   */
  void addConnection (int id, const char *signal,
                      const QObject *receiver, const char *slot);
  /**
   * Enable/disable item.
   */
  void setItemEnabled( int id, bool enabled );

  /**
   * Set button pixmap.
   *
   * Can be used while button is visible.
   */
  void setButtonPixmap( int id, const QPixmap& _pixmap );

  /**
   * Set button icon.
   *
   * Can be used while button is visible.
   */
  void setButtonIcon( int id, const QString& _icon );

  /**
   * Set delayed popup for a button.
   *
   * Delayed popup is what you see in
   * Netscape Navigator's Previous and Next buttons: If you click them you
   * go back
   * or forth. If you press them long enough, you get a history-menu.
   * This is exactly what we do here.
   *
   * You will insert normal a button with connection (or use signals from
   * toolbar):
   * <pre>
   * bar->insertButton(pixmap, id, const SIGNAL(clicked ()), this,
   *                   SLOT (slotClick()), true, "click or wait for popup");
   * </pre> And then add a delayed popup:
   * <pre>
   * bar->setDelayedPopup (id, historyPopup); </pre>
   *
   * Don't add delayed popups to buttons which have normal popups.
   *
   * You may add popups which are derived from @ref QPopupMenu. You may
   * add popups that are already in the menu bar or are submenus of
   * other popups.
   */
  void setDelayedPopup (int id , QPopupMenu *_popup, bool toggle = false);

 /**
   * Make a button an autorepeat button.
   *
   * Toggle buttons, buttons with menus, or
   * buttons with delayed menus cannot be made into autorepeat buttons.
   * Moreover, you can and will receive
   * only the signal clicked(), but not pressed() or released().
   * When the user presses this button, you will receive the signal clicked(),
   * and if the button is still pressed after some time,
   * you will receive more clicked() signals separated by regular
   * intervals.  Since this uses @ref QButton::setAutoRepeat() ,
   * I can't quantify 'some'.
   */
  void setAutoRepeat (int id, bool flag=true);


  /**
   * Make button a toggle button if @p flag is true.
   */
  void setToggle (int id, bool flag = true);

  /**
   * Toggle a togglebutton.
   *
   * If the button is a toggle button  (see @ref setToggle())
   * the button state will be toggled. This will also cause the toolbar to
   * emit the signal @ref KButton::toggled() with parameter @p id. You must connect to
   * this signal, or use @ref addConnection() to connect directly to the
   * button signal @ref KButton::toggled().
   */
  void toggleButton (int id);

  /**
   * See a toggle button state.
   *
   * If the button is a toggle button (see @ref setToggle())
   * this will set its state flag. This will also emit the signal
   * @ref KButton::toggled().
   *
   * @see setToggle()
   */
  void setButton (int id, bool flag);

  /**
   * @return @p true if button is on, @p false if button is off or if the
   * button is not a toggle button.
   * @see setToggle()
   */
  bool isButtonOn (int id);

  /**
   * Set text in line editor.
   *
   * Cursor is set at end of text.
   */
  void setLinedText (int id, const QString& text);

  /**
   * Retreive line editor text.
   *
   * If you want to store this text, you have to deep-copy it somewhere.
   */
  QString getLinedText (int id);

  /**
   * Insert text in combobox @p id at position @p index.
   */
  void insertComboItem (int id, const QString& text, int index);

  /**
   * Insert list in combobox @p id at position @p index.
   */
  void insertComboList (int id, QStrList *list, int index);

  /**
   * Insert list in combobox @p id at position @p index.
   */
  void insertComboList (int id, const QStringList &list, int index);

  /**
   * Remove item @p index from combobox @p id.
   */
  void removeComboItem (int id, int index);

  /**
   * Set item @p index to be current item in combobox @p id.
   */
  void setCurrentComboItem (int id, int index);

  /**
   * Change item @p index in combobox @p id to text.
   *
   * @p index = -1 refers current item (one displayed in the button).
   */
  void changeComboItem  (int id, const QString& text, int index=-1);

  /**
   * Clear combobox @p id.
   *
   * Does not delete it or hide it.
   */
  void clearCombo (int id);

  /**
   * Return text of item @p index from combobox @p id.
   *
   * @p index = -1 refers to current item.
   */

  QString getComboItem (int id, int index=-1);

  /**
   * Retrieve a pointer to the combobox.
   *
   * Example:
   * <pre>
   * QComboBox *combo = toolbar->getCombo(combo_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref QComboBox provides.
   */
  QComboBox * getCombo(int id);

  /**
   * Retrieve a pointer to KToolBarLined.
   *
   * Example:
   * <pre>
   * KLineEdit * lined = toolbar->getKTollBarLined(lined_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KLineEdit provides. @ref KLineEdit is the same thing
   * as @ref QLineEdit plus completion signals.
   */
  KLineEdit * getLined (int id);

  /**
   * Retrieve a pointer to KToolBarButton.
   *
   * Example:
   * <pre>
   * KToolBarButton * button = toolbar->getButton(button_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KToolBarButton provides.
   *
   *  Using this method is not recommended.
   */
  KToolBarButton * getButton (int id);

  /**
   * Align item to the right.
   *
   * This works only if toolbar is set to full width.
   * @see setFullWidth()
   */
  void alignItemRight (int id, bool right = true);

  /**
   * Retrieve a pointer to an inserted widget.
   *
   * Wrong ids are not tested.
   * You can do with this whatever you want,
   * except change its height (hardcoded). If you change its width
   * you will probbably have to call toolbar->@ref updateRects (true)
   * @see QWidget
   * @see updateRects()
   */
  QWidget *getWidget (int id);

  /**
   * Set item autosized.
   *
   * This works only if the toolbar is set to full width.
   * @bf Only @bf one item can be autosized, and it has to be
   * the last left-aligned item. Items that come after this must be right
   * aligned. Items that can be right aligned are Lineds, Frames, Widgets and
   * Combos. An autosized item will resize itself whenever the toolbar geometry
   * changes to the last right-aligned item (or to end of toolbar if there
   * are no right-aligned items.)
   * @see setFullWidth()
   * @see alignItemRight()
   */
  void setItemAutoSized (int id, bool yes = true);

  /**
   * Remove all items.
   *
   * The toolbar is redrawn after it.
   */
  void clear ();

  /**
   * Remove item @p id.
   *
   * Item is deleted. Toolbar is redrawn after it.
   */
  void removeItem (int id);

  /**
   * Hide item.
   */
  void hideItem (int id);

  /**
   * Show item.
   */
  void showItem (int id);

  /**
   * Set toolbar to full parent size (default).
   *
   *  In full size mode the bar
   * extends over the parent's full width or height. If the mode is disabled
   * the toolbar tries to take as much space as it needs without wrapping, but
   * it does not exceed the parent box. You can force a certain width or
   * height with @ref setMaxWidth() or @ref setMaxHeight().
   *
   * If you want to use right-aligned items or auto-sized items you must use
   * full size mode.
   */
  void setFullSize(bool flag = true);

  /**
   * @return @p true if the full-size mode is enabled. Otherwise
   * it returns @false.
   */
  bool fullSize() const;

  /**
   * Enable or disable moving of toolbar.
   */
  void enableMoving(bool flag = true);

  /**
   * Set position of toolbar.
   * @see BarPosition()
   */
  void setBarPos (BarPosition bpos);

  /**
   * Retrieve position of toolbar.
   */
  BarPosition barPos() const;

  /**
   * Show, hide, or toggle toolbar.
   *
   * If toolbar floats,
   * hiding means minimizing. Warning: kwm will not show a minimized toolbar
   * on taskbar. Therefore hiding means hiding.
   * @see BarStatus
   */
  bool enable(BarStatus stat);

  /**
   * Set maximal height of vertical (Right or Left) toolbar.
   *
   * You normally
   * do not have to call it, since it's called from
   * @ref KTMainWindow::updateRects()
   * If you reimplement @ref KTMainWindow::resizeEvent() or
   * KTMainWindow::updateRects(),
   * be sure to call this function with the maximal height the toolbar can have.
   * In 0xFE cases out of 0xFF (i.e., quite nearly always) you don't need to use this function.
   * @see updateRects()
   */
  void setMaxHeight (int h);  // Set max height for vertical toolbars

  /**
   * Retrieve the value that was set with @ref setMaxHeight().
   */
  int maxHeight();

  /**
   * Set maximal width of horizontal (top or bottom) toolbar.
   *
   * This works
   * only for horizontal toolbars (at Top or Bottom), and has no effect
   * otherwise. Has no effect when toolbar is floating.
   */
  void setMaxWidth (int dw);

  /**
   * Retrieve the value that was set with @ref setMaxWidth().
   */
  int maxWidth();

  /**
   * Set title for toolbar when it floats.
   *
   * Titles are however not (yet)
   * visible. You can't change toolbar's title while it's floating.
   */
  void setTitle (const QString& _title);

  /**
   * Enable or disable floating.
   *
   * Floating is enabled by default.
   * This only disables menu entry Floating in popup menu, so
   * toolbar can still be moved by @ref setBarPos() or by dragging.
   * This function is obsolete and do not use it. If you want to make
   * toolbar static use @ref enableMoving
   * @deprecated
   */
  void enableFloating (bool arrrrrrgh);

  /**
   * Set the kind of painting for buttons.
   *
   * Choose from:
   * @li IconOnly (only icons),
   * @li IconTextRight (icon and text, text is left from icons),
   * @li TextOnly (only text),
   * @li IconTextBottom (icons and text, text is under icons).
   * @see IconText
   *
   */
  void setIconText(IconText it);
  // Note: don't merge with the next one, it breaks Qt properties

  /**
   * Similar to @ref setIconText(IconText it) but allows you to
   * disable or enable updating.  If @p update is false, then the
   * buttons will not be updated.  This is useful only if you know
   * that you will be forcing an update later.
   */
  void setIconText(IconText it, bool update);

  /**
   * @return The current text style for buttons.
   */
  IconText iconText() const;

  /**
   * Set the icon size to load. Usually you should not call
   * this, the icon size is taken care of by KIconLoader
   * and globally configured.
   * By default, the toolbar will load icons of size 32 for main
   * toolbars and 22 for other toolbars (@see KIconLoader).
   *
   * @param size The size to use
   */
  void setIconSize(int size);
  // Note: don't merge with the next one, it breaks Qt properties

  /**
   * Same as @ref setIconText(KIconLoader::Size size) but allows you
   * to disable the toolbar update.
   *
   * @param size   The size to use
   * @param update If true, then the toolbar will be updated after
   *               this
   */
  void setIconSize(int size, bool update);

  /**
   * @return The current icon size for buttons.
   */
  int iconSize() const;

  /**
   * This allows you to enable or disable the context menu.
   *
   * @param enable If false, then the context menu will be disabled
   */
  void setEnableContextMenu(bool enable = true);

  /**
   * Returns whether or not the context menu is disabled
   *
   * @return The context menu state
   */
  bool contextMenuEnabled() const;

  /**
   * This will inform a toolbar button to ignore certain style
   * changes.  Specifically, it will ignore IconText (always IconOnly)
   * and will not allow image effects to apply.
   *
   * @param id The button to exclude from styles
   * @param no_style If true, then it is excluded (default: true).
   */
  void setItemNoStyle(int id, bool no_style = true);

  /**
   * Arrange the toolbar items and calculates their
   * position and size.
   *
   * Most of the work is done by @ref layoutHorizontal()
   * and @ref layoutVertical() though. In some cases it may be desirable to
   * trigger a resize operation, then set resize to true. Do not call
   * updateRects(true) when processing a resize event as this will start
   * an infinite recursion!
   */
  void updateRects(bool resize = false);

  /**
   * Retrieve the preferred size.
   *
   * This function is required for the Qt layout management to work.
   */
  virtual QSize sizeHint() const;

  /**
   * Retrieve the minimum size.
   *
   * This function is required for the Qt layout management to work.
   */
  virtual QSize minimumSizeHint() const;

  /**
   * Retrieve the maximum size.
   *
   * This function return the maximum size the toolbar would need without
   * wrapping. Use this function when you want to fix the toolbar to it's
   * maximum width/height.
   */
  virtual QSize maximumSizeHint() const;

  /**
   * Retrieve the width needed to properly display at a given @p height.
   *
   * This function is required for the Qt layout management to work. It
   * return the minimum width for a given height. It makes only sense for
   * vertical tool bars.
   */
  virtual int widthForHeight(int height) const;

  /**
   * Retrieve the height needed to properly display at a given @p width.
   *
   * This function is required for the Qt layout management to work. It
   * return the minimum height for a given width. It makes only sense for
   * horizontal tool bars.
   */
  virtual int heightForWidth(int width) const;

  /**
   * This function is required for the Qt layout management to work. It
   * returns information about the size policy.
   */
  virtual QSizePolicy sizePolicy() const;

  void setFlat (bool flag);

  /**
   * @return the number of items in the toolbar
   */
  int count();

signals:
    /**
     * Emitted when button @p id is clicked.
     */
    void clicked(int id);

    /**
     * Emitted when button @p id is double-clicked.
     *
     * Note: you will always
     * recive two @ref clicked() , @ref pressed() and @ref released() signals.
     * There is no way to avoid it - at least no easy way.
     * If you need to resolve this all you can do is set up timers
     * which wait for @ref QApplication::doubleClickInterval() to expire.
     * If in that time you don't get this signal, you may belive that
     * button was only clicked and not double-clicked.
     * And please note that butons with popup menus do not emit this signal,
     * but those with delayed popup do.
     */
    void doubleClicked (int id);

    /**
     * Emitted when button @p id is pressed.
     */
    void pressed(int);

    /**
     * Emits when button @p id is released.
     */
    void released(int);

    /**
     * Emitted when a toggle button changes state.
     *
     * Emitted also if you change state
     * with @ref setButton() or @ref toggleButton()
     * If you make a button normal again, with
     * @ref setToggle (false), this signal won't
     * be emitted.
     */
    void toggled(int);

    /**
     * This signal is emitted when item id gets highlighted/unhighlighted
     * (i.e when mouse enters/exits).
     *
     * Note that this signal is emitted from
     * all buttons (normal, disabled and toggle) even when there is no visible
     * change in buttons (i.e., buttons do not raise when mouse enters).
     * The parameter @p isHighlighted is @p true when mouse enters and @p false when
     * mouse exits.
     */
    void highlighted(int id, bool isHighlighted);

    /**
     * This signal is emitted when item id gets highlighted/unhighlighted
     * (i.e when mouse enters/exits).
     *
     * Note that this signal is emitted from
     * all buttons (normal, disabled and toggle) even when there is no visible
     * change in buttons (i.e., buttons do not raise when mouse enters).
     */
    void highlighted(int id );

    /**
     * Emitted when toolbar changes position, or when
     * an item is removed from toolbar.
     *
     * This is normaly connected to
     * @ref KTMainWindow::updateRects().
     * If you subclass @ref KTMainWindow and reimplement
     * @ref KTMainWindow::resizeEvent() or
     * @ref KTMainWindow::updateRects(), be sure to connect to
     * this signal. Note: You can connect this signal to a slot that
     * doesn't take parameter.
     * @see updateRects()
     */
    void moved( BarPosition );

    /**
     * @internal
     * This signal is emited when toolbar detects changing of
     * following parameters:
     * highlighting, button-size, button-mode. This signal is
     * internal, aimed to buttons.
     */
    void modechange ();

protected:
  KPopupMenu *context;

  void drawContents ( QPainter *);
  void resizeEvent(QResizeEvent*);
  void paintEvent(QPaintEvent*);
  void closeEvent (QCloseEvent *);
  void mousePressEvent ( QMouseEvent *);
  void mouseMoveEvent ( QMouseEvent *);
  void mouseReleaseEvent ( QMouseEvent *);
  void init();
  void layoutVertical(int maxVerHeight);
  void layoutHorizontal(int maxHorWidth);
  void leaveEvent (QEvent *e);

  /**
   * This will return a pointer to the list of toolbar items
   */
  KToolBarItemList *items();
  bool highlight() const;

private slots:
  void ButtonClicked(int);
  void ButtonDblClicked( int id );
  void ButtonPressed(int);
  void ButtonReleased(int);
  void ButtonToggled(int);
  void ButtonHighlighted(int,bool);

  void ContextCallback(int);
  void slotReadConfig ();
  void slotIconChanged(int);
  void slotHotSpot (int i);

private:
  bool fullSizeMode;
  bool moving;
  int toolbarWidth;
  int toolbarHeight;

  int oldX;
  int oldY;
  int oldWFlags;

  int min_width;
  int min_height;

  BarPosition lastPosition; // Where was I last time I was?
  BarPosition movePos;      // Where was I moved to?
  bool mouseEntered;  // Did the mouse touch the cheese?
  bool localResize;   // Am I trying to understand recursion?
  bool wasfullSize;  // Was I loong when I was?
  bool haveAutoSized; // Do I have a problem?

  KToolBoxManager *mgr;
  bool buttonDownOnHandle;

  KToolBarPrivate *d;
};

#endif
