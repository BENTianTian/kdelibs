/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Gunnstein Lye (gunnstein@netcom.no)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
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
#include "dom2_range.h"
#include "dom2_traversal.h"
#include "dom_node.h"
#include "dom_doc.h"
#include "dom_string.h"
#include "dom_text.h"
#include "dom_exception.h"
#include "dom_docimpl.h"
#include <qstring.h>
#include <stdio.h>               // for printf

/* Functions not yet implemented:
   deleteContents
   extractContents
   cloneContents
   insertNode
   surroundContents
   toHTML

   Functions not working correctly:
   toString
 */

//using namespace CSS;       // leff
using namespace DOM;


Range::Range()
{
    startContainer = 0;
    endContainer = 0;
    startOffset = 0;
    endOffset = 0;
    commonAncestorContainer = 0;
    collapsed = true;
    detached = false;
}

Range::Range(const Document rootContainer)
{
    ownerDocument = rootContainer;
    startContainer = (Node)rootContainer;
    endContainer = (Node)rootContainer;
    startOffset = 0;
    endOffset = 0;
    commonAncestorContainer = (Node)rootContainer;
    collapsed = true;
    detached = false;
}

Range::Range(const Range &other)
{
    ownerDocument = other.ownerDocument;
    startContainer = other.startContainer;
    startOffset = other.startOffset;
    endContainer = other.endContainer;
    endOffset = other.endOffset;
    commonAncestorContainer = other.commonAncestorContainer;
    collapsed = other.collapsed;
    detached = false;
}

Range::Range(const Node sc, const long so, const Node ec, const long eo)
{
    startContainer = sc;
    startOffset = so;
    endContainer = ec;
    endOffset = eo;
    commonAncestorContainer = getCommonAncestorContainer();
    if( startContainer == endContainer && startOffset == endOffset )  collapsed = true;
    else  collapsed = false;
    detached = false;
}

Range &Range::operator = (const Range &other)
{
    startContainer = other.startContainer;
    startOffset = other.startOffset;
    endContainer = other.endContainer;
    endOffset = other.endOffset;
    commonAncestorContainer = other.commonAncestorContainer;
    collapsed = other.collapsed;
    detached = false;
    return *this;
}

Range::~Range()
{
}

Node Range::getStartContainer() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return startContainer;
}

long Range::getStartOffset() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return startOffset;
}

Node Range::getEndContainer() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return endContainer;
}

long Range::getEndOffset() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return endOffset;
}

Node Range::getCommonAncestorContainer() /*const*/
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    
    Node parentStart = startContainer;
    Node parentEnd = endContainer;
    
    while( !parentStart.isNull() && (parentStart != parentEnd) )
    {
        while( !parentEnd.isNull() && (parentStart != parentEnd) )
            parentEnd = parentEnd.parentNode();

        if(parentStart == parentEnd)  break;
        parentStart = parentStart.parentNode();
    }

    if(parentStart == parentEnd)
        commonAncestorContainer = parentStart;
    else
    {
        return Node();
    }
    
    return commonAncestorContainer;
}

bool Range::isCollapsed() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return collapsed;
}

bool Range::isDetached() const
{
    return detached;
}

void Range::setStart( const Node &refNode, long offset )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );

    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( offset < 0 )
        throw DOMException( DOMException::INDEX_SIZE_ERR );

    if( refNode.nodeType() != Node::TEXT_NODE )
    {
	if( (unsigned)offset > refNode.childNodes().length() )
            throw DOMException( DOMException::INDEX_SIZE_ERR );
    }
    else
    {
	Text t;
	t = refNode;
        if( t.isNull() || (unsigned)offset > t.length() )
            throw DOMException( DOMException::INDEX_SIZE_ERR );
    }

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    startContainer = refNode;
    startOffset = offset;

    if( endContainer != 0 )
    {
        if( commonAncestorContainer != 0 )
        {
            Node oldCommonAncestorContainer = commonAncestorContainer;
            if( oldCommonAncestorContainer != getCommonAncestorContainer() )
                collapse( true );
            if( !boundaryPointsValid() )
                collapse( true );
        }
        else  getCommonAncestorContainer();
    }
}

void Range::setEnd( const Node &refNode, long offset )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );

    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( offset < 0 )
        throw DOMException( DOMException::INDEX_SIZE_ERR );

    if( refNode.nodeType() != Node::TEXT_NODE )
    {
	if( (unsigned)offset > refNode.childNodes().length() )
            throw DOMException( DOMException::INDEX_SIZE_ERR );
    }
    else
    {
	Text t;
	t = refNode;
        if( t.isNull() || (unsigned)offset > t.length() )
            throw DOMException( DOMException::INDEX_SIZE_ERR );
    }

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    endContainer = refNode;
    endOffset = offset;

    if( startContainer != 0 )
    {
        if( commonAncestorContainer != 0 )
        {
            Node oldCommonAncestorContainer = commonAncestorContainer;
            if( oldCommonAncestorContainer != getCommonAncestorContainer() )
                collapse( true );
            if( !boundaryPointsValid() )
                collapse( true );
        }
        else  getCommonAncestorContainer();
    }
}

void Range::setStartBefore( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setStart( refNode.parentNode(), refNode.index() );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::setStartAfter( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setStart( refNode.parentNode(), refNode.index()+1 );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::setEndBefore( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setEnd( refNode.parentNode(), refNode.index() );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::setEndAfter( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setEnd( refNode.parentNode(), refNode.index()+1 );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::collapse( bool toStart )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    if( toStart )   // collapse to start
    {
        endContainer = startContainer;
        endOffset = startOffset;
        collapsed = true;
        commonAncestorContainer = startContainer;
    }
    else            // collapse to end
    {
        startContainer = endContainer;
        startOffset = endOffset;
        collapsed = true;
        commonAncestorContainer = endContainer;
    }
}

void Range::selectNode( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setStartBefore( refNode );
        setEndAfter( refNode );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::selectNodeContents( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );

    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setStartBefore( refNode.firstChild() );
        setEndAfter( refNode.lastChild() );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

short Range::compareBoundaryPoints( CompareHow how, const Range &sourceRange )
{
    if( commonAncestorContainer.ownerDocument() != sourceRange.commonAncestorContainer.ownerDocument() )
        throw DOMException( DOMException::WRONG_DOCUMENT_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    switch(how)
    {
    case StartToStart:
        return compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                      sourceRange.getStartContainer(), sourceRange.getStartOffset() );
        break;
    case StartToEnd:
        return compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                      sourceRange.getEndContainer(), sourceRange.getEndOffset() );
        break;
    case EndToEnd:
        return compareBoundaryPoints( getEndContainer(), getEndOffset(),
                                      sourceRange.getEndContainer(), sourceRange.getEndOffset() );
        break;
    case EndToStart:
        return compareBoundaryPoints( getEndContainer(), getEndOffset(),
                                      sourceRange.getStartContainer(), sourceRange.getStartOffset() );
        break;
    default:
        printf( "Function compareBoundaryPoints: Invalid CompareHow\n" );
        return 2;     // undocumented - should throw an exception here
    }
}

short Range::compareBoundaryPoints( Node containerA, long offsetA, Node containerB, long offsetB )
{
    if( offsetA < 0 || offsetB < 0 )
    {
        printf( "Function compareBoundaryPoints: No negative offsets allowed\n" );
        return 2;     // undocumented - should throw an exception here
    }

    if( containerA == containerB )
    {
        if( offsetA == offsetB )  return 0;    // A is equal to B
        if( offsetA < offsetB )  return -1;    // A is before B
        else  return 1;                        // A is after B
    }

    Node n = containerA;
    while( n != 0 )
    {
        if( n == containerB)  return -1;       // A is before B

        Node next;
        if( n == containerA )  next = n.childNodes().item( offsetA );
        else  next = n.firstChild();
        if( next == 0 )  next = n.nextSibling();
        while( n != 0 && next == 0 )
        {
            n = n.parentNode();
            next = n.nextSibling();
        }
        n = next;
    }
    return 1;                                  // A is after B
}

bool Range::boundaryPointsValid(  )
{
    short valid =  compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                          getEndContainer(), getEndOffset() );
    if( valid == 1 )  return false;
    else  return true;
    
}

void Range::deleteContents(  )
{
    Node cmnRoot = getCommonAncestorContainer();
    printf("CommonAC: %s \n", cmnRoot.nodeName().string().ascii());
//    printf("end: %d, start: %d", startOffset, endOffset);
    
    if(startContainer == endContainer)
    {
        if(startOffset == endOffset)            // we have a collapsed range
        {printf("collapsed\n");return;} 

        // TODO: we need to delete the text Node if a whole text is selected!!
        if( startContainer.nodeType() == Node::TEXT_NODE )
        {
            startContainer.nodeValue().remove(startOffset, endOffset);
            startContainer.applyChanges();
        }
        else 
        {
            printf("same but not a text node\n");
            Node _tempParent = startContainer;
            Node _tempCurrent = startContainer.firstChild();
            unsigned int i;

            for(i=0; i < startOffset; i++)    // get the node given by the offset
                _tempCurrent = _tempCurrent.nextSibling();

            /* now delete all nodes between the offsets */
            unsigned int range = endOffset - startOffset;
            Node _nextCurrent = _tempCurrent;                  // to keep track of which node to take next

            for(i=0; i<range && !_tempCurrent.isNull(); i++)   
            {
                if(_tempParent == _tempCurrent.parentNode() )
                    printf("like\n");
                _nextCurrent = _tempCurrent.nextSibling();
                printf("just before remove\n");
                _tempParent.removeChild(_tempCurrent);
                printf("just after remove\n");
                _tempCurrent = _nextCurrent;
            }
        _tempParent.applyChanges();
        }
        return;
    }// END COMMON CONTAINER CASE!!

    printf("end common case\n");
    Node _nextCurrent;                 
    Node _tempCurrent;

    // cleanup left side
    Node _leftParent = startContainer;
    if( startContainer.nodeType() == Node::TEXT_NODE )
    {
        printf("left side text\n");
        (void)startContainer.nodeValue().split(startOffset); // what about complete removals?
    }
    else
    {
     
        _tempCurrent = startContainer.firstChild();
        unsigned int i;
        
        for(i=0; i < startOffset; i++)    // get the node given by the offset
            _tempCurrent = _tempCurrent.nextSibling();
        
        _nextCurrent = _tempCurrent;                  // to keep track of which node to take next
        
        while( !_tempCurrent.isNull() )   
        {
            _nextCurrent = _tempCurrent.nextSibling();
            _leftParent.removeChild(_tempCurrent);
            _tempCurrent = _nextCurrent;
        }
    }
    _tempCurrent = _leftParent;
    _leftParent = _leftParent.parentNode();
    while( _leftParent != cmnRoot )
    {
        while( !_tempCurrent.isNull() )   
        {
            _nextCurrent = _tempCurrent.nextSibling();
            _leftParent.removeChild(_tempCurrent);
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _leftParent;
        _leftParent = _leftParent.parentNode();
    }


    // cleanup right side
    Node _rightParent = endContainer;
    if( endContainer.nodeType() == Node::TEXT_NODE )
    {
        endContainer.nodeValue().remove(0, endOffset); // what about complete removals?
    }
    else
    {
     
        Node _tempCurrent = endContainer.firstChild();
        unsigned int i;
        
        for(i=0; i < endOffset; i++)    // get the node given by the offset
            _tempCurrent = _tempCurrent.nextSibling();
        
        Node _nextCurrent = _tempCurrent;                  // to keep track of which node to take next
        
        while( !_tempCurrent.isNull() )   
        {
            _nextCurrent = _tempCurrent.previousSibling();
            _leftParent.removeChild(_tempCurrent);
            _tempCurrent = _nextCurrent;
        }
    }
    _tempCurrent = _rightParent;
    _rightParent = _rightParent.parentNode();
    while( _rightParent != cmnRoot )
    {
        while( !_tempCurrent.isNull() )   
        {
            _nextCurrent = _tempCurrent.previousSibling();
            _rightParent.removeChild(_tempCurrent);
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _rightParent;
        _rightParent = _rightParent.parentNode();
    }

    // cleanup middle
    _leftParent = _leftParent.nextSibling();
    while( _leftParent != _rightParent )
    {
        cmnRoot.removeChild(_leftParent);
        _leftParent = _leftParent.nextSibling();
    }


    // FIXME! this allways collapses to the front (see DOM specs)
    //collapse(true);
    return;

}

DocumentFragment Range::extractContents(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    return masterTraverse( true );
}

DocumentFragment Range::cloneContents(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    return masterTraverse( false );
}

void Range::insertNode( const Node &newNode )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    startContainer.insertBefore( newNode, startContainer.childNodes().item( startOffset ) );
}

void Range::surroundContents( const Node &/*newParent*/ )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );
}

Range Range::cloneRange(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    return Range( this );
}

DOMString Range::toString(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    NodeIterator iterator( getStartContainer().childNodes().item( getStartOffset() ) );
//    NodeIterator iterator( getCommonAncestorContainer() );
    DOMString _string;
    Node _node = iterator.nextNode();
    
    while( !_node.isNull() )
    {
        printf( "\nNodetype: %s\n", _node.nodeName().string().ascii() );
        if( _node.nodeType() == Node::TEXT_NODE )
        {
            QString str = _node.nodeValue().string();
            if( _node == getStartContainer() && _node == getEndContainer() )
                _string = str.mid( getStartOffset(), getEndOffset() - getStartOffset() );
            else if( _node == getStartContainer() )
                _string = str.mid( getStartOffset() );
            else if( _node == getEndContainer() )
                _string += str.left( getStartOffset() );
            else
                _string += str;
        }
        else if( _node.nodeName() == "BR" )  _string += "\n";
        else if( _node.nodeName() == "P" || _node.nodeName() == "TD" )  _string += "\n\n";
        else  _string += " ";
        
        _node = iterator.nextNode();
    }
    return _string;
}

DOMString Range::toHTML(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    // this is just to avoid compiler warnings
    DOMString d;
    return d;
}

void Range::detach(  )
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        detached = true;
}

DocumentFragment Range::masterTraverse(bool contentExtract)
{
    /* function description easy case, startContainer == endContainer
     * If we have a text node simply clone/extract the contents between
     * start & end and put them into the fragment
     * If we don't have a text node, find the offset and copy/clone the content
     * between the two offsets
     * We end with returning the fragment of course
     */
    Node _clone;
    DocumentFragment _endFragment( ownerDocument.createDocumentFragment() );

    if(startContainer == endContainer)
    {
        if(startOffset == endOffset)            // we have a collapsed range
            return DocumentFragment();

        // TODO: we need to delete the text Node if a whole text is selected!!
        if( startContainer.nodeType() == Node::TEXT_NODE )    // we have a text node.. special :)
        {
            _clone = startContainer.cloneNode(false);
            _clone.nodeValue().remove(0, startOffset);  // we need to get the SUBSTRING
            _clone.nodeValue().remove(endOffset, _clone.nodeValue().length() - endOffset);
            if(contentExtract)
            {
                // full trim :)
                startContainer.nodeValue().remove(startOffset, endOffset - startOffset);
            }
            _endFragment.appendChild(_clone);
        }
        else  // we have the same container class but we are not a text node
        {
            Node _tempCurrent = startContainer.firstChild();
            unsigned int i;

            for(i=0; i < startOffset; i++)    // get the node given by the offset
                _tempCurrent = _tempCurrent.nextSibling();

            /* now copy (or move) all the nodes in the range into the document fragment */
            unsigned int range = endOffset - startOffset;
            Node _nextCurrent = _tempCurrent;                  // to keep track of which node to take next
            for(i=0; i<range && !_tempCurrent.isNull(); i++)   // check of isNull in case of strange errors
            {
                _nextCurrent = _tempCurrent.nextSibling();

                if(contentExtract)
                {
                    _endFragment.appendChild(_tempCurrent);
                }
                else
                {
                    _clone = _tempCurrent.cloneNode(true);
                    _endFragment.appendChild(_clone);
                }

                _tempCurrent = _nextCurrent;
            }
        }
        return _endFragment;
    }// END COMMON CONTAINER HERE!!!


    /* Ok here we go for the harder part, first a general desription:
     * First we copy all the border nodes (the have to be copied as long
     * as they are partially selected) from the startContainer to the CmnAContainer. Then we do
     * the same for the endContainer. After this we add all fully selected
     * nodes that are between these two!
     */

    Node _cmnRoot = getCommonAncestorContainer();
    Node _tempCurrent = startContainer;
    Node _tempPartial;
    // we still have Node _clone!!

    // Special case text is first:
    if( _tempCurrent.nodeType() == Node::TEXT_NODE )
    {
        _clone = _tempCurrent.cloneNode(false);
        _clone.nodeValue().remove(0, startOffset);  // we need to get the SUBSTRING
        if(contentExtract)
        {
            // remove what wasn't in the substring
            startContainer.nodeValue().split(startOffset);
        }
    }
    else // container node was not a text node
    {
        _tempCurrent = _tempCurrent.firstChild();
        unsigned int i;
        for(i=0; i < startOffset; i++)
            _tempCurrent = _tempCurrent.nextSibling();

        if(contentExtract)
            _clone = _tempCurrent.cloneNode(true);
        else
            _clone = _tempCurrent;   // is this enough? Don't we have to delete the node from the original tree??
    }

    Node _tempParent;                       // we use this to traverse upwords trough the tree
    Node _cloneParent;                      // this one is used to copy the current parent
    Node _fragmentRoot;                     // this is eventually becomming the root of the DocumentFragment


    while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                    // we are in luck, all these node must be cloned because they are partially selected
        _tempParent = _tempCurrent.parentNode();

        if(_tempParent == _cmnRoot)
        {
            _cloneParent = _endFragment;
            _fragmentRoot = _tempCurrent;
        }
        else
        {
            _cloneParent = _tempParent.cloneNode(false);
            if( _tempPartial.isNull() && _tempParent != _cmnRoot )
            {
                _tempPartial = _tempParent;
                // TODO: this means we should collapse after I think... :))
            }
        }

        // we must not forget to grab with us the rest of this nodes siblings
        Node _nextCurrent;

        _tempCurrent = _tempCurrent.nextSibling();
        _cloneParent.appendChild( _tempCurrent );
        while( !_tempCurrent.isNull() )
        {
            _nextCurrent = _tempCurrent.nextSibling();
            if( !_tempCurrent.isNull() && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
                if(contentExtract)
                {
                    _cloneParent.appendChild(_tempCurrent);         // delete from old tree?
                }
                else
                {
                    _clone = _tempCurrent.cloneNode(true);
                    _cloneParent.appendChild(_clone);
                }
            }
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _tempParent;
        _clone = _cloneParent;
    }

    //****** we should now be FINISHED with startContainer **********
    _tempCurrent = endContainer;
    Node _tempEnd;
    // we still have Node _clone!!

    // Special case text is first:
    if( _tempCurrent.nodeType() == Node::TEXT_NODE )
    {
        _clone = _tempCurrent.cloneNode(false);
        _clone.nodeValue().split(endOffset);  // we need to get the SUBSTRING
        if(contentExtract)
        {
            // remove what wasn't in the substring
            endContainer.nodeValue().remove(endOffset, endContainer.nodeValue().length() - endOffset );
        }
    }
    else // container node was not a text node
    {
        if(endOffset == 0)
            _tempCurrent = endContainer;
        else
        {
            _tempCurrent = _tempCurrent.firstChild();
            unsigned int i;
            for(i=0; i< endOffset; i++)
                _tempCurrent = _tempCurrent.nextSibling();
        }
        if(contentExtract)
            _clone = _tempCurrent;
        else
            _clone = _tempCurrent.cloneNode(true);
    }




    while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                  // we are in luck, all these node must be cloned because they are partially selected
        _tempParent = _tempCurrent.parentNode();

        if(_tempParent == _cmnRoot)
        {
            _cloneParent = _endFragment;
            _fragmentRoot = _tempCurrent;
        }
        else
        {
            _cloneParent = _tempParent.cloneNode(false);
            if( _tempPartial.isNull() && _tempParent != _cmnRoot )
            {
                _tempPartial = _tempParent;
                // TODO: this means we should collapse before I think... :))
            }
        }

        // we must not forget to grab with us the rest of this nodes siblings
        Node _nextCurrent;
        Node _stopNode = _tempCurrent;
        _tempCurrent = _tempParent.firstChild();


        _cloneParent.appendChild(_clone);

        while( _tempCurrent != _stopNode && !_tempCurrent.isNull() )
        {
            _nextCurrent = _tempCurrent.nextSibling();
            if( !_tempCurrent.isNull() && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
                if(contentExtract)
                {
                    _cloneParent.appendChild(_tempCurrent);         // delete from old tree?
                }
                else
                {
                    _clone = _tempCurrent.cloneNode(true);
                    _cloneParent.appendChild(_clone);
                }
            }
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _tempParent;
        _clone = _cloneParent;
    }
    // now we should copy all the shit in between!!

    Node _clonePrevious = _endFragment.lastChild();
    _tempCurrent = _tempEnd.previousSibling();
    Node _nextCurrent;

    while( (_nextCurrent != _fragmentRoot) && (!_tempCurrent.isNull()) )
    {
        _nextCurrent = _tempCurrent.previousSibling();

        if(contentExtract)
            _clone = _tempCurrent.cloneNode(true);
        else
            _clone = _tempCurrent;

        _endFragment.insertBefore(_clone, _clonePrevious);

        _tempCurrent = _nextCurrent;
        _clonePrevious = _tempCurrent;
    }
    // WHAT ABOUT THE COLLAPSES??
    return _endFragment;
}

// ---------------------------------------------------------------

DocumentRange::DocumentRange()
{
}

DocumentRange::DocumentRange(const DocumentRange &other)
{
    impl = other.impl;
}

DocumentRange &DocumentRange::operator = (const DocumentRange &other)
{
    DocumentRange::operator = (other);
    return *this;
}

DocumentRange::~DocumentRange()
{
}

//Range DocumentRange::createRange(  )
//{
//}

