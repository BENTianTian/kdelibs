/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>

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
#ifndef KLISTVIEW_H
#define KLISTVIEW_H

#include <qcursor.h>
#include <qlistview.h>

/**
 * This Widget extends the functionality of QListView to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either
 * QListView::clicked() or QListView::doubleClicked() depending on the KDE
 * wide Single Click/Double Click settings. It is strongly recomended that
 * you use this signal instead of the above mentioned. This way you don�t
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the
 * QListView::selectionChanged() signal.
 **/
class KListView : public QListView
{
    Q_OBJECT

public:
    KListView( QWidget *parent = 0, const char *name = 0 );

  /**
   * This function determines whether the given coordinates are within the 
   * execute area. The execute area is the part of an QListViewItem where mouse
   * clicks or double clicks respectively generate a executed() signal.
   * Depending on @ref QListView::allColumnsShowFocus() this is either the 
   * whole item or only the first column.
   * @return true if point is inside execute area of an item, false in all 
   * other cases including the case that it is over the viewport.
   */
  virtual bool isExecuteArea( const QPoint& point );

signals:

  /**
   * This signal is emitted whenever the user executes an listview item. 
   * That means depending on the KDE wide Single Click/Double Click 
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listview item. 
   *
   * Note that you may not delete any @ref QListViewItem objects in slots 
   * connected to this signal.
   */
  void executed( QListViewItem *item );

  /**
   * This signal is emitted whenever the user executes an listview item. 
   * That means depending on the KDE wide Single Click/Double Click 
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listview item. 
   * @param pos is the position where the user has clicked
   * @param c is the column into which the user clicked. 
   *
   * Note that you may not delete any @ref QListViewItem objects in slots 
   * connected to this signal.
   */
  void executed( QListViewItem *item, const QPoint &pos, int c );

  /**
   * This signal gets emitted whenever the user double clicks into the 
   * listview. 
   * @param item is the pointer to the clicked listview item. 
   * @param pos is the position where the user has clicked, and 
   * @param c the column into which the user clicked. 
   *
   * Note that you may not delete any @ref QListViewItem objects in slots
   * connected to this signal.  
   *
   * This signal is more or less here for the sake of completeness.
   * You should normally not need to use this. In most cases it�s better 
   * to use @ref #executed instead.
   */
  void doubleClicked( QListViewItem *item, const QPoint &pos, int c );

protected slots:
  void slotOnItem( QListViewItem *item );
  void slotOnViewport();

  void slotSettingsChanged(int);

  /**
   * Auto selection happend.
   */
  void slotAutoSelect();

protected:
  void emitExecute( QListViewItem *item, const QPoint &pos, int c );

  virtual void focusOutEvent( QFocusEvent *fe );
  virtual void leaveEvent( QEvent *e );
  virtual void contentsMousePressEvent( QMouseEvent *e );
  virtual void contentsMouseMoveEvent( QMouseEvent *e );
  virtual void contentsMouseDoubleClickEvent ( QMouseEvent *e );
 
  QCursor oldCursor;
  bool m_bUseSingle;
  bool m_bChangeCursorOverItem;

  QListViewItem* m_pCurrentItem;
  bool m_cursorInExecuteArea;

  QTimer* m_pAutoSelect;
  int m_autoSelectDelay;

private slots:
  void slotMouseButtonClicked( int btn, QListViewItem *item, const QPoint &pos, int c );

private:
  class KListViewPrivate;
  KListViewPrivate *d;
};

#endif
