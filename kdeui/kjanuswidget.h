/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef _KJANUS_WIDGET_H_
#define _KJANUS_WIDGET_H_

#include <qgrid.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qwidget.h>

#include <klistbox.h>
#include <klistview.h>


class QGrid;
class QHBox;
class QLabel;
class QTabWidget;
class QVBox;
class QWidgetStack;
class KSeparator;

/**
 * This class provides a number of ready to use layouts (faces). It is used 
 * as an internal widget in @ref KDialogBase, but can also used as a 
 * widget of its own. 
 *
 * It provides TreeList, IconList, Tabbed, Plain and Swallow layouts. 
 * 
 * The TreeList face provides a list in the left area and pages in the 
 * right. The area are separated by a movable splitter. The style is somewhat
 * similar to the layout in the Control Center. A page is raised by 
 * selecting the corresponding tree list item.
 * 
 * The IconList face provides an icon list in the left area and pages in the 
 * right. For each entry the Icon is on top with the text below. The style
 * is somewhat similar to the layout of the Eudora configuation dialog box. 
 * A page is raised by selecting the corresponding icon list item. The 
 * preferred icon size is 32x32 pixels.
 *
 * The Tabbed face is a common tabbed widget. The procedure for creating a 
 * page is similar for creating a TreeList. This has the advantage that if
 * your widget contain too many pages it is trivial to convert it into a 
 * TreeList. Just change the face in the @ref KJanusWidget constructor to
 * KJanusWidget::TreeList and you have a tree list layout instead.
 *
 * The Plain face provides an empty widget (QFrame) where you can place your
 * widgets. The KJanusWidget makes no assumptions regarding the contents so
 * you are free to add whatever you want.
 *
 * The Swallow face is provided in order to simplify the usage of existing
 * widgets and to allow changing the visible widget. You specify the widget
 * to be displayed by @ref setSwallowedWidget . Your widget will be 
 * reparented inside the widget. You can specify a Null (0) widget. A empty
 * space is then displayed.
 *
 * For all modes it is important that you specify the @ref QWidget::minimumSize
 * on the page, plain widget or the swallowed widget. If you use a QLayout
 * on the page, plain widget or the swallowed widget this will be taken care
 * of automatically. The size is used when the KJanusWidget determines its 
 * own minimum size. You get the minimum size by using the 
 * @ref minimumSizeHint or @ref sizeHint methods.
 *
 *
 * @short Easy to use widget with many layouts
 * @author Espen Sand (espen@kde.org)
 */
class KJanusWidget : public QWidget
{
  Q_OBJECT

  private:
    class IconListBox : public KListBox
    {
      public:
        IconListBox( QWidget *parent=0, const char *name=0, WFlags f=0 );
	void updateMinimumHeight();
	void invalidateHeight();
	void setShowAll( bool showAll );
      
      private:
	bool mShowAll;
	bool mHeightValid;
    };
  
  public:
    enum Face
    {
      TreeList = 0,
      Tabbed,
      Plain,
      Swallow,
      IconList
    };

  public:

    /** 
     * Constructor where you specify the face.
     * 
     * @param parent Parent of the widget.
     * @param name Widget name.
     * @param int face The kind of dialog, Use TreeList, Tabbed, Plain or
     * Swallow.
     */
    KJanusWidget( QWidget *parent=0, const char *name=0, int face=Plain );

    /**
     * Destructor.
     */
    ~KJanusWidget();

    /**
     * Raises the page which was added by @ref addPage().
     *
     * @parame index The index of the page you want to raise.
     */
    bool showPage( int index );

    /**
     * Retrieve the index of the page that are currently displayed.
     *
     * @return The index or -1 of the face is not TreeList or Tabbed
     */
    int  activePageIndex() const;

    /**
     * Use this to verify 
     * that no memory allocation failed.
     *
     * @return true if the widget was properly created. 
     */
    bool isValid() const;

    /**
     * Retrieve the face type.
     *
     * @return The face type.
     */
    int face() const;

    /**
     * Retrieve the minimum size that must be made available for the widget
     * so that UIs can be displayed properly
     *
     * @return The minimum size.
     */
    virtual QSize minimumSizeHint() const;

    /**
     * Returns the recommended size for the widget in order to be displayed
     * properly.
     *
     * @return The recommended size.
     */
    virtual QSize sizeHint() const;

    /**
     * Retrieve the empty widget that is available in Plain mode.
     *
     * @return The widget or 0 if the face in not Plain.
     */
    QFrame *plainPage();

    /**
     * Add a new page when the class is used in TreeList, IconList or Tabbed 
     * mode. The returned widget is empty and you must add your widgets 
     * as children to this widget. In most cases you must create a layout
     * manager and associate it with this widget as well.
     *
     * @param item String used in the list or Tab item.
     * @param header A longer string used in TreeList and IconList mode to 
     *        describe the contents of a page. If empty, the item string 
     *        will be used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The empty page or 0 if the face is not TreeList, IconList or 
     *         Tabbed.
     */
    QFrame *addPage(const QString &item,const QString &header=QString::null,
		    const QPixmap &pixmap=QPixmap() );

    /**
     * Add a new page when the class is used in TreeList, IconList or Tabbed
     * mode. The returned widget is empty and you must add your widgets 
     * as children to this widget. The returned widget is a @ref QVBox
     * so it contains a QVBoxLayout layout that lines up the child widgets 
     * are vertically.
     *
     * @param item String used in the list or Tab item.
     * @param header A longer string used in TreeList and IconList mode to 
     *        describe the contents of a page. If empty, the item string 
     *        will be used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The empty page or 0 if the face is not TreeList, IconList or 
     *         Tabbed.
     */
    QVBox *addVBoxPage( const QString &item, 
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * Add a new page when the class is used in TreeList, IconList or Tabbed
     * mode. The returned widget is empty and you must add your widgets 
     * as children to this widget. The returned widget is a @ref QHBox
     * so it contains a QHBoxLayout layout that lines up the child widgets 
     * are horizontally.
     *
     * @param item String used in the list or Tab item.
     * @param header A longer string used in TreeList and IconList mode to 
     *        describe the contents of a page. If empty, the item string 
     *        will be used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The empty page or 0 if the face is not TreeList, IconList or 
     *         Tabbed.
     */
    QHBox *addHBoxPage( const QString &itemName, 
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * Add a new page when the class is used in either TreeList or Tabbed 
     * mode. The returned widget is empty and you must add your widgets 
     * as children to this widget. The returned widget is a @ref QGrid
     * so it contains a QGridLayout layout that places up the child widgets 
     * in a grid.
     *
     * @param n Specifies the number of columns if 'dir' is QGrid::Horizontal
     *          or the number of rows if 'dir' is QGrid::Vertical.
     * @param dir Can be QGrid::Horizontal or QGrid::Vertical.
     * @param item String used in the list or Tab item.
     * @param header A longer string used in TreeList and IconList mode to 
     *        describe the contents of a page. If empty, the item string 
     *        will be used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The empty page or 0 if the face is not TreeList, IconList or 
     *         Tabbed.
     */
    QGrid *addGridPage( int n, QGrid::Direction dir, 
			const QString &itemName, 
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * Defines the widget to be swallowed. 
     *
     * This method can be used several 
     * times. Only the latest defined widget will be shown.
     *
     * @param widget The widget to be swallowed. If 0, then an empty rectangle
     * is displayed.
     */
    bool setSwallowedWidget( QWidget *widget );

    /**
     * This function has only effect in TreeList mode.
     *
     * Defines how the tree list is resized when the widget is resized
     * horizontally. By default the tree list keeps its width when the  
     * widget becomes wider.
     *
     * @param state The resize mode. If false (default) the TreeList keeps
     *              its current width when the widget becomes wider.
     */
     void setTreeListAutoResize( bool state );

    /**
     * This function has only effect in IconList mode.
     *
     * Defines how the icon list widget is displayed. By default it is 
     * the widgets in the pages that decide the minimum height 
     * of the toplevel widget. A vertical scrollbar can be used in 
     * the icon list area.
     *
     * @param state The visibility mode. If true, the minimum height is 
     *        adjusted so that every icon in the list is visible at the 
     *        same time. The vertical scrollbar will never be visible.
     */
    void setIconListAllVisible( bool state );

  public slots:
    /**
     * Give the keyboard input focus to the widget.
     */
    virtual void setFocus(); 

  protected:
    /**
     * Reimplemented to handle the splitter width when the the face
     * is TreeList
     */
    virtual void showEvent( QShowEvent * );

    /**
     * This function is used internally when in IconList mode. If you 
     * reimplment this class a make your own event filter, make sure to
     * call this function from your filter.
     *
     * @param o Object that has received an event.
     * @param e The event.
     */
    virtual bool eventFilter( QObject *o, QEvent *e );

  private slots:
    bool slotShowPage();
    void slotFontChanged();

  private:
    bool showPage( QWidget *w );
    void addPageWidget( QFrame *page, const QString &itemName, 
			const QString &header, const QPixmap &pixmap );

  private:
    bool mValid;

    QList<QWidget> *mPageList;
    QList<QListViewItem> *mTreeNodeList;
    QList<QListBoxItem> *mIconNodeList;
    QStringList *mTitleList;

    int          mFace;
    KListView    *mTreeList;
    IconListBox  *mIconList;
    QWidgetStack *mPageStack;
    QLabel       *mTitleLabel;
    QTabWidget   *mTabControl;
    QFrame       *mPlainPage;
    QWidget      *mSwallowPage;
    QWidget      *mActivePageWidget;
    KSeparator   *mTitleSep;
    int          mActivePageIndex;
    QSplitter::ResizeMode mTreeListResizeMode;

    class KJanusWidgetPrivate;
    KJanusWidgetPrivate *d;
};

#endif





