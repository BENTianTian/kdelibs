/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#ifndef HTML_FORMIMPL_H
#define HTML_FORMIMPL_H

#include "html/html_elementimpl.h"
#include "dom/html_element.h"

#include <qvaluelist.h>
#include <qptrlist.h>
#include <qcstring.h>
#include <qmemarray.h>

class KHTMLView;
class QTextCodec;

namespace khtml
{
    class RenderFormElement;
    class RenderTextArea;
    class RenderSelect;
    class RenderLineEdit;

    typedef QValueList<QCString> encodingList;
}

namespace DOM {

class HTMLFormElement;
class DOMString;
class HTMLGenericFormElementImpl;
class HTMLOptionElementImpl;

class HTMLFormElementImpl : public HTMLElementImpl
{
public:
    HTMLFormElementImpl(DocumentPtr *doc);
    virtual ~HTMLFormElementImpl();

    virtual Id id() const;

    long length() const;

    QByteArray formData( );

    DOMString enctype() const { return m_enctype; }
    void setEnctype( const DOMString & );

    DOMString boundary() const { return m_boundary; }
    void setBoundary( const DOMString & );

    bool autoComplete() const { return m_autocomplete; }

    virtual void parseAttribute(AttrImpl *attr);

    virtual void attach();
    virtual void detach();

    void radioClicked( HTMLGenericFormElementImpl *caller );

    void registerFormElement(khtml::RenderFormElement *);
    void removeFormElement(khtml::RenderFormElement *);

    void registerFormElement(HTMLGenericFormElementImpl *);
    void removeFormElement(HTMLGenericFormElementImpl *);

    /*
     * state() and restoreState() are complimentary functions.
     */
    virtual QString state() { return QString::null; }
    virtual void restoreState(const QString &) { };

    bool prepareSubmit();
    void submit();
    void reset();

    static void i18nData();

    friend class HTMLFormElement;
    friend class HTMLFormCollectionImpl;

    QPtrList<HTMLGenericFormElementImpl> formElements;
    DOMString m_url;
    DOMString m_target;
    DOMString m_enctype;
    DOMString m_boundary;
    DOMString m_acceptcharset;
    QString m_encCharset;
    KHTMLView *view;
    bool m_post : 1;
    bool m_multipart : 1;
    bool m_autocomplete : 1;
    bool m_insubmit : 1;
    bool m_doingsubmit : 1;
    bool m_inreset : 1;
};

// -------------------------------------------------------------------------

class HTMLGenericFormElementImpl : public HTMLElementImpl
{
    friend class HTMLFormElementImpl;
    friend class khtml::RenderFormElement;

public:
    HTMLGenericFormElementImpl(DocumentPtr *doc);
    HTMLGenericFormElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);
    virtual ~HTMLGenericFormElementImpl();

    HTMLFormElementImpl *form() { return m_form; }

    virtual void parseAttribute(AttrImpl *attr);

    virtual void attach();
    virtual void detach();

    virtual void reset() {}

    void onSelect();
    void onChange();

    bool disabled() const { return m_disabled; }
    void setDisabled(bool _disabled);

    virtual bool isSelectable() const;
    virtual bool isEnumeratable() const { return false; }

    bool readOnly() const { return m_readOnly; }
    void setReadOnly(bool _readOnly) { m_readOnly = _readOnly; }

    void setForm(HTMLFormElementImpl *f) { m_form = f; }

    DOMString name() const;
    void setName(const DOMString& name);

    /*
     * override in derived classes to get the encoded name=value pair
     * for submitting
     * return true for a successful control (see 17.13.2)
     */
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool) { return false; }

    virtual void setParent(NodeImpl *parent);

    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool isEditable();

private:
    void init();

protected:
    HTMLFormElementImpl *getForm() const;

    DOMStringImpl* m_name;
    HTMLFormElementImpl *m_form;
    KHTMLView *view;
    bool m_disabled, m_readOnly;
};

// -------------------------------------------------------------------------

class HTMLButtonElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLButtonElementImpl(DocumentPtr *doc);
    HTMLButtonElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);

    virtual ~HTMLButtonElementImpl();

    enum typeEnum {
        SUBMIT,
        RESET,
        BUTTON
    };

    virtual Id id() const;

    DOMString type() const;

    void parseAttribute(AttrImpl *attr);

    virtual void attach();

    virtual void defaultEventHandler(EventImpl *evt);

    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);

protected:
    DOMString m_value;
    QString   m_currValue;
    typeEnum  m_type : 2;
    bool      m_dirty : 1;
    bool      m_clicked : 1;
    bool      m_activeSubmit : 1;
};

// -------------------------------------------------------------------------

class HTMLFieldSetElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLFieldSetElementImpl(DocumentPtr *doc);
    HTMLFieldSetElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);

    virtual ~HTMLFieldSetElementImpl();

    virtual Id id() const;
};

// -------------------------------------------------------------------------

class HTMLInputElementImpl : public HTMLGenericFormElementImpl
{
    friend class khtml::RenderLineEdit;

public:
    enum typeEnum {
        TEXT,
        PASSWORD,
        CHECKBOX,
        RADIO,
        SUBMIT,
        RESET,
        FILE,
        HIDDEN,
        IMAGE,
        BUTTON,
        ISINDEX
    };

    HTMLInputElementImpl(DocumentPtr *doc);
    HTMLInputElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);
    virtual ~HTMLInputElementImpl();

    virtual Id id() const;

    virtual bool isEnumeratable() const { return inputType() != IMAGE; }

    bool autoComplete() const { return m_autocomplete; }

    bool checked() const { return m_checked; }
    void setChecked(bool);
    long maxLength() const { return m_maxLen; }
    int size() const { return m_size; }
    DOMString type() const;
    void setType(const DOMString& t);

    DOMString value() const;
    void setValue(DOMString val);

    void blur();
    void focus();

    DOMString filename() const { return m_filename; }
    void setFilename(DOMString _filename) { m_filename = _filename; }

    virtual QString state();
    virtual void restoreState(const QString &);

    void select();
    void click();

    virtual void parseAttribute(AttrImpl *attr);

    virtual void attach();
    virtual void applyChanges(bool top=true, bool force=true);
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);

    typeEnum inputType() const { return m_type; }
    virtual void reset();

    // used in case input type=image was clicked.
    int clickX() const { return xPos; }
    int clickY() const { return yPos; }

    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool isEditable();

protected:

    DOMString altText() const;

    DOMString m_value;
    DOMString m_filename;
    DOMString m_src;
    DOMString m_defaultValue;
    int       xPos;
    short     m_maxLen;
    short     m_size;
    short     yPos;

    typeEnum m_type : 4;
    bool m_clicked : 1 ;
    bool m_defaultChecked : 1;
    bool m_checked : 1;
    bool m_haveType : 1;
    bool m_firstAttach :1;
    bool m_activeSubmit : 1;
    bool m_autocomplete : 1;

private:
    void init();
};

// -------------------------------------------------------------------------

class HTMLLabelElementImpl : public HTMLElementImpl
{
public:
    HTMLLabelElementImpl(DocumentPtr *doc);
    virtual ~HTMLLabelElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttrImpl *attr);

    /**
     * the form element this label is associated to.
     */
    ElementImpl *formElement();
 private:
    DOMString m_formElementID;
};

// -------------------------------------------------------------------------

class HTMLLegendElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLLegendElementImpl(DocumentPtr *doc);
    HTMLLegendElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);
    virtual ~HTMLLegendElementImpl();

    virtual Id id() const;
};


// -------------------------------------------------------------------------

class HTMLSelectElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLSelectElementImpl(DocumentPtr *doc);
    HTMLSelectElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);

    virtual Id id() const;

    DOMString type() const;

    long selectedIndex() const;
    void setSelectedIndex( long index );

    virtual bool isEnumeratable() const { return true; }

    long length() const;

    long minWidth() const { return m_minwidth; }

    long size() const { return m_size; }

    bool multiple() const { return m_multiple; }

    void add ( const HTMLElement &element, const HTMLElement &before );
    void remove ( long index );
    void blur();
    void focus();

    DOMString value();
    void setValue(DOMStringImpl* value);

    virtual QString state();
    virtual void restoreState(const QString &);

    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode );
    virtual NodeImpl *replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *removeChild ( NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *appendChild ( NodeImpl *newChild, int &exceptioncode );

    virtual void parseAttribute(AttrImpl *attr);

    virtual void attach();
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);

    // get the actual listbox index of the optionIndexth option
    int optionToListIndex(int optionIndex) const;
    // reverse of optionToListIndex - get optionIndex from listboxIndex
    int listToOptionIndex(int listIndex) const;
    void recalcListItems();
    QMemArray<HTMLGenericFormElementImpl*> listItems() const { return m_listItems; }
    virtual void reset();
    void notifyOptionSelected(HTMLOptionElementImpl *selectedOption, bool selected);

protected:
    QMemArray<HTMLGenericFormElementImpl*> m_listItems;
    short m_minwidth;
    short m_size : 15;
    bool m_multiple : 1;
};

// -------------------------------------------------------------------------

class HTMLKeygenElementImpl : public HTMLSelectElementImpl
{
public:
    HTMLKeygenElementImpl(DocumentPtr *doc);
    HTMLKeygenElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);

    virtual Id id() const;

    DOMString type() const;

    long selectedIndex() const;
    void setSelectedIndex( long index );

    // ### this is just a rough guess
    virtual bool isEnumeratable() const { return false; }

    virtual void parseAttribute(AttrImpl *attr);
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);

private:
    void init(DocumentPtr*);
};

// -------------------------------------------------------------------------

class HTMLOptGroupElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLOptGroupElementImpl(DocumentPtr *doc);
    HTMLOptGroupElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);
    virtual ~HTMLOptGroupElementImpl();

    virtual Id id() const;

    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode );
    virtual NodeImpl *replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *removeChild ( NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *appendChild ( NodeImpl *newChild, int &exceptioncode );
    virtual void parseAttribute(AttrImpl *attr);
    void recalcSelectOptions();
    virtual void setChanged(bool);

};


// ---------------------------------------------------------------------------

class HTMLOptionElementImpl : public HTMLGenericFormElementImpl
{
    friend class khtml::RenderSelect;
    friend class DOM::HTMLSelectElementImpl;

public:
    HTMLOptionElementImpl(DocumentPtr *doc);
    HTMLOptionElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);

    virtual Id id() const;

    DOMString text() const;

    long index() const;
    void setIndex( long );
    virtual void parseAttribute(AttrImpl *attr);
    DOMString value() const;
    void setValue(DOMStringImpl* value);

    bool selected() const { return m_selected; }
    void setSelected(bool _selected);

    HTMLSelectElementImpl *getSelect() const;

    virtual void setChanged(bool);

protected:
    DOMString m_value;
    bool m_selected;
};


// -------------------------------------------------------------------------

class HTMLTextAreaElementImpl : public HTMLGenericFormElementImpl
{
    friend class khtml::RenderTextArea;

public:
    enum WrapMethod {
        ta_NoWrap,
        ta_Virtual,
        ta_Physical
    };

    HTMLTextAreaElementImpl(DocumentPtr *doc);
    HTMLTextAreaElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);

    virtual Id id() const;

    long cols() const { return m_cols; }

    long rows() const { return m_rows; }

    WrapMethod wrap() const { return m_wrap; }

    virtual bool isEnumeratable() const { return true; }

    DOMString type() const;

    virtual QString state();
    virtual void restoreState(const QString &);

    void select (  );

    virtual void parseAttribute(AttrImpl *attr);
    virtual void attach();
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);
    virtual void reset();
    DOMString value();
    void setValue(DOMString _value);
    DOMString defaultValue();
    void setDefaultValue(DOMString _defaultValue);
    void blur();
    void focus();

    virtual bool isEditable();

protected:
    int m_rows;
    int m_cols;
    WrapMethod m_wrap;
    QString m_value;
    bool m_dirtyvalue;
};

// -------------------------------------------------------------------------

class HTMLIsIndexElementImpl : public HTMLInputElementImpl
{
public:
    HTMLIsIndexElementImpl(DocumentPtr *doc);
    HTMLIsIndexElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f);

    ~HTMLIsIndexElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttrImpl *attr);
    virtual void attach();

protected:
    DOMString m_prompt;
};


}; //namespace

#endif
