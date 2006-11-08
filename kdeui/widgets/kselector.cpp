/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QStyle>
#include <QStyleOption>

#include <kimageeffect.h>

#include "kselector.h"
#include <stdio.h>

//-----------------------------------------------------------------------------
/*
 * 1D value selector with contents drawn by derived class.
 * See KColorDialog for example.
 */

#define ARROWWINGSIZE 2

class KSelector::Private
{
public:
  QPoint m_previousPos;
  bool m_indent;
};

KSelector::KSelector( QWidget *parent )
  : QAbstractSlider( parent )
 , d(new Private)
{
    setOrientation(Qt::Horizontal);
    d->m_indent = true;
    d->m_previousPos = calcArrowPos( 0 );
}

KSelector::KSelector( Qt::Orientation o, QWidget *parent )
  : QAbstractSlider( parent )
 , d(new Private)
{
    setOrientation(o);
    d->m_indent = true;
    d->m_previousPos = calcArrowPos( 0 );
}


KSelector::~KSelector()
{
    delete d;
}

void KSelector::setIndent( bool i )
{
    d->m_indent = i;
}

bool KSelector::indent() const
{
    return d->m_indent;
}

QRect KSelector::contentsRect() const
{
  int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  int iw = (w < ARROWWINGSIZE) ? ARROWWINGSIZE : w;

  if ( orientation() == Qt::Vertical )
    return QRect( w, iw, width() - w * 2 - 5, height() - 2 * iw );
  else
    return QRect( iw, w, width() - 2 * iw, height() - w * 2 - 5 );
}

void KSelector::paintEvent( QPaintEvent * )
{
  QPainter painter;
  int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  int iw = (w < ARROWWINGSIZE) ? ARROWWINGSIZE : w;

  painter.begin( this );

  drawContents( &painter );

  QBrush brush;

  QPoint pos = calcArrowPos( value() );
  drawArrow( &painter, pos );

  if ( indent() )
  {
    QStyleOptionFrame opt;
    opt.initFrom( this );
    opt.state = QStyle::State_Sunken;
    if ( orientation() == Qt::Vertical )
      opt.rect.adjust( 0, iw - w, -5, w - iw );
    else
      opt.rect.adjust(iw - w, 0, w - iw, -5);
    style()->drawPrimitive( QStyle::PE_Frame, &opt, &painter, this );
  }


  painter.end();
}

void KSelector::mousePressEvent( QMouseEvent *e )
{
    setSliderDown(true);
    moveArrow( e->pos() );
}

void KSelector::mouseMoveEvent( QMouseEvent *e )
{
  moveArrow( e->pos() );
}

void KSelector::mouseReleaseEvent( QMouseEvent *e )
{
    moveArrow( e->pos() );
    setSliderDown(false);
}

void KSelector::wheelEvent( QWheelEvent *e )
{
    int val = value() + e->delta()/120;
    setSliderDown(true);
    setValue( val );
    setSliderDown(false);
}

void KSelector::moveArrow( const QPoint &pos )
{
    int val;
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int iw = (w < ARROWWINGSIZE) ? ARROWWINGSIZE : w;

    if ( orientation() == Qt::Vertical )
        val = ( maximum() - minimum() ) * (height() - pos.y() - iw)
            / (height() - iw * 2) + minimum();
    else
        val = ( maximum() - minimum() ) * ( pos.x() - iw)
            / (width() - iw * 2) + minimum();

    setValue( val );
    update();
}

QPoint KSelector::calcArrowPos( int val )
{
    QPoint p;
    int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
    int iw = ( w < ARROWWINGSIZE ) ? ARROWWINGSIZE : w;

    if ( orientation() == Qt::Vertical )
    {
        p.setY( height() - iw - 1 - (height() - 2 * iw - 1) * val  / ( maximum() - minimum() ) );
        p.setX( width() - 5 );
    }
    else
    {
        p.setX( iw + (width() - 2 * iw - 1) * val  / ( maximum() - minimum() ) );
        p.setY( height() - 5 );
    }

    return p;
}

void KSelector::drawContents( QPainter * )
{}

void KSelector::drawArrow( QPainter *painter, const QPoint &pos )
{
#if 0

    QPolygon array(3);

    painter->setPen( QPen() );
    painter->setBrush( QBrush( palette().color(QPalette::ButtonText) ) );
    array.setPoint( 0, pos.x()+0, pos.y()+0 );
    array.setPoint( 1, pos.x()+5, pos.y()+5 );
    if ( orientation() == Qt::Vertical )
    {
      array.setPoint( 2, pos.x()+5, pos.y()-5 );
    }
    else
    {
      array.setPoint( 2, pos.x()-5, pos.y()+5 );
    }

    painter->drawPolygon( array );
#else
    QPolygon array(3);

    painter->setPen( QPen() );
    painter->setBrush( QBrush( palette().color(QPalette::ButtonText) ) );
    array.setPoint( 0, pos.x()+0, pos.y()+0 );
    if ( orientation() == Qt::Vertical )
    {
      array.setPoint( 1, pos.x()+5, pos.y() + ARROWWINGSIZE + 1 );
      array.setPoint( 2, pos.x()+5, pos.y() - ARROWWINGSIZE - 1 );
    }
    else
    {
      array.setPoint( 1, pos.x() + ARROWWINGSIZE + 1, pos.y() + 5 );
      array.setPoint( 2, pos.x() - ARROWWINGSIZE -1, pos.y() + 5 );
    }

    painter->drawPolygon( array );
#endif
}

//----------------------------------------------------------------------------

KGradientSelector::KGradientSelector( QWidget *parent )
    : KSelector( parent )
{
    init();
}


KGradientSelector::KGradientSelector( Qt::Orientation o, QWidget *parent )
    : KSelector( o, parent )
{
    init();
}


KGradientSelector::~KGradientSelector()
{}


void KGradientSelector::init()
{
    color1.setRgb( 0, 0, 0 );
    color2.setRgb( 255, 255, 255 );

    text1 = text2 = "";
}


void KGradientSelector::drawContents( QPainter *painter )
{
  QImage image( contentsRect().width(), contentsRect().height(), QImage::Format_RGB32 );

  QColor col;
  float scale;

  int redDiff   = color2.red() - color1.red();
  int greenDiff = color2.green() - color1.green();
  int blueDiff  = color2.blue() - color1.blue();

  if ( orientation() == Qt::Vertical )
  {
    for ( int y = 0; y < image.height(); y++ )
    {
      scale = 1.0 * y / image.height();
      col.setRgb( color1.red() + int(redDiff*scale),
            color1.green() + int(greenDiff*scale),
            color1.blue() + int(blueDiff*scale) );

      unsigned int *p = (uint *) image.scanLine( y );
      for ( int x = 0; x < image.width(); x++ )
        *p++ = col.rgb();
    }
  }
  else
  {
    unsigned int *p = (uint *) image.scanLine( 0 );

    for ( int x = 0; x < image.width(); x++ )
    {
      scale = 1.0 * x / image.width();
      col.setRgb( color1.red() + int(redDiff*scale),
            color1.green() + int(greenDiff*scale),
            color1.blue() + int(blueDiff*scale) );
      *p++ = col.rgb();
    }

    for ( int y = 1; y < image.height(); y++ )
      memcpy( image.scanLine( y ), image.scanLine( y - 1),
         sizeof( unsigned int ) * image.width() );
  }

  QColor ditherPalette[8];

  for ( int s = 0; s < 8; s++ )
    ditherPalette[s].setRgb( color1.red() + redDiff * s / 8,
                color1.green() + greenDiff * s / 8,
                color1.blue() + blueDiff * s / 8 );

  KImageEffect::dither( image, ditherPalette, 8 );

  QPixmap p = QPixmap::fromImage(image);

  painter->drawPixmap( contentsRect().x(), contentsRect().y(), p );

  if ( orientation() == Qt::Vertical )
  {
    int yPos = contentsRect().top() + painter->fontMetrics().ascent() + 2;
    int xPos = contentsRect().left() + (contentsRect().width() -
       painter->fontMetrics().width( text2 )) / 2;
    QPen pen( color2 );
    painter->setPen( pen );
    painter->drawText( xPos, yPos, text2 );

    yPos = contentsRect().bottom() - painter->fontMetrics().descent() - 2;
    xPos = contentsRect().left() + (contentsRect().width() -
      painter->fontMetrics().width( text1 )) / 2;
    pen.setColor( color1 );
    painter->setPen( pen );
    painter->drawText( xPos, yPos, text1 );
  }
  else
  {
    int yPos = contentsRect().bottom()-painter->fontMetrics().descent()-2;

    QPen pen( color2 );
    painter->setPen( pen );
    painter->drawText( contentsRect().left() + 2, yPos, text1 );

    pen.setColor( color1 );
    painter->setPen( pen );
    painter->drawText( contentsRect().right() -
       painter->fontMetrics().width( text2 ) - 2, yPos, text2 );
  }
}


#include "kselector.moc"
