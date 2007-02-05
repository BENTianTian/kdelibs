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
//-----------------------------------------------------------------------------
// KDE color selection dialog.
//
// 1999-09-27 Espen Sand <espensa@online.no>
// KColorDialog is now subclassed from KDialog. I have also extended
// KColorDialog::getColor() so that in contains a parent argument. This
// improves centering capability.
//
// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>
//


#include "kcolorcombo.h"

#include <stdio.h>
#include <stdlib.h>

#include <qdrawutil.h>
#include <qevent.h>
#include <qfile.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kpalette.h>
#include <kimageeffect.h>

#include "kcolordialog.h"
//#include "kcolordrag.h"

// This is repeated from the KColorDlg, but I didn't
// want to make it public BL.
// We define it out when compiling with --enable-final in which case
// we use the version defined in KColorDlg

#ifndef KDE_USE_FINAL
#define STANDARD_PAL_SIZE 17

static QColor *standardPalette = 0;

static void createStandardPalette()
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
#endif

class KColorCombo::KColorComboPrivate
{
	protected:
	friend class KColorCombo;
  
	KColorComboPrivate(KColorCombo *q){}
	~KColorComboPrivate(){}
        
        KColorCombo *q;
	QColor customColor;
	QColor internalcolor;
	bool showEmptyList;
};

KColorCombo::KColorCombo( QWidget *parent )
	: QComboBox( parent )
{
	d=new KColorComboPrivate(this);
	d->showEmptyList=false;

	d->customColor.setRgb( 255, 255, 255 );
	d->internalcolor.setRgb( 255, 255, 255 );

	createStandardPalette();

	addColors();

	connect( this, SIGNAL( activated(int) ), SLOT( slotActivated(int) ) );
	connect( this, SIGNAL( highlighted(int) ), SLOT( slotHighlighted(int) ) );
}


KColorCombo::~KColorCombo()
{
	delete d;
}
/**
   Sets the current color
 */
void KColorCombo::setColor( const QColor &col )
{
	d->internalcolor = col;
	d->showEmptyList=false;
	addColors();
}


/**
   Returns the currently selected color
 */
QColor KColorCombo::color() const {
  return d->internalcolor;
}

void KColorCombo::resizeEvent( QResizeEvent *re )
{
	QComboBox::resizeEvent( re );

	addColors();
}

/**
   Show an empty list, till the next color is set with setColor
 */
void KColorCombo::showEmptyList()
{
	d->showEmptyList=true;
	addColors();
}

void KColorCombo::slotActivated( int index )
{
	if ( index == 0 )
	{
	    if ( KColorDialog::getColor( d->customColor, this ) == QDialog::Accepted )
		{
			QPainter painter;
			QPen pen;
			QRect rect( 0, 0, width(), QFontMetrics(painter.font()).height()+4);
			QPixmap pixmap( rect.width(), rect.height() );

			if ( qGray( d->customColor.rgb() ) < 128 )
				pen.setColor( Qt::white );
			else
				pen.setColor( Qt::black );

			painter.begin( &pixmap );
			QBrush brush( d->customColor );
			painter.fillRect( rect, brush );
			painter.setPen( pen );
			painter.drawText( 2, QFontMetrics(painter.font()).ascent()+2, i18n("Custom...") );
			painter.end();

			setItemIcon( 0, QIcon(pixmap) );
			pixmap.detach();
		}

		d->internalcolor = d->customColor;
	}
	else
		d->internalcolor = standardPalette[ index - 1 ];

	emit activated( d->internalcolor );
}

void KColorCombo::slotHighlighted( int index )
{
	if ( index == 0 )
		d->internalcolor = d->customColor;
	else
		d->internalcolor = standardPalette[ index - 1 ];

	emit highlighted( d->internalcolor );
}

void KColorCombo::addColors()
{
	QPainter painter;
	QPen pen;
	QRect rect( 0, 0, width(), QFontMetrics(painter.font()).height()+4 );
	QPixmap pixmap( rect.width(), rect.height() );
	int i;

	clear();
	if (d->showEmptyList) return;

	createStandardPalette();

	for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
		if ( standardPalette[i] == d->internalcolor ) break;

	if ( i == STANDARD_PAL_SIZE )
		d->customColor = d->internalcolor;

	if ( qGray( d->customColor.rgb() ) < 128 )
		pen.setColor( Qt::white );
	else
		pen.setColor( Qt::black );

	painter.begin( &pixmap );
	QBrush brush( d->customColor );
	painter.fillRect( rect, brush );
	painter.setPen( pen );
	painter.drawText( 2, QFontMetrics(painter.font()).ascent()+2, i18n("Custom...") );
	painter.end();

	addItem( QIcon(pixmap), QString() );
	pixmap.detach();

	for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
	{
		painter.begin( &pixmap );
		QBrush brush( standardPalette[i] );
		painter.fillRect( rect, brush );
		painter.end();

		addItem( QIcon(pixmap), QString() );
		pixmap.detach();

		if ( standardPalette[i] == d->internalcolor )
			setCurrentIndex( i + 1 );
	}
}


#include "kcolorcombo.moc"
