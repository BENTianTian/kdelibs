/* This file is part of the KDE project
   Copyright (C) 2002 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "pcx.h"

#include <qimage.h>

#include <kdebug.h>

QDataStream &operator>>( QDataStream &s, struct RGB &rgb )
{
  s >> rgb.r >> rgb.g >> rgb.b;

  return s;
}

QDataStream &operator>>( QDataStream &s, Palette &pal )
{
  for ( int i=0; i<16; ++i )
    s >> pal.rgb[ i ];

  return s;
}

QDataStream &operator>>( QDataStream &s, PCXHEADER &ph )
{
  s >> ph.Manufacturer;
  s >> ph.Version;
  s >> ph.Encoding;
  s >> ph.Bpp;
  s >> ph.XMin >> ph.YMin >> ph.XMax >> ph.YMax;
  s >> ph.HDpi >> ph.YDpi;
  s >> ph.ColorMap;
  s >> ph.Reserved;
  s >> ph.NPlanes;
  s >> ph.BytesPerLine;
  s >> ph.PaletteInfo;
  s >> ph.HScreenSize;
  s >> ph.VScreenSize;

  return s;
}

QDataStream &operator<<( QDataStream &s, struct RGB &rgb )
{

  s << rgb.r << rgb.g << rgb.b;

  return s;
}

QDataStream &operator<<( QDataStream &s, Palette &pal )
{
  for ( int i=0; i<16; ++i )
    s << pal.rgb[ i ];

  return s;
}

QDataStream &operator<<( QDataStream &s, PCXHEADER &ph )
{
  s << ph.Manufacturer;
  s << ph.Version;
  s << ph.Encoding;
  s << ph.Bpp;
  s << ph.XMin << ph.YMin << ph.XMax << ph.YMax;
  s << ph.HDpi << ph.YDpi;
  s << ph.ColorMap;
  s << ph.Reserved;
  s << ph.NPlanes;
  s << ph.BytesPerLine;
  s << ph.PaletteInfo;
  s << ph.HScreenSize;
  s << ph.VScreenSize;

  Q_UINT8 byte = 0;
  for ( int i=0; i<54; ++i )
    s << byte;

  return s;
}

static PCXHEADER header;
static QImage img;
Q_UINT16 w, h;

static void readLine( QDataStream &s, QByteArray &buf )
{
  Q_UINT32 i=0;
  Q_UINT32 size = buf.size();
  Q_UINT8 byte, count;

  if ( header.Encoding == 1 )
  {
    // Uncompress the image data
    while ( i < size )
    {
      count = 1;
      s >> byte;
      if ( byte > 0xc0 )
      {
        count = byte - 0xc0;
        s >> byte;
      }
      while ( count-- && i < size )
        buf[ i++ ] = byte;
    }
  }
  else
  {
    // Image is not compressed (possible?)
    while ( i < size )
    {
      s >> byte;
      buf[ i++ ] = byte;
    }
  }
}

static void readImage1( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine );

  img.create( w, h, 1, 2, QImage::BigEndian );

  for ( int y=0; y<h; ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, buf );

    for ( int x=0; x<header.BytesPerLine; ++x )
      *( img.scanLine( y )+x ) = buf[ x ];
  }

  // Set the color palette
  img.setColor( 0, qRgb( 0, 0, 0 ) );
  img.setColor( 1, qRgb( 255, 255, 255 ) );
}

static void readImage4( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine*4 );
  QByteArray pixbuf( w );

  img.create( w, h, 8, 16, QImage::IgnoreEndian );

  for ( int y=0; y<h; ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    pixbuf.fill( 0 );
    readLine( s, buf );

    for ( int p=0; p<4; p++ )
    {
      Q_UINT32 offset = p*header.BytesPerLine;
      for ( int x=0; x<w; ++x )
        if ( buf[ offset + ( x/8 ) ] & ( 128 >> ( x%8 ) ) )
          pixbuf[ x ] += ( 1 << p );
    }

    for ( int x=0; x<w; ++x )
      *( img.scanLine( y )+x ) = pixbuf[ x ];
  }

  // Read the palette
  for ( int i=0; i<16; ++i )
    img.setColor( i, header.ColorMap.color( i ) );
}

static void readImage8( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine );

  img.create( w, h, 8, 256, QImage::IgnoreEndian );

  for ( int y=0; y<h; ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, buf );

    for ( int x=0; x<header.BytesPerLine; ++x )
      *( img.scanLine( y )+x ) = buf[ x ];
  }

  Q_UINT8 flag;
  s >> flag;
  kdDebug() << "Flag: " << flag << endl;

  if ( flag == 12 && header.Version == 5 )
  {
    // Read the palette
    Q_UINT8 r, g, b;
    for ( int i=0; i<256; ++i )
    {
      s >> r >> g >> b;
      img.setColor( i, qRgb( r, g, b ) );
    }
  }
}

static void readImage24( QDataStream &s )
{
  QByteArray r_buf( header.BytesPerLine );
  QByteArray g_buf( header.BytesPerLine );
  QByteArray b_buf( header.BytesPerLine );

  img.create( w, h, 32 );

  for ( int y=0; y<h; ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, r_buf );
    readLine( s, g_buf );
    readLine( s, b_buf );

    for ( int x=0; x<header.BytesPerLine; ++x )
    {
      uint *p = ( uint * )img.scanLine( y )+x;
      *p = qRgb( r_buf[ x ], g_buf[ x ], b_buf[ x ] );
    }
  }
}

void kimgio_pcx_read( QImageIO *io )
{
  QDataStream s( io->ioDevice() );
  s.setByteOrder( QDataStream::LittleEndian );

  if ( s.device()->size() < 128 )
  {
    io->setStatus( -1 );
    return;
  }

  s >> header;

  if ( header.Manufacturer != 10 || s.atEnd())
  {
    io->setStatus( -1 );
    return;
  }

  w = ( header.XMax-header.XMin ) + 1;
  h = ( header.YMax-header.YMin ) + 1;

  kdDebug() << "Manufacturer: " << header.Manufacturer << endl;
  kdDebug() << "Version: " << header.Version << endl;
  kdDebug() << "Encoding: " << header.Encoding << endl;
  kdDebug() << "Bpp: " << header.Bpp << endl;
  kdDebug() << "Width: " << w << endl;
  kdDebug() << "Height: " << h << endl;
  kdDebug() << "Window: " << header.XMin << "," << header.XMax << "," 
            << header.YMin << "," << header.YMax << endl;
  kdDebug() << "BytesPerLine: " << header.BytesPerLine << endl;
  kdDebug() << "NPlanes: " << header.NPlanes << endl;

  // Skip the rest of the header
  Q_UINT8 byte;
  while ( s.device()->at() < 128 )
    s >> byte;

  if ( header.Bpp == 1 && header.NPlanes == 1 )
  {
    readImage1( s );
  }
  else if ( header.Bpp == 1 && header.NPlanes == 4 )
  {
    readImage4( s );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 1 )
  {
    readImage8( s );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 3 )
  {
    readImage24( s );
  }

  kdDebug() << "Image Bytes: " << img.numBytes() << endl;
  kdDebug() << "Image Bytes Per Line: " << img.bytesPerLine() << endl;
  kdDebug() << "Image Depth: " << img.depth() << endl;

  if ( !img.isNull() )
  {
    io->setImage( img );
    io->setStatus( 0 );
  }
  else
  {
    io->setStatus( -1 );
  }
}

static void writeLine( QDataStream &s, QByteArray &buf )
{
  Q_UINT32 i = 0;
  Q_UINT32 size = buf.size();
  Q_UINT8 count, data;
  char byte;

  while ( i < size )
  {
    count = 1;
    byte = buf[ i++ ];

    while ( ( i < size ) && ( byte == buf[ i ] ) && ( count < 63 ) )
    {
      ++i;
      ++count;
    }

    data = byte;

    if ( count > 1 || data >= 0xc0 )
    {
      count |= 0xc0;
      s << count;
    }

    s << data;
  }
}

static void writeImage1( QDataStream &s )
{
  img = img.convertBitOrder( QImage::BigEndian );

  header.Bpp = 1;
  header.NPlanes = 1;

  header.ColorMap.setColor( 0, qRgb( 0, 0, 0 ) );
  header.ColorMap.setColor( 1, qRgb( 255, 255, 255 ) );

  s << header;

  QByteArray buf( header.BytesPerLine );

  for ( int y=0; y<h; ++y )
  {
    Q_UINT8 *p = img.scanLine( y );

    for ( int i=0; i<header.BytesPerLine; ++i )
      buf[ i ] = p[ i ];

    writeLine( s, buf );
  }
}

void kimgio_pcx_write( QImageIO *io )
{
  QDataStream s( io->ioDevice() );
  s.setByteOrder( QDataStream::LittleEndian );

  img = io->image();

  w = img.width();
  h = img.height();

  struct PCXHEADER header;

  // Initialize header with zeros
  QByteArray dummy( 128 );
  dummy.fill( 0 );
  QDataStream ds( dummy, IO_ReadOnly );
  ds >> header;

  header.Manufacturer = 10;
  header.Version = 5;
  header.Encoding = 1;
  header.XMin = 0;
  header.YMin = 0;
  header.XMax = w-1;
  header.YMax = h-1;
  header.HDpi = 300;
  header.YDpi = 300;
  header.Reserved = 0;
  header.BytesPerLine = img.bytesPerLine();
  header.PaletteInfo =1;

  if ( img.depth() == 1 )
  {
    writeImage1( s );
  }

  kdDebug() << "Manufacturer: " << header.Manufacturer << endl;
  kdDebug() << "Version: " << header.Version << endl;
  kdDebug() << "Encoding: " << header.Encoding << endl;
  kdDebug() << "Bpp: " << header.Bpp << endl;
  kdDebug() << "Width: " << w << endl;
  kdDebug() << "Height: " << h << endl;
  kdDebug() << "Window: " << header.XMin << "," << header.XMax << "," 
            << header.YMin << "," << header.YMax << endl;
  kdDebug() << "BytesPerLine: " << header.BytesPerLine << endl;
  kdDebug() << "NPlanes: " << header.NPlanes << endl;

  io->setStatus( 0 );
}

/* vim: et sw=2 ts=2
*/

