/**
 * KStyle for KDE4
 * Copyright (C) 2004-2005 Maksim Orlovich <maksim@kde.org>
 *
 * Based in part on the following software:
 *  KStyle for KDE3
 *      Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *      Portions  (C) 1998-2000 TrollTech AS
 *  Keramik for KDE3,
 *      Copyright (C) 2002      Malte Starostik   <malte@kde.org>
 *                (C) 2002-2003 Maksim Orlovich  <maksim@kde.org>
 *      Portions  (C) 2001-2002 Karol Szwed     <gallium@kde.org>
 *                (C) 2001-2002 Fredrik H�glund <fredrik@kde.org>
 *                (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *                (C) 2000 Dirk Mueller         <mueller@kde.org>
 *                (C) 2001 Martijn Klingens    <klingens@kde.org>
 *                (C) 2003 Sandro Giessl      <sandro@giessl.com>
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

#include "kstyle.h"
#include <qalgorithms.h>
#include <qicon.h>
#include <qpainter.h>
#include <qstyleoption.h>
#include <QEvent>
#include <QScrollBar>
//#include <QStyleOptionButton>

#include <stdio.h> //###debug

/**
 TODO: lots of missing widgets, SH_ settings, etc.

 Minor stuff:
    ProgressBar::Precision handling
*/

KStyle::KStyle()
{
    //Set up some default metrics...
    setWidgetLayoutProp(WT_PushButton, PushButton::ContentsMargin, 10);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin,    5);
    setWidgetLayoutProp(WT_PushButton, PushButton::PressedShiftHorizontal, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::PressedShiftVertical,   2);
    setWidgetLayoutProp(WT_PushButton, PushButton::MenuIndicatorSize,      8);
    setWidgetLayoutProp(WT_PushButton, PushButton::TextToIconSpace,        6);
    
    setWidgetLayoutProp(WT_Splitter, Splitter::Size, 6); //As KStyle in KDE3
    
    setWidgetLayoutProp(WT_CheckBox, CheckBox::Size, 16);
    setWidgetLayoutProp(WT_CheckBox, CheckBox::BoxTextSpace, 6);
    
    setWidgetLayoutProp(WT_RadioButton, RadioButton::Size, 16);
    setWidgetLayoutProp(WT_RadioButton, RadioButton::BoxTextSpace, 6);

    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::GrooveMargin,  2);
    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::SideTextSpace, 3); //(Matches QCommonStyle)
    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::MaxBusyIndicatorSize, 10000);
    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::BusyIndicatorSize,    10);
    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::Precision,            1);

    setWidgetLayoutProp(WT_MenuBar, MenuBar::ItemSpacing,   14);
    setWidgetLayoutProp(WT_MenuBar, MenuBar::Margin,        2);
    setWidgetLayoutProp(WT_MenuBar, MenuBar::Margin + Left,  4);
    setWidgetLayoutProp(WT_MenuBar, MenuBar::Margin + Right, 4);

    setWidgetLayoutProp(WT_MenuBarItem, MenuBarItem::Margin, 1);

    setWidgetLayoutProp(WT_Menu, Menu::FrameWidth, 1);
    setWidgetLayoutProp(WT_Menu, Menu::Margin,     3);

    setWidgetLayoutProp(WT_MenuItem, MenuItem::CheckWidth, 12);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::CheckSpace, 3);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::ArrowWidth, 11);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::ArrowSpace, 3);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::Margin,     2);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::SeparatorHeight, 0); //the margins give enough rooms
    setWidgetLayoutProp(WT_MenuItem, MenuItem::MinHeight,  16);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::TextColor, ColorMode(QPalette::Text));
    setWidgetLayoutProp(WT_MenuItem, MenuItem::ActiveTextColor, ColorMode(QPalette::HighlightedText));
    setWidgetLayoutProp(WT_MenuItem, MenuItem::DisabledTextColor,       ColorMode(QPalette::Text));
    setWidgetLayoutProp(WT_MenuItem, MenuItem::ActiveDisabledTextColor, ColorMode(QPalette::Text));

    //KDE default is single top button, double bottom one
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleTopButton, 0);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton, 1);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::SingleButtonHeight, 16);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleButtonHeight, 32);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::BarWidth, 16);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor,
                            ColorMode(ColorMode::BWAutoContrastMode, QPalette::Button));
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor,
                            ColorMode(ColorMode::BWAutoContrastMode, QPalette::ButtonText));

    setWidgetLayoutProp(WT_Tab, Tab::ContentsMargin, 6);

    setWidgetLayoutProp(WT_Tab, Tab::FocusMargin, 3);
}


void KStyle::drawInsideRect(QPainter* p, const QRect& r) const
{
    p->drawRect(r.x(), r.y(), r.width() - 1, r.height() - 1);
}

void KStyle::drawKStylePrimitive(WidgetType widgetType, int primitive, 
                                 const QStyleOption* opt,
                                 QRect r, QPalette pal, State flags,
                                 QPainter* p, 
                                 const QWidget* widget,
                                 KStyle::Option* kOpt) const
{
    if (primitive == Generic::Text)
    {
        KStyle::TextOption* textOpts = extractOption<KStyle::TextOption*>(kOpt);

        //### debug
        //p->setPen(Qt::green);
        //drawInsideRect(p, r);

        QColor col = textOpts->color.color(pal);
        QPen   old = p->pen();
        p->setPen(col);
        drawItemText(p, r, Qt::AlignVCenter | Qt::TextShowMnemonic | textOpts->hAlign, pal, flags & State_Enabled,
                        textOpts->text);
        p->setPen(old);
    }
    else if (primitive == Generic::Bevel)
    {
        //### TODO: use proper cg. Actually, why aren't we passing a CG?
        //p->fillRect(r, Qt::red);
        
        //p->setPen(Qt::white);
        p->fillRect(r, pal.button());
        //p->drawRect(r);
        
    }
    else if (primitive == Generic::Icon)
    {
        KStyle::IconOption* iconOpts = extractOption<KStyle::IconOption*>(kOpt);
        QIcon::Mode mode;

        // Select the correct icon from the iconset
        if (flags & State_Enabled)
            if (iconOpts->active)
                mode = QIcon::Active;
            else
                mode = QIcon::Normal;
        else
            mode = QIcon::Disabled;
        
        QPixmap icon = iconOpts->icon.pixmap(pixelMetric(PM_SmallIconSize), mode);
        p->drawPixmap(centerRect(r, icon.size()), icon);
    }
    else if (primitive == Generic::FocusIndicator)
    {
        QPen pen;
        pen.setWidth(0);
        pen.setStyle(Qt::DotLine);
        p->setPen(pen);
        drawInsideRect(p, r);
    }
    else
    {
        p->setPen(Qt::red);
        drawInsideRect(p, r);
    }
}


void KStyle::setWidgetLayoutProp(WidgetType widget, int metric, int value)
{
    if (metrics.size() <= widget)
        metrics.resize(widget + 1);
        
    QVector<int>& widgetMetrics = metrics[widget];
    if (widgetMetrics.size() <= metric)
        widgetMetrics.resize(metric + 1);
       
    widgetMetrics[metric] = value;
}

int KStyle::widgetLayoutProp(WidgetType widget, int metric) const
{
    if (metrics.size() <= widget)
        return 0;
        
    const QVector<int>& widgetMetrics = metrics[widget];
    if (widgetMetrics.size() <= metric)
        return 0;
        
    return widgetMetrics[metric];
}

QSize KStyle::expandDim(QSize orig, WidgetType widget, int baseMarginMetric) const
{
    int width = orig.width() +  2*widgetLayoutProp(widget, baseMarginMetric + MainMargin) +
                                  widgetLayoutProp(widget, baseMarginMetric + Left) +
                                  widgetLayoutProp(widget, baseMarginMetric + Right);
                                  
    int height = orig.height() + 2*widgetLayoutProp(widget, baseMarginMetric + MainMargin) +
                                   widgetLayoutProp(widget, baseMarginMetric + Left) +
                                   widgetLayoutProp(widget, baseMarginMetric + Right);
                                   
    return QSize(width, height);                                   
}

QRect KStyle::insideMargin(QRect orig, WidgetType widget, int baseMarginMetric) const
{
    int x1 = orig.topLeft().x(); 
    int y1 = orig.topLeft().y();
    int x2 = orig.bottomRight().x();
    int y2 = orig.bottomRight().y();
    
    x1 += widgetLayoutProp(widget, baseMarginMetric + MainMargin);
    x1 += widgetLayoutProp(widget, baseMarginMetric + Left);
    
    y1 += widgetLayoutProp(widget, baseMarginMetric + MainMargin);
    y1 += widgetLayoutProp(widget, baseMarginMetric + Top);
    
    x2 -= widgetLayoutProp(widget, baseMarginMetric + MainMargin);
    x2 -= widgetLayoutProp(widget, baseMarginMetric + Right);
    
    y2 -= widgetLayoutProp(widget, baseMarginMetric + MainMargin);
    y2 -= widgetLayoutProp(widget, baseMarginMetric + Bot);
    
    return QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

QRect KStyle::handleRTL(const QStyleOption* opt, const QRect& subRect) const
{
    return visualRect(opt->direction, opt->rect, subRect);
}

QPoint KStyle::handleRTL(const QStyleOption* opt, const QPoint& pos) const
{
    return visualPos(opt->direction, opt->rect, pos);
}

void KStyle::drawPrimitive(PrimitiveElement elem, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    //Extract the stuff we need out of the option
    State flags = option->state;
    QRect      r     = option->rect;
    QPalette   pal   = option->palette;

    switch (elem)
    {
        case PE_FrameFocusRect:
            drawKStylePrimitive(WT_Generic, Generic::FocusIndicator, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorCheckBox:
            if (flags & State_NoChange)
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckTriState, option, r, pal, flags, painter, widget);
            else if (flags & State_On)
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckOn, option, r, pal, flags, painter, widget);
            else
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckOff, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorRadioButton:
            if (flags & State_On)
                drawKStylePrimitive(WT_RadioButton, RadioButton::RadioOn, option, r, pal, flags, painter, widget);
            else
                drawKStylePrimitive(WT_RadioButton, RadioButton::RadioOff, option, r, pal, flags, painter, widget);
            return;
        case PE_PanelButtonCommand:
        //case PE_PanelButtonBevel: // ### CHECKME
            drawKStylePrimitive(WT_PushButton, Generic::Bevel, option, r, pal, flags, painter, widget);
            return;
        case PE_FrameDefaultButton:
            drawKStylePrimitive(WT_PushButton, PushButton::DefaultButtonBevel, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorArrowUp:
            drawKStylePrimitive(WT_Generic, Generic::ArrowUp, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorArrowDown:
            drawKStylePrimitive(WT_Generic, Generic::ArrowDown, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorArrowLeft:
            drawKStylePrimitive(WT_Generic, Generic::ArrowLeft, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorArrowRight:
            drawKStylePrimitive(WT_Generic, Generic::ArrowRight, option, r, pal, flags, painter, widget);
            return;

        case PE_FrameMenu:
            drawKStylePrimitive(WT_Menu, Menu::Frame, option, r, pal, flags, painter, widget);
            return;
            
    }
    
    QCommonStyle::drawPrimitive(elem, option, painter, widget);
}

void KStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* p, const QWidget* widget) const
{
    //Extract the stuff we need out of the option
    State flags = option->state;
    QRect      r     = option->rect;
    QPalette   pal   = option->palette;

    switch (element)
    {
        case CE_PushButton:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;
        
            //Draw the bevel outside
            drawControl(CE_PushButtonBevel, option, p, widget);
            
            //Now, draw the label...
            QRect labelRect = r;
            
            //Move inside of default indicator margin if need be
            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                labelRect = insideMargin(labelRect, WT_PushButton, PushButton::DefaultIndicatorMargin);
            
            //now get the contents area
            labelRect = insideMargin(labelRect, WT_PushButton, PushButton::ContentsMargin);

            //### do we do anything for RTL here?

            //### ugly                        
            const_cast<QStyleOption*>(option)->rect = labelRect;
            drawControl(CE_PushButtonLabel, option, p, widget);
            
            //Finally, renderer the focus indicator if need be
            if (flags & State_HasFocus)
            {
                QRect focusRect = insideMargin(r, WT_PushButton, PushButton::FocusMargin);
                
                QStyleOptionFocusRect foOpts;
                foOpts.palette         = pal;
                foOpts.rect            = focusRect;
                foOpts.state           = flags;
                
                drawKStylePrimitive(WT_PushButton, Generic::FocusIndicator, &foOpts, focusRect, pal, flags, p, widget);
            }
            
            return;
        }
    
        case CE_PushButtonBevel:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            
            //Check whether we should draw default indicator.
            if (bOpt->features & QStyleOptionButton::DefaultButton)
                drawKStylePrimitive(WT_PushButton, PushButton::DefaultButtonBevel, option, r, pal, flags, p, widget);
            
            QRect bevelRect = r;
            //Exclude the margin if default or auto-default 
            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                bevelRect = insideMargin(r, WT_PushButton, PushButton::DefaultIndicatorMargin);
            
            //Now draw the bevel itself.
            drawKStylePrimitive(WT_PushButton, Generic::Bevel, option, bevelRect, pal, flags, p, widget); 
            return;
        }
        
        case CE_PushButtonLabel:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;
            
            //Extract out coordinates for easier manipulation
            //(OK, OK, for easier stealing of code from Keramik)
            int x, y, w, h;
            r.getRect(&x, &y, &w, &h);
            
            //Are we active? If so, shift contents
            bool active = (flags & State_On) || (flags & State_Sunken);
            if (active)
            {
                x += widgetLayoutProp(WT_PushButton, PushButton::PressedShiftHorizontal);
                y += widgetLayoutProp(WT_PushButton, PushButton::PressedShiftVertical);
            }
            
            //Layout the stuff. Do we need space for indicator?
            //we do this separately, and push it to the end, removing its space from layout.            
            if (bOpt->features & QStyleOptionButton::HasMenu)
            {
                int indicatorWidth = widgetLayoutProp(WT_PushButton, PushButton::MenuIndicatorSize);
                w -= indicatorWidth;
                
                //Draw the arrow...
                drawKStylePrimitive(WT_PushButton, Generic::ArrowDown, option, 
                                    handleRTL(bOpt, QRect(x + w, y, indicatorWidth, h)),
                                    pal, flags, p, widget);
            }
                
            // Draw the icon if there is one
            if (!bOpt->icon.isNull())
            {
                int iconSize = pixelMetric(PM_SmallIconSize);
                IconOption icoOpt;
                icoOpt.icon   = bOpt->icon;
                icoOpt.active = flags & State_HasFocus;

                
                if (!bOpt->text.isEmpty())
                {
                    int margin = widgetLayoutProp(WT_PushButton, PushButton::TextToIconSpace);
                    //Center text + icon w/margin in between..
                    
                    //Calculate length of both.
                    int length = iconSize + margin
                                  + p->fontMetrics().size(Qt::TextShowMnemonic, bOpt->text).width();
                    
                    //Calculate offset.
                    int offset = (w - length)/2;
                    
                    //draw icon
                    QRect rect = QRect(x + offset, y + h/2 - iconSize/2, iconSize, iconSize);
                    drawKStylePrimitive(WT_PushButton, Generic::Icon, option,
                                        handleRTL(bOpt, rect),
                                        pal, flags, p, widget, &icoOpt);
                    
                    //new bounding rect for the text
                    x += offset + iconSize + margin;
                    w =  length - iconSize - margin;
                }
                else
                {
                    //Icon only. Center it. (Thankfully, they killed the icon + pixmap insanity in Qt4. Whee!                  
                    //(no need to do anything for RTL here, it's symmetric)
                    drawKStylePrimitive(WT_PushButton, Generic::Icon, option,
                                        QRect(x, y, w, h),
                                        pal, flags, p, widget, &icoOpt);
                }
            }
            else
            {
                //Center the text
                int textW = p->fontMetrics().size(Qt::TextShowMnemonic, bOpt->text).width();
                x += (w - textW)/2;
                w =  textW;
            }

            TextOption lbOpt(bOpt->text);
            drawKStylePrimitive(WT_PushButton, Generic::Text, option, handleRTL(bOpt, QRect(x, y, w, h)),
                                    pal, flags, p, widget, &lbOpt);
            
            return;
        }
        
        case CE_DockWidgetTitle:
        {
            const QStyleOptionDockWidget* dwOpt = ::qstyleoption_cast<const QStyleOptionDockWidget*>(option);
            if (!dwOpt) return;
            
            QRect textRect = insideMargin(r, WT_DockWidgetTitle, DockWidgetTitle::Margin);
            drawKStylePrimitive(WT_DockWidgetTitle, Generic::Bevel, option, r, pal, flags, p, widget);
            
            TextOption lbOpt(dwOpt->title);
            lbOpt.color = QPalette::HighlightedText;
            drawKStylePrimitive(WT_DockWidgetTitle, Generic::Text, option, textRect, pal, flags, p, widget, &lbOpt);
            return;
        }
        
        case CE_Splitter:
        {
            drawKStylePrimitive(WT_Splitter, Generic::Bevel, option, r, pal, flags, p, widget);
            return;
        }
        
        case CE_CheckBox:
        {
            //### FIXME: Icon labels???
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            //Draw the checkbox
            QRect checkBox = subElementRect(SE_CheckBoxIndicator, option, widget);
            if (flags & State_NoChange)
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckTriState, option, handleRTL(bOpt, checkBox), 
                                    pal, flags, p, widget);
            else if (flags & State_On)
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckOn, option, handleRTL(bOpt, checkBox), 
                                    pal, flags, p, widget);
            else
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckOff, option, handleRTL(bOpt, checkBox), 
                                    pal, flags, p, widget);
            
            //Draw the label, if there is one
            if (!bOpt->text.isEmpty())
            {
                QRect labelBox = subElementRect(SE_CheckBoxContents, option, widget);

                TextOption lbOpt(bOpt->text);
                drawKStylePrimitive(WT_CheckBox, Generic::Text, option, handleRTL(bOpt, labelBox),
                                    pal, flags, p, widget, &lbOpt);
            }
            
            //Draw the focus rect...
            if (flags & State_HasFocus)
            {
                QRect focusRect = subElementRect(SE_CheckBoxFocusRect, option, widget);
                drawKStylePrimitive(WT_CheckBox, Generic::FocusIndicator, option, handleRTL(bOpt, focusRect),
                                    pal, flags, p, widget);
            }
            return;
        }
        
        case CE_CheckBoxLabel:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;
                        
            TextOption lbOpt(bOpt->text);
            drawKStylePrimitive(WT_CheckBox, Generic::Text, option, r,
                                pal, flags, p, widget, &lbOpt);
            return;
        }
        
        case CE_RadioButton:
        {
            //### FIXME: Icon labels???
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            //Draw the indicator
            QRect indicator = subElementRect(SE_RadioButtonIndicator, option, widget);
            if (flags & State_On)
                drawKStylePrimitive(WT_RadioButton, RadioButton::RadioOn, option, handleRTL(bOpt, indicator), 
                                    pal, flags, p, widget);
            else
                drawKStylePrimitive(WT_RadioButton, RadioButton::RadioOff, option, handleRTL(bOpt, indicator), 
                                    pal, flags, p, widget);
            
            //Draw the label, if there is one
            if (!bOpt->text.isEmpty())
            {
                QRect labelBox = subElementRect(SE_RadioButtonContents, option, widget);

                TextOption lbOpt(bOpt->text);
                drawKStylePrimitive(WT_RadioButton, Generic::Text, option, handleRTL(bOpt, labelBox),
                                    pal, flags, p, widget, &lbOpt);
            }
            
            //Draw the focus rect...
            if (flags & State_HasFocus)
            {
                QRect focusRect = subElementRect(SE_RadioButtonFocusRect, option, widget);
                drawKStylePrimitive(WT_RadioButton, Generic::FocusIndicator, option, handleRTL(bOpt, focusRect),
                                    pal, flags, p, widget);
            }
            return;
        }
        
        case CE_RadioButtonLabel:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            TextOption lbOpt(bOpt->text);
            drawKStylePrimitive(WT_RadioButton, Generic::Text, option, r,
                                pal, flags, p, widget, &lbOpt);
            return;
        }
        
        //The CE_ProgressBar implementation inside QCommonStyle is acceptible.
        //We just implement the subElementRect's it uses
        
        case CE_ProgressBarGroove:
        {
            drawKStylePrimitive(WT_ProgressBar, Generic::Bevel, option, r,
                                pal, flags, p, widget);
            return;
        }
        
        case CE_ProgressBarContents:
        {
            const QStyleOptionProgressBar* pbOpt = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            if  (!pbOpt) return;

            //We layout as if LTR, relying on visualRect to fix it up
            double progress    = pbOpt->progress - pbOpt->minimum;
            int steps          = pbOpt->maximum  - pbOpt->minimum + 1;
            bool busyIndicator = (steps <= 1); //### not only min = 0 / max = 0

            //Do we have to draw anything?
            if (!progress && ! busyIndicator)
                return;
                
            //Calculate width fraction
            double widthFrac;
            if (busyIndicator)            
                widthFrac = widgetLayoutProp(WT_ProgressBar, ProgressBar::BusyIndicatorSize) / 100.0;
            else
                widthFrac = progress / steps;

            //And now the pixel width
            int width = qMin(r.width(), (int)(widthFrac * r.width()));

            if (busyIndicator)
            {
                //Clamp to upper width limit
                if (width > widgetLayoutProp(WT_ProgressBar, ProgressBar::MaxBusyIndicatorSize))
                    width = widgetLayoutProp(WT_ProgressBar, ProgressBar::MaxBusyIndicatorSize);

                if (width < 1) width = 1; //A busy indicator with width 0 is kind of useless


                int remWidth = r.width() - width; //The space around which we move around...
                if (remWidth <= 0) remWidth = 1;  //Do something non-crashy when too small...

                int pstep =  int(progress)%(2*remWidth);

                if (pstep > remWidth)
                {
                    //Bounce about.. We're remWidth + some delta, we want to be remWidth - delta...
                    // - ( (remWidth + some delta) - 2* remWidth )  = - (some deleta - remWidth) = remWidth - some delta..
                    pstep = -(pstep - 2*remWidth);
                }

                QRect indicatorRect = QRect(r.x() + pstep, r.y(), width, r.height());
                drawKStylePrimitive(WT_ProgressBar, ProgressBar::BusyIndicator, option, handleRTL(option, indicatorRect),
                                    pal, flags, p, widget);
            }
            else
            {
                QRect indicatorRect = QRect(r.x(), r.y(), width, r.height());
                drawKStylePrimitive(WT_ProgressBar, ProgressBar::Indicator, option, handleRTL(option, indicatorRect),
                                    pal, flags, p, widget);
            }
            return;
        }
        
        case CE_ProgressBarLabel:
        {
            const QStyleOptionProgressBar* pbOpt = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            if (pbOpt)
            {
                TextOption lbOpt(pbOpt->text);
                
                if (useSideText(pbOpt))
                {
                    lbOpt.color = QPalette::ButtonText;
                    
                    //### or other way around?
                    if (option->direction == Qt::LeftToRight)
                        lbOpt.hAlign = Qt::AlignRight;
                    else
                        lbOpt.hAlign = Qt::AlignLeft;

                    //Handle side margin.
                    int marWidth = widgetLayoutProp(WT_ProgressBar, ProgressBar::SideTextSpace);

                    drawKStylePrimitive(WT_ProgressBar, Generic::Text, option,
                            QRect(r.x() + marWidth, r.y(), r.width() - 2*marWidth, r.height()),
                            pal, flags, p, widget, &lbOpt);
                }
                else
                {
                    if (pbOpt->textAlignment == Qt::AlignLeft) //TODO: Check BIDI?
                        lbOpt.hAlign = Qt::AlignHCenter;
                    else
                        lbOpt.hAlign = pbOpt->textAlignment;

                    //Now, we need to figure out the geometry of the indicator.
                    QRect progressRect;
                    double progress    = pbOpt->progress - pbOpt->minimum;
                    int steps          = pbOpt->maximum  - pbOpt->minimum + 1;
                    bool busyIndicator = (steps <= 1);

                    int width;
                    if (busyIndicator)
                    {
                        //how did this happen? handle as 0%
                        width = 0;
                    }
                    else
                    {
                        double widthFrac = progress / steps;;
                        width = qMin(r.width(), (int)(widthFrac * r.width()));
                    }

                    //If there is any indicator, we do two paths, with different
                    //clipping rects, for the two colors.                    
                    if (width)
                    {
                        p->setClipRect(handleRTL(option, QRect(r.x(), r.y(), width, r.height())));
                        lbOpt.color = QPalette::HighlightedText;
                        drawKStylePrimitive(WT_ProgressBar, Generic::Text, option, r,
                                            pal, flags, p, widget, &lbOpt);
                                            
                        p->setClipRect(handleRTL(option, QRect(r.x() + width, r.y(), r.width() - width, r.height())));
                        lbOpt.color = QPalette::ButtonText;
                        drawKStylePrimitive(WT_ProgressBar, Generic::Text, option, r,
                                            pal, flags, p, widget, &lbOpt);
                        p->setClipping(false);
                    }
                    else
                    {
                        lbOpt.color = QPalette::ButtonText;
                        drawKStylePrimitive(WT_ProgressBar, Generic::Text, option, r,
                                            pal, flags, p, widget, &lbOpt);
                    }
                }
            }     
            return;
        }

        case CE_MenuBarEmptyArea:
        {
            drawKStylePrimitive(WT_MenuBar, Generic::Bevel, option, r,
                                pal, flags, p, widget);
            return;
        }

        case CE_MenuBarItem:
        {
            const QStyleOptionMenuItem* mOpt = ::qstyleoption_cast<const QStyleOptionMenuItem*>(option);
            if (!mOpt) return;
        
            //Bevel...
            drawKStylePrimitive(WT_MenuBarItem, Generic::Bevel, option, r,
                                pal, flags, p, widget);

            //Text...
            QRect textRect = insideMargin(r, WT_MenuBarItem, MenuBarItem::Margin);


            TextOption lbOpt(mOpt->text);
            drawKStylePrimitive(WT_MenuBarItem, Generic::Text, option, textRect,
                                pal, flags, p, widget, &lbOpt);

            return;
        }

        case CE_MenuScroller:
        {
            drawKStylePrimitive(WT_Menu, Menu::Scroller, option, r,
                                pal, flags, p, widget);
            return;
        }

        case CE_MenuTearoff:
        {
            drawKStylePrimitive(WT_Menu, Menu::TearOff, option, r,
                                pal, flags, p, widget);
            return;
        }

        case CE_MenuEmptyArea:
        case CE_MenuVMargin:
        case CE_MenuHMargin:
        {
            drawKStylePrimitive(WT_Menu, Menu::Background,  option, r,
                                pal, flags, p, widget);
            return;
        }

        case CE_MenuItem:
        {
                   
            //First of all,render the background.
            drawKStylePrimitive(WT_Menu, Menu::Background, option, r,
                                pal, flags, p, widget);

            const QStyleOptionMenuItem* miOpt = ::qstyleoption_cast<const QStyleOptionMenuItem*>(option);
            if (!miOpt || miOpt->menuItemType == QStyleOptionMenuItem::EmptyArea) return;

            //Remove the margin (for everything but the column background)
            QRect ir = insideMargin(r, WT_MenuItem, MenuItem::Margin);


            //First, figure out the left column width.
            int leftColW = miOpt->maxIconWidth;
            leftColW     = qMax(leftColW, widgetLayoutProp(WT_MenuItem, MenuItem::CheckWidth));
                         
            //And the right one...
            int rightColW = widgetLayoutProp(WT_MenuItem, MenuItem::ArrowSpace) +
                            widgetLayoutProp(WT_MenuItem, MenuItem::ArrowWidth);

            //Render left column background. This is a bit tricky, since we don't use the V margin.
            QRect leftColRect(ir.x(), r.y(), leftColW, r.height());
            drawKStylePrimitive(WT_MenuItem, MenuItem::CheckColumn, option, handleRTL(option, leftColRect),
                                pal, flags, p, widget);

            //Separators: done with the bg, can paint them and bail them out.
            if (miOpt->menuItemType == QStyleOptionMenuItem::Separator)
            {
                drawKStylePrimitive(WT_MenuItem, MenuItem::Separator, option, ir, pal, flags, p, widget);
                return;
            }

            //Now paint the active indicator --- other stuff goes on top of it
            bool active = (flags & State_Selected);

            //Active indicator...
            if (active)
                drawKStylePrimitive(WT_MenuItem, MenuItem::ItemIndicator, option, handleRTL(option, r), pal, flags, p, widget);


            ColorMode textColor = (flags & State_Enabled) ? (widgetLayoutProp(WT_MenuItem, active ?
                                                                  MenuItem::ActiveTextColor :
                                                                  MenuItem::TextColor))
                                                          : (widgetLayoutProp(WT_MenuItem, active ?
                                                                  MenuItem::ActiveDisabledTextColor:
                                                                  MenuItem::DisabledTextColor));

            //Readjust the column rectangle back to proper height
            leftColRect = QRect(ir.x(), ir.y(), leftColW, ir.height());
            //### render checkbox, etc. properly
            if (!miOpt->icon.isNull())
            {
                int iconSize = pixelMetric(PM_SmallIconSize);
                IconOption icoOpt;
                icoOpt.icon   = miOpt->icon;
                icoOpt.active = flags & State_Selected;
                drawKStylePrimitive(WT_MenuItem, Generic::Icon, option,
                                    handleRTL(option, centerRect(leftColRect, iconSize, iconSize)),
                                    pal, flags, p, widget, &icoOpt);
            }

            //Now include the spacing when calculating the next columns
            leftColW += widgetLayoutProp(WT_MenuItem, MenuItem::CheckSpace);

            //Render the text, including any accel.
            QString text = miOpt->text;
            QRect   textRect = QRect(ir.x() + leftColW, ir.y(), ir.width() - leftColW - rightColW, ir.height());
            
           
            int tabPos = miOpt->text.indexOf('\t');
            if (tabPos != -1)
            {
                text = miOpt->text.left(tabPos);
                QString accl = miOpt->text.mid (tabPos + 1);

                //Draw the accel.
                TextOption lbOpt(accl);
                lbOpt.color  = textColor;
                lbOpt.hAlign = Qt::AlignRight;
                drawKStylePrimitive(WT_MenuItem, Generic::Text, option, handleRTL(option, textRect),
                                pal, flags, p, widget, &lbOpt);                
            }

            //Draw the text.
            TextOption lbOpt(text);
            lbOpt.color = textColor;
            drawKStylePrimitive(WT_MenuItem, Generic::Text, option, handleRTL(option, textRect),
                                pal, flags, p, widget, &lbOpt);

            //Render arrow, if need be.
            if (miOpt->menuItemType == QStyleOptionMenuItem::SubMenu)
            {
                ColorOption arrowColor;
                arrowColor.color = textColor;
            
                int aw = widgetLayoutProp(WT_MenuItem, MenuItem::ArrowWidth);

                QRect arrowRect(ir.x() + ir.width() - aw, ir.y(), aw, ir.height());
                drawKStylePrimitive(WT_MenuItem, option->direction == Qt::LeftToRight ?
                                                       Generic::ArrowRight : Generic::ArrowLeft,
                                    option, handleRTL(option, arrowRect), pal, flags, p, widget, &arrowColor);
            }

            return;
        }

        case CE_ScrollBarAddLine:
        case CE_ScrollBarSubLine:
        {
            const QStyleOptionSlider* slOpt = ::qstyleoption_cast<const QStyleOptionSlider*>(option);
            if (!slOpt) return;

            //Fix up the rectangle to be what we want
            r = internalSubControlRect(CC_ScrollBar, slOpt,
                element == CE_ScrollBarAddLine ? SC_ScrollBarAddLine : SC_ScrollBarSubLine, widget);
            const_cast<QStyleOption*>(option)->rect = r;


            bool doubleButton = false;
            
            //See whether we're a double-button...
            if (element == CE_ScrollBarAddLine && widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton))
                doubleButton = true;
            if (element == CE_ScrollBarSubLine && widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleTopButton))
                doubleButton = true;

            if (doubleButton)
            {
                if (flags & State_Horizontal)
                {
                    DoubleButtonOption::ActiveButton ab = DoubleButtonOption::None;

                    //Depending on RTL direction, the one on the left is either up or down.
                    bool leftAdds, rightAdds;
                    if (slOpt->direction == Qt::LeftToRight)
                    {
                        leftAdds  = false;
                        rightAdds = true;
                    }
                    else
                    {
                        leftAdds  = true;
                        rightAdds = false;
                    }

                    //Determine whether any of the buttons is active
                    if (((slOpt->activeSubControls & SC_ScrollBarAddLine) && leftAdds) ||
                        ((slOpt->activeSubControls & SC_ScrollBarSubLine) && !leftAdds))
                        ab = DoubleButtonOption::Left;

                    if (((slOpt->activeSubControls & SC_ScrollBarAddLine) && rightAdds) ||
                        ((slOpt->activeSubControls & SC_ScrollBarSubLine) && !rightAdds))
                        ab = DoubleButtonOption::Right;

                    DoubleButtonOption bOpt(ab);
                    drawKStylePrimitive(WT_ScrollBar, ScrollBar::DoubleButtonHor,
                                        option, r, pal, flags, p, widget, &bOpt);

                    //Draw the left arrow..
                    QRect leftSubButton = QRect(r.x(), r.y(), r.width()/2, r.height());

                    ColorOption colOpt;
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor);
                    if (ab == DoubleButtonOption::Left)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor);

                    drawKStylePrimitive(WT_ScrollBar, Generic::ArrowLeft, option, leftSubButton, pal,
                                        flags, p, widget, &colOpt);

                    //Right half..
                    QRect rightSubButton;
                    rightSubButton.setBottomRight(r.bottomRight());
                    rightSubButton.setLeft       (leftSubButton.right() + 1);
                    rightSubButton.setTop        (r.top());

                    //Chose proper color
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor);
                    if (ab == DoubleButtonOption::Right)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor);

                    drawKStylePrimitive(WT_ScrollBar, Generic::ArrowRight, option, rightSubButton, pal,
                                        flags, p, widget, &colOpt);
                }
                else
                {
                    DoubleButtonOption::ActiveButton ab = DoubleButtonOption::None;

                    //Determine whether any of the buttons is active
                    if (slOpt->activeSubControls & SC_ScrollBarSubLine)
                        ab = DoubleButtonOption::Top;

                    if (slOpt->activeSubControls & SC_ScrollBarAddLine)
                        ab = DoubleButtonOption::Bottom;

                    //Paint the bevel
                    DoubleButtonOption bOpt(ab);
                    drawKStylePrimitive(WT_ScrollBar, ScrollBar::DoubleButtonVert,
                                        option, r, pal, flags, p, widget, &bOpt);

                    //Paint top button.
                    ColorOption colOpt;
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor);

                    if (ab == DoubleButtonOption::Top)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor);


                    QRect topSubButton = QRect(r.x(), r.y(), r.width(), r.height()/2);
                    drawKStylePrimitive(WT_ScrollBar, Generic::ArrowUp, option, topSubButton, pal,
                                        flags, p, widget, &colOpt);

                    //Paint bot button
                    QRect botSubButton;
                    botSubButton.setBottomRight(r.bottomRight());
                    botSubButton.setLeft       (r.left());
                    botSubButton.setTop        (topSubButton.bottom() + 1);

                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor);

                    if (ab == DoubleButtonOption::Bottom)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor);

                    drawKStylePrimitive(WT_ScrollBar, Generic::ArrowDown, option, botSubButton, pal,
                                        flags, p, widget, &colOpt);
                }
            }
            else 
            {
                if (flags & State_Horizontal)
                {
                    drawKStylePrimitive(WT_ScrollBar, ScrollBar::SingleButtonHor,
                                        option, r, pal, flags, p, widget);

                    int  primitive;
                    bool active   = false;

                    if (element == CE_ScrollBarAddLine)
                    {
                        if (slOpt->direction == Qt::LeftToRight)
                            primitive = Generic::ArrowRight;
                        else
                            primitive = Generic::ArrowLeft;
                            
                        if (slOpt->activeSubControls & SC_ScrollBarAddLine)
                            active = true;
                    }
                    else
                    {
                        if (slOpt->direction == Qt::LeftToRight)
                            primitive = Generic::ArrowLeft;
                        else
                            primitive = Generic::ArrowRight;

                        if (slOpt->activeSubControls & SC_ScrollBarSubLine)
                            active = true;
                    }

                    ColorOption colOpt;
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor);
                    if (active)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor);

                    drawKStylePrimitive(WT_ScrollBar, primitive, option, r, pal,
                                        flags, p, widget, &colOpt);
                }
                else
                {
                    drawKStylePrimitive(WT_ScrollBar, ScrollBar::SingleButtonVert,
                                        option, r, pal, flags, p, widget);

                    int  primitive;
                    bool active   = false;

                    if (element == CE_ScrollBarAddLine)
                    {
                        primitive = Generic::ArrowDown;
                        if (slOpt->activeSubControls & SC_ScrollBarAddLine)
                            active = true;
                    }
                    else
                    {
                        primitive = Generic::ArrowUp;
                        if (slOpt->activeSubControls & SC_ScrollBarSubLine)
                            active = true;
                    }

                    ColorOption colOpt;
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor);
                    if (active)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor);

                    drawKStylePrimitive(WT_ScrollBar, primitive, option, r, pal,
                                        flags, p, widget, &colOpt);
                }
            }
            return;
        }

        case CE_ScrollBarAddPage:
        case CE_ScrollBarSubPage:
        case CE_ScrollBarFirst:
        case CE_ScrollBarLast:
            drawKStylePrimitive(WT_ScrollBar, (flags & State_Horizontal) ?
                                                    ScrollBar::GrooveAreaHor  :
                                                    ScrollBar::GrooveAreaVert,
                                option, r, pal, flags, p, widget);
            return;

        case CE_ScrollBarSlider:
            drawKStylePrimitive(WT_ScrollBar, (flags & State_Horizontal) ?
                                                    ScrollBar::SliderHor  :
                                                    ScrollBar::SliderVert,
                                option, r, pal, flags, p, widget);
            return;

        //QCS's CE_TabBarTab is perfectly fine, so we just handle the subbits
        
        case CE_TabBarTabShape:
        {
            const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option);
            if (!tabOpt) return;

            int prim;
            switch (tabSide(tabOpt))
            {
            case North:
                prim = Tab::NorthTab; break;
            case South:
                prim = Tab::SouthTab; break;
            case East:
                prim = Tab::EastTab; break;
            default:
                prim = Tab::WestTab; break;
            }

            drawKStylePrimitive(WT_Tab, prim, option, r, pal, flags, p, widget);

            break;
        }
            
        case CE_TabBarTabLabel:
        {
            const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option);
            if (!tabOpt) return;

            //First, we get our content region.
            QRect labelRect = marginAdjustedTab(tabOpt, Tab::ContentsMargin);

            Side tabSd = tabSide(tabOpt);
            
            //Now, what we do, depends on rotation, LTR vs. RTL, and text/icon combinations.
            //First, figure out if we have to deal with icons, and place them if need be.
            if (!tabOpt->icon.isNull())
            {
                int iconSize = pixelMetric(PM_SmallIconSize);
                IconOption icoOpt;
                icoOpt.icon   = tabOpt->icon;
                icoOpt.active = flags & State_Selected;

                if (tabOpt->text.isNull())
                {
                    //Icon only. Easy.
                    drawKStylePrimitive(WT_Tab, Generic::Icon, option, labelRect,
                                        pal, flags, p, widget, &icoOpt);
                    return;
                }

                //OK, we have to stuff both icon and text. So we figure out where to stick the icon.
                QRect iconRect;
                
                if (tabSd == North || tabSd == South)
                {
                    //OK, this is simple affair, we just pick a side for the icon
                    //based on layout direction. (Actually, I guess text
                    //would be more accurate, but I am -so- not doing BIDI here)
                    if (tabOpt->direction == Qt::LeftToRight)
                    {
                        //We place icon on the left.
                        iconRect = QRect(labelRect.x(), labelRect.y(), iconSize, labelRect.height());

                        //Adjust the text rect.
                        labelRect.setLeft(labelRect.x() + iconSize +
                            widgetLayoutProp(WT_Tab, Tab::TextToIconSpace));
                    }
                    else
                    {
                        //We place icon on the right
                        iconRect = QRect(labelRect.width() - iconSize, labelRect.y(),
                                         iconSize, labelRect.height());

                        //Adjust the text rect
                        labelRect.setWidth(labelRect.width() - iconSize -
                            widgetLayoutProp(WT_Tab, Tab::TextToIconSpace));
                    }
                }
                else
                {
                    bool aboveIcon = false;
                    if (tabSd == West && tabOpt->direction == Qt::RightToLeft)
                        aboveIcon = true;
                    if (tabSd == East && tabOpt->direction == Qt::LeftToRight)
                        aboveIcon = true;

                    if (aboveIcon)
                    {
                        iconRect = QRect(labelRect.x(), labelRect.y(),
                                         labelRect.width(), iconSize);
                        labelRect.setTop(labelRect.x() + iconSize +
                            widgetLayoutProp(WT_Tab, Tab::TextToIconSpace));
                    }
                    else
                    {
                        iconRect = QRect(labelRect.x(), labelRect.height() - iconSize,
                                         labelRect.width(), iconSize);
                        labelRect.setHeight(labelRect.height() - iconSize -
                            widgetLayoutProp(WT_Tab, Tab::TextToIconSpace));
                    }
                }

                //Draw the thing
                drawKStylePrimitive(WT_Tab, Generic::Icon, option, iconRect,
                                    pal, flags, p, widget, &icoOpt);
            } //if have icon.

            //Draw text
            if (!tabOpt->text.isNull())
            {
                switch (tabSd)
                {
                    case North:
                    case South:
                    {
                        TextOption lbOpt(tabOpt->text);
                        drawKStylePrimitive(WT_Tab, Generic::Text, option, labelRect,
                                            pal, flags, p, widget, &lbOpt);
                        break;
                    }
                    default:
                        //### TODO
                        p->setPen(Qt::yellow);
                        drawInsideRect(p, labelRect);
                };
            }

            //If need be, draw focus rect
            if (tabOpt->state & State_HasFocus)
            {
                QRect focusRect = marginAdjustedTab(tabOpt, Tab::FocusMargin);
                drawKStylePrimitive(WT_Tab, Generic::FocusIndicator, option, focusRect,
                                    pal, flags, p, widget);
            }
            return;
        }
    }
    
    QCommonStyle::drawControl(element, option, p, widget);
}


int KStyle::styleHint (StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint)
    {
        case SH_MenuBar_MouseTracking:
        case SH_Menu_MouseTracking:
            return true;
    };

    return QCommonStyle::styleHint(hint, option, widget, returnData);
}

int KStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric)
    {
        case PM_ButtonMargin:
            return widgetLayoutProp(WT_PushButton, PushButton::ContentsMargin);
        case PM_ButtonShiftHorizontal:
            return widgetLayoutProp(WT_PushButton, PushButton::PressedShiftHorizontal);
        case PM_ButtonShiftVertical:
            return widgetLayoutProp(WT_PushButton, PushButton::PressedShiftVertical);
        case PM_MenuButtonIndicator:
            return widgetLayoutProp(WT_PushButton, PushButton::MenuIndicatorSize);
            
        case PM_SplitterWidth:
            return widgetLayoutProp(WT_Splitter, Splitter::Size);
            
        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
            return widgetLayoutProp(WT_CheckBox, CheckBox::Size);
            
        case PM_ExclusiveIndicatorWidth:
        case PM_ExclusiveIndicatorHeight:
            return widgetLayoutProp(WT_RadioButton, RadioButton::Size);
            
        case PM_DockWidgetFrameWidth:
            return widgetLayoutProp(WT_DockWidgetTitle, DockWidgetTitle::Margin);

        case PM_ProgressBarChunkWidth:
            return widgetLayoutProp(WT_ProgressBar, ProgressBar::Precision);

        case PM_MenuBarPanelWidth:
            return 0; //Simplification: just one primitive is used and it includes the border

        case PM_MenuBarHMargin:
        {
            //Calculate how much extra space we need besides the frame size. We use the left margin
            //here, and adjust the total rect by the difference between it and the right margin
            int spaceL = widgetLayoutProp(WT_MenuBar, MenuBar::Margin) + widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Left);

            return spaceL;
        }

        case PM_MenuBarVMargin:
        {
            //As above, we return the top one, and fudge the total size for the bottom.
            int spaceT = widgetLayoutProp(WT_MenuBar, MenuBar::Margin) + widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Top);
            return spaceT;
        }

        case PM_MenuBarItemSpacing:
            return widgetLayoutProp(WT_MenuBar, MenuBar::ItemSpacing);

        case PM_MenuDesktopFrameWidth:
            return 0; //### CHECKME

        case PM_MenuPanelWidth:
            return widgetLayoutProp(WT_Menu, Menu::FrameWidth);

            /* ### seems to trigger Qt bug. So we loose the margins for now
        case PM_MenuHMargin:
        {
            //Calculate how much extra space we need besides the frame size. We use the left margin
            //here, and adjust the total rect by the difference between it and the right margin
            int spaceL = widgetLayoutProp(WT_Menu, Menu::Margin) + widgetLayoutProp(WT_Menu, Menu::Margin + Left) -
                    widgetLayoutProp(WT_Menu, Menu::FrameWidth);

            return spaceL;
        }

        case PM_MenuVMargin:
        {
            //As above, we return the top one, and fudge the total size for the bottom.
            int spaceT = widgetLayoutProp(WT_Menu, Menu::Margin) + widgetLayoutProp(WT_Menu, Menu::Margin + Top) -
                widgetLayoutProp(WT_Menu, Menu::FrameWidth);
            return spaceT;
        }     */       

        case PM_MenuScrollerHeight:
            return widgetLayoutProp(WT_Menu, Menu::ScrollerHeight);

        case PM_MenuTearoffHeight:
            return widgetLayoutProp(WT_Menu, Menu::TearOffHeight);

        case PM_TabBarTabHSpace:
            const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option);
            if (tabOpt)
            {
                //Perhaps we can avoid the extra margin...
                if (tabOpt->text.isNull() && !tabOpt->icon.isNull())
                    return 0;
                if (tabOpt->icon.isNull() && !tabOpt->text.isNull());
                    return 0;
            }
            
            return widgetLayoutProp(WT_Tab, Tab::TextToIconSpace);

        case PM_TabBarTabVSpace:
            return 0;
    }

    return QCommonStyle::pixelMetric(metric, option, widget);
}

bool KStyle::isVerticalTab(const QStyleOptionTab* tbOpt) const
{
    switch (tbOpt->shape)
    {
    case QTabBar::RoundedWest:
    case QTabBar::RoundedEast:
    case QTabBar::TriangularWest:
    case QTabBar::TriangularEast:
        return true;
    default:
        return false;
    }
}

bool KStyle::isReflectedTab(const QStyleOptionTab* tbOpt) const
{
    switch (tbOpt->shape)
    {
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        return true;
    default:
        return false;
    }
}

KStyle::Side KStyle::tabSide(const QStyleOptionTab* tbOpt) const
{
    switch (tbOpt->shape)
    {
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        return East;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        return West;
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        return North;
    default:
        return South;
    }
}

QRect KStyle::marginAdjustedTab(const QStyleOptionTab* tabOpt, int property) const
{
    QRect r = tabOpt->rect;
    
    //For region, we first figure out the geometry if it was normal, and adjust.
    //this takes some rotating
    bool vertical = isVerticalTab (tabOpt);
    bool flip     = isReflectedTab(tabOpt);

    QRect idializedGeometry = vertical ? QRect(0, 0, r.height(), r.width())
                                        : QRect(0, 0, r.width(),  r.height());

    QRect contentArea = insideMargin(idializedGeometry, WT_Tab, property);

    int leftMargin  = contentArea.x();
    int rightMargin = idializedGeometry.width() - 1 - contentArea.right();
    int topMargin   = contentArea.y();
    int botMargin   = idializedGeometry.height() - 1 - contentArea.bottom();

    if (vertical)
    {
        int t       = rightMargin;
        rightMargin = topMargin;
        topMargin   = leftMargin;
        leftMargin  = botMargin;
        botMargin   = t;

        if (flip)
            qSwap(leftMargin, rightMargin);
    }
    else if (flip)
    {
        qSwap(topMargin, botMargin);
        //For horizontal tabs, we also want to reverse stuff for RTL!
        if (tabOpt->direction == Qt::RightToLeft)
            qSwap(leftMargin, rightMargin);
    }

    QRect geom =
        QRect(QPoint(leftMargin, topMargin),
                QPoint(r.width()  - 1 - rightMargin,
                        r.height() - 1 - botMargin));
    geom.translate(r.topLeft());
    return geom;
}

bool KStyle::useSideText(const QStyleOptionProgressBar* pbOpt) const
{
    if (widgetLayoutProp(WT_ProgressBar, ProgressBar::SideText) == 0)
        return false;

    if (!pbOpt) return false; //Paranoia

    if (!pbOpt->textVisible) return false; //Don't allocate side margin if text display is off...

    if (pbOpt->textAlignment & Qt::AlignHCenter) return false; //### do we want this? we don't
                                                              //force indicator to the side outside
                                                              //the main otherwise.

    if (pbOpt->minimum == pbOpt->maximum) return false;
    
    int widthAlloc = pbOpt->fontMetrics.width("100%");
     
    if (pbOpt->fontMetrics.width(pbOpt->text) > widthAlloc)
        return false; //Doesn't fit!
    
    return true;
}

int KStyle::sideTextWidth(const QStyleOptionProgressBar* pbOpt) const
{
    return pbOpt->fontMetrics.width("100%") +
                                    2*widgetLayoutProp(WT_ProgressBar, ProgressBar::SideTextSpace);
}

QRect KStyle::subElementRect(SubElement sr, const QStyleOption* option, const QWidget* widget) const
{
    QRect r = option->rect;
    
    switch (sr)
    {
        case SE_PushButtonContents:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return r;
        
            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                r = insideMargin(r, WT_PushButton, PushButton::DefaultIndicatorMargin);

            return insideMargin(r, WT_PushButton, PushButton::ContentsMargin);
        }
            
        case SE_PushButtonFocusRect:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return r;
        
            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                r = insideMargin(r, WT_PushButton, PushButton::DefaultIndicatorMargin);
            
            return insideMargin(r, WT_PushButton, PushButton::FocusMargin);
        }
        
        case SE_CheckBoxIndicator:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return r;

            int size = widgetLayoutProp(WT_CheckBox, CheckBox::Size);
            
            if (bOpt->text.isEmpty())
                return centerRect(r, size, size);
            else
                return QRect(r.x(), r.y(), size, r.height());  
        }
        
        case SE_RadioButtonIndicator:
        {
            int size = widgetLayoutProp(WT_RadioButton, RadioButton::Size);
            
            return QRect(r.x(), r.y(), size, r.height());  
        }
        
        case SE_CheckBoxContents:
        {
            r.setX(r.x() + widgetLayoutProp(WT_CheckBox, CheckBox::Size) +
                           widgetLayoutProp(WT_CheckBox, CheckBox::BoxTextSpace));
            return r;
        }
        
        case SE_RadioButtonContents:
        {
            r.setX(r.x() + widgetLayoutProp(WT_RadioButton, RadioButton::Size) +
                    widgetLayoutProp(WT_RadioButton, RadioButton::BoxTextSpace));
            return r;            
        }
        
        case SE_CheckBoxFocusRect:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return r;

            if (bOpt->text.isEmpty())
            {
                QRect checkRect = subElementRect(SE_CheckBoxIndicator, option, widget);
                return insideMargin(checkRect, WT_CheckBox, CheckBox::NoLabelFocusMargin);
            }
            else
            {            
                QRect contentsRect = subElementRect(SE_CheckBoxContents, option, widget);
                return insideMargin(contentsRect, WT_CheckBox, CheckBox::FocusMargin);
            }
        }
        
        case SE_RadioButtonFocusRect:
        {
            QRect contentsRect = subElementRect(SE_RadioButtonContents, option, widget);
            return insideMargin(contentsRect, WT_RadioButton, RadioButton::FocusMargin);            
        }

        case SE_ProgressBarGroove:
        {
            const QStyleOptionProgressBar* pbOpt = ::qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            if (useSideText(pbOpt))
            { 
                r.setWidth(r.width() - sideTextWidth(pbOpt));
                return r;
            }
            
            //Centering mode --- could be forced or side... so the groove area is everything
            return r;
        }

        case SE_ProgressBarContents:
        {
            QRect grooveRect = subElementRect(SE_ProgressBarGroove, option, widget);
            return insideMargin(grooveRect, WT_ProgressBar, ProgressBar::GrooveMargin);
        }

        case SE_ProgressBarLabel:
        {
            const QStyleOptionProgressBar* pbOpt = ::qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            if (useSideText(pbOpt))
            {
                int width = sideTextWidth(pbOpt);
                return QRect(r.x() + r.width() - width, r.y(), width, r.height());
            }

            //The same as the contents area..
            return subElementRect(SE_PushButtonContents, option, widget);
        }
    }
    
    return QCommonStyle::subElementRect(sr, option, widget);
}

void  KStyle::drawComplexControl (ComplexControl cc, const QStyleOptionComplex* opt,
                                   QPainter *p,      const QWidget* w) const
{
    if (cc == CC_ScrollBar)
    {
        QStyleOptionComplex* mutableOpt = const_cast<QStyleOptionComplex*>(opt);
        if ((mutableOpt->subControls & SC_ScrollBarSubLine) || (mutableOpt->subControls & SC_ScrollBarAddLine))
        {
            //If we paint one of the buttons, must paint both!
            mutableOpt->subControls |= SC_ScrollBarSubPage | SC_ScrollBarAddLine;
        }
    }

    QCommonStyle::drawComplexControl(cc, opt, p, w);
}


QRect KStyle::internalSubControlRect (ComplexControl control, const QStyleOptionComplex* option,
                                       SubControl subControl, const QWidget* w) const
{
    QRect r = option->rect;
    
    if (control == CC_ScrollBar)
    {
        switch (subControl)
        {
            //The "top" arrow
            case SC_ScrollBarSubLine:
            {
                int majorSize;
                if (widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleTopButton))
                    majorSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleButtonHeight);
                else
                    majorSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::SingleButtonHeight);

                if (option->state & State_Horizontal)
                    return handleRTL(option, QRect(r.x(), r.y(), majorSize, r.height()));
                else
                    return handleRTL(option, QRect(r.x(), r.y(), r.width(), majorSize));
                    
            }

            //The "bottom" arrow
            case SC_ScrollBarAddLine:
            {
                int majorSize;
                if (widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton))
                    majorSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleButtonHeight);
                else
                    majorSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::SingleButtonHeight);

                if (option->state & State_Horizontal)
                    return handleRTL(option, QRect(r.right() - majorSize + 1, r.y(), majorSize, r.height()));
                else
                    return handleRTL(option, QRect(r.x(), r.bottom() - majorSize + 1, r.width(), majorSize));
            }
        }
    }
    
    return QRect();
}


QRect KStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option,
                                SubControl subControl, const QWidget* widget) const
{
    QRect r = option->rect;
    
    if (control == CC_ScrollBar)
    {
        switch (subControl)
        {
            //For both arrows, we return -everything-,
            //to get stuff to repaint right. See internalSubControlRect
            //for the real thing
            case SC_ScrollBarSubLine:
            case SC_ScrollBarAddLine:
                return r;

            //The main groove area. This is used to compute the others...
            case SC_ScrollBarGroove:
            {
                QRect top = handleRTL(option, internalSubControlRect(control, option, SC_ScrollBarSubLine, widget));
                QRect bot = handleRTL(option, internalSubControlRect(control, option, SC_ScrollBarAddLine, widget));

                QPoint topLeftCorner, botRightCorner;
                if (option->state & State_Horizontal)
                {
                    topLeftCorner  = QPoint(top.right() + 1, top.top());
                    botRightCorner = QPoint(bot.left()  - 1, top.bottom());
                }
                else
                {
                    topLeftCorner  = QPoint(top.left(),  top.bottom() + 1);
                    botRightCorner = QPoint(top.right(), bot.top()    - 1);
                }

                return handleRTL(option, QRect(topLeftCorner, botRightCorner));
            }

            case SC_ScrollBarFirst:
            case SC_ScrollBarLast:
                return QRect();

            case SC_ScrollBarSlider:
            {
                const QStyleOptionSlider* slOpt = ::qstyleoption_cast<const QStyleOptionSlider*>(option);

                //We do handleRTL here to unreflect things if need be
                QRect groove = handleRTL(option, subControlRect(control, option, SC_ScrollBarGroove, widget));

                if (slOpt->minimum == slOpt->maximum)
                    return groove;

                //Figure out how much room we have..
                int space; 
                if (option->state & State_Horizontal)
                    space = groove.width();
                else
                    space = groove.height();

                //Calculate the portion of this space that the slider should take up.
                int sliderSize = int(space * float(slOpt->pageStep) /
                                        (slOpt->maximum - slOpt->minimum + slOpt->pageStep));

                if (sliderSize < widgetLayoutProp(WT_ScrollBar, ScrollBar::MinimumSliderHeight))
                    sliderSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::MinimumSliderHeight);

                if (sliderSize > space)
                    sliderSize = space;

                //What do we have remaining?
                space = space - sliderSize;

                //uhm, yeah, nothing much
                if (space <= 0)
                    return groove;

                int pos = qRound(float(slOpt->sliderPosition - slOpt->minimum)/
                                        (slOpt->maximum - slOpt->minimum)*space);
                if (option->state & State_Horizontal)
                    return handleRTL(option, QRect(groove.x() + pos, groove.y(), sliderSize, groove.height()));
                else
                    return handleRTL(option, QRect(groove.x(), groove.y() + pos, groove.width(), sliderSize));
            }

            case SC_ScrollBarSubPage:
            {
                //We do handleRTL here to unreflect things if need be
                QRect slider = handleRTL(option, subControlRect(control, option, SC_ScrollBarSlider, widget));
                QRect groove = handleRTL(option, subControlRect(control, option, SC_ScrollBarGroove, widget));

                //We're above the slider in the groove.
                if (option->state & State_Horizontal)
                    return handleRTL(option, QRect(groove.x(), groove.y(), slider.x() - groove.x(), groove.height()));
                else
                    return handleRTL(option, QRect(groove.x(), groove.y(), groove.width(), slider.y() - groove.y()));
            }

            case SC_ScrollBarAddPage:
            {
                //We do handleRTL here to unreflect things if need be
                QRect slider = handleRTL(option, subControlRect(control, option, SC_ScrollBarSlider, widget));
                QRect groove = handleRTL(option, subControlRect(control, option, SC_ScrollBarGroove, widget));

                //We're below the slider in the groove.
                if (option->state & State_Horizontal)
                    return handleRTL(option,
                            QRect(slider.right() + 1, groove.y(), groove.right() - slider.right(), groove.height()));
                else
                    return handleRTL(option,
                            QRect(groove.x(), slider.bottom() + 1, groove.width(), groove.bottom() - slider.bottom()));
            }
        }
    }
    return QCommonStyle::subControlRect(control, option, subControl, widget);
}

/*
 Checks whether the point is before the bound rect for
 bound of given orientation
*/
static bool preceeds(QPoint pt, QRect bound, const QStyleOption* opt)
{
    if (opt->state & QStyle::State_Horizontal)
    {
        //What's earlier depends on RTL or not
        if (opt->direction == Qt::LeftToRight)
            return pt.x() < bound.right();
        else
            return pt.x() > bound.x();
    }
    else
    {
        return pt.y() < bound.y();
    }
}

static QStyle::SubControl buttonPortion(QRect totalRect, QPoint pt, const QStyleOption* opt)
{
   if (opt->state & QStyle::State_Horizontal)
   {
        //What's earlier depends on RTL or not
        if (opt->direction == Qt::LeftToRight)
            return pt.x() < totalRect.center().x() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
        else
            return pt.x() > totalRect.center().x() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
    }
    else
    {
        return pt.y() < totalRect.center().y() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
    }
}

QStyle::SubControl KStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt,
                                             const QPoint& pt, const QWidget* w) const
{
    if (cc == CC_ScrollBar)
    {
        //First, check whether we're inside the groove or not...
        QRect groove = subControlRect(CC_ScrollBar, opt, SC_ScrollBarGroove, w);

        if (groove.contains(pt))
        {
            //Must be either page up/page down, or just click on the slider.
            //Grab the slider to compare
            QRect slider = subControlRect(CC_ScrollBar, opt, SC_ScrollBarSlider, w);

            if (slider.contains(pt))
                return SC_ScrollBarSlider;
            else if (preceeds(pt, slider, opt))
                return SC_ScrollBarSubPage;
            else
                return SC_ScrollBarAddPage;
        }
        else
        {
            //This is one of the up/down buttons. First, decide which one it is.
            if (preceeds(pt, groove, opt))
            {
                //"Upper" button
                if (widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleTopButton))
                {
                    QRect buttonRect = internalSubControlRect(CC_ScrollBar, opt, SC_ScrollBarSubLine, w);
                    return buttonPortion(buttonRect, pt, opt);
                }
                else
                    return SC_ScrollBarSubLine; //Easy one!
            }
            else
            {
                //"Bottom" button
                if (widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton))
                {
                    QRect buttonRect = internalSubControlRect(CC_ScrollBar, opt, SC_ScrollBarAddLine, w);
                    return buttonPortion(buttonRect, pt, opt);
                }
                else
                    return SC_ScrollBarAddLine; //Easy one!
            }
        }
    }

    return QCommonStyle::hitTestComplexControl(cc, opt, pt, w);
}


QSize KStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
{
    switch (type)
    {
        case CT_PushButton:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return contentsSize;
            
            QSize size = contentsSize;
            
            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                size = expandDim(size, WT_PushButton, PushButton::DefaultIndicatorMargin);                        
            
            //### TODO: Handle minimum size limits, extra spacing as in current styles ??
            return expandDim(size, WT_PushButton, PushButton::ContentsMargin);
        }
        
        case CT_CheckBox:        
        {
            //Add size for indicator ### handle empty case differently?            
            int indicator = widgetLayoutProp(WT_CheckBox, CheckBox::Size);
            int spacer    = widgetLayoutProp(WT_CheckBox, CheckBox::BoxTextSpace);
            
            //Make sure we include space for the focus rect margin 
            QSize size = expandDim(contentsSize, WT_CheckBox, CheckBox::FocusMargin);
            
            //Make sure we can fit the indicator (### an extra margin around that?)
            size.setHeight(qMax(size.height(), indicator));
            
            //Add space for the indicator and the icon
            size.setWidth(size.width() + indicator + spacer);
            
            return size;
        }
        
        case CT_RadioButton:
        {
            //Add size for indicator
            int indicator = widgetLayoutProp(WT_RadioButton, RadioButton::Size);
            int spacer    = widgetLayoutProp(WT_RadioButton, RadioButton::BoxTextSpace);
            
            //Make sure we include space for the focus rect margin 
            QSize size = expandDim(contentsSize, WT_RadioButton, RadioButton::FocusMargin);
            
            //Make sure we can fit the indicator (### an extra margin around that?)
            size.setHeight(qMax(size.height(), indicator));
            
            //Add space for the indicator and the icon
            size.setWidth(size.width() + indicator + spacer);
            
            return size;            
        }

        case CT_ProgressBar:
        {
            QSize size = contentsSize;
            
            const QStyleOptionProgressBar* pbOpt = ::qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            if (useSideText(pbOpt))
            {
                //Allocate extra room for side text
                size.setWidth(size.width() + sideTextWidth(pbOpt));
            }

            return size;
        }

        
        case CT_MenuBar:
        {
            int extraW = widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Right) -
                            widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Left);

            int extraH = widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Bot) -
                            widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Top);
                            
            return QSize(contentsSize.width() + extraW, contentsSize.height() + extraH);
        }

        case CT_Menu:
        {
            int extraW = widgetLayoutProp(WT_Menu, Menu::Margin + Right) -
                            widgetLayoutProp(WT_Menu, Menu::Margin + Left);

            int extraH = widgetLayoutProp(WT_Menu, Menu::Margin + Bot) -
                            widgetLayoutProp(WT_Menu, Menu::Margin + Top);
                            
            return QSize(contentsSize.width() + extraW, contentsSize.height() + extraH);
        }

        case CT_MenuItem:
        {
            const QStyleOptionMenuItem* miOpt = ::qstyleoption_cast<const QStyleOptionMenuItem*>(option);
            if (!miOpt) return contentsSize; //Someone is asking for trouble..
            
            //First, we calculate the intrinsic size of the item..
            QSize insideSize;

            switch (miOpt->menuItemType)
            {
                case QStyleOptionMenuItem::Normal:
                case QStyleOptionMenuItem::DefaultItem: //huh?
                case QStyleOptionMenuItem::SubMenu:
                {
                    int leftColW =  miOpt->maxIconWidth;
                    leftColW     =  qMax(leftColW, widgetLayoutProp(WT_MenuItem, MenuItem::CheckWidth));
                    leftColW     += widgetLayoutProp(WT_MenuItem, MenuItem::CheckSpace);

                    int rightColW = widgetLayoutProp(WT_MenuItem, MenuItem::ArrowSpace) +
                                    widgetLayoutProp(WT_MenuItem, MenuItem::ArrowWidth);
                    
                    QFontMetrics fm(miOpt->font);
                    
                    int textW;
                    int tabPos = miOpt->text.indexOf('\t');
                    if (tabPos == -1)
                    {
                        //No accel..
                        textW = fm.width(miOpt->text);
                    }
                    else
                    {
                        QString text = miOpt->text.left(tabPos);
                        QString accl = miOpt->text.mid (tabPos + 1);

                        textW = fm.width(text) +
                                fm.width(accl) +
                                widgetLayoutProp(WT_MenuItem, MenuItem::AccelSpace);
                    }

                    
                    int h = qMax(contentsSize.height(), widgetLayoutProp(WT_MenuItem, MenuItem::MinHeight));
                    insideSize = QSize(leftColW + textW + rightColW, h);
                    break;
                }

                case QStyleOptionMenuItem::Separator:
                {
                    insideSize = QSize(10, widgetLayoutProp(WT_MenuItem, MenuItem::SeparatorHeight));
                }
                break;


                //Double huh if we get those.
                case QStyleOptionMenuItem::Scroller:
                case QStyleOptionMenuItem::TearOff:
                case QStyleOptionMenuItem::Margin:
                case QStyleOptionMenuItem::EmptyArea:
                    return contentsSize;
            }
            

            //...now apply the outermost margin.
            return expandDim(insideSize, WT_MenuItem, MenuItem::Margin);
        }

        case CT_MenuBarItem:
            return expandDim(contentsSize, WT_MenuBarItem, MenuBarItem::Margin);

        case CT_TabBarTab:
            //With our PM_TabBarTabHSpace/VSpace, Qt should give us what we want for
            //contentsSize, so we just expand that. Qt also takes care of
            //the vertical thing.
            return expandDim(contentsSize, WT_Tab, Tab::ContentsMargin);
    }
    
    return QCommonStyle::sizeFromContents(type, option, contentsSize, widget);
}


// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
