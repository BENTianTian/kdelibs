/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
#include <klibloader.h>
#include <klocale.h>

#include "render_applet.h"

#include <qlabel.h>
#include <qscrollview.h>

#include <java/kjavaappletwidget.h>
#include <misc/htmltags.h>

/**
 * We use single applet context to run all applets in all pages.
 * If all appelts are deleted we delete the context too. It has its
 * drawback: deleting context stops Java process and next applet have to
 * restart it again.
 * TODO: Implement contex manager which will create different context
 * for different URLs.
 */
static KJavaAppletContext *context = 0;
static int context_counter = 0;

using namespace khtml;

RenderApplet::RenderApplet(QScrollView *view,
                           QMap<QString, QString> args, HTMLElementImpl *applet)
    : RenderWidget(view)
{
    if( context == 0 ) {
        context = new KJavaAppletContext();
        context_counter = 0;
    }

    m_applet = applet;
    m_widget = new KJavaAppletWidget(context, view->viewport());
    context_counter++;

    processArguments(args);

    ((KJavaAppletWidget*) m_widget)->create();

    m_layoutPerformed = FALSE;
}

RenderApplet::~RenderApplet()
{
  context_counter--;
  if( context_counter == 0 )
  {
    delete context;
    context = 0;
  }
}

void RenderApplet::layout(bool)
{
    if(m_layoutPerformed)
        return;

    KJavaAppletWidget *tmp = ((KJavaAppletWidget*) m_widget);

    NodeImpl *child = m_applet->firstChild();

    while(child)
    {
        if(child->id() == ID_PARAM)
        {
            HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>(child);
            tmp->setParameter( p->name(), p->value());
        }
        child = child->nextSibling();
    }

    tmp->show();

    m_layoutPerformed = TRUE;
}

void RenderApplet::processArguments(QMap<QString, QString> args)
{
    KJavaAppletWidget *tmp = (KJavaAppletWidget*) m_widget;

    tmp->setBaseURL( args["baseURL"] );
    tmp->setAppletClass( args["code"] );

    m_width = args["width"].toInt();
    m_height = args["height"].toInt();
    tmp->resize( m_width, m_height );

    if( args["codeBase"] != "")
        tmp->setCodeBase( args["codeBase"] );
    if( !args["name"].isNull() )
        tmp->setAppletName( args["name"] );
    else
        tmp->setAppletName( args["code"] );

    if( args["archive"] )
        tmp->setJARFile( args["archive"] );
}


RenderEmptyApplet::RenderEmptyApplet(QScrollView *view, QSize sz)
  : RenderWidget( view )
{
  m_widget =
    new QLabel(i18n("Java Applet is not loaded. (Java interpreter disabled)"),
               view->viewport());

  ((QLabel*)m_widget)->setAlignment( Qt::AlignCenter );

  m_width = sz.width();
  m_height = sz.height();
  m_widget->resize( sz );
}

