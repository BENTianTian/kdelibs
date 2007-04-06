/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "imagepreview.h"

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPaintDevice>

// forward definition
QImage convertImage(const QImage& image, int hue, int saturation, int brightness, int gamma);

ImagePreview::ImagePreview(QWidget *parent ) : QWidget(parent ) {
	brightness_ = 100;
	hue_ = 0;
	saturation_ = 100;
	gamma_ = 1000;
	bw_ = false;

	setAttribute(Qt::WA_NoSystemBackground);
}

ImagePreview::~ImagePreview(){
}

void ImagePreview::setImage(const QImage& image){
	image_ = image.convertToFormat(QImage::Format_RGB32);
	image_.detach();
	resize(image_.size());
	update();
}

void ImagePreview::setParameters(int brightness, int hue, int saturation, int gamma){
	brightness_ = brightness;
	hue_ = hue;
	saturation_ = saturation;
	gamma_ = gamma;
	repaint();
}

void ImagePreview::paintEvent(QPaintEvent*){
	QImage	tmpImage = convertImage(image_,hue_,(bw_ ? 0 : saturation_),brightness_,gamma_);
	int	x = (width()-tmpImage.width())/2, y = (height()-tmpImage.height())/2;

	QPixmap	buffer(width(), height());
	buffer.fill(parentWidget(), 0, 0);
	QPainter	p(&buffer);
	p.drawImage(x,y,tmpImage);
	p.end();

  QPainter painter( this );
  painter.drawImage( QPoint( 0, 0 ), buffer.toImage() );
}

void ImagePreview::setBlackAndWhite(bool on){
	bw_ = on;
	update();
}

QSize ImagePreview::minimumSizeHint() const
{
	return image_.size();
}
