/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Gunnstein Lye (gunnstein@netcom.no)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
 * (C) 2001 Peter Kelly (pmk@post.com)
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

#include "dom/dom2_traversal.h"
#include "dom/dom_node.h"
#include "dom/dom_doc.h"
#include "dom/dom_string.h"
#include "dom/dom_text.h"
#include "dom/dom_exception.h"
#include "dom_docimpl.h"
#include "dom2_rangeimpl.h"
#include "dom2_traversalimpl.h"
#include "dom_textimpl.h"
#include "dom_xmlimpl.h"
#include <qstring.h>

using namespace DOM;


RangeImpl::RangeImpl(DocumentImpl *_ownerDocument)
{
    m_ownerDocument = _ownerDocument;
    m_ownerDocument->ref();
    m_startContainer = _ownerDocument;
    m_startContainer->ref();
    m_endContainer = _ownerDocument;
    m_endContainer->ref();
    m_startOffset = 0;
    m_endOffset = 0;
    m_detached = false;
}

RangeImpl::RangeImpl(DocumentImpl *_ownerDocument,
	      NodeImpl *_startContainer, long _startOffset,
	      NodeImpl *_endContainer, long _endOffset)
{
    m_ownerDocument = _ownerDocument;
    m_startContainer = _startContainer;
    m_startOffset = _startOffset;
    m_endContainer = _endContainer;
    m_endOffset = _endOffset;
    m_detached = false;
}

RangeImpl::~RangeImpl()
{
    m_ownerDocument->deref();
    int exceptioncode;
    if (!m_detached)
	detach(exceptioncode);
}

NodeImpl *RangeImpl::startContainer(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return m_startContainer;
}

long RangeImpl::startOffset(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return m_startOffset;
}

NodeImpl *RangeImpl::endContainer(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return m_endContainer;
}

long RangeImpl::endOffset(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return m_endOffset;
}

NodeImpl *RangeImpl::commonAncestorContainer(int &exceptioncode)
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    NodeImpl *com = commonAncestorContainer(m_startContainer,m_endContainer);
    if (!com) //  should never happen
	exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
    return com;
}

NodeImpl *RangeImpl::commonAncestorContainer(NodeImpl *containerA, NodeImpl *containerB)
{
    NodeImpl *parentStart;

    for (parentStart = containerA; parentStart; parentStart = parentStart->parentNode()) {	
	NodeImpl *parentEnd = containerB;
        while( parentEnd && (parentStart != parentEnd) )
            parentEnd = parentEnd->parentNode();

        if(parentStart == parentEnd)  break;
    }

    return parentStart;
}

bool RangeImpl::collapsed(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return (m_startContainer == m_endContainer && m_startOffset == m_endOffset);
}

void RangeImpl::setStart( NodeImpl *refNode, long offset, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if (!refNode) {
	exceptioncode = DOMException::NOT_FOUND_ERR;
	return;
    }

    if (refNode->getDocument() != m_ownerDocument) {
	exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
	return;
    }

    checkNodeWOffset( refNode, offset, exceptioncode );
    if (exceptioncode)
	return;

    setStartContainer(refNode);
    m_startOffset = offset;

    // check if different root container
    NodeImpl *endRootContainer = m_endContainer;
    while (endRootContainer->parentNode())
	endRootContainer = endRootContainer->parentNode();
    NodeImpl *startRootContainer = m_startContainer;
    while (startRootContainer->parentNode())
	startRootContainer = startRootContainer->parentNode();
    if (startRootContainer != endRootContainer)
	collapse(true,exceptioncode);
    // check if new start after end
    else if (compareBoundaryPoints(m_startContainer,m_startOffset,m_endContainer,m_endOffset) > 0)
	collapse(true,exceptioncode);
}

void RangeImpl::setEnd( NodeImpl *refNode, long offset, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if (!refNode) {
	exceptioncode = DOMException::NOT_FOUND_ERR;
	return;
    }

    if (refNode->getDocument() != m_ownerDocument) {
	exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
	return;
    }

    checkNodeWOffset( refNode, offset, exceptioncode );
    if (exceptioncode)
	return;

    setEndContainer(refNode);
    m_endOffset = offset;

    // check if different root container
    NodeImpl *endRootContainer = m_endContainer;
    while (endRootContainer->parentNode())
	endRootContainer = endRootContainer->parentNode();
    NodeImpl *startRootContainer = m_startContainer;
    while (startRootContainer->parentNode())
	startRootContainer = startRootContainer->parentNode();
    if (startRootContainer != endRootContainer)
	collapse(false,exceptioncode);
    // check if new end before start
    if (compareBoundaryPoints(m_startContainer,m_startOffset,m_endContainer,m_endOffset) > 0)
 	collapse(false,exceptioncode);
}

void RangeImpl::collapse( bool toStart, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if( toStart )   // collapse to start
    {
        setEndContainer(m_startContainer);
        m_endOffset = m_startOffset;
    }
    else            // collapse to end
    {
        setStartContainer(m_endContainer);
        m_startOffset = m_endOffset;
    }
}

short RangeImpl::compareBoundaryPoints( Range::CompareHow how, RangeImpl *sourceRange, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    if (!sourceRange) {
	exceptioncode = DOMException::NOT_FOUND_ERR;
	return 0;
    }

    NodeImpl *thisCont = commonAncestorContainer(exceptioncode);
    NodeImpl *sourceCont = sourceRange->commonAncestorContainer(exceptioncode);
    if (exceptioncode)
	return 0;

    if (thisCont->ownerDocument() != sourceCont->ownerDocument()) { // ### what about if in separate DocumentFragments?
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return 0;
    }

    switch(how)
    {
    case Range::START_TO_START:
        return compareBoundaryPoints( m_startContainer, m_startOffset,
                                      sourceRange->startContainer(exceptioncode), sourceRange->startOffset(exceptioncode) );
        break;
    case Range::START_TO_END:
        return compareBoundaryPoints( m_startContainer, m_startOffset,
                                      sourceRange->endContainer(exceptioncode), sourceRange->endOffset(exceptioncode) );
        break;
    case Range::END_TO_END:
        return compareBoundaryPoints( m_endContainer, m_endOffset,
                                      sourceRange->endContainer(exceptioncode), sourceRange->endOffset(exceptioncode) );
        break;
    case Range::END_TO_START:
        return compareBoundaryPoints( m_endContainer, m_endOffset,
                                      sourceRange->startContainer(exceptioncode), sourceRange->startOffset(exceptioncode) );
        break;
    default:
        exceptioncode = DOMException::SYNTAX_ERR;
        return 0;
    }
}

short RangeImpl::compareBoundaryPoints( NodeImpl *containerA, long offsetA, NodeImpl *containerB, long offsetB )
{
    // see DOM2 traversal & range section 2.5

    // case 1: both points have the same container
    if( containerA == containerB )
    {
        if( offsetA == offsetB )  return 0;    // A is equal to B
        if( offsetA < offsetB )  return -1;    // A is before B
        else  return 1;                        // A is after B
    }

    // case 2: node C (container B or an ancestor) is a child node of A
    NodeImpl *c = containerB;
    while (c && c->parentNode() != containerA)
	c = c->parentNode();
    if (c) {
	int offsetC = 0;
	NodeImpl *n = n = containerA->firstChild();
	while (n != c) {
	    offsetC++;
	    n = n->nextSibling();
	}

        if( offsetA == offsetC )  return 0;    // A is equal to B
        if( offsetA < offsetC )  return -1;    // A is before B
        else  return 1;                        // A is after B
    }

    // case 3: node C (container A or an ancestor) is a child node of B
    c = containerA;
    while (c && c->parentNode() != containerB)
	c = c->parentNode();
    if (c) {
	int offsetC = 0;
	NodeImpl *n = n = containerB->firstChild();
	while (n != c) {
	    offsetC++;
	    n = n->nextSibling();
	}

        if( offsetC == offsetB )  return 0;    // A is equal to B
        if( offsetC < offsetB )  return -1;    // A is before B
        else  return 1;                        // A is after B
    }

    // case 4: containers A & B are siblings, or children of siblings
    NodeImpl *cmnRoot = commonAncestorContainer(containerA,containerB);
    NodeImpl *childA = containerA;
    while (childA->parentNode() != cmnRoot)
	childA = childA->parentNode();
    NodeImpl *childB = containerB;
    while (childB->parentNode() != cmnRoot)
	childB = childB->parentNode();
	
    NodeImpl *n = cmnRoot->firstChild();
    int i = 0;
    int childAOffset = -1;
    int childBOffset = -1;
    while (childAOffset < 0 || childBOffset < 0) {
	if (n == childA)
	    childAOffset = i;
	if (n == childB)
	    childBOffset = i;
	n = n->nextSibling();
	i++;
    }

    if( childAOffset == childBOffset )  return 0;    // A is equal to B
    if( childAOffset < childBOffset )   return -1;    // A is before B
    else  return 1;                        // A is after B
}

bool RangeImpl::boundaryPointsValid(  )
{
    short valid =  compareBoundaryPoints( m_startContainer, m_startOffset,
                                          m_endContainer, m_endOffset );
    if( valid == 1 )  return false;
    else  return true;

}

void RangeImpl::deleteContents( int &exceptioncode ) {
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    processContents(DELETE_CONTENTS,exceptioncode);
}

DocumentFragmentImpl *RangeImpl::processContents ( ActionType action, int &exceptioncode )
{
    // ### when mutation events are implemented, we will have to take into account
    // situations where the tree is being transformed while we delete - ugh!

    // ### perhaps disable node deletion notification for this range while we do this?

    if (collapsed(exceptioncode))
	return 0;
    if (exceptioncode)
	return 0;
	
    NodeImpl *cmnRoot = commonAncestorContainer(exceptioncode);
    if (exceptioncode)
	return 0;
	
    DocumentFragmentImpl *fragment = 0;
    if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS)
	fragment = new DocumentFragmentImpl(m_ownerDocument);

    // Simple case: the start and end containers are the same. We just grab
    // everything >= start offset and < end offset
    if (m_startContainer == m_endContainer) {
        if(m_startContainer->nodeType() == Node::TEXT_NODE ||
           m_startContainer->nodeType() == Node::CDATA_SECTION_NODE ||
           m_startContainer->nodeType() == Node::COMMENT_NODE) {

	    if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
		CharacterDataImpl *c = static_cast<CharacterDataImpl*>(m_startContainer->cloneNode(true,exceptioncode));
		c->deleteData(m_endOffset,static_cast<CharacterDataImpl*>(m_startContainer)->length()-m_endOffset,exceptioncode);
		c->deleteData(0,m_startOffset,exceptioncode);
		fragment->appendChild(c,exceptioncode);
	    }
	    if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS)
		static_cast<CharacterDataImpl*>(m_startContainer)->deleteData(m_startOffset,m_endOffset-m_startOffset,exceptioncode);
        }
        else if (m_startContainer->nodeType() == Node::PROCESSING_INSTRUCTION_NODE) {
            // ### operate just on data ?
        }
        else {
            NodeImpl *n = m_startContainer->firstChild();
            unsigned long i;
            for(i = 0; i < m_startOffset; i++) // skip until m_startOffset
                n = n->nextSibling();
            NodeImpl *next = n->nextSibling();
            while (n && i < m_endOffset) { // delete until m_endOffset
		if (action == EXTRACT_CONTENTS)
		    fragment->appendChild(n,exceptioncode); // will remove n from it's parent
		else if (action == CLONE_CONTENTS)
		    fragment->appendChild(n->cloneNode(true,exceptioncode),exceptioncode);
		else	
		    m_startContainer->removeChild(n,exceptioncode);		
		n = next;
		next = next->nextSibling();
		i++;
	    }
        }
        collapse(true,exceptioncode);
        return fragment;
    }

    // Complex case: Start and end containers are different.
    // There are three possiblities here:
    // 1. Start container == cmnRoot (End container must be a descendant)
    // 2. End container == cmnRoot (Start container must be a descendant)
    // 3. Neither is cmnRoot, they are both descendants
    //
    // In case 3, we grab everything after the start (up until a direct child
    // of cmnRoot) into leftContents, and everything before the end (up until
    // a direct child of cmnRoot) into rightContents. Then we process all
    // cmnRoot children between leftContents and rightContents
    //
    // In case 1 or 2, we skip either processing of leftContents or rightContents,
    // in which case the last lot of nodes either goes from the first or last
    // child of cmnRoot.
    //
    // These are deleted, cloned, or extracted (i.e. both) depending on action.

    NodeImpl *leftContents = 0;
    if (m_startContainer != cmnRoot) {
	// process the left-hand side of the range, up until the last ancestor of
	// m_startContainer before cmnRoot
	if(m_startContainer->nodeType() == Node::TEXT_NODE ||
	   m_startContainer->nodeType() == Node::CDATA_SECTION_NODE ||
	   m_startContainer->nodeType() == Node::COMMENT_NODE) {
	
	    if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
		CharacterDataImpl *c = static_cast<CharacterDataImpl*>(m_startContainer->cloneNode(true,exceptioncode));
		c->deleteData(0,m_startOffset,exceptioncode);
		leftContents = c;
	    }
	    if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS)
		static_cast<CharacterDataImpl*>(m_startContainer)->deleteData(
		    m_startOffset,static_cast<CharacterDataImpl*>(m_startContainer)->length()-m_startOffset,exceptioncode);
	}
	else if (m_startContainer->nodeType() == Node::PROCESSING_INSTRUCTION_NODE) {
	    // ### operate just on data ?
	    // leftContents = ...
	}
	else {
	    leftContents = m_startContainer->parentNode()->cloneNode(false,exceptioncode);
	    NodeImpl *n = m_startContainer->firstChild();
	    unsigned long i;
	    for(i = 0; i < m_startOffset; i++) // skip until m_startOffset
		n = n->nextSibling();
	    NodeImpl *next = n->nextSibling();
	    while (n) { // process until end
		if (action == EXTRACT_CONTENTS)
		    leftContents->appendChild(n,exceptioncode); // will remove n from m_startContainer
		else if (action == CLONE_CONTENTS)
		    leftContents->appendChild(n->cloneNode(true,exceptioncode),exceptioncode);
		else	
		    m_startContainer->removeChild(n,exceptioncode);
		n = next;
		next = next->nextSibling();
	    }
	}
	
	NodeImpl *leftParent = m_startContainer->parentNode();
	NodeImpl *n = m_startContainer->nextSibling();
	for (; leftParent != cmnRoot; leftParent = leftParent->parentNode()) {
	    NodeImpl *leftContentsParent = leftParent->cloneNode(false,exceptioncode);
	    leftContentsParent->appendChild(leftContents,exceptioncode);
	    leftContents = leftContentsParent;
	
	    NodeImpl *next;
	    for (; n; n = next ) {
		next = n->nextSibling();
		if (action == EXTRACT_CONTENTS)
		    leftContents->appendChild(n,exceptioncode); // will remove n from leftParent
		else if (action == CLONE_CONTENTS)
		    leftContents->appendChild(n->cloneNode(true,exceptioncode),exceptioncode);
		else
		    leftParent->removeChild(n,exceptioncode);
	    }
	    n = leftParent->nextSibling();
	}
    }

    NodeImpl *rightContents = 0;;
    if (m_endContainer != cmnRoot) {
	// delete the right-hand side of the range, up until the last ancestor of
	// m_endContainer before cmnRoot
	if(m_endContainer->nodeType() == Node::TEXT_NODE ||
	   m_endContainer->nodeType() == Node::CDATA_SECTION_NODE ||
	   m_endContainer->nodeType() == Node::COMMENT_NODE) {

	    if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
		CharacterDataImpl *c = static_cast<CharacterDataImpl*>(m_endContainer->cloneNode(true,exceptioncode));
		c->deleteData(m_endOffset,static_cast<CharacterDataImpl*>(m_endContainer)->length()-m_endOffset,exceptioncode);
		rightContents = c;
	    }
	    if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS)
		static_cast<CharacterDataImpl*>(m_endContainer)->deleteData(0,m_endOffset,exceptioncode);
	}
	else if (m_startContainer->nodeType() == Node::PROCESSING_INSTRUCTION_NODE) {
	    // ### operate just on data ?
	    // rightContents = ...
	}
	else {
	    rightContents = m_endContainer->parentNode()->cloneNode(false,exceptioncode);
	    NodeImpl *n = m_endContainer->firstChild();
	    unsigned long i;
	    for(i = 0; i+1 < m_endOffset; i++) // skip to m_endOffset
		n = n->nextSibling();
	    NodeImpl *prev;
	    for (; n; n = prev ) {
		prev = n->previousSibling();
		if (action == EXTRACT_CONTENTS)
		    rightContents->insertBefore(n,rightContents->firstChild(),exceptioncode); // will remove n from it's parent
		else if (action == CLONE_CONTENTS)
		    rightContents->insertBefore(n->cloneNode(true,exceptioncode),rightContents->firstChild(),exceptioncode);
		else
		    m_endContainer->removeChild(n,exceptioncode);
	    }
	}

	NodeImpl *rightParent = m_endContainer->parentNode();
	NodeImpl *n = m_endContainer->previousSibling();
	for (; rightParent != cmnRoot; rightParent = rightParent->parentNode()) {
	    NodeImpl *rightContentsParent = rightParent->cloneNode(false,exceptioncode);
	    rightContentsParent->appendChild(rightContents,exceptioncode);
	    rightContents = rightContentsParent;
	
	    NodeImpl *prev;
	    for (; n; n = prev ) {
		prev = n->previousSibling();
		if (action == EXTRACT_CONTENTS)
		    rightContents->insertBefore(n,rightContents->firstChild(),exceptioncode); // will remove n from it's parent
		else if (action == CLONE_CONTENTS)
		    rightContents->insertBefore(n->cloneNode(true,exceptioncode),rightContents->firstChild(),exceptioncode);
		else		
		    rightParent->removeChild(n,exceptioncode);
		
	    }
	    n = rightParent->previousSibling();
	}
    }

    // delete all children of cmnRoot between the start and end container

    NodeImpl *processStart; // child of cmnRooot
    if (m_startContainer == cmnRoot) {
	unsigned long i;
	processStart = m_startContainer->firstChild();
	for (i = 0; i < m_startOffset; i++)
	    processStart = processStart->nextSibling();
    }
    else {
	processStart = m_startContainer;
	while (processStart->parentNode() != cmnRoot)
	    processStart = processStart->parentNode();
	processStart = processStart->nextSibling();
    }
    NodeImpl *processEnd; // child of cmnRooot
    if (m_endContainer == cmnRoot) {
	unsigned long i;
	processEnd = m_endContainer->firstChild();
	for (i = 0; i < m_endOffset; i++)
	    processEnd = processEnd->nextSibling();
    }
    else {
	processEnd = m_endContainer;
	while (processEnd->parentNode() != cmnRoot)
	    processEnd = processEnd->parentNode();
    }

    // Now add leftContents, stuff in between, and rightContents to the fragment
    // (or just delete the stuff in between)

    if ((action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) && leftContents)
      fragment->appendChild(leftContents,exceptioncode);

    NodeImpl *next;
    NodeImpl *n;
    if (processStart) {
	for (n = processStart; n && n != processEnd; n = next) {
	    next = n->nextSibling();
	
	    if (action == EXTRACT_CONTENTS)
		fragment->appendChild(n,exceptioncode); // will remove from cmnRoot
	    else if (action == CLONE_CONTENTS)
		fragment->appendChild(n->cloneNode(true,exceptioncode),exceptioncode);
	    else
		cmnRoot->removeChild(n,exceptioncode);
	}
    }

    if ((action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) && rightContents)
      fragment->appendChild(rightContents,exceptioncode);

    // ### collapse to the proper position
    collapse(true,exceptioncode);
    return fragment;
}


DocumentFragmentImpl *RangeImpl::extractContents( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return processContents(EXTRACT_CONTENTS,exceptioncode);
}

DocumentFragmentImpl *RangeImpl::cloneContents( int &exceptioncode  )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return processContents(CLONE_CONTENTS,exceptioncode);
}

void RangeImpl::insertNode( NodeImpl *newNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if( newNode->nodeType() == Node::ATTRIBUTE_NODE ||
        newNode->nodeType() == Node::ENTITY_NODE ||
        newNode->nodeType() == Node::NOTATION_NODE ||
        newNode->nodeType() == Node::DOCUMENT_NODE ||
        newNode->nodeType() == Node::DOCUMENT_FRAGMENT_NODE) {
        exceptioncode = RangeException::INVALID_NODE_TYPE_ERR + RangeException::_EXCEPTION_OFFSET;
        return;
    }

    if( newNode->ownerDocument() != m_startContainer->ownerDocument() ) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return;
    }

    if( m_startContainer->nodeType() == Node::TEXT_NODE )
    {
        TextImpl *newText;
        NodeImpl *newParent = newNode->parentNode();
        TextImpl *textNode = static_cast<TextImpl*>(m_startContainer);
        newText = textNode->splitText(m_startOffset,exceptioncode);
        if (exceptioncode)
	    return;
        newParent->insertBefore( newNode, newText, exceptioncode );
    }
    else {
        m_startContainer->insertBefore( newNode, m_startContainer->childNodes()->item( m_startOffset ), exceptioncode );
    }
}

DOMString RangeImpl::toString( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    // ###
/*    NodeIteratorImpl iterator( m_startContainer.childNodes().item( m_startOffset ) );
    DOMString _string;
    Node _node = iterator.nextNode();

    while( !_node.isNull() )
    {
        printf( "\nNodetype: %s\n", _node.nodeName().string().ascii() );
        if( _node.nodeType() == Node::TEXT_NODE )
        {
            QString str = _node.nodeValue().string();
            if( _node == m_startContainer && _node == m_endContainer )
                _string = str.mid( m_startOffset, m_endOffset - m_startOffset );
            else if( _node == m_startContainer )
                _string = str.mid( m_startOffset );
            else if( _node == m_endContainer )
                _string += str.left( m_startOffset );
            else
                _string += str;
        }
        else if( _node.nodeName() == "BR" )  _string += "\n";
        else if( _node.nodeName() == "P" || _node.nodeName() == "TD" )  _string += "\n\n";
        else  _string += " ";

        _node = iterator.nextNode();
    }
    return _string;*/
    return DOMString("");
}

DOMString RangeImpl::toHTML(  )
{
    // ### implement me!!!!

    // this is just to avoid compiler warnings
    DOMString d;
    return d;
}

void RangeImpl::detach( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if (m_startContainer)
	m_startContainer->deref();
    m_startContainer = 0;
    if (m_endContainer)
	m_endContainer->deref();
    m_endContainer = 0;
    m_detached = true;
}

bool RangeImpl::isDetached() const
{
    return m_detached;
}

DocumentFragmentImpl *RangeImpl::masterTraverse(bool contentExtract, int &exceptioncode)
{
    /* function description easy case, m_startContainer == m_endContainer
     * If we have a text node simply clone/extract the contents between
     * start & end and put them into the fragment
     * If we don't have a text node, find the offset and copy/clone the content
     * between the two offsets
     * We end with returning the fragment of course
     */
    NodeImpl *_clone;
    DocumentFragmentImpl *_endFragment = m_ownerDocument->createDocumentFragment(); // ### check this gets deleted where necessary

    if(m_startContainer == m_endContainer)
    {
        if(m_startOffset == m_endOffset)            // we have a collapsed range
            return 0;

        // TODO: we need to delete the text Node if a whole text is selected!!
        if( m_startContainer->nodeType() == Node::TEXT_NODE )    // we have a text node.. special :)
        {
            _clone = m_startContainer->cloneNode(false,exceptioncode);
            if (exceptioncode)
		return 0;
            _clone->nodeValue().remove(0, m_startOffset);  // we need to get the SUBSTRING
            _clone->nodeValue().remove(m_endOffset, _clone->nodeValue().length() - m_endOffset);
            if(contentExtract)
            {
                // full trim :)
                m_startContainer->nodeValue().remove(m_startOffset, m_endOffset - m_startOffset);
            }
            _endFragment->appendChild(_clone,exceptioncode);
            if (exceptioncode)
		return 0;
        }
        else  // we have the same container class but we are not a text node
        {
            NodeImpl *_tempCurrent = m_startContainer->firstChild();
            unsigned int i;

            for(i=0; i < m_startOffset; i++)    // get the node given by the offset
                _tempCurrent = _tempCurrent->nextSibling();

            /* now copy (or move) all the nodes in the range into the document fragment */
            unsigned int range = m_endOffset - m_startOffset;
            NodeImpl *_nextCurrent = _tempCurrent;                  // to keep track of which node to take next
            for(i=0; i<range && _tempCurrent; i++)   // check of isNull in case of strange errors
            {
                _nextCurrent = _tempCurrent->nextSibling();

                if(contentExtract)
                {
                    _endFragment->appendChild(_tempCurrent,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
                else
                {
                    _clone = _tempCurrent->cloneNode(true,exceptioncode);
                    if (exceptioncode)
			return 0;
                    _endFragment->appendChild(_clone,exceptioncode);
                    if (exceptioncode)
			return 0;
                }

                _tempCurrent = _nextCurrent;
            }
        }
        return _endFragment;
    }// END COMMON CONTAINER HERE!!!


    /* Ok here we go for the harder part, first a general desription:
     * First we copy all the border nodes (the have to be copied as long
     * as they are partially selected) from the m_startContainer to the CmnAContainer. Then we do
     * the same for the m_endContainer. After this we add all fully selected
     * nodes that are between these two!
     */

    NodeImpl *_cmnRoot = commonAncestorContainer(exceptioncode);
    NodeImpl *_tempCurrent = m_startContainer;
    NodeImpl *_tempPartial = 0;
    // we still have Node _clone!!

    // Special case text is first:
    if( _tempCurrent->nodeType() == Node::TEXT_NODE )
    {
        _clone = _tempCurrent->cloneNode(false,exceptioncode);
        if (exceptioncode)
	    return 0;
        _clone->nodeValue().remove(0, m_startOffset);
        if(contentExtract)
        {
            m_startContainer->nodeValue().split(m_startOffset);
        }
    }
    else
    {
        _tempCurrent = _tempCurrent->firstChild();
        unsigned int i;
        for(i=0; i < m_startOffset; i++)
            _tempCurrent = _tempCurrent->nextSibling();

        if(contentExtract) {
            _clone = _tempCurrent->cloneNode(true,exceptioncode);
            if (exceptioncode)
		return 0;
        }
        else
            _clone = _tempCurrent;
    }

    NodeImpl *_tempParent;                       // we use this to traverse upwords trough the tree
    NodeImpl *_cloneParent;                      // this one is used to copy the current parent
    NodeImpl *_fragmentRoot = 0;                 // this is eventually becomming the root of the DocumentFragment


    while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                    // all these node must be cloned as they are partially selected
        _tempParent = _tempCurrent->parentNode();

        if(_tempParent == _cmnRoot)
        {
            _cloneParent = _endFragment;
            _fragmentRoot = _tempCurrent;
        }
        else
        {
            _cloneParent = _tempParent->cloneNode(false,exceptioncode);
            if (exceptioncode)
		return 0;
            if( !_tempPartial && _tempParent != _cmnRoot )
            {
                _tempPartial = _tempParent;
                // TODO: this means we should collapse after I think... :))
            }
        }

        // we must not forget to grab with us the rest of this nodes siblings
        NodeImpl *_nextCurrent;

        _tempCurrent = _tempCurrent->nextSibling();
        _cloneParent->appendChild( _clone, exceptioncode );
        if (exceptioncode)
	    return 0;
        while( _tempCurrent )
        {
            _nextCurrent = _tempCurrent->nextSibling();
            if( _tempCurrent && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
                if(contentExtract)
                {
                    _cloneParent->appendChild(_tempCurrent,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
                else
                {
                    _clone = _tempCurrent->cloneNode(true,exceptioncode);
                    if (exceptioncode)
			return 0;
                    _cloneParent->appendChild(_clone,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
            }
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _tempParent;
        _clone = _cloneParent;
    }

    //****** we should now be FINISHED with m_startContainer **********
    _tempCurrent = m_endContainer;
    NodeImpl *_tempEnd = 0;
    // we still have Node _clone!!

    // Special case text is first:
    if( _tempCurrent->nodeType() == Node::TEXT_NODE )
    {
        _clone = _tempCurrent->cloneNode(false,exceptioncode);
        if (exceptioncode)
	    return 0;
        _clone->nodeValue().split(m_endOffset);
        if(contentExtract)
        {
            m_endContainer->nodeValue().remove(m_endOffset, m_endContainer->nodeValue().length() - m_endOffset );
        }
    }
    else
    {
        if(m_endOffset == 0)
            _tempCurrent = m_endContainer;
        else
        {
            _tempCurrent = _tempCurrent->firstChild();
            unsigned int i;
            for(i=0; i< m_endOffset; i++)
                _tempCurrent = _tempCurrent->nextSibling();
        }
        if(contentExtract)
            _clone = _tempCurrent;
        else {
            _clone = _tempCurrent->cloneNode(true,exceptioncode);
            if (exceptioncode)
		return 0;
        }
    }




    while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                  //  all these node must be cloned as they are partially selected
        _tempParent = _tempCurrent->parentNode();

        if(_tempParent == _cmnRoot)
        {
            _cloneParent = _endFragment;
            _fragmentRoot = _tempCurrent;
        }
        else
        {
            _cloneParent = _tempParent->cloneNode(false,exceptioncode);
            if (exceptioncode)
		return 0;
            if( !_tempPartial && _tempParent != _cmnRoot )
            {
                _tempPartial = _tempParent;
                // ### TODO: this means we should collapse before I think... :))
            }
        }

        // we must not forget to grab with us the rest of this nodes siblings
        NodeImpl *_nextCurrent;
        NodeImpl *_stopNode = _tempCurrent;
        _tempCurrent = _tempParent->firstChild();


        _cloneParent->appendChild(_clone,exceptioncode);
        if (exceptioncode)
	    return 0;

        while( _tempCurrent != _stopNode && _tempCurrent )
        {
            _nextCurrent = _tempCurrent->nextSibling();
            if( _tempCurrent && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
                if(contentExtract)
                {
                    _cloneParent->appendChild(_tempCurrent,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
                else
                {
                    _clone = _tempCurrent->cloneNode(true,exceptioncode);
                    if (exceptioncode)
			return 0;
                    _cloneParent->appendChild(_clone,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
            }
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _tempParent;
        _clone = _cloneParent;
    }
    // To end the balade we grab with us any nodes that are between the two topmost parents under
    // the commonRoot

    NodeImpl *_clonePrevious = _endFragment->lastChild();
    _tempCurrent = _tempEnd->previousSibling(); // ### _tempEnd is always 0
    NodeImpl *_nextCurrent = 0;

    while( (_nextCurrent != _fragmentRoot) && _tempCurrent )
    {
        _nextCurrent = _tempCurrent->previousSibling();

        if(contentExtract) {
            _clone = _tempCurrent->cloneNode(true,exceptioncode);
            if (exceptioncode)
		return 0;
        }
        else
            _clone = _tempCurrent;

        _endFragment->insertBefore(_clone, _clonePrevious, exceptioncode);
        if (exceptioncode)
	    return 0;

        _tempCurrent = _nextCurrent;
        _clonePrevious = _tempCurrent;
    }
    // WHAT ABOUT THE COLLAPSES??
    return _endFragment;
}


void RangeImpl::checkCommon(int &exceptioncode) const
{
    if( m_detached )
        exceptioncode = DOMException::INVALID_STATE_ERR;
}

void RangeImpl::checkNode( const NodeImpl *n, int &exceptioncode ) const
{
    checkCommon(exceptioncode);
    if (exceptioncode)
	return;

    if( !n ) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return;
    }

    const NodeImpl *_tempNode = n;
    while( _tempNode )
    {
        if( _tempNode->nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode->nodeType() == Node::ENTITY_NODE ||
            _tempNode->nodeType() == Node::NOTATION_NODE ||
            _tempNode->nodeType() == Node::DOCUMENT_TYPE_NODE ) {
            exceptioncode = RangeException::INVALID_NODE_TYPE_ERR + RangeException::_EXCEPTION_OFFSET;
            return;
        }

        _tempNode = _tempNode->parentNode();
    }
}

void RangeImpl::checkNodeWOffset( NodeImpl *n, int offset, int &exceptioncode) const
{
    checkNode( n, exceptioncode );
    if (exceptioncode)
	return;

    if( offset < 0 ) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
    }

    switch (n->nodeType()) {
	case Node::TEXT_NODE:
	case Node::COMMENT_NODE:
	case Node::CDATA_SECTION_NODE:
	    if ( (unsigned long)offset > static_cast<CharacterDataImpl*>(n)->length() )
		exceptioncode = DOMException::INDEX_SIZE_ERR;
	    break;
	case Node::PROCESSING_INSTRUCTION_NODE:
	    // ### are we supposed to check with just data or the whole contents?
	    if ( (unsigned long)offset > static_cast<ProcessingInstructionImpl*>(n)->data().length() )
		exceptioncode = DOMException::INDEX_SIZE_ERR;
	    break;
	default:
	    NodeListImpl *cn = n->childNodes();
	    if ( (unsigned long)offset >= cn->length() )
		exceptioncode = DOMException::INDEX_SIZE_ERR;
	    delete cn;
	    break;
    }
}

void RangeImpl::checkNodeBA( const NodeImpl *n, int &exceptioncode ) const
{
    checkNode( n, exceptioncode );
    if (exceptioncode)
	return;

    if( n->nodeType() == Node::DOCUMENT_NODE ||
        n->nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        n->nodeType() == Node::ATTRIBUTE_NODE ||
        n->nodeType() == Node::ENTITY_NODE ||
        n->nodeType() == Node::NOTATION_NODE )
        exceptioncode = RangeException::INVALID_NODE_TYPE_ERR  + RangeException::_EXCEPTION_OFFSET;

}

RangeImpl *RangeImpl::cloneRange(int &exceptioncode)
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return new RangeImpl(m_ownerDocument,m_startContainer,m_startOffset,m_endContainer,m_endOffset);
}

void RangeImpl::setStartAfter( NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if (!refNode) {
	exceptioncode = DOMException::NOT_FOUND_ERR;
	return;
    }

    if (refNode->getDocument() != m_ownerDocument) {
	exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setStart( refNode->parentNode(), refNode->index()+1, exceptioncode );
}

void RangeImpl::setEndBefore( NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if (!refNode) {
	exceptioncode = DOMException::NOT_FOUND_ERR;
	return;
    }

    if (refNode->getDocument() != m_ownerDocument) {
	exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setEnd( refNode->parentNode(), refNode->index(), exceptioncode );
}

void RangeImpl::setEndAfter( NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if (!refNode) {
	exceptioncode = DOMException::NOT_FOUND_ERR;
	return;
    }

    if (refNode->getDocument() != m_ownerDocument) {
	exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setEnd( refNode->parentNode(), refNode->index()+1, exceptioncode );

}

void RangeImpl::selectNode( NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setStartBefore( refNode, exceptioncode );
    if (exceptioncode)
	return;
    setEndAfter( refNode, exceptioncode );
}

void RangeImpl::selectNodeContents( NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNode( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setStartBefore( refNode->firstChild(), exceptioncode );
    if (exceptioncode)
	return;
    setEndAfter( refNode->lastChild(), exceptioncode );
}

void RangeImpl::surroundContents( NodeImpl *newParent, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if( !newParent )
        return; // ### are we supposed to throw an exception here?

    NodeImpl *start = m_startContainer;
    if( newParent->ownerDocument() != start->ownerDocument() ) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return;
    }

    if( newParent->nodeType() == Node::ATTRIBUTE_NODE ||
        newParent->nodeType() == Node::ENTITY_NODE ||
        newParent->nodeType() == Node::NOTATION_NODE ||
        newParent->nodeType() == Node::DOCUMENT_TYPE_NODE ||
        newParent->nodeType() == Node::DOCUMENT_NODE ||
        newParent->nodeType() == Node::DOCUMENT_FRAGMENT_NODE) {
        exceptioncode = RangeException::INVALID_NODE_TYPE_ERR + RangeException::_EXCEPTION_OFFSET;
        return;
    }

    // ### revisit: if you set a range without optimizing it (trimming) the following exception might be
    // thrown incorrectly
    NodeImpl *realStart = (start->nodeType() == Node::TEXT_NODE)? start->parentNode() : start;
    NodeImpl *end = m_endContainer;
    NodeImpl *realEnd = (end->nodeType() == Node::TEXT_NODE)? end->parentNode() : end;
    if( realStart != realEnd ) {
        exceptioncode = RangeException::BAD_BOUNDARYPOINTS_ERR + RangeException::_EXCEPTION_OFFSET;
        return;
    }

    DocumentFragmentImpl *fragment = extractContents(exceptioncode);
    if (exceptioncode)
	return;
    insertNode( newParent, exceptioncode );
    if (exceptioncode)
	return;
    // BIC: to avoid this const_cast newParent shouldn't be const
    //(const_cast<Node>(newParent)).appendChild( fragment );
    ((NodeImpl*)(newParent))->appendChild( fragment, exceptioncode );
    if (exceptioncode)
	return;
    selectNode( newParent, exceptioncode );
}

void RangeImpl::setStartBefore( NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if (!refNode) {
	exceptioncode = DOMException::NOT_FOUND_ERR;
	return;
    }

    if (refNode->getDocument() != m_ownerDocument) {
	exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setStart( refNode->parentNode(), refNode->index(), exceptioncode );
}

void RangeImpl::setStartContainer(NodeImpl *_startContainer)
{
    if (m_startContainer == _startContainer)
	return;
	
    if (m_startContainer)
	m_startContainer->deref();
    m_startContainer = _startContainer;
    if (m_startContainer)
	m_startContainer->ref();
}

void RangeImpl::setEndContainer(NodeImpl *_endContainer)
{
    if (m_endContainer == _endContainer)
	return;
	
    if (m_endContainer)
	m_endContainer->deref();
    m_endContainer = _endContainer;
    if (m_endContainer)
	m_endContainer->ref();
}
