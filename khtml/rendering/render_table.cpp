/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
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

//#define TABLE_DEBUG
//#define DEBUG_LAYOUT
//#define BOX_DEBUG

#include "render_table.h"

#include <qlist.h>
#include <qstack.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qdrawutil.h>

#include "dom_string.h"

#include "misc/helper.h"
#include "misc/khtmllayout.h"

#include <kdebug.h>
#include <assert.h>

using namespace khtml;

template class QArray<LengthType>;

#define FOR_EACH_CELL(r,c,cell) \
    for ( unsigned int r = 0; r < totalRows; r++ )                    \
    {                                                                 \
        for ( unsigned int c = 0; c < totalCols; c++ )                \
        {                                                             \
            RenderTableCell *cell = cells[r][c];             \
            if (!cell)                                                \
                continue;                                             \
	    if ( (c < totalCols - 1) && (cell == cells[r][c+1]) )     \
	        continue;                                             \
	    if ( (r < totalRows - 1) && (cells[r+1][c] == cell) )     \
	        continue;

#define END_FOR_EACH } }


RenderTable::RenderTable()
    : RenderBox()
{

    tCaption = 0;
    _oldColElem = 0;
    head = 0;
    foot = 0;
    firstBody = 0;

    _htmlBorder = 0;

    incremental = false;
    m_maxWidth = 0;


    rules = None;
    frame = Void;

    row = 0;
    col = 0;

    maxColSpan = 0;
    totalColInfos = 0;

    colInfos.setAutoDelete(true);

    setParsing();

    _currentCol=0;

    _lastParentWidth = 0;

    columnPos.resize( 2 );
    colMaxWidth.resize( 1 );
    colMinWidth.resize( 1 );
    colValue.resize(1);
    colType.resize(1);
    actColWidth.resize(1);
    columnPos.fill( 0 );
    colMaxWidth.fill( 0 );
    colMinWidth.fill( 0 );
    colValue.fill(0);
    colType.fill(Variable);
    actColWidth.fill(0);

    columnPos[0] = spacing;

    totalCols = 0;   // this should be expanded to the maximum number of cols
                     // by the first row parsed
    totalRows = 1;
    allocRows = 5;   // allocate five rows initially

    cells = new RenderTableCell ** [allocRows];

    for ( unsigned int r = 0; r < allocRows; r++ )
    {
	cells[r] = new RenderTableCell * [totalCols];
	memset( cells[r], 0, totalCols * sizeof( RenderTableCell * ));
    }
}

RenderTable::~RenderTable()
{
    for ( unsigned int r = 0; r < allocRows; r++ )
	delete [] cells[r];
    delete [] cells;
}

void RenderTable::setStyle(RenderStyle *style)
{
    RenderBox::setStyle(style);
    spacing = style->borderSpacing();
    collapseBorders = style->borderCollapse();
}

bool RenderTable::isInline() const
{
    if (style()->display()==INLINE_TABLE)
    	return true;
    return false;
}


inline void
RenderTable::ColInfo::update()
{
    if (minCell)
        min = minCell->minWidth();
    if (maxCell)
        max = maxCell->maxWidth();
}

void RenderTable::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table)::addChild( " << child->renderName() << ", " <<
                       beforeChild ? beforeChild->renderName() : 0 << " )" << endl;
#endif

    switch(child->style()->display())
    {
    case TABLE_CAPTION:
	//setCaption(static_cast<RenderTableCaption *>(child));
	break;
    case TABLE_COLUMN:
    case TABLE_COLUMN_GROUP:
    	{
	RenderObject::addChild(child,beforeChild);
	RenderTableCol* colel = static_cast<RenderTableCol *>(child);
	if (_oldColElem)
	    _currentCol = _oldColElem->lastCol();
	_oldColElem = colel;
	colel->setStartCol(_currentCol);
	if (child->style()->display() == TABLE_COLUMN)
	    _currentCol++;
	else
	    _currentCol+=colel->span();
	addColInfo(colel);	
	incremental = true;
	colel->setTable(this);
	}
	return;
    case TABLE_HEADER_GROUP:
	if(incremental && !columnPos[totalCols]);// calcColWidth();
//	setTHead(static_cast<RenderTableSection *>(child));
	break;
    case TABLE_FOOTER_GROUP:
	if(incremental && !columnPos[totalCols]);// calcColWidth();
//	setTFoot(static_cast<RenderTableSection *>(child));
	break;
    case TABLE_ROW_GROUP:
	if(incremental && !columnPos[totalCols]);// calcColWidth();
	if(!firstBody)
	    firstBody = static_cast<RenderTableSection *>(child);
    default:	
	break;
    }
    RenderObject::addChild(child,beforeChild);
    child->setTable(this);
}

void RenderTable::startRow()
{
    while ( col < totalCols && cells[row][col] != 0 )
	col++;
    if ( col )
	row++;
    col = 0;
    if(row > totalRows) totalRows = row;
}

void RenderTable::addCell( RenderTableCell *cell )
{
    while ( col < totalCols && cells[row][col] != 0L )
	col++;
    setCells( row, col, cell );

    setMinMaxKnown(false);
    setLayouted(false);

    col++;
}

void RenderTable::setCells( unsigned int r, unsigned int c,
				     RenderTableCell *cell )
{
#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "span = " << cell->rowSpan() << "d/" << cell->colSpan() << "d" << endl;
#endif
    cell->setRow(r);
    cell->setCol(c);

    unsigned int endRow = r + cell->rowSpan();
    unsigned int endCol = c + cell->colSpan();

    // The first row sets the number of columns.  Do not allow subsequent
    // rows to change the number of columns.
//WABA: Why not? Let's give crappy HTML a chance
//    if ( row != 0 && endCol > totalCols )
//	endCol = totalCols;

    if ( endCol > totalCols )
	addColumns( endCol - totalCols );

    if ( endRow >= allocRows )
	addRows( endRow - allocRows + 10 );

    if ( endRow > totalRows )
	totalRows = endRow;

    for ( ; r < endRow; r++ )
    {
	for ( unsigned int tc = c; tc < endCol; tc++ )
	{
	    cells[r][tc] = cell;
	}
    }
}

void RenderTable::addRows( int num )
{
    RenderTableCell ***newRows =
	new RenderTableCell ** [allocRows + num];
    memcpy( newRows, cells, allocRows * sizeof(RenderTableCell **) );
    delete [] cells;
    cells = newRows;

    for ( unsigned int r = allocRows; r < allocRows + num; r++ )
    {
	cells[r] = new RenderTableCell * [totalCols];
	memset( cells[r], 0, totalCols * sizeof( RenderTableCell * ));
    }

    allocRows += num;
}

void RenderTable::addColumns( int num )
{
#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "addColumns() totalCols=" << totalCols << " new=" << num << endl;
#endif
    RenderTableCell **newCells;

    int newCols = totalCols + num;
    // resize the col structs to the number of columns
    columnPos.resize(newCols+1);
    memset( columnPos.data() + totalCols + 1, 0, num*sizeof(int));
    colMaxWidth.resize(newCols);
    memset( colMaxWidth.data() + totalCols , 0, num*sizeof(int));
    colMinWidth.resize(newCols);
    memset( colMinWidth.data() + totalCols , 0, num*sizeof(int));
    colValue.resize(newCols);
    memset( colValue.data() + totalCols , 0, num*sizeof(int));
    colType.resize(newCols);
    memset( colType.data() + totalCols , 0, num*sizeof(LengthType));
    actColWidth.resize(newCols);
    memset( actColWidth.data() + totalCols , 0, num*sizeof(LengthType));

    for ( unsigned int r = 0; r < allocRows; r++ )
    {
	newCells = new RenderTableCell * [newCols];
	memcpy( newCells, cells[r],
		totalCols * sizeof( RenderTableCell * ) );
	memset( newCells + totalCols, 0,
		num * sizeof( RenderTableCell * ) );
	delete [] cells[r];
	cells[r] = newCells;
    }

    int mSpan = newCols;

    colInfos.resize(mSpan);

    percentTotals.resize(mSpan);
    memset( percentTotals.data() + totalCols , 0, num*sizeof(int));


    for ( unsigned int c =0 ; c < totalCols; c++ )
    {
    	colInfos[c]->resize(newCols);
    }
    for ( unsigned int c = totalCols; (int)c < newCols; c++ )
    {
    	colInfos.insert(c, new ColInfoLine(newCols-c+1));
    }

    totalCols = newCols;

}


void RenderTable::addColInfo(RenderTableCol *colel)
{
	
    int _startCol = colel->col();
    int span = colel->span();
    int _minSize=0;
    int _maxSize=0;
    Length _width = colel->width();

    for (int n=0; n<span; ++n)
    {
#ifdef TABLE_DEBUG
	kdDebug( 6040 ) << "COL" << endl;
	kdDebug( 6040 ) << "    startCol=" << _startCol << " span=" << span << endl;
	kdDebug( 6040 ) << "    min=" << _minSize << " max=" << _maxSize << " val=" << _width.value << endl;
#endif
        addColInfo(_startCol+n, 1 , _minSize, _maxSize, _width ,0);
    }

}

void RenderTable::addColInfo(RenderTableCell *cell)
{

    int _startCol = cell->col();
    int _colSpan = cell->colSpan();
    int _minSize = cell->minWidth();
    int _maxSize = cell->maxWidth();

    if (collapseBorders)
    {
    	int bw = cell->borderLeft() + cell->borderRight();
    	_minSize -= bw;
	_maxSize -= bw;
    }

    Length _width = cell->style()->width();
    addColInfo(_startCol, _colSpan, _minSize, _maxSize, _width ,cell);
}

void RenderTable::addColInfo(int _startCol, int _colSpan,
				      int _minSize, int _maxSize,
				      Length _width, RenderTableCell* _cell)
{
#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "addColInfo():" << endl;
    kdDebug( 6040 ) << "    startCol=" << _startCol << " span=" << _colSpan << endl;
    kdDebug( 6040 ) << "    min=" << _minSize << " max=" << _maxSize << endl;
    kdDebug( 6040 ) << "    totalCols=" << totalCols << endl;
#endif

    if (_startCol + _colSpan > (int) totalCols)
	addColumns(totalCols - _startCol + _colSpan);

    ColInfo* col = colInfos[_colSpan-1]->at(_startCol);
    if (!col)
    {
    	col = new ColInfo;
	col->span = _colSpan;
    	col->start = _startCol;
	col->minCell = _cell;
	col->maxCell = _cell;
	if (_colSpan>maxColSpan)
	    maxColSpan=_colSpan;
	colInfos[_colSpan-1]->insert(_startCol,col);
    }

    if (_minSize > col->min)
    {
    	col->min = _minSize;
	col->minCell = _cell;
    }
    if (_maxSize > col->max)
    {
    	col->max = _maxSize;
	col->maxCell = _cell;
    }
    if (_width.type > col->type)
    {
        col->type = _width.type;
    	col->value = _width.value;
	if (_width.type==Percent)
    	    percentTotals[_colSpan-1]+=_width.value;
    }
    if (_width.type == col->type)
    {
    	if (_width.value > col->value)
	{
	    if (_width.type==Percent)
	    	percentTotals[_colSpan-1]+=_width.value-col->value;
	    col->value = _width.value;
	}
    }
	
    setMinMaxKnown(false);

#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "  end: min=" << colMinWidth[_startCol] << " max=" << colMaxWidth[_startCol] << " act=" << actColWidth[_startCol] << endl;
#endif
}

void RenderTable::spreadSpanMinMax(int col, int span, int distmin,
    int distmax, LengthType type)
{
#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "RenderTable::spreadSpanMinMax() " << distmin << endl;
#endif

    if (distmin<1 && distmax<1)
    	return;
    if (distmin<1)
    	distmin=0;
    if (distmax<1)
    	distmax=0;	

    bool hasUsableCols=false;
    int tmax=distmax;
    int tmin=distmin;
    int c;

    for (c=col; c < col+span ; ++c)
    {
    	if (colType[c]<=type)
	    hasUsableCols=true;
    }

    if (hasUsableCols)
    {
    	// spread span maxWidth evenly
	c=col;	
	while(tmax)
	{
//	    kdDebug( 6040 ) << "tmax remaining " << tmax << endl;
	    if (colType[c]<=type)
	    {
		colMaxWidth[c]+=distmax/span;
		tmax-=distmax/span;
		colType[c]=type;
		if (tmax<span)
		{
		    colMaxWidth[c]+=tmax;		
		    tmax=0;
		}
	    }
	    if (++c==col+span)
		c=col;
	}
	
    	// spread span minWidth

    	LengthType tt = Variable;
	bool out=false;
	while (tt<=type && !out && tmin)
	{
	    tmin = distributeMinWidth(tmin,type,tt,col,span,true);
    	    switch (tt)
	    {
	    case Undefined:
    	    case Variable: tt=Relative; break;	
	    case Relative: tt=Percent; break;
	    case Percent: tt=Fixed; break;
	    case Fixed: out=true; break;
	    }
	}
	
	// force spread rest of the minWidth
	
    	tt = Variable;
    	out=false;
	while (tt<=type && !out && tmin)
	{
	    tmin = distributeMinWidth(tmin,type,tt,col,span,false);
    	    switch (tt)
	    {
	    case Undefined:
    	    case Variable: tt=Relative; break;	
	    case Relative: tt=Percent; break;
	    case Percent: tt=Fixed; break;
	    case Fixed: out=true; break;
	    }
	}

	for (int c=col; c < col+span ; ++c)
	    colMaxWidth[c]=MAX(colMinWidth[c],colMaxWidth[c]);	    	
		
    }
}



void RenderTable::calcSingleColMinMax(int c, ColInfo* col)
{
#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "RenderTable::calcSingleColMinMax()" << endl;
#endif

    int span=col->span;

    int oldmin=0;
    int oldmax=0;
    for (int o=c; o<c+span; ++o)
    {
	oldmin+=colMinWidth[o];
	oldmax+=colMaxWidth[o];
    }
    int smin = col->min;
    int	smax = col->max;

    if (col->type==Fixed)
    	smax = MAX(smin,col->value);
    	
    if (span==1)
    {
      //kdDebug( 6040 ) << "col (s=1) c=" << c << ",m=" << smin << ",x=" << smax << endl;
        colMinWidth[c] = smin;
        colMaxWidth[c] = smax;
        colValue[c] = col->value;
        colType[c] = col->type;
    }
    else
    {	
	int spreadmin = smin-oldmin-(span-1)*spacing;
//	int spreadmax = smax-oldmax-(span-1)*spacing;
	//kdDebug( 6040 ) << "spreading span " << spreadmin << "," << spreadmax << endl;
	spreadSpanMinMax
	    (c, span, spreadmin, 0 , col->type);
    }

}

void RenderTable::calcFinalColMax(int c, ColInfo* col)
{
#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "RenderTable::calcPercentRelativeMax()" << endl;
#endif
    int span=col->span;

    int oldmax=0;
    int oldmin=0;
    for (int o=c; o<c+span; ++o)
    {
	oldmax+=colMaxWidth[o];
	oldmin+=colMinWidth[o];
    }

    int smin = col->min;
    int	smax = col->max;

    if (col->type==Fixed)
    {
    	smax = MAX(smin,col->value);
    }
    else if (col->type == Percent)
    {
    	smax = m_width * col->value / MAX(100,totalPercent);
    }
    else if (col->type == Relative && totalRelative)
    {
    	smax= m_width * col->value / totalRelative;
    }

    smax = MAX(smax,oldmin);
//    smax = MIN(smax,m_width);

//    kdDebug( 6040 ) << "smin " << smin << " smax " << smax << " span " << span << endl;
    if (span==1)
    {
//       kdDebug( 6040 ) << "col (s=1) c=" << c << ",m=" << smin << ",x=" << smax << endl;
       colMaxWidth[c] = smax;
       colType[c] = col->type;
    }
    else
    {	
	int spreadmax = smax-oldmax-(span-1)*spacing;
//	kdDebug( 6040 ) << "spreading span " << spreadmax << endl;
	spreadSpanMinMax
	    (c, span, 0, spreadmax, col->type);
    }

}



void RenderTable::calcColMinMax()
{
// Calculate minmimum and maximum widths for all
// columns.
// Calculate min and max width for the table.


#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "RenderTable::calcColMinMax(), " << minMaxKnown() << endl;
#endif

    // PHASE 1, prepare

//    if(minMaxKnown())
//    	return;

    colMinWidth.fill(0);
    colMaxWidth.fill(0);		

    int availableWidth = containingBlockWidth() - marginLeft() - marginRight(); ;

    // PHASE 2, calculate simple minimums and maximums

    for ( unsigned int s=0;  (int)s<maxColSpan ; ++s)
    {	
    	ColInfoLine* spanCols = colInfos[s];

    	for ( unsigned int c=0; c<totalCols-s; ++c)
    	{

    	    ColInfo* col;
    	    col = spanCols->at(c);		

	    if (!col || col->span==0)
		continue;
#ifdef TABLE_DEBUG
    	    kdDebug( 6040 ) << " s=" << s << " c=" << c << " min=" << col->min << " value=" << col->value << endl;
#endif	

    	    col->update();
	    	    	
	    calcSingleColMinMax(c, col);
	
	    if ( col->span>1 && m_style->width().type != Percent
	    	&& (col->type==Fixed || col->type==Variable ))
    	    {
	    	calcFinalColMax(c, col);
	    }

	}	
    }


    // PHASE 3, calculate table width
    // NOT as simple as it sounds

    totalPercent=0;
    totalRelative=0;

    int maxFixed=0;
    int minPercent=0;
    int percentWidest=0;
    int percentWidestPercent=0;

    int minRel=0;
    int minVar=0;
    bool hasFixed=false;
    bool hasPercent=false;
    bool hasRel=false;
    bool hasVar=false;

    m_minWidth = spacing;
    m_maxWidth = spacing;

    for(int i = 0; i < (int)totalCols; i++)
    {
    	m_minWidth += colMinWidth[i] + spacing;
	m_maxWidth += colMaxWidth[i] + spacing;

    	switch(colType[i])
	{
	case Fixed:
	    maxFixed += colMaxWidth[i] + spacing;
	    hasFixed=true;
	    break;
	case Percent:
	    hasPercent=true;
	    totalPercent += colValue[i];
	    minPercent += colMinWidth[i] + spacing;	
	    if (colValue[i]>0)
	    {
    		int pw =  colMaxWidth[i]*100/colValue[i];
		if (  pw > percentWidest)
		{
	    	    percentWidest = pw;
		    percentWidestPercent = colValue[i];
		}
	    }
	    break;
	case Relative:
	    hasRel=true;
	    totalRelative += colValue[i] ;
	    minRel += colMinWidth[i] + spacing;
	    break;
	case Undefined:
	case Variable:
	    hasVar=true;
	    minVar += colMinWidth[i] + spacing;
	}

    }


    if(m_style->width().type > Relative) // Percent or fixed table
    {
	m_width = m_style->width().minWidth(availableWidth);
	if(m_minWidth > m_width) m_width = m_minWidth;
/*	kdDebug( 6040 ) << "1 width=" << width << " minWidth=" << minWidth << " m_availableWidth=" << m_availableWidth << " " << endl;
	if (width>1000) for(int i = 0; i < totalCols; i++)
    	{		
	    kdDebug( 6040 ) << "DUMP col=" << i << " type=" << colType[i] << " max=" << colMaxWidth[i] << " min=" << colMinWidth[i] << " value=" << colValue[i] << endl;
	}*/
	
    }
    else if (hasPercent && !hasFixed)
    {    	
//	kdDebug( 6040 ) << "2 percentWidest=" << //	    percentWidest << " percentWidestPercent=" << percentWidestPercent << " " << endl;
	m_width = percentWidest;
	m_width += (totalCols+1)*spacing+1;
	int tot = MIN(99,totalPercent);
	m_width = MAX(m_width, (minVar + minRel)*100/(100-tot) );
	m_width = MIN(m_width, availableWidth);
	totalPercent = MAX(totalPercent,100);
    }
    else if (hasPercent && hasFixed)
    {    	
    	totalPercent = MIN(99,totalPercent);
//	kdDebug( 6040 ) << "3 maxFixed=" << maxFixed << "  totalPercent=" << totalPercent << endl;
	m_width = (maxFixed + minVar + minRel) * 100 /
    	    (100 - totalPercent);
    	m_width = MIN (m_width, availableWidth);
    }
    else
    {
    	m_width = MIN(availableWidth,m_maxWidth);	
    }

    m_width = MAX (m_width, m_minWidth);
    	
//    kdDebug( 6040 ) << "TABLE width " << m_width << endl;


    // PHASE 4, calculate maximums for percent and relative columns

    for ( unsigned int s=0;  (int)s<maxColSpan ; ++s)
    {	
    	ColInfoLine* spanCols = colInfos[s];

    	for ( unsigned int c=0; c<totalCols-s; ++c)
    	{

    	    ColInfo* col;
    	    col = spanCols->at(c);

	    if (!col || col->span==0)
		continue;
	    if (col->type==Fixed || col->type==Variable)
	    	continue;

	    calcFinalColMax(c, col);

	}
	
    }


    // PHASE 5, set table min and max to final values

    if(m_style->width().type == Fixed)
    	m_minWidth = m_maxWidth = m_width;
    else
    {
        m_maxWidth = spacing;

    	for(int i = 0; i < (int)totalCols; i++)
	    m_maxWidth += colMaxWidth[i] + spacing;
    }
    
    m_minWidth += borderLeft() + borderRight();
    m_maxWidth += borderLeft() + borderRight();
    m_width += borderLeft() + borderRight();

    setMinMaxKnown(true);

}

void RenderTable::calcColWidth(void)
{

#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "START calcColWidth() this = " << this << endl;
    kdDebug( 6040 ) << "---- " << totalColInfos << " ----" << endl;
    kdDebug( 6040 ) << "maxColSpan = " << maxColSpan << endl;
#endif

    if (totalCols==0)
    	return;

    /*
     * Calculate min and max width for every column,
     * and the width of the table
     */

    calcColMinMax();

    /*
     * Set actColWidth[] to column minimums, it will
     * grow from there.
     * Collect same statistics for future use.
     */

    int actWidth = spacing + borderLeft() + borderRight();

    int minFixed = 0;
    int minPercent = 0;
    int minRel = 0;
    int minVar = 0;

    int maxFixed = 0;
    int maxPercent = 0;
    int maxRel = 0;
    int maxVar = 0;

    int numFixed = 0;
    int numPercent = 0;
    int numRel = 0;
    int numVar = 0;

    actColWidth.fill(0);

    unsigned int i;
    for(i = 0; i < totalCols; i++)
    {
    	actColWidth[i] = colMinWidth[i];
	actWidth += actColWidth[i] + spacing;	
	
	switch(colType[i])
	{
	case Fixed:
	    minFixed += colMinWidth[i];
	    maxFixed += colMaxWidth[i];
	    numFixed++;
	    break;
	case Percent:
	    minPercent += colMinWidth[i];
	    maxPercent += colMaxWidth[i];
	    numPercent++;
	    break;
	case Relative:
	    minRel += colMinWidth[i];
	    maxRel += colMaxWidth[i];
	    numRel++;
	    break;
	case Undefined:
	case Variable:
	    minVar += colMinWidth[i];
	    maxVar += colMaxWidth[i];
	    numVar++;
	}
    	
    }

#ifdef TABLE_DEBUG
    for(int i = 1; i <= (int)totalCols; i++)
    {
	kdDebug( 6040 ) << "Start->target " << i << ": " << actColWidth[i-1] << "->" << colMaxWidth[i-1] << endl;
    }
#endif

    int tooAdd = m_width - actWidth;      // what we can add

#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "tooAdd = width - actwidth: " << tooAdd << " = " << m_width << " - " << actWidth << endl;
#endif

    /*
     * tooAddute the free width among the columns so that
     * they reach their max width.
     * Order: fixed->percent->relative->variable
     */

    tooAdd = distributeWidth(tooAdd,Fixed,numFixed);
    tooAdd = distributeWidth(tooAdd,Percent,numPercent);
    tooAdd = distributeWidth(tooAdd,Relative,numRel);
    tooAdd = distributeWidth(tooAdd,Variable,numVar);

    /*
     * Some width still left?
     * Reverse order, variable->relative->percent
     */

    tooAdd = distributeRest(tooAdd,Variable,maxVar);
    tooAdd = distributeRest(tooAdd,Relative,maxRel);
    tooAdd = distributeRest(tooAdd,Percent,maxPercent);

#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "final tooAdd " << tooAdd << endl;
#endif

    /*
     * Calculate the placement of colums
     */

    columnPos.fill(0);
    columnPos[0] = spacing;
    for(i = 1; i <= totalCols; i++)
    {
    	columnPos[i] += columnPos[i-1] + actColWidth[i-1] + spacing;
#ifdef TABLE_DEBUG
	kdDebug( 6040 ) << "Actual width col " << i << ": " << actColWidth[i-1] << " pos = " << columnPos[i-1] << endl;
#endif
    }

#ifdef TABLE_DEBUG
    if(m_width != columnPos[totalCols] ) kdDebug( 6040 ) << "========> table layout error!!! <===============================" << endl;
    kdDebug( 6040 ) << "total width = " << m_width << endl;
#endif

}


int RenderTable::distributeWidth(int distrib, LengthType type, int typeCols )
{
    int olddis=0;
    int c=0;

    int tdis = distrib;
//    kdDebug( 6040 ) << "DISTRIBUTING " << distrib << " pixels to type " << type << " cols" << endl;

    while(tdis>0)
    {
	if (colType[c]==type)
	{
	    int delta = MIN(distrib/typeCols,colMaxWidth[c]-actColWidth[c]);
	    delta = MIN(tdis,delta);
	    if (delta==0 && tdis && colMaxWidth[c]>actColWidth[c])
	    	delta=1;
	    actColWidth[c]+=delta;		
	    tdis-=delta;
	}
	if (++c == (int)totalCols)
	{
	    c=0;
	    if (olddis==tdis)
		break;
	    olddis=tdis;
	}
    }
    return tdis;
}


int RenderTable::distributeRest(int distrib, LengthType type, int divider )
{
    if (!divider)
    	return distrib;

//    kdDebug( 6040 ) << "DISTRIBUTING rest, " << distrib << " pixels to type " << type << " cols" << endl;

    int olddis=0;
    int c=0;

    int tdis = distrib;

    while(tdis>0)
    {
	if (colType[c]==type)
	{
	    int delta = (colMaxWidth[c] * distrib) / divider;
	    delta=MIN(delta,tdis);
	    if (delta==0 && tdis)
	    	delta=1;
	    actColWidth[c] += delta;
	    tdis -= delta;
	}
	if (++c == (int)totalCols)
	{
	    c=0;
	    if (olddis==tdis)
		break;
	    olddis=tdis;
	}
    }
    return tdis;
}

int RenderTable::distributeMinWidth(int distrib, LengthType distType,
    	    LengthType toType, int start, int span, bool mlim )
{
//    kdDebug( 6040 ) << "MINDIST, " << distrib << " pixels of type " << distType << " to type " << toType << " cols sp=" << span << " " << endl;

    int olddis=0;
    int c=start;

    int tdis = distrib;

    while(tdis>0)
    {
//    	kdDebug( 6040 ) << "ct=" << colType[c] << endl;
	if (colType[c]==toType)
	{
	    int delta = distrib/span;
	    if (mlim)
	    	delta = MIN(delta,colMaxWidth[c]-colMinWidth[c]);
	    delta = MIN(tdis,delta);
	    if (delta==0 && tdis && (!mlim || colMaxWidth[c]>colMinWidth[c]))
	    	delta=1;
	    colMinWidth[c]+=delta;
	    colType[c]=distType;
	    tdis-=delta;
	}
	if (++c==start+span)
	{
	    c=start;
	    if (olddis==tdis)
		break;
	    olddis=tdis;
	}
    }

    return tdis;
}




void RenderTable::calcRowHeight(int r)
{
    unsigned int c;
    int indx;//, borderExtra = border ? 1 : 0;
    RenderTableCell *cell;

    rowHeights.resize( totalRows+1 );
    rowBaselines.resize( totalRows );
    rowHeights[0] =  spacing + borderTop();

  //int oldheight = rowHeights[r+1] - rowHeights[r];
    rowHeights[r+1] = 0;

    int baseline=0;
    int bdesc=0;
    int ch;

    for ( c = 0; c < totalCols; c++ )
    {
	if ( ( cell = cells[r][c] ) == 0 )
	    continue;
	if ( c < totalCols - 1 && cell == cells[r][c+1] )
	    continue;
	if ( r < totalRows - 1 && cells[r+1][c] == cell )
	    continue;

	if ( ( indx = r - cell->rowSpan() + 1 ) < 0 )
	    indx = 0;

	ch = cell->style()->height().width(0);
	if ( cell->height() > ch)
	    ch = cell->height();

	int rowPos = rowHeights[ indx ] + ch +
	     spacing ; // + padding

	if ( rowPos > rowHeights[r+1] )
	    rowHeights[r+1] = rowPos;

	// find out the baseline
	EVerticalAlign va = cell->vAlign();
	if (va == BASELINE || va == TEXT_BOTTOM || va == TEXT_TOP
	    || va == SUPER || va == SUB)
	{
	    int b=cell->baselineOffset();

	    if (b>baseline)
		baseline=b;

	    int td = rowHeights[ indx ] + ch - b;
	    if (td>bdesc)
		bdesc = td;
	}	    		
    }

    //do we have baseline aligned elements?
    if (baseline)
    {
	// increase rowheight if baseline requires
	int bRowPos = baseline + bdesc  + spacing ; // + 2*padding
    	if (rowHeights[r+1]<bRowPos)
	    rowHeights[r+1]=bRowPos;

	rowBaselines[r]=baseline;
    }

    if ( rowHeights[r+1] < rowHeights[r] )
	rowHeights[r+1] = rowHeights[r];
	
    // html tables with percent height are relative to view
    if (r+1==totalRows)
    {
    	Length h = style()->height();
	int th=0;
	if (h.isFixed())
    	    th = h.value;
	else if (h.isPercent())
	{
    	    Length ch = containingBlock()->style()->height();
	    if (ch.isFixed())
    		th = h.width(ch.value);
	    else if (style()->htmlHacks())
	    {
	    	th = h.width(viewRect().height());
		// not really, but this way the view height change
		// gets propagated correctly
		setContainsPositioned(true);
	    }
	}
	if (rowHeights[r+1]<th)
	    rowHeights[r+1]=th;
    }	

}

void RenderTable::layout(bool deep)
{
    if (layouted() && !containsPositioned() && _lastParentWidth == containingBlockWidth())
   	return;

    _lastParentWidth = containingBlockWidth();

    // ###
    deep = true;

    m_height = 0;

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table)::layout1(" << deep << ") width=" << width() << ", layouted=" << layouted() << endl;
#endif


    FOR_EACH_CELL( r, c, cell)
    {
	cell->calcMinMaxWidth();
    }
    END_FOR_EACH

    calcColWidth();

    setCellWidths();

    if(tCaption)
    {
	tCaption->setYPos(m_height);
	if(deep)
	    tCaption->layout(deep);
	m_height += tCaption->height();
    }

    // layout rows

    for ( unsigned int r = 0; r < totalRows; r++ )
    {
    	layoutRow(r);
    }

    m_height += rowHeights[totalRows];
    m_height += borderBottom();

    setLayouted();

}


void RenderTable::layoutRow(int r)
{
    int rHeight;
    int indx, rindx;

    for ( unsigned int c = 0; c < totalCols; c++ )
    {
        RenderTableCell *cell = cells[r][c];
        if (!cell)
            continue;
	if ( c < totalCols - 1 && cell == cells[r][c+1] )
	    continue;
	if ( r < totalRows - 1 && cell == cells[r+1][c] )
	    continue;
	
	cell->layout(true);
	cell->setCellTopExtra(0);
	cell->setCellBottomExtra(0);	
    }

    calcRowHeight(r);

    for ( unsigned int c = 0; c < totalCols; c++ )
    {
        RenderTableCell *cell = cells[r][c];
        if (!cell)
            continue;
	if ( c < totalCols - 1 && cell == cells[r][c+1] )
	    continue;
	if ( r < totalRows - 1 && cell == cells[r+1][c] )
	    continue;

	if ( ( indx = c-cell->colSpan()+1 ) < 0 )
	    indx = 0;
	
	if ( ( rindx = r-cell->rowSpan()+1 ) < 0 )
	    rindx = 0;

	//kdDebug( 6040 ) << "setting position " << r << "/" << indx << "-" << c << ": " << //columnPos[indx] + padding << "/" << rowHeights[rindx] << " " << endl;
	rHeight = rowHeights[r+1] - rowHeights[rindx] -
	    spacing;

	EVerticalAlign va = cell->vAlign();
	int te=0;
	switch (va)
	{
	case SUB:
	case SUPER:
    	case TEXT_TOP:	
	case TEXT_BOTTOM:	
	case BASELINE:
    	    te = getBaseline(r) - cell->baselineOffset() ;
	    break;
	case TOP:
	    te = 0;
    	    break;	
	case MIDDLE:
	    te = (rHeight - cell->height())/2;
	    break;
	case BOTTOM:
	    te = rHeight - cell->height();
	    break;
    	default:
    	    break;
	}
#ifdef DEBUG_LAYOUT
	kdDebug( 6040 ) << "CELL te=" << te << ", be=" << rHeight - cell->height() - te << ", rHeight=" << rHeight << ", valign=" << va << endl;
#endif
	cell->setCellTopExtra( te );
	cell->setCellBottomExtra( rHeight - cell->height() - te);

    	if (style()->direction()==RTL)
    	{
	    cell->setPos( columnPos[(int)totalCols]
	    	- columnPos[(int)(indx+cell->colSpan())] + borderLeft(),
	    	rowHeights[rindx] );
	}
	else
	    cell->setPos( columnPos[indx] + borderLeft(), rowHeights[rindx] );
	
	cell->setRowHeight(rHeight);
	// ###
	// cell->setHeight(cellHeight);
    }
}


void RenderTable::refreshRow(int r)
{
    for ( unsigned int c = 0; c < totalCols; c++ )
    {
        RenderTableCell *cell = cells[r][c];
        if (!cell)
            continue;
	if ( c < totalCols - 1 && cell == cells[r][c+1] )
	    continue;
	if ( r < totalRows - 1 && cell == cells[r+1][c] )
	    continue;
	
	cell->calcMinMaxWidth();
    }

    setCellWidths();

    layoutRow(r);
    repaint();
}

void RenderTable::setCellWidths()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table, this=0x" << this << ")::setCellWidths()" << endl;
#endif

	
    int indx;
    FOR_EACH_CELL( r, c, cell)
	{
	    if ( ( indx = c-cell->colSpan()+1) < 0 )
		indx = 0;
	    int w = columnPos[c+1] - columnPos[ indx ] - spacing ; //- padding*2;

#ifdef TABLE_DEBUG
	    kdDebug( 6040 ) << "0x" << this << ": setting width " << r << "/" << indx << "-" << c << " (0x" << cell << "): " << w << " " << endl;
#endif
	    if (cell->width() != w)
	    	cell->setLayouted(false);
	    cell->setWidth( w );
	}
    END_FOR_EACH

}

void RenderTable::print( QPainter *p, int _x, int _y,
				  int _w, int _h, int _tx, int _ty)
{

//    if(!layouted()) return;

    _tx += xPos();
    _ty += yPos();

#ifdef TABLE_DEBUG
    kdDebug( 6040 ) << "RenderTable::print() w/h = (" << width() << "/" << height() << ")" << endl;
#endif
    if (!containsPositioned() && !isRelPositioned() && !isPositioned())
    {
    	if((_ty > _y + _h) || (_ty + height() < _y)) return;
    	if((_tx > _x + _w) || (_tx + width() < _x)) return;
    }

#ifdef DEBUG_LAYOUT
     kdDebug( 6040 ) << "RenderTable::print(2) " << _tx << "/" << _ty << " (" << _x << "/" << _y << ")" << endl;
#endif

    printBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);

    if ( tCaption )
    {
	tCaption->print( p, _x, _y, _w, _h, _tx, _ty );
    }

    // draw the cells
    FOR_EACH_CELL(r, c, cell)
    {
#ifdef DEBUG_LAYOUT
	kdDebug( 6040 ) << "printing cell " << r << "/" << c << endl;
#endif
        cell->print( p, _x, _y, _w, _h, _tx, _ty);
    }
    END_FOR_EACH

#ifdef BOX_DEBUG
    outlineBox(p, _tx, _ty, "blue");
#endif
}

void RenderTable::printBorders( QPainter* /*p*/, int, int,
					int, int, int /*_tx*/, int /*_ty*/)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table)::printObject()" << endl;
#endif

    // ### don't call children here...

    // draw the border - needs work to print to printer

}



void RenderTable::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table)::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    calcColMinMax();
}

void RenderTable::close()
{
    setParsing(false);
    calcMinMaxWidth();
    setLayouted(false);
    if(!containingBlockWidth()) return;
    updateSize();
}

void RenderTable::updateSize()
{
//    kdDebug( 6040 ) << "RenderTable::updateSize()" << endl;

//    setMinMaxKnown(false);
//    setLayouted(false);
//    parent()->updateSize();
     RenderBox::updateSize();
}


// --------------------------------------------------------------------------

RenderTableSection::RenderTableSection()
    : RenderObject()
{
}

RenderTableSection::~RenderTableSection()
{
    nrows = 0;
}

void RenderTableSection::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableSection)::addChild( " << child->renderName()  << ", " <<
                       beforeChild ? beforeChild->renderName() : 0 << " )" << endl;
#endif

    table->startRow();
    child->setTable(table);
    RenderObject::addChild(child,beforeChild);
}

// -------------------------------------------------------------------------

RenderTableRow::RenderTableRow()
  : RenderObject()
{
  rIndex = -1;
}

RenderTableRow::~RenderTableRow()
{
}


long RenderTableRow::rowIndex() const
{
    // ###
    return 0;
}

void RenderTableRow::setRowIndex( long  )
{
    // ###
}

void RenderTableRow::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableRow)::addChild( " << child->renderName() << " )"  << ", " <<
                       beforeChild ? beforeChild->renderName() : 0 << " )" << endl;
#endif

    RenderTableCell *cell =
	static_cast<RenderTableCell *>(child);
    cell->setTable(table);
    cell->setRowImpl(this);
    table->addCell(cell);  // ### may not work for beforeChild != 0

    RenderObject::addChild(child,beforeChild);
}

// -------------------------------------------------------------------------

RenderTableCell::RenderTableCell()
  : RenderFlow()
{
  _col = -1;
  _row = -1;
  cSpan = rSpan = 1;
  nWrap = false;
  rowHeight = 0;
  table = 0;
  rowimpl = 0;
  m_printSpecial=true;
}

RenderTableCell::~RenderTableCell()
{
}

void RenderTableCell::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableCell)::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    //if(minMaxKnown()) return;

    int oldMin = m_minWidth;
    int oldMax = m_maxWidth;

    RenderFlow::calcMinMaxWidth();

    if(nWrap && m_style->width().type!=Fixed) m_minWidth = m_maxWidth;

    if (m_minWidth!=oldMin || m_maxWidth!=oldMax)
        table->addColInfo(this);	

}

void RenderTableCell::calcWidth()
{
}

void RenderTableCell::setContainingBlock()
{
    // ### this break absolute positioning of table cells, but that's
    //     nothing one should do anyway ;-)
    m_containingBlock = table;
}


void RenderTableCell::close()
{
    //kdDebug( 6040 ) << "renderFlow::close()" << endl;
    setParsing(false);
    if(haveAnonymousBox())
    {
	m_last->close();
	//kdDebug( 6040 ) << "RenderFlow::close(): closing anonymous box" << endl;
	setHaveAnonymousBox(false);
    }

    //if(m_last)
    //    m_height += m_last->height() + m_last->marginBottom();
    if(m_style->hasBorder())
	m_height += borderBottom();
    if(m_style->hasPadding())
	m_height += paddingBottom();
	
    setMinMaxKnown(false);
    calcMinMaxWidth();
    setLayouted(false);

    table->updateSize();

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderTableCell)::close() total height =" << m_height << endl;
#endif
}


void RenderTableCell::updateSize()
{
//    kdDebug( 6040 ) << renderName() << "(RenderBox)::updateSize()" << endl;

    RenderFlow::updateSize();

}


void RenderTableCell::repaintRectangle(int x, int y, int w, int h)
{
    y += _topExtra;
    RenderFlow::repaintRectangle(x, y, w, h);
}

void RenderTableCell::repaintObject(RenderObject *o, int x, int y)
{
    y += _topExtra;
    RenderFlow::repaintObject(o, x, y);
}

void RenderTableCell::absolutePosition(int &xPos, int &yPos)
{
    RenderFlow::absolutePosition(xPos, yPos);
    if (yPos != -1)
      yPos += _topExtra;
}



void RenderTableCell::print(QPainter *p, int _x, int _y,
				       int _w, int _h, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderTableCell)::print() w/h = (" << width() << "/" << height() << ")" << endl;
#endif

//    if (!layouted())
//  	return;

    _tx += m_x;
    _ty += m_y + cellTopExtra();

    // check if we need to do anything at all...
    if(!containsPositioned() && ((_ty-_topExtra > _y + _h)
    	|| (_ty + m_height+_topExtra+_bottomExtra < _y))) return;

    printObject(p, _x, _y, _w, _h, _tx, _ty);
}


// -------------------------------------------------------------------------

RenderTableCol::RenderTableCol()
  : RenderObject()
{
    _span = 1;
}

RenderTableCol::~RenderTableCol()
{
}


void RenderTableCol::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table)::addChild( " << child->renderName() << " )" <<  << ", " <<
                       beforeChild ? beforeChild->renderName() : 0 << " )" << endl;
#endif

    if (child->style()->display() == TABLE_COLUMN)
    {
	// these have to come before the table definition!	
	RenderObject::addChild(child,beforeChild);
	RenderTableCol* colel = static_cast<RenderTableCol *>(child);
	colel->setStartCol(_currentCol);
//	kdDebug( 6040 ) << "_currentCol=" << _currentCol << endl;
	table->addColInfo(colel);
	_currentCol++;
    }
}

Length RenderTableCol::width()
{
    if (m_style->width().type == Undefined
	&& parent() &&
	    parent()->style()->display()==TABLE_COLUMN_GROUP)
	return static_cast<RenderTableCol*>(parent())->width();
    else
	return m_style->width();	
}

// -------------------------------------------------------------------------

RenderTableCaption::RenderTableCaption()
  : RenderFlow()
{
}

RenderTableCaption::~RenderTableCaption()
{
}

