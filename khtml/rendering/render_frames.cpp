/**
 * This file is part of the KDE project.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 *           (C) 2000 Stefan Schimanski (1Stein@gmx.de)
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
//#define DEBUG_LAYOUT

#include "rendering/render_frames.h"
#include "html/html_baseimpl.h"
#include "html/html_objectimpl.h"
#include "misc/htmlattrs.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_docimpl.h"
#include "misc/htmltags.h"
#include "khtmlview.h"
#include "khtml_part.h"

#include <kapplication.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kdebug.h>
#include <qtimer.h>
#include <qcursor.h>

#include <assert.h>

using namespace khtml;
using namespace DOM;

RenderFrameSet::RenderFrameSet( HTMLFrameSetElementImpl *frameSet, KHTMLView *view )
    : RenderBox()
{
  // init RenderObject attributes
    setInline(false);

  m_element = frameSet;

  // another one for bad html
  // handle <frameset cols="*" rows="100, ...">
  if ( m_element->m_rows && m_element->m_cols ) {
      // lets see if one of them is relative
      if ( m_element->m_rows->count() == 1 && m_element->m_rows->at( 0 )->isRelative() ) {
          delete m_element->m_rows;
          m_element->m_rows = 0;
      }
      if ( m_element->m_cols->count() == 1 && m_element->m_cols->at( 0 )->isRelative() ) {
          delete m_element->m_cols;
          m_element->m_cols = 0;
      }
  }

  m_rowHeight = 0;
  m_colWidth = 0;

  m_resizing = false;

  m_hSplit = -1;
  m_vSplit = -1;

  m_hSplitVar = 0;
  m_vSplitVar = 0;

  m_view = view;
}

RenderFrameSet::~RenderFrameSet()
{
  if ( m_rowHeight ) {
    delete [] m_rowHeight;
    m_rowHeight = 0;
  }
  if ( m_colWidth ) {
    delete [] m_colWidth;
    m_colWidth = 0;
  }

  delete [] m_hSplitVar;
  delete [] m_vSplitVar;
}

void RenderFrameSet::layout( )
{
    assert( !layouted() );

    if ( !parent()->isFrameSet() ) {
        m_width = m_view->visibleWidth();
        m_height = m_view->visibleHeight();
    }

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(FrameSet)::layout( ) width=" << width() << ", height=" << height() << endl;
#endif

    int remainingWidth = m_width - (m_element->totalCols()-1)*m_element->border();
    if(remainingWidth<0) remainingWidth=0;
    int remainingHeight = m_height - (m_element->totalRows()-1)*m_element->border();
    if(remainingHeight<0) remainingHeight=0;
    int widthAvailable = remainingWidth;
    int heightAvailable = remainingHeight;

    if(m_rowHeight) delete [] m_rowHeight;
    if(m_colWidth) delete [] m_colWidth;
    m_rowHeight = new int[m_element->totalRows()];
    m_colWidth = new int[m_element->totalCols()];

    int i;
    int totalRelative = 0;
    int colsRelative = 0;
    int rowsRelative = 0;
    int rowsPercent = 0;
    int colsPercent = 0;
    int remainingRelativeWidth = 0;

    if(m_element->m_rows)
    {
	// another one for bad html. If all rows have a fixed width, convert the numbers to percentages.
	bool allFixed = true;
	int totalFixed = 0;
	for(i = 0; i< m_element->totalRows(); i++) {
	    if(!(m_element->m_rows->at(i)->type == Fixed))
		allFixed = false;
	    else
		totalFixed += m_element->m_rows->at(i)->value;
	}
	if ( allFixed && totalFixed ) {
	    for(i = 0; i< m_element->totalRows(); i++) {
	 	m_element->m_rows->at(i)->type = Percent;
		m_element->m_rows->at(i)->value = m_element->m_rows->at(i)->value *100 / totalFixed;
	    }
	}

        // first distribute the available width for fixed rows, then handle the
        // percentage ones, to fix html like <framesrc rows="123,100%,123"> and
        // finally relative

        for(i = 0; i< m_element->totalRows(); i++)
        {
             if(m_element->m_rows->at(i)->type == Fixed)
            {
                m_rowHeight[i] = QMAX(m_element->m_rows->at(i)->width(heightAvailable), 14);
                if( m_rowHeight[i] > remainingHeight )
                    m_rowHeight[i] = remainingHeight;
                 remainingHeight -= m_rowHeight[i];
            }
            else if(m_element->m_rows->at(i)->type == Relative)
            {
                totalRelative += m_element->m_rows->at(i)->value;
                rowsRelative++;
            }
        }

        for(i = 0; i< m_element->totalRows(); i++)
        {
             if(m_element->m_rows->at(i)->type == Percent)
            {
                m_rowHeight[i] = QMAX(m_element->m_rows->at(i)->width(heightAvailable), 14);
                if( m_rowHeight[i] > remainingHeight )
                    m_rowHeight[i] = remainingHeight;
                 remainingHeight -= m_rowHeight[i];
                 rowsPercent++;
            }
        }

        // ###
        if(remainingHeight < 0) remainingHeight = 0;

        if ( !totalRelative && rowsRelative )
          remainingRelativeWidth = remainingHeight/rowsRelative;

        for(i = 0; i< m_element->totalRows(); i++)
         {
            if(m_element->m_rows->at(i)->type == Relative)
            {
                if ( totalRelative )
                  m_rowHeight[i] = m_element->m_rows->at(i)->value*remainingHeight/totalRelative;
                else
                  m_rowHeight[i] = remainingRelativeWidth;
                remainingHeight -= m_rowHeight[i];
                totalRelative--;
            }
        }

        // support for totally broken frame declarations
        if(remainingHeight)
        {
            // just distribute it over all columns...
            int rows = m_element->totalRows();
            if ( rowsPercent )
                rows = rowsPercent;
            for(i = 0; i< m_element->totalRows(); i++) {
                if( !rowsPercent || m_element->m_rows->at(i)->type == Percent ) {
                    int toAdd = remainingHeight/rows;
                    rows--;
                    m_rowHeight[i] += toAdd;
                    remainingHeight -= toAdd;
                }
            }
        }
    }
    else
        m_rowHeight[0] = m_height;

    if(m_element->m_cols)
    {
	// another one for bad html. If all cols have a fixed width, convert the numbers to percentages.
	// also reproduces IE and NS behaviour.
	bool allFixed = true;
	int totalFixed = 0;
	for(i = 0; i< m_element->totalCols(); i++) {
	    if(!(m_element->m_cols->at(i)->type == Fixed))
		allFixed = false;
	    else
		totalFixed += m_element->m_cols->at(i)->value;
	}
	if ( allFixed && totalFixed) {
	    for(i = 0; i< m_element->totalCols(); i++) {
		m_element->m_cols->at(i)->type = Percent;
		m_element->m_cols->at(i)->value = m_element->m_cols->at(i)->value * 100 / totalFixed;
	    }
	}

        totalRelative = 0;
        remainingRelativeWidth = 0;

        // first distribute the available width for fixed columns, then handle the
        // percentage ones, to fix html like <framesrc cols="123,100%,123"> and
        // finally relative

        for(i = 0; i< m_element->totalCols(); i++)
        {
            if (m_element->m_cols->at(i)->type == Fixed)
            {
                m_colWidth[i] = QMAX(m_element->m_cols->at(i)->width(widthAvailable), 14);
                if( m_colWidth[i] > remainingWidth )
                    m_colWidth[i] = remainingWidth;
                remainingWidth -= m_colWidth[i];
            }
            else if(m_element->m_cols->at(i)->type == Relative)
            {
                totalRelative += m_element->m_cols->at(i)->value;
                colsRelative++;
            }
        }

        for(i = 0; i< m_element->totalCols(); i++)
        {
            if(m_element->m_cols->at(i)->type == Percent)
            {
                m_colWidth[i] = QMAX(m_element->m_cols->at(i)->width(widthAvailable), 14);
                if( m_colWidth[i] > remainingWidth )
                    m_colWidth[i] = remainingWidth;
                remainingWidth -= m_colWidth[i];
                colsPercent++;
            }
        }
        // ###
        if(remainingWidth < 0) remainingWidth = 0;

        if ( !totalRelative && colsRelative )
          remainingRelativeWidth = remainingWidth/colsRelative;

        for(i = 0; i < m_element->totalCols(); i++)
        {
            if(m_element->m_cols->at(i)->type == Relative)
            {
                if ( totalRelative )
                  m_colWidth[i] = m_element->m_cols->at(i)->value*remainingWidth/totalRelative;
                else
                  m_colWidth[i] = remainingRelativeWidth;
                remainingWidth -= m_colWidth[i];
                totalRelative--;
            }
        }

        // support for totally broken frame declarations
        if(remainingWidth)
        {
            // just distribute it over all columns...
            int cols = m_element->totalCols();
            if ( colsPercent )
                cols = colsPercent;
            for(i = 0; i< m_element->totalCols(); i++) {
                if( !colsPercent || m_element->m_cols->at(i)->type == Percent ) {
                    int toAdd = remainingWidth/cols;
                    cols--;
                    m_colWidth[i] += toAdd;
                    remainingWidth -= toAdd;
                }
            }
        }

    }
    else
        m_colWidth[0] = m_width;

    positionFrames();

    RenderObject *child = firstChild();
    if ( !child )
      return;

    if(!m_hSplitVar && !m_vSplitVar)
    {
#ifdef DEBUG_LAYOUT
        kdDebug( 6031 ) << "calculationg fixed Splitters" << endl;
#endif
        if(!m_vSplitVar && m_element->totalCols() > 1)
        {
            m_vSplitVar = new bool[m_element->totalCols()];
            for(int i = 0; i < m_element->totalCols(); i++) m_vSplitVar[i] = true;
        }
        if(!m_hSplitVar && m_element->totalRows() > 1)
        {
            m_hSplitVar = new bool[m_element->totalRows()];
            for(int i = 0; i < m_element->totalRows(); i++) m_hSplitVar[i] = true;
        }

        for(int r = 0; r < m_element->totalRows(); r++)
        {
            for(int c = 0; c < m_element->totalCols(); c++)
            {
                bool fixed = false;

                if ( child->isFrameSet() )
                  fixed = static_cast<RenderFrameSet *>(child)->frameSetImpl()->noResize();
                else
                  fixed = static_cast<RenderFrame *>(child)->frameImpl()->noResize();

                /*
                if(child->id() == ID_FRAMESET)
                    fixed = (static_cast<HTMLFrameSetElementImpl *>(child))->noResize();
                else if(child->id() == ID_FRAME)
                    fixed = (static_cast<HTMLFrameElementImpl *>(child))->noResize();
                */

                if(fixed)
                {
#ifdef DEBUG_LAYOUT
                    kdDebug( 6031 ) << "found fixed cell " << r << "/" << c << "!" << endl;
#endif
                    if( m_element->totalCols() > 1)
                    {
                        if(c>0) m_vSplitVar[c-1] = false;
                        m_vSplitVar[c] = false;
                    }
                    if( m_element->totalRows() > 1)
                    {
                        if(r>0) m_hSplitVar[r-1] = false;
                        m_hSplitVar[r] = false;
                    }
                    child = child->nextSibling();
                    if(!child) goto end2;
                }
#ifdef DEBUG_LAYOUT
                else
                    kdDebug( 6031 ) << "not fixed: " << r << "/" << c << "!" << endl;
#endif
            }
        }

    }
 end2:
    setLayouted();
}

void RenderFrameSet::positionFrames()
{
  int r;
  int c;

  RenderObject *child = firstChild();
  if ( !child )
    return;

  //  NodeImpl *child = _first;
  //  if(!child) return;

  int yPos = 0;

  for(r = 0; r < m_element->totalRows(); r++)
  {
    int xPos = 0;
    for(c = 0; c < m_element->totalCols(); c++)
    {
      child->setPos( xPos, yPos );
#ifdef DEBUG_LAYOUT
      kdDebug(6040) << "child frame at (" << xPos << "/" << yPos << ") size (" << m_colWidth[c] << "/" << m_rowHeight[r] << ")" << endl;
#endif
      bool relayout = (m_colWidth[c] != child->width()) || (m_rowHeight[r] != child->height());
      child->setWidth( m_colWidth[c] );
      child->setHeight( m_rowHeight[r] );
      // has to be resized and itself resize its contents
      if (relayout || !layouted()) {
          child->setLayouted(false);
          child->layout( );
      }

      xPos += m_colWidth[c] + m_element->border();
      child = child->nextSibling();

      if ( !child )
        return;

    }

    yPos += m_rowHeight[r] + m_element->border();
  }

  // all the remaining frames are hidden to avoid ugly
  // spurious nonlayouted frames
  while ( child ) {
      child->setWidth( 0 );
      child->setHeight( 0 );
      child->setLayouted();

      child = child->nextSibling();
  }
}

bool RenderFrameSet::userResize( MouseEventImpl *evt )
{
  bool res = false;
  int _x = evt->clientX();
  int _y = evt->clientY();

  if ( !m_resizing && evt->id() == EventImpl::MOUSEMOVE_EVENT || evt->id() == EventImpl::MOUSEDOWN_EVENT )
  {
#ifdef DEBUG_LAYOUT
    kdDebug( 6031 ) << "mouseEvent:check" << endl;
#endif

    m_hSplit = -1;
    m_vSplit = -1;
    //bool resizePossible = true;

    // check if we're over a horizontal or vertical boundary
    int pos = m_colWidth[0];
    for(int c = 1; c < m_element->totalCols(); c++)
    {
      if(_x >= pos && _x <= pos+m_element->border())
      {
        if(m_vSplitVar && m_vSplitVar[c-1] == true) m_vSplit = c-1;
#ifdef DEBUG_LAYOUT
        kdDebug( 6031 ) << "vsplit!" << endl;
#endif
        res = true;
        break;
      }
      pos += m_colWidth[c] + m_element->border();
    }

    pos = m_rowHeight[0];
    for(int r = 1; r < m_element->totalRows(); r++)
    {
      if( _y >= pos && _y <= pos+m_element->border())
      {
        if(m_hSplitVar && m_hSplitVar[r-1] == true) m_hSplit = r-1;
#ifdef DEBUG_LAYOUT
        kdDebug( 6031 ) << "hsplitvar = " << m_hSplitVar << endl;
        kdDebug( 6031 ) << "hsplit!" << endl;
#endif
        res = true;
        break;
      }
      pos += m_rowHeight[r] + m_element->border();
    }
#ifdef DEBUG_LAYOUT
    kdDebug( 6031 ) << m_hSplit << "/" << m_vSplit << endl;
#endif

    QCursor cursor;
    if(m_hSplit != -1 && m_vSplit != -1)
    {
      cursor = Qt::sizeAllCursor;
    }
    else if( m_vSplit != -1 )
    {
      cursor = Qt::splitHCursor;
    }
    else if( m_hSplit != -1 )
    {
      cursor = Qt::splitVCursor;
    }

    if(evt->id() == EventImpl::MOUSEDOWN_EVENT)
    {
      m_resizing = true;
      KApplication::setOverrideCursor(cursor);
      m_vSplitPos = _x;
      m_hSplitPos = _y;
    }
    else
      m_view->viewport()->setCursor(cursor);

  }

  // ### need to draw a nice movin indicator for the resize.
  // ### check the resize is not going out of bounds.
  if(m_resizing && evt->id() == EventImpl::MOUSEUP_EVENT)
  {
    m_resizing = false;
    KApplication::restoreOverrideCursor();

    if(m_vSplit != -1 )
    {
#ifdef DEBUG_LAYOUT
      kdDebug( 6031 ) << "split xpos=" << _x << endl;
#endif
      int delta = m_vSplitPos - _x;
      m_colWidth[m_vSplit] -= delta;
      m_colWidth[m_vSplit+1] += delta;
    }
    if(m_hSplit != -1 )
    {
#ifdef DEBUG_LAYOUT
      kdDebug( 6031 ) << "split ypos=" << _y << endl;
#endif
      int delta = m_hSplitPos - _y;
      m_rowHeight[m_hSplit] -= delta;
      m_rowHeight[m_hSplit+1] += delta;
    }

    positionFrames( );
  }

  return res;
}

bool RenderFrameSet::canResize( int _x, int _y, DOM::NodeImpl::MouseEventType type )
{
   if(m_resizing || type == DOM::NodeImpl::MousePress)
     return true;

  if ( type != DOM::NodeImpl::MouseMove )
    return false;

  // check if we're over a horizontal or vertical boundary
  int pos = m_colWidth[0];
  for(int c = 1; c < m_element->totalCols(); c++)
    if(_x >= pos && _x <= pos+m_element->border())
      return true;

  pos = m_rowHeight[0];
  for(int r = 1; r < m_element->totalRows(); r++)
    if( _y >= pos && _y <= pos+m_element->border())
      return true;

  return false;
}

void RenderFrameSet::dump(QTextStream *stream, QString ind) const
{
  *stream << " totalrows=" << m_element->totalRows();
  *stream << " totalcols=" << m_element->totalCols();

  uint i;
  for (i = 0; i < (uint)m_element->totalRows(); i++)
    *stream << " hSplitvar(" << i << ")=" << m_hSplitVar[i];

  for (i = 0; i < (uint)m_element->totalCols(); i++)
    *stream << " vSplitvar(" << i << ")=" << m_vSplitVar[i];

  RenderBox::dump(stream,ind);
}


/**************************************************************************************/

RenderPart::RenderPart( KHTMLView *view )
    : RenderWidget( view )
{
    // init RenderObject attributes
    setInline(false);

    m_view = view;
}

void RenderPart::setWidget( QWidget *widget )
{
#ifdef DEBUG_LAYOUT
    kdDebug(6031) << "RenderPart::setWidget()" << endl;
#endif
    setQWidget( widget );
    if(widget->inherits("KHTMLView"))
        connect( widget, SIGNAL( cleared() ), this, SLOT( slotViewCleared() ) );

    setLayouted( false );
    setMinMaxKnown( false );

    // make sure the scrollbars are set correctly for restore
    // ### find better fix
    slotViewCleared();
}

bool RenderPart::partLoadingErrorNotify(khtml::ChildFrame *, const KURL& , const QString& )
{
    return false;
}

short RenderPart::intrinsicWidth() const
{
    return 300;
}

int RenderPart::intrinsicHeight() const
{
    return 200;
}

void RenderPart::slotViewCleared()
{
}

/***************************************************************************************/

RenderFrame::RenderFrame( KHTMLView *view, DOM::HTMLFrameElementImpl *frame )
    : RenderPart( view ), m_element( frame )
{
    setInline( false );

}

void RenderFrame::slotViewCleared()
{
    if(m_widget->inherits("QScrollView")) {
#ifdef DEBUG_LAYOUT
        kdDebug(6031) << "frame is a scrollview!" << endl;
#endif
        QScrollView *view = static_cast<QScrollView *>(m_widget);
        if(!m_element->frameBorder || !((static_cast<HTMLFrameSetElementImpl *>(m_element->parentNode()))->frameBorder()))
            view->setFrameStyle(QFrame::NoFrame);
        view->setVScrollBarMode(m_element->scrolling);
        view->setHScrollBarMode(m_element->scrolling);
        if(view->inherits("KHTMLView")) {
#ifdef DEBUG_LAYOUT
            kdDebug(6031) << "frame is a KHTMLview!" << endl;
#endif
            KHTMLView *htmlView = static_cast<KHTMLView *>(view);
            if(m_element->marginWidth != -1) htmlView->setMarginWidth(m_element->marginWidth);
            if(m_element->marginHeight != -1) htmlView->setMarginHeight(m_element->marginHeight);
        }
    }
}

/****************************************************************************************/

RenderPartObject::RenderPartObject( KHTMLView *view, DOM::HTMLElementImpl *o )
    : RenderPart( view )
{
    // init RenderObject attributes
    setInline(true);

    m_element = o;
}

void RenderPartObject::updateWidget()
{
  QString url;
  QString serviceType;
  QStringList params;
  KHTMLPart *part = m_view->part();

  // ### this should be constant true - move iframe to somewhere else
  if (m_element->id() == ID_OBJECT || m_element->id() == ID_EMBED) {

      for (NodeImpl* child = m_element->firstChild(); child; child=child->nextSibling()) {
          if ( child->id() == ID_PARAM ) {
              HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>( child );

              QString aStr = p->name();
              aStr += QString::fromLatin1("=\"");
              aStr += p->value();
              aStr += QString::fromLatin1("\"");
              params.append(aStr);
          }
      }
      params.append( QString::fromLatin1("__KHTML__PLUGINEMBED=\"YES\"") );
      params.append( QString::fromLatin1("__KHTML__PLUGINBASEURL=\"%1\"").arg( part->url().url() ) );
  }

  if(m_element->id() == ID_OBJECT) {

      // check for embed child object
      HTMLObjectElementImpl *o = static_cast<HTMLObjectElementImpl *>(m_element);
      HTMLEmbedElementImpl *embed = 0;
      for (NodeImpl *child = o->firstChild(); child; child = child->nextSibling())
          if ( child->id() == ID_EMBED ) {
              embed = static_cast<HTMLEmbedElementImpl *>( child );
              break;
          }

      params.append( QString::fromLatin1("__KHTML__CLASSID=\"%1\"").arg( o->classId ) );
      params.append( QString::fromLatin1("__KHTML__CODEBASE=\"%1\"").arg( o->getAttribute(ATTR_CODEBASE).string() ) );

      if ( !embed )
      {
          url = o->url;
          serviceType = o->serviceType;
          if(serviceType.isEmpty() || serviceType.isNull()) {
              if(!o->classId.isEmpty()) {
                  // We have a clsid, means this is activex (Niko)
                  serviceType = "application/x-activex-handler";
                  url = "dummy"; // Not needed, but KHTMLPart aborts the request if empty
              }

              if(o->classId.contains(QString::fromLatin1("D27CDB6E-AE6D-11cf-96B8-444553540000"))) {
                  // It is ActiveX, but the nsplugin system handling
                  // should also work, that's why we don't override the
                  // serviceType with application/x-activex-handler
                  // but let the KTrader in khtmlpart::createPart() detect
                  // the user's preference: launch with activex viewer or
                  // with nspluginviewer (Niko)
                  serviceType = "application/x-shockwave-flash";
              }
              else if(o->classId.contains(QString::fromLatin1("CFCDAA03-8BE4-11cf-B84B-0020AFBBCCFA")))
                  serviceType = "audio/x-pn-realaudio-plugin";

              // TODO: add more plugins here
          }

          if((url.isEmpty() || url.isNull())) {
              // look for a SRC attribute in the params
              NodeImpl *child = o->firstChild();
              while ( child ) {
                  if ( child->id() == ID_PARAM ) {
                      HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>( child );

                      if ( p->name().lower()==QString::fromLatin1("src") ||
                           p->name().lower()==QString::fromLatin1("movie") ||
                           p->name().lower()==QString::fromLatin1("code") )
                      {
                          url = p->value();
                          break;
                      }
                  }
                  child = child->nextSibling();
              }
          }


          if ( url.isEmpty() && serviceType.isEmpty() ) {
#ifdef DEBUG_LAYOUT
              kdDebug() << "RenderPartObject::close - empty url and serverType" << endl;
#endif
              return;
          }
          part->requestObject( this, url, serviceType, params );
      }
      else {
          // render embed object
          url = embed->url;
          serviceType = embed->serviceType;

          if ( url.isEmpty() && serviceType.isEmpty() ) {
#ifdef DEBUG_LAYOUT
              kdDebug() << "RenderPartObject::close - empty url and serverType" << endl;
#endif
              return;
          }
          part->requestObject( this, url, serviceType, params );
      }
  }
  else if ( m_element->id() == ID_EMBED ) {

      HTMLEmbedElementImpl *o = static_cast<HTMLEmbedElementImpl *>(m_element);
      url = o->url;
      serviceType = o->serviceType;

      if ( url.isEmpty() && serviceType.isEmpty() ) {
#ifdef DEBUG_LAYOUT
          kdDebug() << "RenderPartObject::close - empty url and serverType" << endl;
#endif
          return;
      }
      // add all attributes set on the embed object
      NamedAttrMapImpl* a = o->attributes();
      if (a) {
          for (unsigned long i = 0; i < a->length(); ++i) {
              AttributeImpl* it = a->attributeItem(i);
              params.append(o->getDocument()->attrName(it->id()).string() + "=\"" + it->value().string() + "\"");
          }
      }
      part->requestObject( this, url, serviceType, params );
  } else {
      assert(m_element->id() == ID_IFRAME);
      HTMLIFrameElementImpl *o = static_cast<HTMLIFrameElementImpl *>(m_element);
      url = o->url.string();
      if( url.isEmpty()) return;
      KHTMLView *v = static_cast<KHTMLView *>(m_view);
      v->part()->requestFrame( this, url, o->name.string(), QStringList(), true );
  }
  setMinMaxKnown(false);
  setLayouted(false);
}

// ugly..
void RenderPartObject::close()
{
    if ( m_element->id() == ID_OBJECT )
        updateWidget();
    RenderPart::close();
}


bool RenderPartObject::partLoadingErrorNotify( khtml::ChildFrame *childFrame, const KURL& url, const QString& serviceType )
{
    KHTMLPart *part = static_cast<KHTMLView *>(m_view)->part();
    //kdDebug() << "RenderPartObject::partLoadingErrorNotify serviceType=" << serviceType << endl;
    // Check if we just tried with e.g. nsplugin
    // and fallback to the activexhandler if there is a classid
    // and a codebase, where we may download the ocx if it's missing
    if( serviceType != "application/x-activex-handler" && m_element->id()==ID_OBJECT ) {

        // check for embed child object
        HTMLObjectElementImpl *o = static_cast<HTMLObjectElementImpl *>(m_element);
        HTMLEmbedElementImpl *embed = 0;
        NodeImpl *child = o->firstChild();
        while ( child ) {
            if ( child->id() == ID_EMBED )
                embed = static_cast<HTMLEmbedElementImpl *>( child );

            child = child->nextSibling();
        }
        if( embed && !o->classId.isEmpty() &&
            !( static_cast<ElementImpl *>(o)->getAttribute(ATTR_CODEBASE).string() ).isEmpty() )
        {
            KParts::URLArgs args;
            args.serviceType = "application/x-activex-handler";
            if (part->requestObject( childFrame, url, args ))
                return true; // success
        }
    }
    // Dissociate ourselves from the current event loop (to prevent crashes
    // due to the message box staying up)
    QTimer::singleShot( 0, this, SLOT( slotPartLoadingErrorNotify() ) );
    /*
     // The proper fix, but this doesn't work well yet (msg box keeps appearing)
    Tokenizer *tokenizer = static_cast<DOM::DocumentImpl *>(part->document().handle())->tokenizer();
    if (tokenizer) tokenizer->setOnHold( true );
    slotPartLoadingErrorNotify();
    if (tokenizer) tokenizer->setOnHold( false );
    */
    return false;
}

void RenderPartObject::slotPartLoadingErrorNotify()
{
    // First we need to find out the servicetype - again - this code is too duplicated !
    HTMLEmbedElementImpl *embed = 0;
    QString serviceType;
    if( m_element->id()==ID_OBJECT ) {

        // check for embed child object
        HTMLObjectElementImpl *o = static_cast<HTMLObjectElementImpl *>(m_element);
	serviceType = o->serviceType;
        NodeImpl *child = o->firstChild();
        while ( child ) {
            if ( child->id() == ID_EMBED )
                embed = static_cast<HTMLEmbedElementImpl *>( child );

            child = child->nextSibling();
        }

    } else if( m_element->id()==ID_EMBED ) {
        embed = static_cast<HTMLEmbedElementImpl *>(m_element);
    }
    if ( embed )
	serviceType = embed->serviceType;

    KHTMLPart *part = static_cast<KHTMLView *>(m_view)->part();
    KParts::BrowserExtension *ext = part->browserExtension();
    if( embed && !embed->pluginPage.isEmpty() && ext ) {
        // Prepare the mimetype to show in the question (comment if available, name as fallback)
        QString mimeName = serviceType;
        KMimeType::Ptr mime = KMimeType::mimeType(serviceType);
        if ( mime->name() != KMimeType::defaultMimeType() )
            mimeName = mime->comment();
        // Prepare the URL to show in the question (host only if http, to make it short)
        KURL pluginPageURL( embed->pluginPage );
        QString shortURL = pluginPageURL.protocol() == "http" ? pluginPageURL.host() : pluginPageURL.prettyURL();
        int res = KMessageBox::questionYesNo( m_view,
            i18n("No plugin found for '%1'.\nDo you want to download one from %2?").arg(mimeName).arg(shortURL),
	    i18n("Missing plugin"), QString::null, QString::null, QString("plugin-")+serviceType);
	if ( res == KMessageBox::Yes )
	{
          // Display vendor download page
          ext->createNewWindow( pluginPageURL );
	}
    }
}

void RenderPartObject::layout( )
{
    assert( !layouted() );

    qDebug("RenderPartObject::layout()");
    qDebug("parent: %p", RenderObject::parent());

    short m_oldwidth = m_width;
    int m_oldheight = m_height;

    calcWidth();
    calcHeight();

    if (m_width != m_oldwidth || m_height != m_oldheight)
        RenderPart::layout();

    setLayouted(!style()->width().isPercent());
}

void RenderPartObject::slotViewCleared()
{
  if(m_widget->inherits("QScrollView") ) {
#ifdef DEBUG_LAYOUT
      kdDebug(6031) << "iframe is a scrollview!" << endl;
#endif
      QScrollView *view = static_cast<QScrollView *>(m_widget);
      int frameStyle = QFrame::NoFrame;
      QScrollView::ScrollBarMode scroll = QScrollView::Auto;
      int marginw = 0;
      int marginh = 0;
      if ( m_element->id() == ID_IFRAME) {
	  HTMLIFrameElementImpl *frame = static_cast<HTMLIFrameElementImpl *>(m_element);
	  if(frame->frameBorder)
	      frameStyle = QFrame::Box;
	  scroll = frame->scrolling;
	  marginw = frame->marginWidth;
	  marginh = frame->marginHeight;
      }
      view->setFrameStyle(frameStyle);
      view->setVScrollBarMode(scroll);
      view->setHScrollBarMode(scroll);
      if(view->inherits("KHTMLView")) {
#ifdef DEBUG_LAYOUT
          kdDebug(6031) << "frame is a KHTMLview!" << endl;
#endif
          KHTMLView *htmlView = static_cast<KHTMLView *>(view);
          htmlView->setIgnoreWheelEvents( m_element->id() == ID_IFRAME );
          if(marginw != -1) htmlView->setMarginWidth(marginw);
          if(marginh != -1) htmlView->setMarginHeight(marginh);
        }
  }
}

#include "render_frames.moc"
