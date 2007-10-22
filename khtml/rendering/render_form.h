/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef RENDER_FORM_H
#define RENDER_FORM_H

#include "rendering/render_replaced.h"
#include "rendering/render_image.h"
#include "rendering/render_flow.h"
#include "rendering/render_style.h"
#include "khtmlview.h"
#include "html/html_formimpl.h"

class QWidget;

#include <ktextedit.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <QtGui/QCheckBox>
#include <QtGui/QRadioButton>
#include <QtGui/QPushButton>
#include <klistwidget.h>
#include <kcombobox.h>
#include "dom/dom_misc.h"

class KFindDialog;
class KReplaceDialog;
class KFind;
class KReplace;
class KAction;
class KUrlRequester;

namespace DOM {
    class HTMLInputElementImpl;
    class HTMLSelectElementImpl;
    class HTMLGenericFormElementImpl;
    class HTMLTextAreaElementImpl;
}

namespace khtml {


// -------------------------------------------------------------------------

class RenderFormElement : public khtml::RenderWidget
{
public:
    RenderFormElement(DOM::HTMLGenericFormElementImpl* node);
    virtual ~RenderFormElement();

    virtual const char *renderName() const { return "RenderForm"; }

    virtual bool isFormElement() const { return true; }

    // form elements never have padding
    virtual int paddingTop() const { return 0; }
    virtual int paddingBottom() const { return 0; }
    virtual int paddingLeft() const { return 0; }
    virtual int paddingRight() const { return 0; }

    virtual void updateFromElement();

    virtual void layout();
    virtual short baselinePosition( bool ) const;

    DOM::HTMLGenericFormElementImpl *element() const
    { return static_cast<DOM::HTMLGenericFormElementImpl*>(RenderObject::element()); }

protected:
    virtual bool isRenderButton() const { return false; }
    virtual bool isEditable() const { return false; }
	Qt::AlignmentFlag textAlignment() const;

//     QPoint m_mousePos;
//     int m_state;
};

// -------------------------------------------------------------------------

// generic class for all buttons
class RenderButton : public RenderFormElement
{
    Q_OBJECT
public:
    RenderButton(DOM::HTMLGenericFormElementImpl* node);

    virtual const char *renderName() const { return "RenderButton"; }

    virtual short baselinePosition( bool ) const;

    // don't even think about making this method virtual!
    DOM::HTMLInputElementImpl* element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }

protected:
    virtual bool isRenderButton() const { return true; }
};

// -------------------------------------------------------------------------

class CheckBoxWidget: public QCheckBox, public KHTMLWidget 
{ 
public:
    CheckBoxWidget(QWidget *p): QCheckBox(p) { m_kwp->setIsRedirected(true); }
};

class RenderCheckBox : public RenderButton
{
    Q_OBJECT
public:
    RenderCheckBox(DOM::HTMLInputElementImpl* node);

    virtual const char *renderName() const { return "RenderCheckBox"; }
    virtual void updateFromElement();
    virtual void calcMinMaxWidth();

    virtual bool handleEvent(const DOM::EventImpl&);

    QCheckBox *widget() const { return static_cast<QCheckBox*>(m_widget); }

public Q_SLOTS:
    virtual void slotStateChanged(int state);
};

// -------------------------------------------------------------------------

class RadioButtonWidget: public QRadioButton, public KHTMLWidget 
{ 
public:
    RadioButtonWidget(QWidget* p): QRadioButton(p) { m_kwp->setIsRedirected(true); }
};

class RenderRadioButton : public RenderButton
{
    Q_OBJECT
public:
    RenderRadioButton(DOM::HTMLInputElementImpl* node);

    virtual const char *renderName() const { return "RenderRadioButton"; }

    virtual void calcMinMaxWidth();
    virtual void updateFromElement();

    virtual bool handleEvent(const DOM::EventImpl&);

    QRadioButton *widget() const { return static_cast<QRadioButton*>(m_widget); }

public Q_SLOTS:
    virtual void slotToggled(bool);
};


// -------------------------------------------------------------------------

class PushButtonWidget: public QPushButton, public KHTMLWidget 
{ 
public:
    PushButtonWidget(QWidget* p): QPushButton(p) { m_kwp->setIsRedirected(true); }
};

class RenderSubmitButton : public RenderButton
{
public:
    RenderSubmitButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderSubmitButton"; }

    virtual void calcMinMaxWidth();
    virtual void updateFromElement();
    virtual short baselinePosition( bool ) const;
private:
    QString rawText();
};

// -------------------------------------------------------------------------

class RenderImageButton : public RenderImage
{
public:
    RenderImageButton(DOM::HTMLInputElementImpl *element)
        : RenderImage(element) {}

    virtual const char *renderName() const { return "RenderImageButton"; }
};


// -------------------------------------------------------------------------

class RenderResetButton : public RenderSubmitButton
{
public:
    RenderResetButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderResetButton"; }

};

// -------------------------------------------------------------------------

class RenderPushButton : public RenderSubmitButton
{
public:
    RenderPushButton(DOM::HTMLInputElementImpl *element)
        : RenderSubmitButton(element) {}

};

// -------------------------------------------------------------------------

class RenderLineEdit : public RenderFormElement
{
    Q_OBJECT
public:
    RenderLineEdit(DOM::HTMLInputElementImpl *element);

    virtual void calcMinMaxWidth();

    virtual const char *renderName() const { return "RenderLineEdit"; }
    virtual void updateFromElement();
    virtual void setStyle(RenderStyle *style);

    void select();

    KLineEdit *widget() const { return static_cast<KLineEdit*>(m_widget); }
    DOM::HTMLInputElementImpl* element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }
    void highLightWord( unsigned int length, unsigned int pos );

    long selectionStart();
    long selectionEnd();
    void setSelectionStart(long pos);
    void setSelectionEnd(long pos);
    void setSelectionRange(long start, long end);
public Q_SLOTS:
    void slotReturnPressed();
    void slotTextChanged(const QString &string);
protected:
    virtual void handleFocusOut();

private:
    virtual bool isEditable() const { return true; }
    virtual bool canHaveBorder() const { return true; }
};

// -------------------------------------------------------------------------

class LineEditWidget : public KLineEdit, public KHTMLWidget
{
    Q_OBJECT
public:
    LineEditWidget(DOM::HTMLInputElementImpl* input,
                   KHTMLView* view, QWidget* parent);
    ~LineEditWidget();
    void setFocus();
    void highLightWord( unsigned int length, unsigned int pos );

protected:
    virtual bool event( QEvent *e );
    virtual void paintEvent( QPaintEvent *pe );
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void contextMenuEvent(QContextMenuEvent *e);
private Q_SLOTS:
    void clearHistoryActivated();
    void slotCheckSpelling();
    void slotSpellCheckDone( const QString &s );
    void spellCheckerMisspelling( const QString &text, int pos);
    void spellCheckerCorrected( const QString &, int, const QString &);
    void spellCheckerFinished();

private:
    enum LineEditMenuID {
        ClearHistory
    };
    DOM::HTMLInputElementImpl* m_input;
    KHTMLView* m_view;
    KAction *m_spellAction;
};

// -------------------------------------------------------------------------

class RenderFieldset : public RenderBlock
{
public:
    RenderFieldset(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderFieldSet"; }
    virtual RenderObject* layoutLegend(bool relayoutChildren);
    virtual void setStyle(RenderStyle* _style);

protected:
    virtual void paintBoxDecorations(PaintInfo& pI, int _tx, int _ty);
    void paintBorderMinusLegend(QPainter *p, int _tx, int _ty, int w,
                                  int h, const RenderStyle *style, int lx, int lw);
    RenderObject* findLegend();
};

// -------------------------------------------------------------------------

class FileButtonWidget: public KUrlRequester, public KHTMLWidget
{
public:
    FileButtonWidget(QWidget* p): KUrlRequester(p) { m_kwp->setIsRedirected(true); }
};

class RenderFileButton : public RenderFormElement
{
    Q_OBJECT
public:
    RenderFileButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderFileButton"; }
    virtual void calcMinMaxWidth();
    virtual void updateFromElement();
    void select();

    KUrlRequester *widget() const { return static_cast<KUrlRequester*>(m_widget); }

    DOM::HTMLInputElementImpl *element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }

public Q_SLOTS:
    void slotReturnPressed();
    void slotTextChanged(const QString &string);
    void slotUrlSelected(const KUrl &url);

protected:
    virtual void handleFocusOut();

    virtual bool isEditable() const { return true; }
    virtual bool canHaveBorder() const { return true; }
    virtual bool acceptsSyntheticEvents() const { return false; }

    bool m_clicked;
    bool m_haveFocus;
};


// -------------------------------------------------------------------------

class RenderLabel : public RenderFormElement
{
public:
    RenderLabel(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderLabel"; }

protected:
    virtual bool canHaveBorder() const { return true; }
};


// -------------------------------------------------------------------------

class RenderLegend : public RenderBlock
{
public:
    RenderLegend(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderLegend"; }
};

// -------------------------------------------------------------------------

class ComboBoxWidget : public KComboBox, public KHTMLWidget
{
public:
    ComboBoxWidget(QWidget *parent);

protected:
    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *dest, QEvent *e);
    virtual void showPopup();
    virtual void hidePopup();
};

// -------------------------------------------------------------------------

class ListBoxWidget: public KListWidget, public KHTMLWidget 
{ 
public:
    ListBoxWidget(QWidget* p): KListWidget(p) { m_kwp->setIsRedirected(true); }
protected:
    void scrollContentsBy(int, int)
    {
        viewport()->update();
    }
        
};

class RenderSelect : public RenderFormElement
{
    Q_OBJECT
public:
    RenderSelect(DOM::HTMLSelectElementImpl *element);

    virtual const char *renderName() const { return "RenderSelect"; }

    virtual void calcMinMaxWidth();
    virtual void layout();

    void setOptionsChanged(bool _optionsChanged);

    bool selectionChanged() { return m_selectionChanged; }
    void setSelectionChanged(bool _selectionChanged) { m_selectionChanged = _selectionChanged; }
    virtual void updateFromElement();

    void updateSelection();

    DOM::HTMLSelectElementImpl *element() const
    { return static_cast<DOM::HTMLSelectElementImpl*>(RenderObject::element()); }

protected:
    ListBoxWidget *createListBox();
    ComboBoxWidget *createComboBox();

    unsigned  m_size;
    bool m_multiple;
    bool m_useListBox;
    bool m_selectionChanged;
    bool m_ignoreSelectEvents;
    bool m_optionsChanged;

protected Q_SLOTS:
    void slotSelected(int index);
    void slotSelectionChanged();
};

// -------------------------------------------------------------------------
class TextAreaWidget : public KTextEdit, public KHTMLWidget
{
    Q_OBJECT
public:
    TextAreaWidget(int wrap, QWidget* parent);
    virtual ~TextAreaWidget();

protected:
    virtual bool event (QEvent *e );
    virtual void contextMenuEvent(QContextMenuEvent * e);
    virtual void scrollContentsBy(int dx, int dy);

private Q_SLOTS:
    void slotFind();
    void slotDoFind();
    void slotFindNext();
    void slotReplace();
    void slotDoReplace();
    void slotReplaceNext();
    void slotReplaceText(const QString&, int, int, int);
    void slotFindHighlight(const QString&, int, int);
private:
    KFindDialog *m_findDlg;
    KFind *m_find;
    KReplaceDialog *m_repDlg;
    KReplace *m_replace;
    KAction *m_findAction;
    KAction *m_findNextAction;
    KAction *m_replaceAction;
    int m_findIndex, m_findPara;
    int m_repIndex, m_repPara;
};


// -------------------------------------------------------------------------

class RenderTextArea : public RenderFormElement
{
    Q_OBJECT
public:
    RenderTextArea(DOM::HTMLTextAreaElementImpl *element);
    ~RenderTextArea();

    virtual const char *renderName() const { return "RenderTextArea"; }
    virtual void calcMinMaxWidth();
    virtual void layout();
    virtual void setStyle(RenderStyle *style);

    virtual void updateFromElement();

    // don't even think about making this method virtual!
    TextAreaWidget *widget() const { return static_cast<TextAreaWidget*>(m_widget); }
    DOM::HTMLTextAreaElementImpl* element() const
    { return static_cast<DOM::HTMLTextAreaElementImpl*>(RenderObject::element()); }

    QString text();
    void    setText(const QString& text);

    void highLightWord( unsigned int length, unsigned int pos );

    void select();

    long selectionStart();
    long selectionEnd();
    void setSelectionStart(long pos);
    void setSelectionEnd(long pos);
    void setSelectionRange(long start, long end);
protected Q_SLOTS:
    void slotTextChanged();

protected:
    virtual void handleFocusOut();

    virtual bool isEditable() const { return true; }
    virtual bool canHaveBorder() const { return true; }

    bool scrollbarsStyled;
};

// -------------------------------------------------------------------------

class ScrollBarWidget: public QScrollBar, public KHTMLWidget
{
public:
    ScrollBarWidget( QWidget * parent = 0 ): QScrollBar(parent) { m_kwp->setIsRedirected( true ); }
    ScrollBarWidget( Qt::Orientation orientation, QWidget * parent = 0 ): QScrollBar(orientation, parent) { m_kwp->setIsRedirected( true ); }
};

} //namespace

#endif
