/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
#include "html_formimpl.h"

//#include <klocale.h>

//#include <qstack.h>

#include "khtmlview.h"
#include "khtml_part.h"
#include "html_documentimpl.h"

#include "htmlhashes.h"

#include "css/cssstyleselector.h"
#include "css/cssproperties.h"

#include "rendering/render_form.h"

#include <kdebug.h>

using namespace DOM;
using namespace khtml;

template class QList<khtml::RenderFormElement>;

HTMLFormElementImpl::HTMLFormElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    post = false;
    formElements.setAutoDelete(false);
}

HTMLFormElementImpl::~HTMLFormElementImpl()
{
    // ### set the form for all formElements to 0
}

ushort HTMLFormElementImpl::id() const
{
    return ID_FORM;
}

long HTMLFormElementImpl::length() const
{
    // ###
    return 0;
}


QString HTMLFormElementImpl::formData()
{
    kdDebug( 6030 ) << "form: saveSate()" << endl;

    QString form_data;
    bool first = true;

    RenderFormElement *current = formElements.first();
    while(current)
    {
	kdDebug( 6030 ) << "getting data from " << current << endl;

        if(current->type() == RenderFormElement::HiddenButton || current->isEnabled()) {
            QString enc = current->encoding();
            if(enc.length())
            {
                if(!first)
                    form_data += '&';
                form_data += enc;
                first = false;
            }
        }
	
	current = formElements.next();
    }
    return form_data;
}



void HTMLFormElementImpl::submit(  )
{
    kdDebug( 6030 ) << "submit pressed!" << endl;
    if(!view) return;

    QString form_data = formData();

    kdDebug( 6030 ) << "formdata = " << form_data << "\npost = " << post << endl;
    if(post)
    {
        view->part()->submitForm( "post", url.string(), form_data.latin1(),
                                  target.string() );
    }
    else
        view->part()->submitForm( "get", url.string(), form_data.latin1(),
                                  target.string() );
}

void HTMLFormElementImpl::reset(  )
{
    kdDebug( 6030 ) << "reset pressed!" << endl;

    RenderFormElement *current = formElements.first();
    while(current)
    {
	current->reset();
	current = formElements.next();
    }
}

void HTMLFormElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_ACTION:
	url = attr->value();
	break;
    case ATTR_TARGET:
	target = attr->value();
	break;
    case ATTR_METHOD:
	if ( strcasecmp( attr->value(), "post" ) == 0 )
	    post = true;
	break;
    case ATTR_ENCTYPE:
    case ATTR_ACCEPT_CHARSET:
    case ATTR_ACCEPT:
	// ignore these for the moment...
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFormElementImpl::attach(KHTMLView *_view)
{
    view = _view;
    HTMLElementImpl::attach(_view);
}

void HTMLFormElementImpl::detach()
{
    view = 0;
    NodeBaseImpl::detach();
}

void HTMLFormElementImpl::radioClicked( RenderFormElement *caller )
{
    if(!view) return;

    RenderFormElement *current = formElements.first();
    while(current)
    {
	if(current->type() == RenderFormElement::RadioButton
           && current->name() == caller->name() && current != caller)
	    current->setChecked(false);

	current = formElements.next();
    }
}

void HTMLFormElementImpl::maybeSubmit()
{
    if(!view) return;

    int le = 0;
    int total = 0;

    // count number of LineEdits / total number
    RenderFormElement *current = formElements.first();
    while(current)
    {
        if(current->isEnabled() && !current->readonly()) {
            if(current->type() == RenderFormElement::LineEdit)
                le++;

            // we're not counting hidden input's here, as they're not enabled
            total++;
        }
	current = formElements.next();
    }

    // if there's only one lineedit or only one possibly successful one, submit
    if (le < 2 || total < 2)
        submit();
}


void HTMLFormElementImpl::registerFormElement(RenderFormElement *e)
{
    formElements.append(e);
}

void HTMLFormElementImpl::removeFormElement(RenderFormElement *e)
{
  // ### make sure this get's called, when formElements get removed from
  // the document tree
    formElements.remove(e);
}

// -------------------------------------------------------------------------

HTMLGenericFormElementImpl::HTMLGenericFormElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLElementImpl(doc)
{
    _form = f;

    view = 0;
    m_disabled = m_readonly = false;
}

HTMLGenericFormElementImpl::HTMLGenericFormElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    _form = getForm();

    view = 0;
    m_disabled = m_readonly = false;
}

HTMLGenericFormElementImpl::~HTMLGenericFormElementImpl()
{
}

void HTMLGenericFormElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_NAME:
	_name = attr->value();
	break;
    case ATTR_DISABLED:
	m_disabled = true;
	break;
    case ATTR_READONLY:
        m_readonly = true;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

HTMLFormElementImpl *HTMLGenericFormElementImpl::getForm() const
{
    NodeImpl *p = parentNode();
    while(p)
    {
	if( p->id() == ID_FORM )
	    return static_cast<HTMLFormElementImpl *>(p);
	p = p->parentNode();
    }
    kdDebug( 6030 ) << "couldn't find form!" << endl;
    return 0;
}


void HTMLGenericFormElementImpl::attach(KHTMLView *_view)
{
    view = _view;
    HTMLElementImpl::attach(_view);
}

void HTMLGenericFormElementImpl::detach()
{
    view = 0;
    NodeBaseImpl::detach();
}

// -------------------------------------------------------------------------

HTMLButtonElementImpl::HTMLButtonElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    _clicked = false;
    _type = SUBMIT;
    dirty = true;
}

HTMLButtonElementImpl::HTMLButtonElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    _clicked = false;
    _type = SUBMIT;
    dirty = true;
}

HTMLButtonElementImpl::~HTMLButtonElementImpl()
{
}

const DOMString HTMLButtonElementImpl::nodeName() const
{
    return "BUTTON";
}

ushort HTMLButtonElementImpl::id() const
{
    return ID_BUTTON;
}

long HTMLButtonElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLButtonElementImpl::setTabIndex( long  )
{
}

DOMString HTMLButtonElementImpl::type() const
{
    // ###
    return DOMString();
}

void HTMLButtonElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_TYPE:
	if ( strcasecmp( attr->value(), "submit" ) == 0 )
	    _type = SUBMIT;
	else if ( strcasecmp( attr->value(), "reset" ) == 0 )
	    _type = RESET;
	else if ( strcasecmp( attr->value(), "button" ) == 0 )
	    _type = BUTTON;
	break;
    case ATTR_VALUE:
	_value = attr->value();
	currValue = _value.string();
	break;
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
    case ATTR_ONFOCUS:
    case ATTR_ONBLUR:
	// ignore for the moment
	break;
    case ATTR_NAME:
	// handled by parent...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLButtonElementImpl::attach(KHTMLView *_view)
{
    view = _view;
    HTMLElementImpl::attach(_view);
}

// -------------------------------------------------------------------------

HTMLFieldSetElementImpl::HTMLFieldSetElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

HTMLFieldSetElementImpl::HTMLFieldSetElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLFieldSetElementImpl::~HTMLFieldSetElementImpl()
{
}

const DOMString HTMLFieldSetElementImpl::nodeName() const
{
    return "FIELDSET";
}

ushort HTMLFieldSetElementImpl::id() const
{
    return ID_FIELDSET;
}

// -------------------------------------------------------------------------

HTMLInputElementImpl::HTMLInputElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    _type = TEXT;
    m_checked = false;
    _maxLen = 0;
    _size = 20;
    _clicked = false;

    view = 0;
}

HTMLInputElementImpl::HTMLInputElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    _type = TEXT;
    m_checked = false;
    _maxLen = 0;
    _size = 20;
    _clicked = false;

    view = 0;
}

HTMLInputElementImpl::~HTMLInputElementImpl()
{
    ownerDocument()->removeElement(this);
}

const DOMString HTMLInputElementImpl::nodeName() const
{
    return "INPUT";
}

ushort HTMLInputElementImpl::id() const
{
    return ID_INPUT;
}

void HTMLInputElementImpl::setMaxLength( long  )
{
}

long HTMLInputElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLInputElementImpl::setTabIndex( long  )
{
}

DOMString HTMLInputElementImpl::type() const
{
    // ###
    return DOMString();
}

QString HTMLInputElementImpl::state( )
{
   RenderFormElement *formElement = dynamic_cast<RenderFormElement *>(m_render);
   if (formElement)
      return formElement->state();
   return QString::null;
}

void HTMLInputElementImpl::blur(  )
{
    kdDebug( 6030 ) << "HTMLInputElementImpl::blur(  )" << endl;

}

void HTMLInputElementImpl::focus(  )
{
    kdDebug( 6030 ) << " HTMLInputElementImpl::focus(  )" << endl;

}

void HTMLInputElementImpl::select(  )
{
    kdDebug( 6030 ) << " HTMLInputElementImpl::select(  )" << endl;
}

void HTMLInputElementImpl::click(  )
{
    kdDebug( 6030 ) << " HTMLInputElementImpl::click(  )" << endl;
}

void HTMLInputElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_TYPE:
	if ( strcasecmp( attr->value(), "text" ) == 0 )
	    _type = TEXT;
	else if ( strcasecmp( attr->value(), "password" ) == 0 )
	    _type = PASSWORD;
	else if ( strcasecmp( attr->value(), "checkbox" ) == 0 )
	    _type = CHECKBOX;
	else if ( strcasecmp( attr->value(), "radio" ) == 0 )
	    _type = RADIO;
	else if ( strcasecmp( attr->value(), "submit" ) == 0 )
	    _type = SUBMIT;
	else if ( strcasecmp( attr->value(), "reset" ) == 0 )
	    _type = RESET;
	else if ( strcasecmp( attr->value(), "file" ) == 0 )
	    _type = FILE;
	else if ( strcasecmp( attr->value(), "hidden" ) == 0 )
	    _type = HIDDEN;
	else if ( strcasecmp( attr->value(), "image" ) == 0 )
	    _type = IMAGE;
	else if ( strcasecmp( attr->value(), "button" ) == 0 )
	    _type = BUTTON;
	break;
    case ATTR_VALUE:
	_value = attr->value();
	currValue = _value.string();
	break;
    case ATTR_CHECKED:
	m_checked = true;
	break;
    case ATTR_MAXLENGTH:
	_maxLen = attr->val()->toInt();
	break;
    case ATTR_SIZE:
	_size = attr->val()->toInt();
	break;
    case ATTR_SRC:
	_src = attr->value();
	break;
    case ATTR_ALT:
    case ATTR_USEMAP:
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ignore for the moment
	break;
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value(), false);
	break;
    case ATTR_NAME:
	// handled by parent...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLInputElementImpl::attach(KHTMLView *_view)
{
    m_style = document->styleSelector()->styleForElement(this);
    view = _view;

    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
	RenderFormElement *f = 0;
	
	switch(_type)
	{
	case TEXT:
	{
	    f = new RenderLineEdit(view, _form, _maxLen, _size, false);
	    break;
	}
	case PASSWORD:
	{
	    f = new RenderLineEdit(view, _form, _maxLen, _size, true);
	    break;
	}
	case CHECKBOX:
	{
	    f = new RenderCheckBox(view, _form);
	    f->setChecked(m_checked);
	    break;
	}
	case RADIO:
	{
	    f = new RenderRadioButton(view, _form);
	    f->setChecked(m_checked);
	    break;
	}
	case SUBMIT:
	{
	    f = new RenderSubmitButton(view, _form);
	    break;
	}
	case IMAGE:
	{
	    RenderImageButton *i = new RenderImageButton(view, _form);
	    i->setImageUrl(_src, static_cast<HTMLDocumentImpl *>(document)->URL());
	    f = i;
	    break;
	}
	case RESET:
	{
	    f = new RenderResetButton(view, _form);
	    break;
	}
	case FILE:
        {
            f = new RenderFileButton(view, _form);
            break;
        }
	case HIDDEN:
        {
            f = new RenderHiddenButton(view, _form);
            break;
        }
	case BUTTON:
	{
	    f = new RenderPushButton(view, _form, this);
	    break;
	}
	}
	if (f)
	{
	    f->setName(_name);
	    f->setValue(_value);
	    f->setEnabled(!m_disabled);
            f->setReadonly(m_readonly);
            f->setGenericFormElement(this);
	
    	    m_render = f;
	    m_render->setStyle(m_style);
	    m_render->ref();
            kdDebug( 6030 ) << "adding " << m_render->renderName() << " as child of " << r->renderName() << endl;
            QString state = document->registerElement(this);
            if ( !state.isEmpty())
            {
               kdDebug( 6030 ) << "Restoring InputElem name=" << _name.string() <<
                            " state=" << state << endl;
               f->restoreState( state );
            }

	    r->addChild(m_render, _next ? _next->renderer() : 0);
	}
    }
}

void HTMLInputElementImpl::detach()
{
    HTMLGenericFormElementImpl::detach();
}


// -------------------------------------------------------------------------

HTMLLabelElementImpl::HTMLLabelElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

HTMLLabelElementImpl::HTMLLabelElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLLabelElementImpl::~HTMLLabelElementImpl()
{
}

const DOMString HTMLLabelElementImpl::nodeName() const
{
    return "LABEL";
}

ushort HTMLLabelElementImpl::id() const
{
    return ID_LABEL;
}

// -------------------------------------------------------------------------

HTMLLegendElementImpl::HTMLLegendElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

HTMLLegendElementImpl::HTMLLegendElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLLegendElementImpl::~HTMLLegendElementImpl()
{
}

const DOMString HTMLLegendElementImpl::nodeName() const
{
    return "LEGEND";
}

ushort HTMLLegendElementImpl::id() const
{
    return ID_LEGEND;
}

// -------------------------------------------------------------------------

HTMLSelectElementImpl::HTMLSelectElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    m_multiple = false;
    view = 0;
    // 0 means invalid (i.e. not set)
    m_size = 0;
}

HTMLSelectElementImpl::HTMLSelectElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_multiple = false;
    view = 0;
    // 0 means invalid (i.e. not set)
    m_size = 0;
}

ushort HTMLSelectElementImpl::id() const
{
    return ID_SELECT;
}

DOMString HTMLSelectElementImpl::type() const
{
    // ###
    return DOMString();
}

long HTMLSelectElementImpl::selectedIndex() const
{
    kdDebug( 6030 ) << " HTMLSelectElementImpl::selectedIndex()" << endl;

    // ###
    return 0;
}

void HTMLSelectElementImpl::setSelectedIndex( long  )
{
}

long HTMLSelectElementImpl::length() const
{
    // ###
    return 0;
}

void HTMLSelectElementImpl::setSize( long  )
{
}

long HTMLSelectElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLSelectElementImpl::setTabIndex( long  )
{
}

void HTMLSelectElementImpl::add( const HTMLElement &/*element*/, const HTMLElement &/*before*/ )
{
}

void HTMLSelectElementImpl::remove( long /*index*/ )
{
}

QString HTMLSelectElementImpl::state( )
{
   RenderFormElement *formElement = dynamic_cast<RenderFormElement *>(m_render);
   if (formElement)
      return formElement->state();
   return QString::null;
}

void HTMLSelectElementImpl::blur(  )
{
}

void HTMLSelectElementImpl::focus(  )
{
}

void HTMLSelectElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_SIZE:
	m_size = attr->val()->toInt();
	break;
    case ATTR_MULTIPLE:
	m_multiple = true;
	break;
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ignore for the moment
	break;
    case ATTR_ONFOCUS:
    case ATTR_ONBLUR:
    case ATTR_ONSELECT:
    case ATTR_ONCHANGE:
	// ###
	break;
    case ATTR_NAME:
	// handled by parent...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLSelectElementImpl::attach(KHTMLView *_view)
{
    m_style = document->styleSelector()->styleForElement(this);
    view = _view;

    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
        RenderSelect *f = new RenderSelect(m_size, m_multiple,
                                           view, _form);
	if (f)
	{
	    f->setName(_name);
            f->setReadonly(m_readonly);
            f->setEnabled(!m_disabled);
            f->setGenericFormElement(this);

    	    m_render = f;
	    m_render->setStyle(m_style);
	    m_render->ref();
	    r->addChild(m_render, _next ? _next->renderer() : 0);
	}
    }
}

// -------------------------------------------------------------------------

HTMLOptGroupElementImpl::HTMLOptGroupElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

HTMLOptGroupElementImpl::HTMLOptGroupElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLOptGroupElementImpl::~HTMLOptGroupElementImpl()
{
}

ushort HTMLOptGroupElementImpl::id() const
{
    return ID_OPTGROUP;
}

// -------------------------------------------------------------------------

HTMLOptionElementImpl::HTMLOptionElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_selected = false;
}

HTMLOptionElementImpl::HTMLOptionElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    m_selected = false;
}

const DOMString HTMLOptionElementImpl::nodeName() const
{
    return "TEXTAREA";
}

ushort HTMLOptionElementImpl::id() const
{
    return ID_OPTION;
}

DOMString HTMLOptionElementImpl::text() const
{
//    if(firstChild() && firstChild()->id() == ID
    // ###
    return DOMString();
}

long HTMLOptionElementImpl::index() const
{
    // ###
    return 0;
}

void HTMLOptionElementImpl::setIndex( long  )
{
}

void HTMLOptionElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_SELECTED:
        m_selected = true;
        break;
    case ATTR_VALUE:
        m_value = attr->value();
        break;
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

bool HTMLOptionElementImpl::selected() const
{
    return m_selected;
}

// -------------------------------------------------------------------------

HTMLTextAreaElementImpl::HTMLTextAreaElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    m_rows = m_cols = 0;
    m_wrap = ta_Virtual;
}


HTMLTextAreaElementImpl::HTMLTextAreaElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_rows = m_cols = 0;
    m_wrap = ta_Virtual;
}

ushort HTMLTextAreaElementImpl::id() const
{
    return ID_TEXTAREA;
}

long HTMLTextAreaElementImpl::tabIndex() const
{
    // ###
    return 0;
}


void HTMLTextAreaElementImpl::setTabIndex( long  )
{
    // ###
}

DOMString HTMLTextAreaElementImpl::type() const
{
    // ###
    return DOMString();
}

QString HTMLTextAreaElementImpl::state( )
{
   RenderFormElement *formElement = dynamic_cast<RenderFormElement *>(m_render);
   if (formElement)
      return formElement->state();
   return QString::null;
}

void HTMLTextAreaElementImpl::blur(  )
{
    // ###
}

void HTMLTextAreaElementImpl::focus(  )
{
    //###
}

void HTMLTextAreaElementImpl::select(  )
{

}

void HTMLTextAreaElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_ROWS:
        m_rows = attr->val()->toInt();
        break;
    case ATTR_COLS:
        m_cols = attr->val()->toInt();
        break;
    case ATTR_WRAP:
        if ( strcasecmp( attr->value(), "virtual" ) == 0 )
            m_wrap = ta_Virtual;
        else if ( strcasecmp ( attr->value(), "physical" ) == 0)
            m_wrap = ta_Physical;
        else
            m_wrap = ta_NoWrap;
        break;
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ignore for the moment
	break;
    case ATTR_ONFOCUS:
    case ATTR_ONBLUR:
    case ATTR_ONSELECT:
    case ATTR_ONCHANGE:
	// ###
	break;
    case ATTR_NAME:
	// handled by parent...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLTextAreaElementImpl::attach(KHTMLView *_view)
{
    m_style = document->styleSelector()->styleForElement(this);
    view = _view;

    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
	RenderTextArea *f = new RenderTextArea(m_wrap, view, _form);

	if (f)
	{
	    f->setName(_name);
            f->setReadonly(m_readonly);
	    f->setEnabled(!m_disabled);
            f->setGenericFormElement(this);

    	    m_render = f;
	    m_render->setStyle(m_style);
	    m_render->ref();
	    r->addChild(m_render, _next ? _next->renderer() : 0);
	}
    }
}


// -------------------------------------------------------------------------
