/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Mirko Sucker (mirko@kde.org) and 
 *  Espen Sand (espen@kde.org)
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
 *
 */
#ifndef _KDIALOG_BASE_H_
#define _KDIALOG_BASE_H_

#include <qpushbutton.h>
#include <kdialog.h>
#include <kjanuswidget.h>
#include <qlist.h>

class KSeparator;
class KURLLabel;
class QVBoxLayout;
class QPixmap;

/**
 * Used internally by @ref KDialogBase.
 * @internal
 */
class KDialogBaseButton : public QPushButton
{
  Q_OBJECT
  
  public:
    KDialogBaseButton( const QString &text, int key, QWidget *parent=0, 
		       const char *name=0 ); 
    inline int id();

  private:
    int mKey;
};


inline int KDialogBaseButton::id()
{
  return( mKey );
}



/**
 * Used internally by @ref KDialogBase.
 * @internal
 */
class KDialogBaseTile : public QObject
{
  Q_OBJECT

  public:
    KDialogBaseTile( QObject *parent=0, const char *name=0 );
    ~KDialogBaseTile();

    void set( const QPixmap *pix );
    const QPixmap *get() const;
  
  public slots:
    void cleanup();

  signals:
    void pixmapChanged();

  private:
    QPixmap *mPixmap;
    class KDialogBaseTilePrivate;
    KDialogBaseTilePrivate *d;
};


/**
 * This base class provides basic functionality needed by nearly all dialogs.
 * It offers the standard action buttons you'd expect to find in a dialog
 * as well as the ability to define at most three configurable buttons. You 
 * can define a main widget which contains your specific dialog layout or use 
 * a predefined layout. Currently, TreeList/Paged, Tabbed, Plain, Swallow
 * and IconList mode layouts (faces) are available.
 *
 * The class takes care of the geometry management. You only need to define
 * a minimum size for the widget you want to use as the main widget.
 *
 * You can set a background tile (pixmap) for parts of the dialog. The 
 * tile you select is shared by all instances of this class in your 
 * application so that they all get the same look and feel.
 *
 * There is a tutorial available on http://developer.kde.org/ (NOT YET) 
 * that contains
 * copy/paste examples as well a screenshots on how to use this class.
 *
 * @sect Standard buttons (action buttons):
 *
 * You select which buttons should be displayed, but you do not choose the 
 * order in which they are displayed. This ensures a standard interface in 
 * KDE. The button order can be changed, but this ability is only available
 * for a central KDE control tool. The following buttons are available:
 * OK, Cancel/Close, Apply/Try, Default, Help and three user definable 
 * buttons: User1, User1 and User3. You must specify the text of the UserN 
 * buttons. Each button has a virtual slot so you can overload the method 
 * when required. The default slots emit a signal as well, so you can choose
 * to connect a signal instead of overriding the slot.
 * The default implementation of @ref slotHelp() will automatically enable the 
 * help system if you have provided a path to the help text. @ref slotCancel() 
 * and @ref slotClose() will run @ref QDialog::reject() while @ref slotOk() 
 * will run @ref QDialog::accept(). You define a default button in the 
 * constructor.
 *
 * @sect Dialog shapes:
 *
 * You can either use one of the prebuilt, easy to use, faces or
 * define your own main widget. The dialog provides ready to use
 * TreeList,  Tabbed, Plain, Swallow and IconList faces. For the first
 * two you then add pages with @ref addPage(). If you want complete
 * control of how the dialog contents should look, then you can define
 * a main widget by using @ref setMainWidget(). You only need to set
 * the minimum size of that widget and the dialog will resize itself
 * to fit this minimum size.  The dialog is resizeable, but can not be
 * made smaller than its minimum 
 * size.
 *
 * @sect Layout:
 *
 * The dialog consists of a help area on top (becomes visible if you define
 * a help path and use @ref enableLinkedHelp()), the built-in dialog face or
 * your own widget in the middle, and the button row at the bottom. You can
 * also specify a separator to be shown above the button row.
 *
 * @sect Standard compliance:
 *
 * The class is derived form @ref KDialog(), so you get automatic access to
 * the @ref KDialog::marginHint(), @ref KDialog::spacingHint() and the 
 * extended @ref KDialog::setCaption() method. NOTE: The main widget you 
 * use will be positioned inside the dialog using a margin (or border) 
 * equal to @ref KDialog::marginHint(). You shall not add a margin yourself.
 * The example below (from kedit) shows how you use the top level widget
 * and its layout. The second argument (the border) to <tt>QVBoxLayout</tt> 
 * is 0. This situation is valid for @ref addPage , @ref addVBoxPage ,
 * @ref addHBoxPage , @ref addGridPage , @ref makeMainWidget, 
 * @ref makeVBoxMainWidget , @ref makeHBoxMainWidget and 
 * @ref makeGridMainWidget as well.
 * 
 * <pre>
 * UrlDlg::UrlDlg( QWidget *parent, const QString& caption, 
 *		const QString& urltext)
 * : KDialogBase( parent, "urldialog", true, caption, Ok|Cancel, Ok, true )
 * {
 *   QWidget *page = new QWidget( this ); 
 *   setMainWidget(page);
 *   QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
 * 
 *   QLabel *label = new QLabel( caption, page, "caption" );
 *   topLayout->addWidget( label );
 * 
 *   lineedit = new QLineEdit( urltext, page, "lineedit" );
 *   lineedit->setMinimumWidth(fontMetrics().maxWidth()*20);
 *   topLayout->addWidget( lineedit );
 *
 *   topLayout->addStretch(10); 
 * }
 * </pre>
 *
 * If you use @ref makeVBoxMainWidget , then the dialog above can be made
 * simpler but you loose the ability to add a stretchable area.
 *
 * <pre>
 * UrlDlg::UrlDlg( QWidget *parent, const QString& caption, 
 *		const QString& urltext)
 * : KDialogBase( parent, "urldialog", true, caption, Ok|Cancel, Ok, true )
 * {
 *   QVBox *page = makeVBoxMainWidget();
 *   QLabel *label = new QLabel( caption, page, "caption" );
 *   
 *   lineedit = new QLineEdit( urltext, page, "lineedit" );
 *   lineedit->setMinimumWidth(fontMetrics().maxWidth()*20);
 * }
 * </pre>
 *
 * @short A dialog base class which standard buttons and predefined layouts.
 * @author Mirko Sucker (mirko@kde.org) and Espen Sand (espen@kde.org)
 */
class KDialogBase : public KDialog
{
  Q_OBJECT

  public:

    /**
     *  @li @p Help    Show Help-button.
     *  @li @p Default Show Default-button.
     *  @li @p Ok      Show Ok-button.
     *  @li @p Apply   Show Apply-button.
     *  @li @p Try     Show Try-button.
     *  @li @p Cancel  Show Cancel-button.
     *  @li @p Close   Show Close-button.
     *  @li @p User1   Show User define-button 1.
     *  @li @p User2   Show User define-button 2.
     *  @li @p User3   Show User define-button 3.
     *  @li @p No      Show No-button.
     *  @li @p Yes     Show Yes-button.
     *  @li @p Stretch Used internally. Ignored when used in a constructor.
     */
    enum ButtonCode
    {
      Help    = 0x00000001,
      Default = 0x00000002,
      Ok      = 0x00000004,
      Apply   = 0x00000008,
      Try     = 0x00000010,
      Cancel  = 0x00000020,
      Close   = 0x00000040,
      User1   = 0x00000080,
      User2   = 0x00000100,
      User3   = 0x00000200,
      No      = 0x00000080,
      Yes     = 0x00000100,
      Stretch = 0x80000000
    };

    enum ActionButtonStyle
    {
      ActionStyle0=0, // KDE std
      ActionStyle1,
      ActionStyle2,
      ActionStyle3,
      ActionStyle4,
      ActionStyleMAX
    };

    /**
     *  @li @p TreeList A dialog with a tree on the left side
     *                  and a representation of the contents on thr right side.
     *  @li @p Tabbed   A dialog using a @ref QTabWidget.
     *  @li @p Plain    A normal dialog.
     *  @li @p Swallow  
     *  @li @p IconList A dialog with an iconlist on the left side
     *                  and a representation of the contents on the right side.
     */
    enum DialogType
    {
      TreeList  = KJanusWidget::TreeList,
      Tabbed    = KJanusWidget::Tabbed,
      Plain     = KJanusWidget::Plain,
      Swallow   = KJanusWidget::Swallow,
      IconList  = KJanusWidget::IconList
    };

  private:
    struct SButton
    {
      int mask;
      int style;
      QList<KDialogBaseButton> list;
      
      QPushButton *append( int key, const QString &text )
      {
	KDialogBaseButton *p = new KDialogBaseButton( text, key, box );
	list.append( p );
	return( p );
      }

      void resize( bool sameWidth, int margin, int spacing )
      {
	KDialogBaseButton *p;
	int w = 0;
	int t = 0;

	for( p = list.first(); p!=0; p =  list.next() )
	{
	  if( p->sizeHint().width() > w ) { w = p->sizeHint().width(); }
	}
	for( p = list.first(); p!=0; p =  list.next() )
	{
	  QSize s( p->sizeHint() );
	  if( sameWidth == true ) { s.setWidth( w ); }
	  p->setFixedSize( s );
	  t += s.width() + spacing;
	}
	
	p = list.first();
	box->setMinimumHeight( margin*2 + ( p==0? 0:p->sizeHint().height()) );
	box->setMinimumWidth( margin*2 + t - spacing );
      }


      QPushButton *button( int key )
      {
	KDialogBaseButton *p;
	for( p = list.first(); p != 0; p = list.next() )
	{
	  if( p->id() == key )
	  { 
	    return( p );
	  }
	}
	return( 0 );
      }

      QWidget *box;
    };

  public:

    /** 
     * Constructor for the standard mode where you must specify the main widget
     * with @ref setMainWidget() .
     * 
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only)
     * @param modal Controls dialog modality. If @p false, the rest of the 
     *        program interface (example: other dialogs) is accessible while 
     *        the dialog is open.
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies which buttons will be visible.
     * @param defaultButton Specifies which button we be marked as the default.
     * @param separator If @p true, a separator line is drawn between the 
     *        action buttons and the main widget.
     * @param user1 User button1 text.
     * @param user2 User button2 text.
     * @param user3 User button3 text.
     */
    KDialogBase( QWidget *parent=0, const char *name=0, bool modal=true,
		 const QString &caption=QString::null,
		 int buttonMask=Ok|Apply|Cancel, ButtonCode defaultButton=Ok,
		 bool separator=false, 
		 const QString &user1=QString::null,
		 const QString &user2=QString::null,
		 const QString &user3=QString::null);

    /** 
     * Constructor for the predefined layout mode where you specify the kind of
     * layout (face).
     * 
     * @param dialogFace You can use TreeList, Tabbed, Plain, Swallow or IconList.
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies what buttons will be visible.
     * @param defaultButton Specifies what button we be marked as the default.
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only).
     * @param modal Controls dialog modality. If @p false, the rest of the 
     *        program interface (example: other dialogs) is accessible while 
     *        the dialog is open.
     * @param separator If @p true, a separator line is drawn between the 
     *        action buttons and the main widget.
     * @param user1 User button1 text.
     * @param user2 User button2 text.
     * @param user3 User button3 text.
     */
    KDialogBase( int dialogFace, const QString &caption, 
		 int buttonMask, ButtonCode defaultButton, 
		 QWidget *parent=0, const char *name=0, bool modal=true, 
		 bool separator=false,
		 const QString &user1=QString::null,
		 const QString &user2=QString::null,
		 const QString &user3=QString::null);

    
    /** 
     * Constructor for a message box mode where the @p buttonMask can only 
     * contain Yes, No, or Cancel. 
     *
     * If you need other names you can rename
     * the buttons with @ref setButtonText(). The dialog box is not resizeable 
     * by default but this can be changed by @ref setInitialSize(). If you 
     * select 'modal' to be true, the dialog will return Yes, No, or Cancel 
     * when closed otherwise you can use the signals @ref yesClicked(), 
     * @ref noClicked(), or @ref cancelClicked() to determine the state.
     * 
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies what buttons will be visible.
     * @param defaultButton Specifies what button we be marked as the default.
     * @param escapeButton Specifies what button that will be activated by
     *        when the dialog receives a @p Key_Escape keypress. 
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only).
     * @param modal Controls dialog modality. If @p false, the rest of the 
     *        program interface (example: other dialogs) is accessible 
     *        while the dialog is open.
     * @param separator If @p true, a separator line is drawn between the 
     *        action buttons and the main widget.
     * @param user1 User button1 text.
     * @param user2 User button2 text.
     * @param user3 User button3 text.
     */
    KDialogBase( const QString &caption, int buttonMask=Yes|No|Cancel, 
		 ButtonCode defaultButton=Yes, ButtonCode escapeButton=Cancel, 
		 QWidget *parent=0, const char *name=0, bool modal=true, 
		 bool separator=false,
		 QString yes = QString::null, // i18n("&Yes")
		 QString no = QString::null, // i18n("&No"),
		 QString cancel = QString::null // i18n("&Cancel") 
	       );

    /**
     * Destructor.
     */
    ~KDialogBase();

    /**
     * Adjust the size of the dialog to fit the contents just before
     * @ref QDialog::exec() or @ref QDialog::show() is called.
     *
     * This method will not 
     * be called if the dialog has been explicitly resized before 
     * showing it.
     **/
    virtual void adjustSize();

    /**
     * Hide or display the a separator line drawn between the action
     * buttons an the main widget.
     */
    void enableButtonSeparator( bool state );
    
    /**
     * Hide or display a general action button.
     *
     *  Only buttons that have
     * been created in the constructor can be displayed. This method will
     * not create a new button.
     *
     * @param id Button identifier.
     * @param state true display the button(s). 
     */
    void showButton( ButtonCode id, bool state );
    
    /**
     * Hide or display the OK button.
     *
     *  The OK button must have
     * been created in the constructor to be displayed.
     *
     * @param state If @p true, display the button(s). 
     */
    void showButtonOK( bool state );
    
    /**
     * Hide or display the Apply button.
     *
     *  The Apply button must have
     * been created in the constructor to be displayed.
     *
     * @param state true display the button(s). 
     */
    void showButtonApply( bool state );

    /**
     * Hide or display the Cancel button. The Cancel button must have
     * been created in the constructor to be displayed.
     *
     * @param state @p true display the button(s). 
     */
    void showButtonCancel( bool state );

    /**
     * Set the page with @ref index to be displayed.
     *
     *  This method will only 
     * work when the dialog is using the predefined shape of TreeList or
     * Tabbed.
     *
     * @param index Index of the page to be shown.
     * @return @p true if the page is shown, @p false otherwise.
     */
    bool showPage( int index );

    /**
     * Retrieve the index of the active page.
     *
     * This method will only work when the dialog is using the 
     * predefined shape of TreeList or Tabbed.
     *
     * @return The page index or -1 if there is no active page.
     */
    int  activePageIndex() const;

    /**
     * Set the main user definable widget.
     *
     * If the dialog is using the predefined Swallow mode, the widget will 
     * be reparented to the internal swallow control widget. If the dialog 
     * is being used in the standard mode then the @p widget must have the 
     * dialog as parent.
     *
     * @param widget The widget to be displayed as main widget. If it
     * is 0, then the dialog will show an empty space of 100x100 pixels
     * instead.
     */
    void setMainWidget( QWidget *widget );

    /**
     * Retrieve the main widget if any.
     *
     * @return The current main widget. Can be 0 if no widget has been defined.
     */
    QWidget *getMainWidget(); 

    /**
     * Convenience method.
     *
     *  Freezes the dialog size using the minimum size 
     * of the dialog. This method should only be called right before 
     * @ref show() or @ref exec().
     */
    void disableResize();

    /**
     * Convenience method. Set the initial dialog size.
     *
     *  This method should 
     * only be called right before @ref show() or @ref exec(). The initial
     * size will be 
     * ignored if smaller than the dialog's minimum size.
     *
     * @param s Startup size.
     * @param noResize If @p true the dialog can not be resized.
     */
    void setInitialSize( const QSize &s, bool noResize=false );

    /**
     * Convenience method. Addd a size to the default minimum size of a 
     * dialog.
     *
     * This method should only be called right before @ref show() or 
     * @ref exec().
     *
     * @param s  Size added to minimum size.
     * @param noResize If @p true the dialog can not be resized.
     */
    void incInitialSize( const QSize &s, bool noResize=false );

    /** 
     * Sets the text of the OK button.
     *
     * If the default parameters are used 
     * (that is, if no parameters are given) the standard texts are set:
     * The button shows "OK", the tooltip contains "Accept settings." 
     * (internationalized) and the quickhelp text explains the standard 
     * behavior of the OK button in dialogs. 
     *
     * @param text Button text.
     * @param tooltip Tooltip text.
     * @param quickhelp Quick help text.
     */
    void setButtonOKText( const QString &text=QString::null, 
			  const QString &tooltip=QString::null, 
			  const QString &quickhelp=QString::null );

    /** 
     * Sets the text of the Apply button.
     *
     * If the default parameters are 
     * used (that is, if no parameters are given) the standard texts are set:
     * The button shows "Apply", the tooltip contains "Apply settings." 
     * (internationalized) and the quickhelp text explains the standard 
     * behavior of the apply button in dialogs. 
     *
     * @param text Button text.
     * @param tooltip Tooltip text.
     * @param quickhelp Quick help text.
     */
    void setButtonApplyText( const QString &text=QString::null, 
			     const QString &tooltip=QString::null, 
			     const QString &quickhelp=QString::null );

    /** 
     * Set the text of the Cancel button.
     *
     *  If the default parameters are 
     * used (that is, if no parameters are given) the standard texts are set:
     * The button shows "Cancel", the tooltip contains "Cancel settings." 
     * (internationalized) and the quickhelp text explains the standard 
     * behaviour of the cancel button in dialogs. 
     *
     * @param text Button text.
     * @param tooltip Tooltip text.
     * @param quickhelp Quick help text.
     */
    void setButtonCancelText( const QString &text=QString::null, 
			      const QString &tooltip=QString::null, 
			      const QString &quickhelp=QString::null );

    /**
     * Set the text of any button.
     * 
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonText( ButtonCode id, const QString &text );

    /**
     * Set the tooltip text of any button.
     * 
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonTip( ButtonCode id, const QString &text );

    /**
     * Sets the "What's this?" text of any button.
     * 
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonWhatsThis( ButtonCode id, const QString &text );

    
    /**
     * This function has only effect in TreeList mode.
     *
     * Defines how the tree list widget is resized when the dialog is 
     * resized horizontally. By default the tree list keeps its width 
     * when the dialog becomes wider.
     *
     * @param state The resize mode. If false (default) the tree list keeps
     *        its current width when the dialog becomes wider.
     */
     void setTreeListAutoResize( bool state );
     
    /**
     * This function has only effect in IconList mode.
     *
     * Defines how the icon list widget is displayed. By default it is 
     * the widgets in the dialog pages that decide the minimum height 
     * of the dialog. A vertical scrollbar can be used in the icon list
     * area.
     *
     * @param state The visibility mode. If true, the minimum height is 
     *        adjusted so that every icon in the list is visible at the 
     *        same time. The vertical scrollbar will never be visible.
     */
    void setIconListAllVisible( bool state );
     

    /** 
     * Check whether the background tile is set or not. 
     *
     * @return @true if there is defined a background tile.
     */
    static bool haveBackgroundTile();
    
    /** 
     * Retrieve a pointer to the background tile if there is one. 
     *
     * @return The tile pointer or 0 if no tile is defined.
     *
     **/
    static const QPixmap *getBackgroundTile();
    
    /** 
     * Set the background tile.
     *
     *  If it is Null (0), the background image is deleted. 
     *
     * @param pix The background tile.
     */
    static void setBackgroundTile( const QPixmap *pix );

    /**
     * Enable hiding of the background tile (if any).
     *
     * @param state @p true will make the tile visible. 
     */
    void showTile( bool state );

    /** 
     * Do not use this method. It is included for compatibility reasons.     
     *
     * This method returns the border widths in all directions the dialog 
     * needs for itself. Respect this, or get bad looking results. 
     * The references are upper left x (@p ulx), upper left y (@p uly),
     * lower right x (@p lrx), and lower left y (@p lly).
     * The results are differences in pixels from the 
     * dialogs corners. 
     */
    void  getBorderWidths( int& ulx, int& uly, int& lrx, int& lry ) const;

    /**
     * Do not use this method. It is included for compatibility reasons. 
     *
     * This method returns the contents rectangle of the work area. Place 
     * your widgets inside this rectangle, and use it to set up 
     * their geometry. Be careful: The rectangle is only valid after 
     * resizing the dialog, as it is a result of the resizing process. 
     * If you need the "overhead" the dialog needs for its elements, 
     * use @ref getBorderWidths().
     */
    QRect getContentsRect();

    /** 
     * Calculate the size hint for the dialog.
     *
     * With this method it is easy to calculate a size hint for a 
     * dialog derived from KDialogBase if you know the width and height of 
     * the elements you add to the widget. The rectangle returned is 
     * calculated so that all elements exactly fit into it. Thus, you may 
     * set it as a minimum size for the resulting dialog. 
     *
     * You should not need to use this method and never if you use one of
     * the predefined shapes.
     *
     * @param w The width of you special widget.
     * @param h The height of you special widget.
     * @return The minimum width and height of the dialog using @p w and @p h
     * as the size of the main widget.
     */
    QSize calculateSize( int w, int h );

    /**
     * Retrieve the help link text.
     *
     *  If no text has been defined, 
     * "Get help..." (internationalized) is returned.
     *
     * @return The help link text.
     */
    QString helpLinkText();

    /**
     * Returns the action button that corresponds to the id. Normally
     * you should not use this function. NEVER delete the object returned
     * by this function. See also @ref enableButton @ref showButton 
     * @ref setButtonTip @ref setButtonWhatsThis and @ref setButtonText
     *
     * @param id Integer identifier of the button.
     * @return The action button or 0 if the button does not exists.
     * 
     */
    QPushButton *actionButton( ButtonCode id );

  public slots:
    /**
     * Enable or disable (gray out) a general action button.
     *
     * @param id Button identifier.
     * @param state @p true enables the button(s). 
     */
    void enableButton( ButtonCode id, bool state );

    /**
     * Enable or disable (gray out) the OK button.
     *
     * @param state @p true enables the button. 
     */
    void enableButtonOK( bool state );

    /**
     * Enable or disable (gray out) the Apply button.
     *
     * @param state true enables the button. 
     */
    void enableButtonApply( bool state );
    
    /**
     * Enable or disable (gray out) the Cancel button.
     *
     * @param state true enables the button. 
     */
    void enableButtonCancel( bool state );

    /**
     * Display or hide the help link area on the top of the dialog.
     *
     * @param state @p true will display the area.
     */
    void enableLinkedHelp( bool state );

    /**
     * Set the text that is shown as the linked text.
     *
     * If text is empty, 
     * the text "Get help..." (internationalized) is used instead.
     *
     * @param text The link text.
     */
    void setHelpLinkText( const QString &text );
    
    /** 
     * Set the help path and topic.
     *
     * @param path Path to help text.
     * @param topic Topic in help text.
     */
    void setHelp( const QString &path, const QString &topic );

    /** 
     * Connected to help link label. 
     */
    void helpClickedSlot( const QString & );

    /** 
     * This method is called automatically whenever the background has 
     * changed. You do not need to use this method.
     */
    void updateBackground();

  signals:
    /** 
     * The Help button was pressed. This signal is only emitted if
     * @ref slotHelp() is not replaced.
     */
    void helpClicked();

    /** 
     * The Default button was pressed. This signal is only emitted if
     * @ref slotDefault() is not replaced.
     */
    void defaultClicked();
    
    /** 
     * The User3 button was pressed. This signal is only emitted if
     * @ref slotUser3() is not replaced.
     */
    void user3Clicked();

    /** 
     * The User2 button was pressed. This signal is only emitted if
     * @ref slotUser2() is not replaced.
     */
    void user2Clicked();

    /** 
     * The User1 button was pressed. This signal is only emitted if
     * @ref slotUser1() is not replaced.
     */
    void user1Clicked();

    /** 
     * The Apply button was pressed. This signal is only emitted if
     * @ref slotApply() is not replaced.
     */
    void applyClicked();

    /** 
     * The Try button was pressed. This signal is only emitted if
     * @ref slotTry() is not replaced.
     */
    void tryClicked();

    /** 
     * The OK button was pressed. This signal is only emitted if
     * @ref slotOk() is not replaced.
     */
    void okClicked();

    /** 
     * The Yes button was pressed. This signal is only emitted if
     * @ref slotYes() is not replaced.
     */
    void yesClicked();

    /** 
     * The No button was pressed. This signal is only emitted if
     * @ref slotNo() is not replaced.
     */
    void noClicked();

    /** 
     * The Cancel button was pressed. This signal is only emitted if
     * @ref slotCancel() is not replaced.
     */
    void cancelClicked();
    
    /** 
     * The Close button was pressed. This signal is only emitted if
     * @ref slotClose() is not replaced.
     */
    void closeClicked();

    /** 
     * Do not use this signal. Is is kept for compatibility reasons.
     * Use @ref applyClicked() instead.
     */
    void apply();

    /** 
     * The background tile has changed.
     */
    void backgroundChanged(); 

    /**
     * The dialog is about to be hidden.
     *
     *  If you have stored a pointer to the 
     * dialog do @bf not try to delete the pointer in the slot that is 
     * connected to this signal. Instead you must start a timer and delete 
     * the pointer when this timer expires.
     */
    void hidden();

  protected:
    /**
     * Returns the empty page when the predefined layout is used in Plain
     * mode. This widget must used as the toplevel widget of your dialog
     * code.
     *
     * @return The widget or 0 if the predefined layout mode is not Plain
     *         or if you don't use any predefined layout
     */
    QFrame *plainPage();

    /**
     * Adds a page to the dialog when the class is used in TreeList, 
     * IconList or Tabbed mode. The returned widget must be used as the 
     * toplevel widget for this particular page. 
     * Note: The returned frame widget has no
     * layout manager associated with it. In order to use it you must 
     * create a layout yourself as the example below illustrates:
     * <pre>
     *
     * QFrame *page = addPage( i18n("Layout") );
     * QVBoxLayout *topLayout = new QVBoxLayout( page, 0, 6 );
     * QLabel *label = new QLabel( i18n("Layout type"), page );
     * topLayout->addWidget( label );
     * ..
     * </pre>
     *
     * @param itemName String used in the list or as tab item name.
     * @param header Header text use in the list modes. Ignored in Tabbed 
     *        mode. If empty, the item text is used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The page widget which must be used as the toplevel widget for
     *         the page.
     */
    QFrame  *addPage( const QString &item, 
                      const QString &header=QString::null,
		      const QPixmap &pixmap=QPixmap() );

    /**
     * Adds a page to the dialog when the class is used in TreeList, 
     * IconList or Tabbed mode. The returned widget must be used as the 
     * toplevel widget for this particular page. The widget contains a 
     * QVBoxLayout layout so the widget children are lined up vertically. 
     * You can use it as follows:
     * <pre>
     *
     * QVBox *page = addVBoxPage( i18n("Layout") );
     * QLabel *label = new QLabel( i18n("Layout type"), page );
     * ..
     * </pre>
     *
     * @param itemName String used in the list or as tab item name.
     * @param header Header text use in the list modes. Ignored in Tabbed 
     *        mode. If empty, the item text is used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The page widget which must be used as the toplevel widget for
     *         the page.
     */
    QVBox *addVBoxPage( const QString &itemName, 
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * Adds a page to the dialog when the class is used in TreeList, 
     * IconList or Tabbed mode. The returned widget must be used as the 
     * toplevel widget for this particular page. The widget contains a 
     * QHBoxLayout layout so the widget children are lined up horizontally. 
     * You can use it as follows:
     *
     * @param itemName String used in the list or as tab item name.
     * @param header Header text use in the list modes. Ignored in Tabbed 
     *        mode. If empty, the item text is used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The page widget which must be used as the toplevel widget for
     *         the page.
     */
    QHBox *addHBoxPage( const QString &itemName, 
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

   
    /**
     * Adds a page to the dialog when the class is used in TreeList, 
     * IconList or Tabbed mode. The returned widget must be used as the 
     * toplevel widget for this particular page. The widget contains a 
     * QGridLayout layout so the widget children are  positioned in a grid.
     *
     * @param n Specifies the number of columns if 'dir' is QGrid::Horizontal
     *          or the number of rows if 'dir' is QGrid::Vertical.
     * @param dir Can be QGrid::Horizontal or QGrid::Vertical.
     * @param itemName String used in the list or as tab item name.
     * @param header Header text use in the list modes Ignored in Tabbed 
     *        mode. If empty, the item text is used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The page widget which must be used as the toplevel widget for
     *         the page.
     */
    QGrid *addGridPage( int n, QGrid::Direction dir, 
			const QString &itemName, 
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * Makes a main widget. The function will make a @ref QFrame widget
     * and use @ref setMainWidget to register it. You can NOT use this
     * function more than once, NOT if you have already defined a 
     * main widget with @ref setMainWidget and NOT if you have used the 
     * constructor where you define the face (Plain, Swallow, Tabbed, 
     * TreeList).
     *
     * @return The main widget or 0 if any of the rules described above 
     *         were broken.
     */
    QFrame *makeMainWidget();

    /**
     * Makes a main widget. The function will make a @ref QVBox widget
     * and use @ref setMainWidget to register it. You can NOT use this
     * function more than once, NOT if you have already defined a 
     * main widget with @ref setMainWidget and NOT if you have used the 
     * constructor where you define the face (Plain, Swallow, Tabbed, 
     * TreeList, IconList).
     *
     * @return The main widget or 0 if any of the rules described above 
     *         were broken.
     */
    QVBox *makeVBoxMainWidget();

    /**
     * Makes a main widget. The function will make a @ref QHBox widget
     * and use @ref setMainWidget to register it. You can NOT use this
     * function more than once, NOT if you have already defined a 
     * main widget with @ref setMainWidget and NOT if you have used the 
     * constructor where you define the face (Plain, Swallow, Tabbed, 
     * TreeList, IconList).
     *
     * @return The main widget or 0 if any of the rules described above 
     *         were broken.
     */
    QHBox *makeHBoxMainWidget();

    /**
     * Makes a main widget. The function will make a @ref QGrid widget
     * and use @ref setMainWidget to register it. You can NOT use this
     * function more than once, NOT if you have already defined a 
     * main widget with @ref setMainWidget and NOT if you have used the 
     * constructor where you define the face (Plain, Swallow, Tabbed, 
     * TreeList, IconList).
     *
     * @param n Specifies the number of columns if 'dir' is QGrid::Horizontal
     *          or the number of rows if 'dir' is QGrid::Vertical.
     * @param dir Can be QGrid::Horizontal or QGrid::Vertical.
     *
     * @return The main widget or 0 if any of the rules described above 
     *         were broken.
     */
    QGrid *makeGridMainWidget( int n, QGrid::Direction dir );

    /**
     * Maps some keys to the actions buttons. F1 is mapped to the Help
     * button if present and Escape to the Cancel or Close if present. The
     * button action event is animated.
     */
    virtual void keyPressEvent( QKeyEvent *e );

    /**
     * Emits the @ref hidden signal. You can connect to that signal to
     * detect when a dialog has been closed.
     */
    virtual void hideEvent( QHideEvent * );
    
    /**
     * Detects when a dialog is being closed from the window manager 
     * controls. If the Cancel or Close button is present then the button
     * is activated. Otherwise standard @ref QDialog behavior 
     * will take place.
     */
    virtual void closeEvent( QCloseEvent *e );

  protected slots:
    /**
     * Activated when the Help button has been clicked. If a help
     * text has been defined, the help system will be activated.
     */
    virtual void slotHelp();
  
    /**
     * Activated when the Default button has been clicked.
     */
    virtual void slotDefault();

    /**
     * Activated when the User3 button has been clicked.
     */
    virtual void slotUser3();

    /**
     * Activated when the User2 button has been clicked.
     */
    virtual void slotUser2();

    /**
     * Activated when the User1 button has been clicked.
     */
    virtual void slotUser1();

    /**
     * Activated when the Ok button has been clicked. The 
     * @ref QDialog::accept() is activated.
     */
    virtual void slotOk();

    /**
     * Activated when the Apply button has been clicked.
     */
    virtual void slotApply();

    /**
     * Activated when the Try button has been clicked.
     */
    virtual void slotTry();

    /**
     * Activated when the Yes button has been clicked. The 
     * @ref QDialog::done( Yes ) is activated.
     */
    virtual void slotYes();

    /**
     * Activated when the Yes button has been clicked. The 
     * @ref QDialog::done( No ) is activated.
     */
    virtual void slotNo();

    /**
     * Activated when the Cancel button has been clicked. The 
     * @ref QDialog::reject() is activated in regular mode and
     * @ref QDialog::done( Cancel ) when in message box mode.
     */
    virtual void slotCancel();

    /**
     * Activated when the Close button has been clicked. The 
     * @ref QDialog::reject() is activated.
     */
    virtual void slotClose();

    /**
     * Do not use this slot. Is is kept for compatibility reasons.
     * Activated when the Apply button has been clicked
     */
    virtual void applyPressed();

    /**
     * Updates the margins and spacings. 
     */
    void updateGeometry();

  private:
    /**
     * Prepares the layout that manages the widgets of the dialog
     */
    void setupLayout();

    /**
     * Prepares a relay that is used to send signals between
     * all KDialogBase instances of a program. Should only be used in the 
     * constructor.
     */
    void makeRelay();

    /**
     * Makes the button box and all the buttons in it. This method must 
     * only be ran once from the constructor.
     *
     * @param buttonMask Specifies what buttons will be made.
     * @param defaultButton Specifies what button we be marked as the 
     * default.
     * @param user1 User button1 text.
     * @param user2 User button2 text.
     * @param user2 User button3 text. 
     */
    void makeButtonBox( int mask, ButtonCode defaultButton, 
			const QString &user1 = QString::null,
			const QString &user2 = QString::null,
			const QString &user3 = QString::null );

    /**
     * Sets the action button that is marked as default and has focus.
     *
     * @param p The action button.
     * @param isDefault If true, make the button default 
     * @param isFocus If true, give the button focus.
     */
    void setButtonFocus( QPushButton *p, bool isDefault, bool isFocus );

    /**
     * Prints an error message using qDebug if @ref makeMainWidget , 
     * @ref makeVBoxMainWidget , @ref makeHBoxMainWidget or 
     * @ref makeGridMainWidget failed.
     */
    void printMakeMainWidgetError();

  private slots:
    /**
     * Sets the action button order according to the 'style'.
     *
     * @param style The style index.
     */
    void setButtonStyle( int style );

    
  private:
    QVBoxLayout  *mTopLayout;
    QWidget      *mMainWidget;
    KURLLabel    *mUrlHelp;
    KJanusWidget *mJanus;
    KSeparator   *mActionSep;

    SButton mButton;
    bool mIsActivated;

    QString mHelpPath;
    QString mHelpTopic;
    QString mHelpLinkText;

    static KDialogBaseTile *mTile;
    bool   mShowTile;

    bool mMessageBoxMode;
    ButtonCode mEscapeButton;

    class KDialogBasePrivate;
    KDialogBasePrivate *d;
};






#endif



