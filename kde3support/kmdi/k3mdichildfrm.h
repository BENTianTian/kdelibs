//----------------------------------------------------------------------------
//    filename             : k3mdichildfrm.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : */2000        Lars Beikirch (Lars.Beikirch@gmx.net)
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//------------------------------------------------------------------------------
#ifndef _K3MDI_CHILD_FRM_H_
#define _K3MDI_CHILD_FRM_H_

#include <q3ptrlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdatetime.h>
#include <qlayout.h>
#include <Q3Frame>
#include <QCustomEvent>
#include <q3dict.h>

#include "k3mdichildfrmcaption.h"

class K3MdiChildArea;
class K3MdiChildView;
class Q3PopupMenu;
class QToolButton;

//==============================================================================
/**
  * @short Internal class, only used on Win32.
  * This class provides a label widget that can process mouse click events.
  */
class K3MDI_EXPORT K3MdiWin32IconButton : public QLabel
{
	Q_OBJECT
public:
	K3MdiWin32IconButton( QWidget* parent, const char* name = 0 );
	virtual void mousePressEvent( QMouseEvent* );

signals:
	void pressed();
};

//==============================================================================
/* some special events, see k3mdidefines.h
*/ 
//------------------------------------------------------------------------------
/**
 * @short a QCustomEvent for move
 * This special event will be useful, to inform view about child frame event.
 */
class K3MDI_EXPORT K3MdiChildFrmMoveEvent : public QCustomEvent
{
public:
	K3MdiChildFrmMoveEvent( QMoveEvent *e ) : QCustomEvent( QEvent::Type( QEvent::User + int( K3Mdi::EV_Move ) ), e ) {}

};

//------------------------------------------------------------------------------
/**
 * @short a QCustomEvent for begin of dragging
 * This special event will be useful, to inform view about child frame event.
 */
class K3MDI_EXPORT K3MdiChildFrmDragBeginEvent : public QCustomEvent
{
public:
	K3MdiChildFrmDragBeginEvent( QMouseEvent *e ) : QCustomEvent( QEvent::Type( QEvent::User + int( K3Mdi::EV_DragBegin ) ), e ) {}
};

//------------------------------------------------------------------------------
/**
 * @short a QCustomEvent for end of dragging
 * This special event will be useful, to inform view about child frame event.
 */
class K3MDI_EXPORT K3MdiChildFrmDragEndEvent : public QCustomEvent
{
public:
	K3MdiChildFrmDragEndEvent( QMouseEvent *e ) : QCustomEvent( QEvent::Type( QEvent::User + int( K3Mdi::EV_DragEnd ) ), e ) {}
};

//------------------------------------------------------------------------------
/**
 * @short a QCustomEvent for begin of resizing
 * This special event will be useful, to inform view about child frame event.
 */
class K3MDI_EXPORT K3MdiChildFrmResizeBeginEvent : public QCustomEvent
{
public:
	K3MdiChildFrmResizeBeginEvent( QMouseEvent *e ) : QCustomEvent( QEvent::Type( QEvent::User + int( K3Mdi::EV_ResizeBegin ) ), e ) {}
};

//------------------------------------------------------------------------------
/**
 * @short a QCustomEvent for end of resizing
 * This special event will be useful, to inform view about child frame event.
 */
class K3MDI_EXPORT K3MdiChildFrmResizeEndEvent : public QCustomEvent
{
public:
	K3MdiChildFrmResizeEndEvent( QMouseEvent *e ) : QCustomEvent( QEvent::Type( QEvent::User + int( K3Mdi::EV_ResizeEnd ) ), e ) {}
};


class K3MdiChildFrmPrivate;
//==============================================================================
/**
  * @short Internal class.
  * It's an MDI child frame widget. It contains a view widget and a frame caption. Usually you derive from its view.
  */ 
//------------------------------------------------------------------------------
class K3MDI_EXPORT K3MdiChildFrm : public Q3Frame
{
	friend class K3MdiChildArea;
	friend class K3MdiChildFrmCaption;

	Q_OBJECT

	// attributes
public:
	enum MdiWindowState { Normal, Maximized, Minimized };
	//positions same in h and cpp for fast order check
	K3MdiChildView* m_pClient;

protected:
	K3MdiChildArea* m_pManager;
	K3MdiChildFrmCaption* m_pCaption;
	K3MdiWin32IconButton* m_pWinIcon;
	QToolButton* m_pUnixIcon;
	QToolButton* m_pMinimize;
	QToolButton* m_pMaximize;
	QToolButton* m_pClose;
	QToolButton* m_pUndock;
	MdiWindowState m_state;
	QRect m_restoredRect;
	int m_iResizeCorner;
	int m_iLastCursorCorner;
	bool m_bResizing;
	bool m_bDragging;
	QPixmap* m_pIconButtonPixmap;
	QPixmap* m_pMinButtonPixmap;
	QPixmap* m_pMaxButtonPixmap;
	QPixmap* m_pRestoreButtonPixmap;
	QPixmap* m_pCloseButtonPixmap;
	QPixmap* m_pUndockButtonPixmap;
	
	/**
	* Every child frame window has an temporary ID in the Window menu of the child area.
	*/
	int m_windowMenuID;
	
	/**
	* Imitates a system menu for child frame windows
	*/
	Q3PopupMenu* m_pSystemMenu;
	
	QSize m_oldClientMinSize;
	QSize m_oldClientMaxSize;
	QLayout::SizeConstraint m_oldLayoutResizeMode;
	QTime m_timeMeasure;

	// methods
public:
	
	/**
	 * Creates a new K3MdiChildFrm class.
	 */
	K3MdiChildFrm( K3MdiChildArea *parent );
	
	/**
	 * Destroys this K3MdiChildFrm
	 * If a child is still here managed (no recreation was made) it is destroyed too.
	 */
	~K3MdiChildFrm();
	
	/**
	 * Reparents the widget w to this K3MdiChildFrm (if this is not already done)
	 * Installs an event filter to catch focus events.
	 * Resizes this mdi child in a way that the child fits perfectly in.
	 */
	void setClient( K3MdiChildView *w, bool bAutomaticResize = false );
	
	/**
	 * Reparents the client widget to 0 (desktop), moves with an offset from the original position
	 * Removes the event filter.
	 */
	void unsetClient( QPoint positionOffset = QPoint( 0, 0 ) );
	
	/**
	 * Sets the window icon pointer.
	 */
	void setIcon( const QPixmap &pxm );
	
	/**
	 * Returns the child frame icon.
	 */
	QPixmap* icon() const;
	
	/**
	 * Enables or disables the close button
	 */
	void enableClose( bool bEnable );
	
	/**
	 * Sets the caption of this window
	 */
	void setCaption( const QString& text );
	
	/**
 	 * Gets the caption of this mdi child.
	 */
	const QString& caption() { return m_pCaption->m_szCaption; }
	
	/**
	 * Minimizes, Maximizes, or restores the window.
	 */
	void setState( MdiWindowState state, bool bAnimate = true );
	
	/**
	 * Returns the current state of the window
	 */
	inline MdiWindowState state() const { return m_state; }
	
	/**
	 * Returns the inner client area of the parent of this (which is K3MdiChildArea).
	 */
	QRect mdiAreaContentsRect() const;
	
	/**
	 * Returns the geometry that will be restored by calling restore().
	 */
	QRect restoreGeometry() const;
	
	/**
	 * Sets the geometry that will be restored by calling restore().
	 */
	void setRestoreGeometry( const QRect& newRestGeo );
	
	/**
	 * Forces updating the rects of the caption and so...
	 * It may be useful when setting the mdiCaptionFont of the MdiManager
	 */
	void updateRects() { resizeEvent( 0 ); }
	
	/**
	 * Returns the system menu.
	 */
	Q3PopupMenu* systemMenu() const;
	
	/**
	 * Returns the caption bar height 
	 */
	inline int captionHeight() const { return m_pCaption->height(); }
	
	/**
	 * sets new raise behavior and pixmaps of the buttons depending on the current decoration style
	 */
	void redecorateButtons();

	/**
	 * returns the mouse state "In Drag"
	 */
	bool isInDrag() const { return m_bDragging; }
	
	/**
	 * returns the mouse state "In Resize"
	 */
	bool isInResize() const { return m_bResizing; }
	
	/**
	 * Internally called from the signal focusInEventOccurs.
	 * It raises the MDI childframe to the top of all other MDI child frames and sets the focus on it.
	 */
	void raiseAndActivate();

	/**
	 * Sets the minimum size of the widget to w by h pixels.
	 * It extends it's base clase method in a way that the minimum size of
	 * the child area will be set additionally if the view is maximized.
	 */
	virtual void setMinimumSize ( int minw, int minh );

public slots:
	
	void slot_resizeViaSystemMenu();

protected:
	
	/**
	 * Reimplemented from the base class.
	 * Resizes the captionbar, relayouts the position of the system buttons,
	 * and calls resize for its embedded K3MdiChildView with the proper size
	 */
	virtual void resizeEvent( QResizeEvent * );
	
	/**
	 * Reimplemented from the base class.
	 * Detects if the mouse is on the edge of window and what resize cursor must be set.
	 * Calls K3MdiChildFrm::resizeWindow if it is in m_bResizing.
	 */
	virtual void mouseMoveEvent( QMouseEvent *e );
	
	/**
	 * Reimplemented from the base class.
	 * Colours the caption, raises the childfrm widget and
	 * turns to resize mode if it is on the edge (resize-sensitive area)
	 */
	virtual void mousePressEvent( QMouseEvent *e );
	
	/**
	 * Reimplemented from the base class.
	 * Sets a normal cursor and leaves the resize mode.
	 */
	virtual void mouseReleaseEvent( QMouseEvent * );
	
	/**
	 * Reimplemented from the base class.
	 * give its child view the chance to notify a childframe move... that's why it sends
	 * a K3MdiChildMovedEvent to the embedded K3MdiChildView .
	 */
	virtual void moveEvent( QMoveEvent* me );
	
	/**
	 * Reimplemented from the base class. If not in resize mode, it sets the mouse cursor to normal appearance.
	 */
	virtual void leaveEvent( QEvent * );
	
	/** 
	 * Reimplemented from the base class.
	 * In addition, the following are caught
	 * -the client's mousebutton press events which raises and activates the childframe
	 * -the client's resize event which resizes this widget as well
	 */
	virtual bool eventFilter( QObject*, QEvent* ); 
	
	/**
	 * Calculates the new geometry from the new mouse position given as parameters
	 * and calls K3MdiChildFrm::setGeometry
	 */
	void resizeWindow( int resizeCorner, int x, int y );
	
	/**
	 * Override the cursor appearance depending on the widget corner given as parameter
	 */
	void setResizeCursor( int resizeCorner );
	
	/**
	 * Changes from the resize cursor to the normal (previous) cursor
	 */
	void unsetResizeCursor();
	
	/**
	 * That means to show a mini window showing the childframe's caption bar, only.
	 * It cannot be resized.
	 */
	virtual void switchToMinimizeLayout();
	
	/**
	 * Does the actual resize. Called from various places but from resizeEvent in general.
	 */
	void doResize();
	
	/**
	 * Does the actual resize, like doResize() but skips resize of the client if \a captionOnly is true.
	 * @todo: merge with doResize()
	 */
	void doResize( bool captionOnly );

protected slots:
	
	/**
	 * Handles a click on the Maximize button
	 */
	void maximizePressed();
	
	/**
	 * Handles a click on the Restore (Normalize) button
	 */
	void restorePressed();
	
	/**
	 * Handles a click on the Minimize button.
	 */
	void minimizePressed();
	
	/**
	 * Handles a click on the Close button.
	 */
	void closePressed();
	
	/**
	 * Handles a click on the Undock (Detach) button
	 */
	void undockPressed();
	
	/**
	 * Shows a system menu for child frame windows.
	 */
	void showSystemMenu();

protected:

	/**
	 * Restore the focus policies for _all_ widgets in the view using the list given as parameter.
	 * Install the event filter for all direct child widgets of this. (See K3MdiChildFrm::eventFilter)
	 */
	void linkChildren( Q3Dict<Qt::FocusPolicy>* pFocPolDict );

	/**
	 * Backups all focus policies of _all_ child widgets in the MDI childview since they get lost during a reparent.
	 * Remove all event filters for all direct child widgets of this. (See K3MdiChildFrm::eventFilter)
	 */
	Q3Dict<Qt::FocusPolicy>* unlinkChildren();
	
	/**
	 * Calculates the corner id for the resize cursor. The return value can be tested for:
	 * K3MDI_RESIZE_LEFT, K3MDI_RESIZE_RIGHT, K3MDI_RESIZE_TOP, KMDI_RESIZE_BOTTOM
	 * or an OR'd variant of them for the corners.
	 */
	int getResizeCorner( int ax, int ay );

private:
	K3MdiChildFrmPrivate *d;
};

#endif //_K3MDICHILDFRM_H_

// kate: space-indent off; replace-tabs off; tab-width 4; indent-mode csands;
