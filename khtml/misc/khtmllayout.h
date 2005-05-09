/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Lars Knoll (knoll@kde.org)

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

    This widget holds some useful definitions needed for layouting Elements
*/
#ifndef HTML_LAYOUT_H
#define HTML_LAYOUT_H


/*
 * this namespace contains definitions for various types needed for
 * layouting.
 */
namespace khtml
{

    const int UNDEFINED = -1;

    // alignment
    enum VAlign { VNone=0, Qt::DockBottom, VCenter, Qt::DockTop, Baseline };
    enum HAlign { HDefault, Qt::DockLeft, HCenter, Qt::DockRight, HNone = 0 };

    /*
     * %multiLength and %Length
     */
    enum LengthType { Variable = 0, Relative, Percent, Fixed, Static };
    struct Length
    {
	Length() : _length(0) {}
        Length(LengthType t) { _length = 0; l.type = t; }
        Length(int v, LengthType t, bool q=false)
        {  _length= 0; l.value = v; l.type = t; l.quirk = q; }
        bool operator==(const Length& o) const
        { return _length == o._length; }
        bool operator!=(const Length& o) const
        { return _length != o._length; }

	/*
	 * works only for Fixed and Percent, returns -1 otherwise
	 */
	int width(int maxWidth) const
	    {
		switch(l.type)
		{
		case Fixed:
		    return l.value;
		case Percent:
		    return maxWidth*l.value/100;
		case Variable:
		    return maxWidth;
		default:
		    return -1;
		}
	    }
	/*
	 * returns the minimum width value which could work...
	 */
	int minWidth(int maxWidth) const
	    {
		switch(l.type)
		{
		case Fixed:
		    return l.value;
		case Percent:
		    return maxWidth*l.value/100;
		case Variable:
		default:
		    return 0;
		}
	    }
        bool isVariable() const { return ((LengthType) l.type == Variable); }
        bool isRelative() const { return ((LengthType) l.type == Relative); }
        bool isPercent() const { return ((LengthType ) l.type == Percent); }
        bool isFixed() const { return ((LengthType) l.type == Fixed); }
        bool isStatic() const { return ((LengthType) l.type == Static); }
        bool isQuirk() const { return l.quirk; }

        int value() const { return l.value; }
        LengthType type() const { return (LengthType) l.type; }

        union {
            struct {
                signed int value : 28;
                unsigned type : 3;
                bool quirk : 1;
            } l;
            Q_UINT32 _length;
        };
    };

}

#endif
