/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)

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
// KDE HTML Widget -- Tokenizers
// $Id$

#ifndef HTMLTOKENIZER_H
#define HTMLTOKENIZER_H


//
// External Classes
//
///////////////////

class KCharsets;

//
// Internal Classes
//
///////////////////

class StringTokenizer;
class HTMLTokenizer;

#include <qstring.h>

#include "stringit.h"
#include "htmltoken.h"
#include "htmlparser.h"

class KHTMLParser;
class KHTMLView;

// The count of spaces used for each tab.
#define TAB_SIZE 8

//-----------------------------------------------------------------------------

/**
 * @internal
 * This class takes QStrings as input, and splits up the input streams into
 * tokens, which are passed on to the @ref KHTMLParser.
 */
class HTMLTokenizer
{
public:
    HTMLTokenizer(KHTMLParser *, KHTMLView * = 0);
    ~HTMLTokenizer();

    void begin();
    void setPlainText();
    void write( const QString &str );
    void end();

protected:
    void reset();
    void addPending();
    void processToken();

    void addListing(khtml::DOMStringIt list);

    void parseComment(khtml::DOMStringIt &str);
    void parseText(khtml::DOMStringIt &str);
    void parseStyle(khtml::DOMStringIt &str);
    void parseScript(khtml::DOMStringIt &str);
    void parseListing(khtml::DOMStringIt &str);
    void parseTag(khtml::DOMStringIt &str);
    void parseEntity(khtml::DOMStringIt &str, bool start = false);
    void parseProcessingInstruction(khtml::DOMStringIt &str);
    
    // check if we have enough space in the buffer.
    // if not enlarge it
    void checkBuffer(int len = 10);
protected:
    // Internal buffers
    ///////////////////
    QChar *buffer;
    QChar *dest;

    Token *currToken;

    // the size of buffer
    int size;

    // Tokenizer flags
    //////////////////
    // are we in quotes within a html tag
    typedef enum
    {
        NoQuote = 0,
        SingleQuote,
        DoubleQuote
    } HTMLQuote;

    HTMLQuote tquote;

    typedef enum
    {
	NonePending = 0,
	SpacePending,
	LFPending,
	TabPending
    } HTMLPendingType;

    // To avoid multiple spaces
    HTMLPendingType pending;

    typedef enum
    {
	NoneDiscard = 0,
	SpaceDiscard,
	LFDiscard,
	AllDiscard  // discard all spaces, LF's etc until next non white char
    } HTMLDiscardType;

    // Discard line breaks immediately after start-tags
    // Discard spaces after '=' within tags
    HTMLDiscardType discard;

    // Discard the LF part of CRLF sequence
    bool skipLF;

    // Flag to say that we have the '<' but not the character following it.
    bool startTag;

    typedef enum {
	NoTag = 0,
	TagName,
	SearchAttribute,
	AttributeName,
	SearchEqual,
	SearchValue,
	QuotedValue,
	Value,
	SearchEnd
    } HTMLTagParse;

    // Flag to say, we are just parsing a tag, meaning, we are in the middle
    // of <tag...
    HTMLTagParse tag;

    // Flag to say that we are just parsing an attribute
    bool parseAttr;

    // Are we in a <title> ... </title> block
    bool title;

    // Are we in a <pre> ... </pre> block
    bool pre;

    // if 'pre == true' we track in which column we are
    int prePos;

    // Are we in a <script> ... </script> block
    bool script;

    // Are we in a <style> ... </style> block
    bool style;

    // Are we in a <select> ... </select> block
    bool select;

    // Are we in a <listing> ... </listing> block
    bool listing;

    // Are we in plain textmode ?
    bool plaintext;

    // Are we in a &... character entity description?
    bool charEntity;

    // XML processing instructions. Ignored at the moment
    bool processingInstruction;
    
    // Area we in a <!-- comment --> block
    bool comment;

    // Are we in a <textarea> ... </textarea> block
    bool textarea;

    // Used to store the code of a srcipting sequence
    QChar *scriptCode;
    // Size of the script sequenze stored in @ref #scriptCode
    int scriptCodeSize;
    // Maximal size that can be stored in @ref #scriptCode
    int scriptCodeMaxSize;

    // Stores characters if we are scanning for a string like "</script>"
    QChar searchBuffer[ 10 ];
    // Counts where we are in the string we are scanning for
    int searchCount;
    // The string we are searching for
    const QChar *searchFor;

    QChar entityBuffer[10];
    uint entityPos;

    QString _src;
    khtml::DOMStringIt src;

    KCharsets *charsets;
    KHTMLParser *parser;

    KHTMLView *view;
};

#endif // HTMLTOKENIZER

