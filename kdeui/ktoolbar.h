/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)

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

// $Id$
// $Log$
// Revision 1.68  1999/12/18 22:00:17  granroth
// Added convience method: 'clear()'  Basically, it just iterates through
// all of it's children and removes them in turn.
//
// Revision 1.67  1999/11/26 12:48:06  dfaure
// More proofreading. Found what I was looking for, will stop proofreading :-)
//
// Revision 1.66  1999/10/10 08:18:58  bero
// Code cleanup ((void) stuff)
//
// Revision 1.65  1999/10/08 22:49:18  bero
// - Replace KTopLevelWidget with KTMainWindow where it's still used
// - Disable ktopwidget.h
// - Remove ktopwidget stuff from libs
//
// Revision 1.64  1999/09/21 11:03:55  waba
// WABA: Clean up interface
//
// Revision 1.63  1999/07/26 19:42:45  pbrown
// fixed for qcombobox.
//
// Revision 1.62  1999/06/15 20:36:34  cschlaeg
// some more cleanup in ktmlayout; fixed random toolbar handle highlighting
//
// Revision 1.61  1999/06/10 21:47:50  cschlaeg
// setFullWidth(false) ignore feature re-implemented; floating resize bug fixed; layout manager documented; resizing floating bars still does not work properly
//
// Revision 1.60  1999/06/09 21:52:27  cschlaeg
// serveral fixes for recently implemented layout management; removed segfault on close; removed segfault for no menubar apps; setFullWidth(false) is working again; floating a bar does not segfault any more but still does not work properly; I will look into this again.
//
// Revision 1.59  1999/06/07 21:11:05  cschlaeg
// more work done for layout management integration; setFullWidth(false) still does not work; will work on this tomorrow
//
// Revision 1.58  1999/06/06 22:48:39  pbrown
// "warning: extra qualification `KToolBar::' on member `sizePolicy'
// ignored" fixed.
//
// Revision 1.57  1999/06/06 17:29:45  cschlaeg
// New layout management implemented for KTMainWindow. This required
// updates for KToolBar, KMenuBar and KStatusBar. KTMainWindow::view_*
// public variables removed. Use mainViewGeometry() instead if you really
// have to. Added new classes in ktmlayout to handle the new layout
// management.
//
// Revision 1.56  1999/06/04 15:43:55  pbrown
// improved KLineEdit to have a right popup menu with cut, copy, past, clear
// etc. like newer windows (heh) applications have.  Renamed class from
// KLined to KLineEdit for consistency -- provided a #define for backwards
// comptability, but I am working on stamping the old class name out now.
//
// Revision 1.55  1999/05/28 10:17:21  kulow
// several fixes to make --enable-final work. Most work is done by changing
// the order of the files in _SOURCES
//
// Revision 1.54  1999/05/23 00:53:59  kulow
// CVS_SILENT moving some header files that STL comes before Qt
//
// Revision 1.53  1999/05/22 20:54:13  ssk
// Minor doc reformat.
//
// Revision 1.52  1999/05/04 04:28:08  ssk
// Updated KToolBar description.
//
// Revision 1.51  1999/04/22 15:59:44  shausman
// - support QStringList for combos
//
// Revision 1.50  1999/03/06 18:03:37  ettrich
// the nifty "flat" feature of kmenubar/ktoolbar is now more visible:
// It has its own menu entry and reacts on simple LMP clicks.
//
// Revision 1.49  1999/03/01 23:35:26  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.48  1999/02/10 19:51:23  koss
// *** empty log message ***
//
// Revision 1.46  1998/11/25 13:22:00  radej
// sven: Someone made some private things protected (was it me?).
//
// Revision 1.45  1998/11/21 19:27:20  radej
// sven: doubleClicked signal for buttons.
//
// Revision 1.44  1998/11/11 14:32:11  radej
// sven: *Bars can be made flat by MMB (Like in Netscape, but this works)
//
// Revision 1.43  1998/11/09 00:28:43  radej
// sven: Docs update (more to come)
//
// Revision 1.42  1998/11/06 12:54:54  radej
// sven: radioGroup is in. handle changed again (broken in vertical mode)
//
// Revision 1.41  1998/10/09 12:42:21  radej
// sven: New: (un) highlight sugnals, Autorepeat buttons, button down when
//       pressed. kdetest/kwindowtest updated. This is Binary COMPATIBLE.
//
// Revision 1.40  1998/09/15 05:56:47  antlarr
// I've added a setIconText function to change the state of a variable
// in KToolBar
//
// Revision 1.39  1998/09/01 20:22:24  kulow
// I renamed all old qt header files to the new versions. I think, this looks
// nicer (and gives the change in configure a sense :)
//
// Revision 1.38  1998/08/09 14:01:19  radej
// sven: reintroduced makeDisabledPixmap code, and dumped QIconSet. Fixed a bug
//       with paletteChange too.
//
// Revision 1.37  1998/08/06 15:39:03  radej
// sven: Popups & delayedPopups. Uses QIconSet. Needs Qt-1.4x
//
// Revision 1.36  1998/06/20 10:57:00  radej
// sven: mispelled something...
//
// Revision 1.35  1998/06/19 13:09:31  radej
// sven: Docs.
//
// Revision 1.34  1998/05/04 16:38:36  radej
// Bugfixes for moving + opaque moving
//
// Revision 1.33  1998/04/28 09:17:49  radej
// New moving and docking BINARY INCOMPATIBLE
//

#ifndef _KTOOLBAR_H
#define _KTOOLBAR_H

#include <qlist.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qbutton.h>
#include <qfont.h>
#include <qsize.h>
#include <qintdict.h>
#include <qstringlist.h>
#include <qcombobox.h>

//#include <qiconset.h>

class KLineEdit;
class KToolBar;
class KToolBarItemList;
class KToolBoxManager;

 /**
  * KToolBar can be "floated", dragged and docked from its parent window.
  *
  * A KToolBar can contain standard or toggle buttons, line edit widgets,
  * combo boxes, frames or any developer-defined custom widget, with
  * automatic full-width resize for each widget except buttons. Buttons
  * can also be connected to popup menus with a delay option.
  *
  * KToolBar can be used as a standalone widget, but @ref KTMainWindow
  * provides easy factories and management of one or more toolbars.
  * Once you have a KToolBar object, you can insert items into it with the
  * insert... methods, or remove them with the @ref removeItem method. This
  * can be done at any time; the toolbar will be automatically updated.
  * There are also many methods to set per-child properties like alignment
  * and toggle behaviour.
  *
  * KToolBar uses a global config group to load toolbar settings on
  * construction. It will reread this config group on a
  * @ref KApplication::apearanceChanged signal.
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

public:
  enum BarStatus{Toggle, Show, Hide};
  enum BarPosition{Top, Left, Bottom, Right, Floating, Flat};

  /**
   * Constructor.
   * Toolbar will read global-config file for item Size highlight
   * option and button type. However, you can pass desired height.
   * If you specify height here, config value has no effect.
   * Exception is if you set Icontext mode to 3 (icons under text) which sets
   * size to minimum 40 pixels. For setting IconText mode, see
   * @ref #setIconText .
   * Setting size in constructor is not recommended.
   */
  KToolBar(QWidget *parent=0L, const char *name=0L, int _item_size = -1);

  /**
   * Destructor. If toolbar is floating it will cleanup itself.
   */
  virtual ~KToolBar();

  /**
   * Inserts KButton with pixmap. You should connect to one or more signals in
   * KToolBar: @ref #clicked , @ref #pressed , @ref #released ,
   * @ref highlighted  and
   * if toolbar is toggle button (@ref #setToggle ) @ref #toggled . Those
   * signals have id of a button that caused the signal.
   * If you want to bind a popup to button, see  @ref #setButton
   * @param index the position of the button. (-1 = at end).
   * @return Returns item index
   */
  int insertButton(const QPixmap& pixmap, int ID, bool enabled = true,
                   const QString& ToolTipText = QString::null, int index=-1 );
  /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected. Button emits
   * signals pressed, clicked and released, and
   * if toolbar is toggle button ( @ref #setToggle ) @ref #toggled .
   * You can add more signals with @ref #addConnection .
   * @return Returns item index
   */
  int insertButton(const QPixmap& pixmap, int ID, const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true,
                   const QString& tooltiptext = QString::null,
		   int index=-1 );

  /**
   * This inserts a button with popupmenu. Button will have small
   * trialngle. You have to connect to popup's signals. The
   * signals pressed, released, clikced or doubleClicked are NOT emmited by
   * this button (see @ref #setDelayedPopup for that).
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
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see KLineEdit
   * @return Returns item index
   */
  int insertLined (const QString& text, int ID,
                   const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true,
                   const QString& toolTipText = QString::null,
		   int size = 70, int index =-1);

  /**
   * Inserts QComboBox with list. Can be writable, but cannot contain
   * pixmaps. By default inserting policy is AtBottom, i.e. typed items
   * are placed at the bottom of the list. Can be autosized.
   *
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see QComboBox
   * @return Returns item index
   */
  int insertCombo (QStrList *list, int id, bool writable,
                   const char *signal, const QObject *receiver,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   QComboBox::Policy policy = QComboBox::AtBottom);
		
  /**
   * Inserts QComboBox with list. Can be writable, but cannot contain
   * pixmaps. By default inserting policy is AtBottom, i.e. typed items
   * are placed at the bottom of the list. Can be autosized.
   *
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see QComboBox
   * @return Returns item index
   */
  int insertCombo (const QStringList &list, int id, bool writable,
                   const char *signal, const QObject *receiver,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   QComboBox::Policy policy = QComboBox::AtBottom);

  /**
   * Inserts QComboBox with text. The rest is the same as above.
   * @see #setItemAutoSized
   *
   * @see QComboBox
   * @return Returns item index
   */
  int insertCombo (const QString& text, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   QComboBox::Policy policy = QComboBox::AtBottom);
  /**
   * Insert separator
   */
  int insertSeparator(int index=-1);

  /**
   * Insert line separator
   */
  int insertLineSeparator(int index=-1);

  /**
   * This function is @deprecated and will be removed. Use @ref #insertWidget
   * to insert anything.
   * Inserts frame with specified width. You can get
   * pointer to this frame with @ref #getFrame
   * Frame can be autosized to full width.
   * @see #setItemAutoSized
   * @return Returns item index
   */
  int insertFrame(int id, int width, int index =-1);

  /**
   * Insert a user defined widget. Widget must have a QWidget for
   * base class.
   * Widget can be autosized to full width. If you forget about it, you
   * can get pointer to this widget with @ref #getWidget .
   * @see #setItemAutoSized
   * @return Returns item index
   */
  int insertWidget(int id, int width, QWidget *_widget, int index=-1);

  /**
   * This adds connection to items. Therefore it is important that you
   * know id of particular item. Nothing happens if you miss id.
   */
  void addConnection (int id, const char *signal,
                      const QObject *receiver, const char *slot);
  /**
   * Enables/disables item.
   */
  void setItemEnabled( int id, bool enabled );

  /**
   * Sets button pixmap.
   * Can be used while button is visible.
   */
  void setButtonPixmap( int id, const QPixmap& _pixmap );

  /**
   * Sets delayed popup to a button. Delayed popup is what you see in
   * netscape's Previous&next buttons: if you click them you go back,
   * or forth. If you press them long enough, you get a history-menu.
   * This is exactly what we do here.
   *
   * You will insert normal button with connection (or use signals from
   * toolbar):
   * <pre>
   * bar->insertButton(pixmap, id, const SIGNAL(clicked ()), this,
   *     		SLOT (slotClick()), true, "click or wait for popup");
   * </pre> And then add a delayed popup:
   * <pre>
   * bar->setDelayedPopup (id, historyPopup); </pre>
   *
   * Don't add delayed popups to buttons which have normal popups.
   *
   * You may add popups which are derived from QPopupMenu. You may
   * add popups that are already in menu bar or are submenus of other popups.
   */
  void setDelayedPopup (int id , QPopupMenu *_popup);

 /**
   * Makes a button autorepeat button. Toggle, buttons with menu or
   * delayed menu cannot be autorepeat. More, you can and will receive
   * only signals clicked, and not pressed or released.
   * When user presses this buton, you will receive signal clicked,
   * and if button is still pressed after some time, more clicks
   * in some interval. Since this uses @ref QButton::setAutoRepeat ,
   * I don't know how much is 'some'.
   */
  void setAutoRepeat (int id, bool flag=true);


  /**
   * Makes button a toggle button if flag is true
   */
  void setToggle (int id, bool flag = true);

  /**
   * If button is toggle (@ref #setToggle must be called first)
   * button state will be toggled. This will also cause toolbar to
   * emit signal @ref #toggled wit parameter id. You must connect to
   * this signal, or use @ref #addConnection to connect directly to
   * button-signal toggled.
   */
  void toggleButton (int id);

  /**
   * If button is toggle (@ref #setToggle must be called first)
   * this will set him to state flag. This will also emit signal
   * #ref toggled.
   *
   * @see #setToggle
   */
  void setButton (int id, bool flag);

  /**
   * Returns true if button is on, false if button is off.
   * If button is not a toggle button returns false
   * @see #setToggle
   */
  bool isButtonOn (int id);

  /**
   * Sets text in Lined.
   * Cursor is set at end of text.
   */
  void setLinedText (int id, const QString& text);

  /**
   * Returns Lined text.
   * If you want to store this text, you have to deep-copy it somwhere.
   */
  QString getLinedText (int id);

  /**
   * Inserts text in combo id with at position index.
   */
  void insertComboItem (int id, const QString& text, int index);

  /**
   * Inserts list in combo id at position index
   */
  void insertComboList (int id, QStrList *list, int index);

  /**
   * Inserts list in combo id at position index
   */
  void insertComboList (int id, const QStringList &list, int index);

  /**
   * Removes item index from Combo id.
   */
  void removeComboItem (int id, int index);

  /**
   * Sets item index to be current item in Combo id.
   */
  void setCurrentComboItem (int id, int index);

  /**
   * Changes item index in Combo id to text.
   * index = -1 means current item (one displayed in the button).
   */
  void changeComboItem  (int id, const QString& text, int index=-1);

  /**
   * Clears combo id.
   * Does not delete it or hide it.
   */
  void clearCombo (int id);

  /**
   * Returns text of item index from Combo id.
   * index = -1 means current item
   */

  QString getComboItem (int id, int index=-1);

  /**
   * This returns pointer to Combo. Example:
   * <pre>
   * QComboBox *combo = toolbar->getCombo(combo_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref QComboBox provides.
   */
  QComboBox * getCombo(int id);

  /**
   * This returns pointer to KToolBarLined. Example:
   * <pre>
   * KLineEdit * lined = toolbar->getKTollBarLined(lined_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KLineEdit provides. @ref KLineEdit is the same thing
   * as @ref QLineEdit plus completion signals.
   */
  KLineEdit * getLined (int id);

  /**
   * This returns a pointer to KToolBarButton. Example:
   * <pre>
   * KToolBarButton * button = toolbar->getButton(button_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KToolBarButton provides. Using of this method is not
   * recommended.
   */
  KToolBarButton * getButton (int id);

  /**
   * Aligns item to the right.
   * This works only if toolbar is set to full width.
   * @see #setFullWidth
   */
  void alignItemRight (int id, bool right = true);

  /**
   * This function is @deprecated and might be removed. Use @ref #insertWidget
   * and @ref #getWidget instead.<br>
   * Returns pointer to inserted frame. Wrong ids are not tested.
   * Example:
   * <pre>
   * QFrame *frame = toolbar->getframe (frameid);
   * </pre>
   * You can do with this frame whatever you want,
   * except change its height (hardcoded). If you change its width
   * you will probbably have to call toolbar->@ref #updateRects (true)
   * @see QFrame
   * @see #updateRects
   */
  QFrame * getFrame (int id);

  /**
   * Returns pointer to inserted widget. Wrong ids are not tested.
   * You can do with this whatever you want,
   * except change its height (hardcoded). If you change its width
   * you will probbably have to call toolbar->@ref #updateRects (true)
   * @see QWidget
   * @see #updateRects
   */
  QWidget *getWidget (int id);

  /**
   * Sets item autosized. This works only if toolbar is set to full width.
   * ONLY ONE item can be autosized, and it has to be
   * the last left-aligned item. Items that come after this must be right
   * aligned. Items that can be right aligned are Lineds, Frames, Widgets and
   * Combos. Auto sized item will resize itself whenever toolbar geometry
   * changes, to last right-aligned item (or till end of toolbar if there
   * are no right aligned items
   * @see #setFullWidth
   * @see #alignItemRight
   */
  void setItemAutoSized (int id, bool yes = true);

  /**
   * Removes all items.  The toolbar is redrawn after it.
   */
  void clear ();

  /**
   * Removes item id.
   * Item is deleted. Toolbar is redrawn after it.
   */
  void removeItem (int id);

  /**
   * Hides item.
   */
  void hideItem (int id);

  /**
   * shows item.
   */
  void showItem (int id);

  /**
   * Obsolete! This function has been replaced by @ref setFullSize().
   * @deprecated
   */
  void setFullWidth(bool flag = true);

  /**
   * Sets toolbar to full parent size (default). In full size mode the bar
   * extends over the parent's full width or height. If the mode is disabled
   * the toolbar tries to take as much space as it needs without wrapping, but
   * it does not exceed the parent box. You can force a certain width or
   * height with @ref setMaxWidth or @ref setMaxHeight.
   *
   * If you want to use right aligned items or auto-sized items you must use
   * full size mode.
   */
  void setFullSize(bool flag = true);

  /**
   * This function returns true of the full-size mode is enabled. Otherwise
   * false.
   */
  bool fullSize() const;

  /**
   * Enables or disables moving of toolbar.
   */
  void enableMoving(bool flag = true);

  /**
   * Sets position of toolbar
   * @see #BarPosition
   */
  void setBarPos (BarPosition bpos);

  /**
   * Returns position of toolbar
   */
  BarPosition barPos() {return position;}

  /**
   * This shows, hides, or toggles toolbar. If toolbar floats,
   * hiding means minimizing. Warning: kwm will not show minimized toolbar
   * on taskbar. Therefore hiding means hiding.
   * @see #BarStatus
   */
  bool enable(BarStatus stat);

  /**
   * Sets maximal height of vertical (Right or Left) toolbar. You normaly
   * do not have to call it, since it's called from
   * @ref KTMainWindow#updateRects
   * If you reimplement @ref KTMainWindow#resizeEvent or
   * KTMainWindow#updateRects,
   * be sure to call this function with maximal height toolbar can have.
   * In 0xFE cases out of 0xFF you don't need to use this function.
   * @see #updateRects
   */
  void setMaxHeight (int h);  // Set max height for vertical toolbars

  /**
   * Returns the value that was set with @ref setMaxHeight().
   */
  int maxHeight()
  {
	  return (maxVerHeight);
  }

  /**
   * Sets maximal width of horizontal (top or bottom) toolbar. This works
   * only for horizontal toolbars (at Top or Bottom), and has no effect
   * otherwise. Has no effect when toolbar is floating.
   */
  void setMaxWidth (int dw);

  /**
   * Return the value that was set with @ref setMaxWidth().
   */
  int maxWidth()
  {
	  return (maxHorWidth);
  }

  /**
   * Sets title for toolbar when it floats. Titles are however not (yet)
   * visible. You can't change toolbar's title while it's floating.
   */
  void setTitle (const QString& _title) {title = _title;}

  /**
   * Enables or disables floating.
   * Floating is enabled by default.
   * This only disables menu entry Floating in popup menu, so
   * toolbar can still be moved by @ref #setBarPos or by dragging.
   * This function is obsolete and do not use it. If you want to make
   * toolbar static use @ref enableMoving
   * @deprecated
   */
  void enableFloating (bool arrrrrrgh);

  /**
   * Sets the kind of painting for buttons between : 0 (only icons),
   * 1 (icon and text, text is left from icons), 2 (only text),
   * and 3 (icons and text, text is under icons).
   */

  void setIconText(int it);

  /**
   * updateRects() arranges the toolbar items and calculates their
   * position and size. Most of the work is done by layoutHorizontal()
   * and layoutVertical() though. In some cases it may be desirable to
   * trigger a resize operation, then set resize to true. Do not call
   * updateRects(true) within a resize event processing, this will start
   * an infinite recursion!
   */
  void updateRects(bool resize = false);

  /**
   * This function is required for the Qt layout management to work. It
   * returns the preferred size.
   */
  virtual QSize sizeHint() const;

  /**
   * This function is required for the Qt layout management to work. It
   * returns the preferred size.
   */
  virtual QSize minimumSizeHint() const;

  /**
   * This function return the maximum size the toolbar would need without
   * wrapping. Use this function when you want to fix the toolbar to it's
   * maximum width/height.
   */
  virtual QSize maximumSizeHint() const;

  /**
   * This function is required for the Qt layout management to work. It
   * return the minimum width for a given height. It makes only sense for
   * vertical tool bars.
   */
  virtual int widthForHeight(int height) const;

  /**
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

signals:
    /**
     * Emits when button id is clicked.
     */
    void clicked(int id);

    /**
     * Emits when button id is double clicked. Note: you will always
     * recive two @ref #clicked , @ref #pressed and @ref #released signals.
     * There is no way to avoid it - at least no easy way.
     * If you need to resolve this all you can do is set up timers
     * which waits for @ref QApplication::doubleClickInterval to expire.
     * if in that time you don't get this signal, you may belive that
     * button was only clicked.
     * And please note that butons with popup menus do not emit this signal,
     * but those with delayed popup do.
     */
    void doubleClicked (int id);

    /**
     * Emits when button id is pressed.
     */
    void pressed(int);

    /**
     * Emits when button id is released.
     */
    void released(int);

    /**
     * Emits when toggle button changes state
     * Emits also if you change state
     * with @ref #setButton or @ref #toggleButton
     * If you make a button normal again, with
     * @ref #setToggle (false), this signal won't
     * be emited.
     */
    void toggled(int);

    /**
     * This signal is emmited when item id gets highlighted/unhighlighted
     * (i.e when mouse enters/exits). Note that this signal is emited from
     * all buttons (normal, disabled and toggle) even when there is no visible
     * change in buttons (meaning, buttons do not raise when mouse enters).
     * Parameter isHighlighted is true when mouse enters and false when
     * mouse exits.
     */
    void highlighted(int id, bool isHighlighted);

    /**
     * Emits when toolbar changes its position, or when
     * item is removed from toolbar. This is normaly connected to
     * @ref KTMainWindow::updateRects.
     * If you subclass @ref KTMainWindow and reimplement
     * @ref KTMainWindow::resizeEvent or
     * @ref KTMainWindow::updateRects, be sure to connect to
     * this signal. You can connect this signal to slot that
     * doesn't take parameter.
     * @see #updateRects
     */
    void moved( BarPosition );

    /**
     * Internal. This signal is emited when toolbar detects changing of
     * following parameters:
     * highlighting, button-size, button-mode. This signal is
     * internal, aimed to buttons.
     */
    void modechange ();

private:

  KToolBarItemList *items;

  QString title;
  bool fullSizeMode;
  BarPosition position;
  bool moving;
  QWidget *Parent;
  int toolbarWidth;
  int toolbarHeight;

  int oldX;
  int oldY;
  int oldWFlags;

  int min_width;
  int min_height;

  int maxHorWidth;
  int maxVerHeight;

  BarPosition lastPosition; // Where was I last time I was?
  BarPosition movePos;      // Where was I moved to?
  bool mouseEntered;  // Did the mouse touch the cheese?
  bool horizontal;    // Do I stand tall?
  bool localResize;   // Am I trying to understand recursion?
  bool wasfullSize;  // Was I loong when I was?
  bool haveAutoSized; // Do I have a problem?

  KToolBoxManager *mgr;
  bool buttonDownOnHandle;

protected:
  QPopupMenu *context;

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


private slots:
  void ButtonClicked(int);
  void ButtonDblClicked( int id );
  void ButtonPressed(int);
  void ButtonReleased(int);
  void ButtonToggled(int);
  void ButtonHighlighted(int,bool);

  void ContextCallback(int);
  void slotReadConfig ();
  void slotHotSpot (int i);


private:
   QPoint pointerOffset;
   QPoint parentOffset;
   int item_size;  // normal: 26
   int icon_text;  // 1 = icon+text, 0 icon+tooltip
   bool highlight; // yes/no
   QSize szh;      // Size for sizeHint
   bool fixed_size; // do not change the toolbar size
   bool transparent; // type of moving
  };

#endif
