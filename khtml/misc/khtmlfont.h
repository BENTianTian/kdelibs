/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Fonts
// $Id$

#ifndef __HTMLFONT_H__
#define __HTMLFONT_H__

#include <qlist.h>
#include <qcolor.h>
#include <qfont.h>
#include <kcharsets.h>
#include <kglobal.h>

// Also defined in khtmldata.h
#ifndef MAXFONTSIZES
#define MAXFONTSIZES 7
#endif

/**
 * @internal
 * This class represents the fonr used during html rendering.
 */
class HTMLFont
{
public:
	HTMLFont( QString _family, int _size, const int fontSizes[MAXFONTSIZES],
	          int _weight=QFont::Normal, bool _italic=FALSE, QFont::CharSet charset=QFont::Latin1 );
	HTMLFont( const HTMLFont &f );

	void setWeight( int w )
		{	font.setWeight( w ); }
	void setItalic( bool u )
		{	font.setItalic( u ); }
	void setUnderline( bool u )
		{	font.setUnderline( u ); }
	void setStrikeOut( bool s )
		{	font.setStrikeOut( s ); }
	void setTextColor( const QColor &col )
		{	textCol = col; }
	void setCharset( QFont::CharSet ch )
	    {	chset=ch;  dirty = true; }
	void setVOffset( int _vOffset )
	    {   VOffset = _vOffset; } 

	QString family() const
		{	return font.family(); }
	const int  weight() const
		{	return font.weight(); }
	const bool italic() const
		{	return font.italic(); }
	const bool underline() const
		{	return font.underline(); }
	const bool strikeOut() const
		{	return font.strikeOut(); }
	const int  pointSize() const
		{	return pointsize; }
	const QColor &textColor() const
		{	return textCol; }
	const QFont::CharSet charset () const
		{	return chset; }
        const int vOffset() const
		{       return VOffset; }
        
	const int size () const
		{	return fsize; }

	const HTMLFont &operator=( const HTMLFont &f );
	bool operator==( const HTMLFont &f );
	operator QFont() const {
	    if(dirty)
	    {
		// I hate hacks like this...
		KGlobal::charsets()->setQFont((QFont &)font, chset);
		((HTMLFont *)this)->dirty = false;
	    }
	    return font; 
	}

private:
	QFont  font;
	QColor textCol;
	QFont::CharSet chset;
	int    fsize;
	int    pointsize;
	bool dirty;
	int    VOffset;
};

inline HTMLFont::HTMLFont( const HTMLFont& f ) : font( f.font )
{
	textCol = f.textCol;
	fsize = f.fsize;
	chset = f.chset;
	pointsize = f.pointsize;
	VOffset = f.VOffset;
}

inline const HTMLFont& HTMLFont::operator=( const HTMLFont& f )
{
	font = f.font;
	textCol = f.textCol;
	fsize = f.fsize;
	chset = f.chset;
	pointsize = f.pointsize;
	VOffset = f.VOffset;

	return *this;
}

inline bool HTMLFont::operator==( const HTMLFont& f )
{
	return ( font.family() == f.font.family() &&
		font.weight() == f.font.weight() &&
		font.italic() == f.font.italic() &&
		font.underline() == f.font.underline() &&
		font.strikeOut() == f.font.strikeOut() &&
		textCol.red() == f.textCol.red() &&
		textCol.green() == f.textCol.green() &&
		textCol.blue() == f.textCol.blue() &&
		fsize == f.fsize &&
		chset == f.chset &&
		VOffset == f.VOffset &&
		pointsize == f.pointsize);
}

//-----------------------------------------------------------------------------

/**
 * @internal
 * manages a list of already loaded @ref HTMLFont's.
 */
class HTMLFontManager
{
public:
	HTMLFontManager();

	const HTMLFont *getFont( const HTMLFont& f );

private:
	QList<HTMLFont> list;
};

extern HTMLFontManager* pFontManager;

//-----------------------------------------------------------------------------

#endif	// __HTMLFONT_H__

