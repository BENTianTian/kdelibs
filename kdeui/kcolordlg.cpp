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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//-----------------------------------------------------------------------------
// KDE color selection dialog.
//
// 1999-09-27 Espen Sand <espensa@online.no>
// KColorDialog is now subclassed from KDialogBase. I have also extended
// KColorDialog::getColor() so that in contains a parent argument. This
// improves centering capability.
//
// layout managment added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>
//



#include <stdlib.h>
#include <qimage.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <kconfig.h>
#include <kglobal.h>
#include "kcolordlg.h"
#include "kcolordlg.h"
#include "kcolordrag.h"
#include <qvalidator.h>

#include <dither.h>
#include <qlineedit.h>
#include <klocale.h>
#include <kapp.h>

#include <kbuttonbox.h>
#include <qlayout.h>
#include <kseparator.h>

#define HSV_X 305
#define RGB_X 385

static QColor *standardPalette = 0;

#define STANDARD_PAL_SIZE 17

void createStandardPalette()
{
    if ( standardPalette )
	return;

    standardPalette = new QColor [STANDARD_PAL_SIZE];

    int i = 0;

    standardPalette[i++] = Qt::red;
    standardPalette[i++] = Qt::green;
    standardPalette[i++] = Qt::blue;
    standardPalette[i++] = Qt::cyan;
    standardPalette[i++] = Qt::magenta;
    standardPalette[i++] = Qt::yellow;
    standardPalette[i++] = Qt::darkRed;
    standardPalette[i++] = Qt::darkGreen;
    standardPalette[i++] = Qt::darkBlue;
    standardPalette[i++] = Qt::darkCyan;
    standardPalette[i++] = Qt::darkMagenta;
    standardPalette[i++] = Qt::darkYellow;
    standardPalette[i++] = Qt::white;
    standardPalette[i++] = Qt::lightGray;
    standardPalette[i++] = Qt::gray;
    standardPalette[i++] = Qt::darkGray;
    standardPalette[i++] = Qt::black;
}

KHSSelector::KHSSelector( QWidget *parent )
	: KXYSelector( parent )
{
	setRange( 0, 0, 359, 255 );
}

void KHSSelector::resizeEvent( QResizeEvent * )
{
	drawPalette();
}

void KHSSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KHSSelector::drawPalette()
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( xSize, ySize, 32 );
	QColor col;
	int h, s;
	uint *p;

	for ( s = ySize-1; s >= 0; s-- )
	{
		p = (uint *) image.scanLine( ySize - s - 1 );
		for( h = 0; h < xSize; h++ )
		{
			col.setHsv( 359*h/(xSize-1), 255*s/(ySize-1), 192 );
			*p = col.rgb();
			p++;
		}
	}

	if ( QColor::numBitPlanes() <= 8 )
	{
		createStandardPalette();
		kFSDither dither( standardPalette, STANDARD_PAL_SIZE );
		QImage tImage = dither.dither( image );
		pixmap.convertFromImage( tImage );
	}
	else
		pixmap.convertFromImage( image );
}

//-----------------------------------------------------------------------------

KValueSelector::KValueSelector( QWidget *parent )
	: KSelector( KSelector::Vertical, parent )
{
	setRange( 0, 255 );
	pixmap.setOptimization( QPixmap::BestOptim );
}

void KValueSelector::resizeEvent( QResizeEvent * )
{
	drawPalette();
}

void KValueSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KValueSelector::drawPalette()
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( xSize, ySize, 32 );
	QColor col;
	uint *p;
	QRgb rgb;

	for ( int v = 0; v < ySize; v++ )
	{
		p = (uint *) image.scanLine( ySize - v - 1 );
		col.setHsv( hue, sat, 255*v/(ySize-1) );
		rgb = col.rgb();
		for ( int i = 0; i < xSize; i++ )
			*p++ = rgb;
	}

	if ( QColor::numBitPlanes() <= 8 )
	{
		createStandardPalette();
		kFSDither dither( standardPalette, STANDARD_PAL_SIZE );
		QImage tImage = dither.dither( image );
		pixmap.convertFromImage( tImage );
	}
	else
		pixmap.convertFromImage( image );
}

//-----------------------------------------------------------------------------

KColorCells::KColorCells( QWidget *parent, int rows, int cols )
	: QTableView( parent )
{
	setNumRows( rows );
	setNumCols( cols );
	colors = new QColor [ rows * cols ];

	for ( int i = 0; i < rows * cols; i++ )
		colors[i] = QColor();

	selected = 0;
        inMouse = false;

	// Drag'n'Drop
	setAcceptDrops( true);
}

KColorCells::~KColorCells()
{
	delete [] colors;
}

void KColorCells::setColor( int colNum, const QColor &col )
{
	colors[colNum] = col;
	updateCell( colNum/numCols(), colNum%numCols() );
}

void KColorCells::paintCell( QPainter *painter, int row, int col )
{
	QBrush brush;
	qDrawShadePanel( painter, 1, 1, cellWidth()-2, cellHeight()-2, colorGroup(),
	            TRUE, 1, &brush );
        QColor color = colors[ row * numCols() + col ];
        if (!color.isValid()) color = backgroundColor();

	painter->setPen( color );
	painter->setBrush( QBrush( color ) );
	painter->drawRect( 2, 2, cellWidth()-4, cellHeight()-4 );

	if ( row * numCols() + col == selected )
		painter->drawWinFocusRect( 2, 2, cellWidth()-4, cellHeight()-4 );
}

void KColorCells::resizeEvent( QResizeEvent * )
{
	setCellWidth( width() / numCols() );
	setCellHeight( height() / numRows() );
}

void KColorCells::mousePressEvent( QMouseEvent *e )
{
    inMouse = true;
    mPos = e->pos();
}

int KColorCells::posToCell(const QPoint &pos, bool ignoreBorders)
{
   int row = pos.y() / cellHeight();
   int col = pos.x() / cellWidth();
   int cell = row * numCols() + col;

   if (!ignoreBorders)
   {
      int border = 2;
      int x = pos.x() - col * cellWidth();
      int y = pos.y() - row * cellHeight();
      if ( (x < border) || (x > cellWidth()-border) ||
           (y < border) || (y > cellHeight()-border))
         return -1;
   }
   return cell;
}

void KColorCells::mouseMoveEvent( QMouseEvent *e )
{
    if( !(e->state() && LeftButton)) return;

    if(inMouse) {
        int delay = KGlobal::dndEventDelay();
        if(e->x() > mPos.x()+delay || e->x() < mPos.x()-delay ||
           e->y() > mPos.y()+delay || e->y() < mPos.y()-delay){
            // Drag color object
            int cell = posToCell(mPos);
            if ((cell != -1) && colors[cell].isValid())
            {
               KColorDrag *d = KColorDrag::makeDrag( colors[cell], this);
               d->dragCopy();
            }
        }
    }
}

void KColorCells::dragEnterEvent( QDragEnterEvent *event)
{
     event->accept( KColorDrag::canDecode( event));
}

void KColorCells::dropEvent( QDropEvent *event)
{
     QColor c;
     if( KColorDrag::decode( event, c)) {
          int cell = posToCell(event->pos(), true);
	  setColor(cell,c);
     }
}

void KColorCells::mouseReleaseEvent( QMouseEvent *e )
{
	int cell = posToCell(mPos);
        int currentCell = posToCell(e->pos());

        // If we release the mouse in another cell and we don't have
        // a drag we should ignore this event.
        if (currentCell != cell)
           cell = -1;

	if ( (cell != -1) && (selected != cell) )
	{
		int prevSel = selected;
		selected = cell;
		updateCell( prevSel/numCols(), prevSel%numCols(), FALSE );
		updateCell( cell/numCols(), cell%numCols(), FALSE );
        }

        inMouse = false;
        if (cell != -1)
	    emit colorSelected( cell );
}

//-----------------------------------------------------------------------------

KColorPatch::KColorPatch( QWidget *parent ) : QFrame( parent )
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	colContext = 0;
	setAcceptDrops( true);
}

KColorPatch::~KColorPatch()
{
  if ( colContext )
    QColor::destroyAllocContext( colContext );
}

void KColorPatch::setColor( const QColor &col )
{
	if ( colContext )
		QColor::destroyAllocContext( colContext );
	colContext = QColor::enterAllocContext();
	color.setRgb( col.rgb() );
	color.alloc();
	QColor::leaveAllocContext();

	QPainter painter;

	painter.begin( this );
	drawContents( &painter );
	painter.end();
}

void KColorPatch::drawContents( QPainter *painter )
{
	painter->setPen( color );
	painter->setBrush( QBrush( color ) );
	painter->drawRect( contentsRect() );
}

void KColorPatch::mouseMoveEvent( QMouseEvent *e )
{
        // Drag color object
        if( !(e->state() && LeftButton)) return;
	KColorDrag *d = KColorDrag::makeDrag( color, this);
	d->dragCopy();
}

void KColorPatch::dragEnterEvent( QDragEnterEvent *event)
{
     event->accept( KColorDrag::canDecode( event));
}

void KColorPatch::dropEvent( QDropEvent *event)
{
     QColor c;
     if( KColorDrag::decode( event, c)) {
	  setColor( c);
	  emit colorChanged( c);
     }
}



KColorDialog::KColorDialog( QWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("Select Color"), Help|Ok|Cancel,
		Ok, true )
{
  setHelp( "kcolordialog.html", QString::null );
  connect( this, SIGNAL(okClicked(void)),this,SLOT(slotWriteSettings(void)));

  QPushButton *button;
  QLabel *label;
  int h, s, v;

  selColor = darkCyan;
  selColor.hsv( &h, &s, &v );

  //
  // Create the top level page and its layout
  //
  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *tl_layout = new QGridLayout( page, 3, 3, 0, spacingHint() );
  tl_layout->addColSpacing( 1, spacingHint() * 2 );

  //
  // add a layout for left-side (colors)
  //
  QVBoxLayout *l_left = new QVBoxLayout;
  tl_layout->addLayout(l_left, 0, 0);

  //
  // We now use the full 40color system palette (taken from kpixmap.cpp)
  // (mosfet)
  //

  //
  // System colors
  //
  label = new QLabel( i18n("System Colors"), page );
  l_left->addWidget(label, 0, AlignLeft);
  sysColorCells = new KColorCells( page, 5, 8 );
  sysColorCells->setMinimumSize(160, 100);

  sysColorCells->setColor( 0, Qt::red );
  sysColorCells->setColor( 1, Qt::green );
  sysColorCells->setColor( 2, Qt::blue );
  sysColorCells->setColor( 3, Qt::cyan );
  sysColorCells->setColor( 4, Qt::magenta );
  sysColorCells->setColor( 5, Qt::yellow );
  sysColorCells->setColor( 6, Qt::darkRed );
  sysColorCells->setColor( 7, Qt::darkGreen );
  sysColorCells->setColor( 8, Qt::darkBlue );
  sysColorCells->setColor( 9, Qt::darkCyan );
  sysColorCells->setColor( 10, Qt::darkMagenta );
  sysColorCells->setColor( 11, Qt::darkYellow );
  sysColorCells->setColor( 12, Qt::white );
  sysColorCells->setColor( 13, Qt::lightGray );
  sysColorCells->setColor( 14, Qt::gray );
  sysColorCells->setColor( 15, Qt::darkGray );
  sysColorCells->setColor( 16, Qt::black );

  //
  // Pastels
  //
  sysColorCells->setColor( 17, QColor(255, 192, 192 ));
  sysColorCells->setColor( 18, QColor(192, 255, 192 ));
  sysColorCells->setColor( 19, QColor(192, 192, 255 ));
  sysColorCells->setColor( 20, QColor(255, 255, 192 ));
  sysColorCells->setColor( 21, QColor(255, 192, 255 ));
  sysColorCells->setColor( 22, QColor(192, 255, 255 ));

  //
  // Reds
  //
  sysColorCells->setColor( 23, QColor(64,   0,   0 ));
  sysColorCells->setColor( 24, QColor(192,  0,   0 ));

  //
  // Oranges
  //
  sysColorCells->setColor( 25, QColor(255, 128,   0 ));
  sysColorCells->setColor( 26, QColor(192,  88,   0 ));
  sysColorCells->setColor( 27, QColor(255, 168,  88 ));
  sysColorCells->setColor( 28, QColor(255, 220, 168 ));

  //
  // Blues
  //
  sysColorCells->setColor( 29, QColor(0,   0, 192 ));

  //
  // Turquoise
  //
  sysColorCells->setColor( 30, QColor(0,  64,  64 ));
  sysColorCells->setColor( 31, QColor(0, 192, 192 ));

  //
  // Yellows
  //
  sysColorCells->setColor(32, QColor(64,  64, 0 ));
  sysColorCells->setColor(33, QColor(192, 192, 0 ));

  //
  // Greens
  //
  sysColorCells->setColor(34,   QColor(0,  64,   0 ));
  sysColorCells->setColor(35,   QColor(0, 192,   0 ));

  //
  // Purples
  //
  sysColorCells->setColor(36,  QColor(192,   0, 192 ));

  //
  // Greys
  //
  sysColorCells->setColor(37,  QColor(88,  88,  88 ));
  sysColorCells->setColor(38,  QColor(48,  48,  48 ));
  sysColorCells->setColor(39,  QColor(220, 220, 220 ));


  connect( sysColorCells, SIGNAL( colorSelected( int ) ),
	   SLOT( slotSysColorSelected( int ) ) );
  l_left->addWidget(sysColorCells, 10);

  //
  // a little space between
  //
  l_left->addStretch(1);

  //
  // add custom colors
  //
  label = new QLabel( i18n("Custom Colors"), page );
  l_left->addWidget(label, 0, AlignLeft);
  custColorCells = new KColorCells( page, 3, 6 );
  custColorCells->setMinimumSize(90, 60);
  connect( custColorCells, SIGNAL( colorSelected( int ) ),
	   SLOT( slotCustColorSelected( int ) ) );
  l_left->addWidget(custColorCells, 10);

  //
  // a little space between
  //
  l_left->addStretch(1);

  //
  // add buttom for adding colors
  //
  button = new QPushButton( i18n("&Add to Custom Colors"), page );
  l_left->addWidget(button, 0, AlignLeft );
  connect( button, SIGNAL( clicked() ), SLOT( slotAddToCustom() ) );

  //
  // the more complicated part: the right side
  // add a V-box
  //
  QVBoxLayout *l_right = new QVBoxLayout();
  tl_layout->addLayout(l_right, 0, 2);

  //
  // add a H-Box for the XY-Selector and a grid for the
  // entry fields
  //
  QHBoxLayout *l_rtop = new QHBoxLayout();
  l_right->addLayout(l_rtop);
  QGridLayout *l_rbot = new QGridLayout(3, 6);
  l_right->addLayout(l_rbot);

  //
  // the palette and value selector go into the H-box
  //
  palette = new KHSSelector( page );
  palette->setMinimumSize(140, 70);
  l_rtop->addWidget(palette, 8);
  connect( palette, SIGNAL( valueChanged( int, int ) ),
	   SLOT( slotHSChanged( int, int ) ) );
	
  valuePal = new KValueSelector( page );
  valuePal->setHue( h );
  valuePal->setSaturation( s );
  valuePal->setMinimumSize(26, 70);
  l_rtop->addWidget(valuePal, 1);
  connect( valuePal, SIGNAL( valueChanged( int ) ),
	   SLOT( slotVChanged( int ) ) );

  //
  // and now the entry fields and the patch
  //
  patch = new KColorPatch( page );
  l_rbot->addMultiCellWidget(patch, 0, 2, 0, 0, AlignVCenter|AlignLeft);
  patch->setFixedSize(48, 48);
  patch->setColor( selColor );
  connect( patch, SIGNAL( colorChanged( const QColor&)),
	   SLOT( setColor( const QColor&)));
  //
  // add the HSV fields
  //
  label = new QLabel( "H:", page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_rbot->addWidget(label, 0, 2);
  hedit = new QLineEdit( page );
  hedit->setValidator( new QIntValidator( hedit ) );
  l_rbot->addWidget(hedit, 0, 3);
  connect( hedit, SIGNAL( returnPressed() ),SLOT( slotHSVChanged() ) );
	
  label = new QLabel( "S:", page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_rbot->addWidget(label, 1, 2);
  sedit = new QLineEdit( page );
  sedit->setValidator( new QIntValidator( sedit ) );
  l_rbot->addWidget(sedit, 1, 3);
  connect( sedit, SIGNAL( returnPressed() ),SLOT( slotHSVChanged() ) );
	
  label = new QLabel( "V:", page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_rbot->addWidget(label, 2, 2);
  vedit = new QLineEdit( page );
  vedit->setValidator( new QIntValidator( vedit ) );
  l_rbot->addWidget(vedit, 2, 3);
  connect( vedit, SIGNAL( returnPressed() ),SLOT( slotHSVChanged() ) );
	
  //
  // add the RGB fields
  //
  label = new QLabel( "R:", page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_rbot->addWidget(label, 0, 4);
  redit = new QLineEdit( page );
  redit->setValidator( new QIntValidator( redit ) );
  l_rbot->addWidget(redit, 0, 5);
  connect( redit, SIGNAL( returnPressed() ), SLOT( slotRGBChanged() ) );
	
  label = new QLabel( "G:", page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_rbot->addWidget( label, 1, 4);
  gedit = new QLineEdit( page );
  gedit->setValidator( new QIntValidator( gedit ) );
  l_rbot->addWidget(gedit, 1, 5);
  connect( gedit, SIGNAL( returnPressed() ), SLOT( slotRGBChanged() ) );
	
  label = new QLabel( "B:", page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_rbot->addWidget(label, 2, 4);
  bedit = new QLineEdit( page );
  bedit->setValidator( new QIntValidator( bedit ) );
  l_rbot->addWidget(bedit, 2, 5);
  connect( bedit, SIGNAL( returnPressed() ), SLOT( slotRGBChanged() ) );

  //
  // the entry fields should be wide enought to hold 88888
  //
  int w = hedit->fontMetrics().width("888888");
  hedit->setFixedWidth(w);
  sedit->setFixedWidth(w);
  vedit->setFixedWidth(w);

  redit->setFixedWidth(w);
  gedit->setFixedWidth(w);
  bedit->setFixedWidth(w);
	
  tl_layout->activate();
  page->setMinimumSize( page->sizeHint() );

  readSettings();
  setRgbEdit();
  setHsvEdit();

  palette->setValues( h, s );
  valuePal->setValue( v );

  disableResize();
}



void KColorDialog::setColor( const QColor &col )
{
  selColor = col;

  setRgbEdit();
  setHsvEdit();

  int h, s, v;
  selColor.hsv( &h, &s, &v );
  palette->setValues( h, s );
  valuePal->setHue( h );
  valuePal->setSaturation( s );
  valuePal->drawPalette();
  valuePal->repaint( FALSE );
  valuePal->setValue( v );
  patch->setColor( selColor );
}

//
// static function to display dialog and return color
//
int KColorDialog::getColor( QColor &theColor, QWidget *parent )
{
  KColorDialog dlg( parent, "Color Selector", TRUE );
  if ( theColor.isValid() )
    dlg.setColor( theColor );
  int result = dlg.exec();

  if ( result == Accepted )
    theColor = dlg.color();

  return result;
}


void KColorDialog::slotRGBChanged( void )
{
  int red = redit->text().toInt();
  int grn = gedit->text().toInt();
  int blu = bedit->text().toInt();

  if ( red > 255 || red < 0 ) return;
  if ( grn > 255 || grn < 0 ) return;
  if ( blu > 255 || blu < 0 ) return;

  selColor.setRgb( red, grn, blu );
  patch->setColor( selColor );

  setRgbEdit();
  setHsvEdit();

  int h, s, v;
  selColor.hsv( &h, &s, &v );
  palette->setValues( h, s );
  valuePal->setHue( h );
  valuePal->setSaturation( s );
  valuePal->drawPalette();
  valuePal->repaint( FALSE );
  valuePal->setValue( v );

  emit colorSelected( selColor );
}

void KColorDialog::slotHSVChanged( void )
{
  int hue = hedit->text().toInt();
  int sat = sedit->text().toInt();
  int val = vedit->text().toInt();

  if ( hue > 359 || hue < 0 ) return;
  if ( sat > 255 || sat < 0 ) return;
  if ( val > 255 || val < 0 ) return;

  selColor.setHsv( hue, sat, val );
  patch->setColor( selColor );

  setRgbEdit();
  setHsvEdit();

  palette->setValues( hue, sat );
  valuePal->setHue( hue );
  valuePal->setSaturation( sat );
  valuePal->drawPalette();
  valuePal->repaint( FALSE );
  valuePal->setValue( val );

  emit colorSelected( selColor );
}

void KColorDialog::slotHSChanged( int h, int s )
{
  selColor.setHsv( h, s, valuePal->value() );

  valuePal->setHue( h );
  valuePal->setSaturation( s );
  valuePal->drawPalette();
  valuePal->repaint( FALSE );

  patch->setColor( selColor );

  setRgbEdit();
  setHsvEdit();

  emit colorSelected( selColor );
}

void KColorDialog::slotVChanged( int v )
{
  selColor.setHsv( palette->xValue(), palette->yValue(), v );
  patch->setColor( selColor );

  setRgbEdit();
  setHsvEdit();

  emit colorSelected( selColor );
}

void KColorDialog::slotSysColorSelected( int col )
{
  selColor = sysColorCells->color( col );

  patch->setColor( selColor );

  setRgbEdit();
  setHsvEdit();

  int h, s, v;
  selColor.hsv( &h, &s, &v );
  palette->setValues( h, s );
  valuePal->setHue( h );
  valuePal->setSaturation( s );
  valuePal->drawPalette();
  valuePal->repaint( FALSE );
  valuePal->setValue( v );
	
  emit colorSelected( selColor );
}

void KColorDialog::slotCustColorSelected( int col )
{
  QColor color = custColorCells->color( col );

  // if a color has not been assigned to this cell, don't change current col
  if ( !color.isValid() )
    return;

  selColor = color;

  patch->setColor( selColor );

  setRgbEdit();
  setHsvEdit();

  int h, s, v;
  selColor.hsv( &h, &s, &v );
  palette->setValues( h, s );
  valuePal->setHue( h );
  valuePal->setSaturation( s );
  valuePal->drawPalette();
  valuePal->repaint( FALSE );
  valuePal->setValue( v );

  emit colorSelected( selColor );
}

void KColorDialog::slotAddToCustom( void )
{
  custColorCells->setColor( custColorCells->getSelected(), selColor );
}



void KColorDialog::slotWriteSettings( void )
{
  QColor color;
  QString key;

  KConfig* config = KGlobal::config();

  QString oldgroup = config->group();
  config->setGroup( "Custom Colors");

  for ( int i = 0; i < custColorCells->numCells(); i++ )
  {
    color = custColorCells->color( i );
    key = QString( "Color%1").arg( i );
    if (color.isValid())
       config->writeEntry( key, color, true, true );
    else
       config->writeEntry( key, "", true, true );
  }

  config->setGroup( oldgroup );
}

void KColorDialog::readSettings( void )
{
  QColor col;
  QString key;

  KConfig* config = KGlobal::config();

  QString oldgroup = config->group();
  config->setGroup( "Custom Colors");

  for ( int i = 0; i < custColorCells->numCells(); i++ )
  {
    key = QString( "Color%1").arg( i );
    col = config->readColorEntry( key );
    custColorCells->setColor( i, col );
  }
  config->setGroup( oldgroup );
}


void KColorDialog::setRgbEdit( void )
{
  int r, g, b;
  selColor.rgb( &r, &g, &b );
  QString num;

  num.setNum( r );
  redit->setText( num );
  num.setNum( g );
  gedit->setText( num );
  num.setNum( b );
  bedit->setText( num );
}


void KColorDialog::setHsvEdit( void )
{
  int h, s, v;
  selColor.hsv( &h, &s, &v );
  QString num;

  num.setNum( h );
  hedit->setText( num );
  num.setNum( s );
  sedit->setText( num );
  num.setNum( v );
  vedit->setText( num );
}

//----------------------------------------------------------------------------

KColorCombo::KColorCombo( QWidget *parent, const char *name )
	: QComboBox( parent, name )
{
	customColor.setRgb( 255, 255, 255 );
	color.setRgb( 255, 255, 255 );

	createStandardPalette();

	addColors();

	connect( this, SIGNAL( activated(int) ), SLOT( slotActivated(int) ) );
	connect( this, SIGNAL( highlighted(int) ), SLOT( slotHighlighted(int) ) );
}

void KColorCombo::setColor( const QColor &col )
{
	color = col;

	addColors();
}

void KColorCombo::resizeEvent( QResizeEvent *re )
{
	QComboBox::resizeEvent( re );

	addColors();
}

void KColorCombo::slotActivated( int index )
{
	if ( index == 0 )
	{
	    if ( KColorDialog::getColor( customColor ) == QDialog::Accepted )
		{
			QRect rect( 0, 0, width(), 20 );
			QPixmap pixmap( rect.width(), rect.height() );
			QPainter painter;
			QPen pen;

			if ( qGray( customColor.rgb() ) < 128 )
				pen.setColor( white );
			else
				pen.setColor( black );

			painter.begin( &pixmap );
			QBrush brush( customColor );
			painter.fillRect( rect, brush );
			painter.setPen( pen );
			painter.drawText( 2, 18,
					  i18n("Custom...") );
			painter.end();

			changeItem( pixmap, 0 );
			pixmap.detach();
		}

		color = customColor;
	}
	else
		color = standardPalette[ index - 1 ];

	emit activated( color );
}

void KColorCombo::slotHighlighted( int index )
{
	if ( index == 0 )
		color = customColor;
	else
		color = standardPalette[ index - 1 ];

	emit highlighted( color );
}

void KColorCombo::addColors()
{
	QRect rect( 0, 0, width(), 20 );
	QPixmap pixmap( rect.width(), rect.height() );
	QPainter painter;
	QPen pen;
	int i;

	clear();

	createStandardPalette();

	for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
		if ( standardPalette[i] == color ) break;

	if ( i == STANDARD_PAL_SIZE )
		customColor = color;

	if ( qGray( customColor.rgb() ) < 128 )
		pen.setColor( white );
	else
		pen.setColor( black );

	painter.begin( &pixmap );
	QBrush brush( customColor );
	painter.fillRect( rect, brush );
	painter.setPen( pen );
	painter.drawText( 2, 18, i18n("Custom...") );
	painter.end();

	insertItem( pixmap );
	pixmap.detach();
	
	for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
	{
		painter.begin( &pixmap );
		QBrush brush( standardPalette[i] );
		painter.fillRect( rect, brush );
		painter.end();

		insertItem( pixmap );
		pixmap.detach();

		if ( standardPalette[i] == color )
			setCurrentItem( i + 1 );
	}
}
#include "kcolordlg.moc"

