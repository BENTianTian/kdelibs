/*
 * (C) Copyright 1999, Daniel M. Duley <mosfet@kde.org>
 */
 

#ifndef __KTHEMESTYLE_H
#define __KTHEMESTYLE_H

#include <kthemebase.h>
#include <qwindowdefs.h>
#include <qobject.h>
#include <qbutton.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qstring.h>


/**
 * This is a class for KDE themed styles. It provides methods for
 * drawing most widgets with user-specified borders, highlights, pixmaps,
 * etc.. It also handles various other settings such as scrollbar types,
 * rounded buttons, and shading types. For a full list of parameters this
 * class handles refer to the KDE theme configuration documentation.
 *
 */
 
class KThemeStyle: public KThemeBase
{
    Q_OBJECT
public:
    /**
     * Constructs a new KThemeStyle object.
     *
     * @param configFile A KConfig file to use as the theme configuration.
     * Defaults to ~/.kderc.
     */
    KThemeStyle(const QString &configFile = QString::null) :
        KThemeBase(configFile) {;}
    ~KThemeStyle(){warning("Kthemestyle destructor called");}
    virtual void polish(QWidget* );
    virtual void unPolish(QWidget* );
    /**
     * By default this just sets the background brushes to the pixmapped
     * background.
     */
    virtual void polish(QApplication *app);
    virtual void unPolish(QApplication*);

    /// @internal
    // to make it possible for derived classes to overload this function
    virtual void polish(QPalette& pal);

    /**
     * This is a convenience method for drawing widgets with
     * borders, highlights, pixmaps, colors, etc...
     * You specify the widget type and it will draw it according to the
     * config file settings. 
     *
     * @param p The QPainter to draw on.
     * @param g The color group to use.
     * @param rounded True if the widget is rounded, false if rectangular.
     * @param type The widget type to paint.
     * @param fill An optional fill brush. Currently ignored (the config file
     * is used instead).
     */
    virtual void drawBaseButton(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool sunken = FALSE,
                                bool rounded = FALSE, WidgetType type=Bevel,
                                const QBrush *fill = 0);
    /**
     * Draws a mask with for widgets that may be rounded. Currently used
     * by pushbuttons and comboboxes.
     *
     * @param p The QPainter to draw on.
     * @param rounded True if the widget is rounded, false if rectangular.
     */
    virtual void drawBaseMask(QPainter *p, int x, int y, int w, int h,
                              bool rounded);
    /**
     * Draws a pushbutton. This calls drawBaseButton with PushButton as the
     * widget type.
     */
    virtual void drawButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken = FALSE,
                            const QBrush *fill = 0);
    /**
     * Draws a bevel button. This calls drawBaseButton with Bevel as the
     * widget type.
     */
    virtual void drawBevelButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken = FALSE,
                                 const QBrush *fill = 0);
    /**
     * Draws a toolbar button.
     */
    virtual void drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken=false,
                                    bool raised = true, bool enabled = true,
                                    bool popup = false, KToolButtonType type = Icon,
                                    const QString &btext=QString::null,
                                    const QPixmap *icon=NULL,
                                    QFont *font=NULL);             
    /**
     * Draws the handle used in menu and toolbars.
     */
    virtual void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool horizontal = false,
                                QBrush *fill = NULL);
    /**
     * Draws a toolbar.
     */
    virtual void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool floating = false);
    /**
     * Returns the space available in a pushbutton, taking configurable
     * borders and highlights into account.
     */
    virtual QRect buttonRect(int x, int y, int w, int h);
    /**
     * Draws an arrow in the style specified by the config file.
     */
    virtual void drawArrow(QPainter *p, Qt::ArrowType type, bool down,
                           int x, int y, int w, int h, const QColorGroup &g,
                           bool enabled=true, const QBrush *fill = 0);
    /**
     * Returns the size of the exclusive indicator pixmap if one is specified
     * in the config file, otherwise it uses the base style's size.
     */
    virtual QSize exclusiveIndicatorSize() const;
    /**
     * Draws an exclusive indicator widget. If a pixmap is specified in the
     * config file that is used, otherwise the base style's widget is drawn.
     */
    virtual void drawExclusiveIndicator(QPainter* p, int x, int y, int w,
                                        int h, const QColorGroup &g, bool on,
                                        bool down = FALSE,
                                        bool enabled = TRUE);
    /**
     * Sets the mask of an exclusive indicator widget. If a pixmap is specified
     * it is masked according to it's transparent pixels, otherwise the
     * base style's mask is used.
     */
    virtual void drawExclusiveIndicatorMask(QPainter *p, int x, int y, int w,
                                            int h, bool on);
    /**
     * Sets the mask of an indicator widget. If a pixmap is specified
     * it is masked according to it's transparent pixels, otherwise the
     * base style's mask is used.
     */
    virtual void drawIndicatorMask(QPainter *p, int x, int y, int w, int h,
                                   int state);
    /**
     * Sets the mask for pushbuttons. 
     */
    virtual void drawButtonMask(QPainter *p, int x, int y, int w, int h);
    /**
     * Sets the mask for combo boxes.
     */
    virtual void drawComboButtonMask(QPainter *p, int x, int y, int w, int h);
    /**
     * Returns the size of the indicator pixmap if one is specified
     * in the config file, otherwise it uses the base style's size.
     */
    virtual QSize indicatorSize() const;
    /**
     * Draws an indicator widget. If a pixmap is specified in the
     * config file that is used, otherwise the base style's widget is drawn.
     */
    virtual void drawIndicator(QPainter* p, int x, int y, int w, int h,
                               const QColorGroup &g, int state,
                               bool down = FALSE, bool enabled = TRUE);
    /**
     * Draws a combobox.
     */
    virtual void drawComboButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken = FALSE,
                                 bool editable = FALSE, bool enabled = TRUE,
                                 const QBrush *fill = 0);
    /**
     * Draws a pushbutton.
     */
    virtual void drawPushButton(QPushButton* btn, QPainter *p);
    /**
     * Returns the amount of button content displacement specified by the
     * config file.
     */
    virtual void getButtonShift(int &x, int &y);
    /**
     * Returns the frame width.
     */
    virtual int defaultFrameWidth() const;
    /**
     * Calculates the metrics of the scrollbar controls according to the
     * layout specified by the config file.
     */
    virtual void scrollBarMetrics(const QScrollBar*, int&, int&, int&, int&);
    /**
     * Draws a themed scrollbar.
     */
    virtual void drawScrollBarControls(QPainter*, const QScrollBar*,
                                       int sliderStart, uint controls,
                                       uint activeControl);
    /**
     * Returns the control that the given point is over according to the
     * layout in the config file.
     */
    virtual ScrollControl scrollBarPointOver(const QScrollBar*,
                                             int sliderStart, const QPoint& );
    /**
     * Returns the configurable default slider length.
     */
    virtual int sliderLength() const;
    /**
     * Draws a slider control.
     */
    virtual void drawSlider(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, Orientation, bool tickAbove,
                            bool tickBelow);
    /**
     * Draws a slider groove.
     */
    void drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup& g, QCOORD c,
                          Orientation );
    /**
     * Draws the mask for a slider (both the control and groove.
     */
    virtual void drawSliderMask(QPainter *p, int x, int y, int w, int h,
                                Orientation, bool tickAbove, bool tickBelow);
//    void drawSliderGrooveMask(QPainter *p,int x, int y, int w, int h,
//                              QCOORD c, Orientation );
    /**
     * Convience method for drawing themed scrollbar grooves. Since the
     * grooves may be a scaled pixmap you cannot just bitblt the pixmap at
     * any offset. This generates a cached pixmap at full size if needed and
     * then copies the requested area.
     *
     * @param p The painter to draw on.
     * @param sb The scrollbar (usually given by drawScrollBarControls).
     * @param horizontal Is the scrollBar horizontal?
     * @param r The rectangle to fill.
     * @param g The color group to use.
     */
    virtual void drawScrollBarGroove(QPainter *p, const QScrollBar *sb,
                                     bool horizontal, QRect r, QColorGroup g);
    /**
     * Draws a shaded rectangle using the given style.
     *
     * @param p The painter to draw on.
     * @param g The color group to use.
     * @param rounded Draws a rounded shape if true. Requires bWidth to be
     * at least 1.
     * @param hWidth The highlight width.
     * @param bWidth The border width.
     * @param style The shading style to use.
     */
    virtual void drawShade(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, bool sunken, bool rounded,
                           int hWidth, int bWidth, ShadeStyle style);
    /**
     * Draws the text for a pushbutton.
     */
    virtual void drawPushButtonLabel(QPushButton *btn, QPainter *p);
    /**
     * Draws a menubar.
     */
    virtual void drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, QBrush *fill=NULL);
    /**
     * Draws a menubar item.
     */
    virtual void drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool active,
                               QMenuItem *item, QBrush *fill=NULL);  
    /**
     * Returns the width of the splitter as specified in the config file.
     */
    virtual int splitterWidth() const;
    /**
     * Draws a splitter widget.
     */
    virtual void drawSplitter(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, Orientation);
    /**
     * Draws a checkmark.
     */
    virtual void drawCheckMark(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool act, bool dis);
    /**
     * Draws a menu item. Note: This method manually handles applying
     * inactive menu backgrounds to the entire widget.
     */
    virtual void drawPopupMenuItem(QPainter *p, bool checkable, int maxpmw,
                                  int tab, QMenuItem *mi, const QPalette &pal,
                                  bool act, bool enabled, int x, int y, int w,
                                   int h);
    int popupMenuItemHeight(bool checkable, QMenuItem *mi,
                            const QFontMetrics &fm);
    /**
     * Draws the focus rectangle.
     */
    void drawFocusRect(QPainter *p, const QRect &r, const QColorGroup &g,
                       const QColor *c=0, bool atBorder=false);
    /**
     * Draws a KProgess Bar.
     */
    virtual void drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QBrush *fill);
    /**
     * Returns the background for KProgress.
     */
    virtual void getKProgressBackground(const QColorGroup &g, QBrush &bg);
    virtual void tabbarMetrics(const QTabBar*, int&, int&, int&);
    virtual void drawTab(QPainter*, const QTabBar*, QTab*, bool selected);    
    virtual void drawTabMask(QPainter*, const QTabBar*, QTab*, bool selected);
    /**
     * Provided for compatibility with with QToolBar for use with the new
     * KDE Parts mechanism. This should not be reimplemented,
     * drawKToolBarButton should be used instead.
     */
    virtual void drawToolButton(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool sunken=false,
								const QBrush *fill=0);
    /**
     * Provided for compatibility with with QToolBar for use with the new
     * KDE Parts mechanism. This should not be reimplemented,
     * drawKToolBar should be used instead.
     */
    virtual void drawOPToolBar(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, QBrush *fill=NULL);
protected:
    QPalette oldPalette, popupPalette, indiPalette, exIndiPalette;
};

#endif
