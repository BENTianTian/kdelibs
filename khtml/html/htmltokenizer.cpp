/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)
              (C) 1999 Lars Knoll (knoll@kde.org)
              (C) 1999 Antti Koivisto (koivisto@kde.org)
              (C) 2001 Dirk Mueller (mueller@kde.org)

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
// KDE HTML Widget - Tokenizers
// $Id$

//#define TOKEN_DEBUG 1
//#define TOKEN_DEBUG 2
//#define TOKEN_PRINT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "htmltokenizer.h"
#include "misc/loader.h"
#include "khtmlview.h"
#include "khtml_part.h"
#include "htmlparser.h"
#include "html_documentimpl.h"
#include "css/csshelper.h"
#include "dtd.h"
#include "htmlhashes.h"
#include <kcharsets.h>
#include <kglobal.h>
#include <ctype.h>
#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>

#include "kjs.h"
#include "kentities.c"

using namespace khtml;

static const QChar commentStart [] = { '<','!','-','-', QChar::null };
static const QChar scriptEnd [] = { '<','/','s','c','r','i','p','t','>', QChar::null };
static const QChar styleEnd [] = { '<','/','s','t','y','l','e','>', QChar::null };
static const QChar listingEnd [] = { '<','/','l','i','s','t','i','n','g','>', QChar::null };
static const QChar textareaEnd [] = { '<','/','t','e','x','t','a','r','e','a','>', QChar::null };

#define KHTML_ALLOC_QCHAR_VEC( N ) (QChar*) malloc( sizeof(QChar)*( N ) )
#define KHTML_REALLOC_QCHAR_VEC(P, N ) (QChar*) P = realloc(p, sizeof(QChar)*( N ))
#define KHTML_DELETE_QCHAR_VEC( P ) free((char*)( P ))

// Partial support for MS Windows Latin-1 extensions
// full list http://www.bbsinc.com/iso8859.html
// There may be better equivalents
#define fixUpChar(x) \
            if (!(x).row() ) { \
                switch ((x).cell()) \
                { \
                case 0x82: (x) = ','; break; \
                case 0x84: (x) = '"'; break; \
                case 0x8b: (x) = '<'; break; \
                case 0x9b: (x) = '>'; break; \
                case 0x91: (x) = '\''; break; \
                case 0x92: (x) = '\''; break; \
                case 0x93: (x) = '"'; break; \
                case 0x94: (x) = '"'; break; \
                case 0x95: (x) = '*'; break; \
                case 0x96: (x) = '-'; break; \
                case 0x97: (x) = '-'; break; \
                case 0x98: (x) = '~'; break; \
                case 0xb7: (x) = '*'; break; \
                default: break; \
                } \
            }

// ----------------------------------------------------------------------------

HTMLTokenizer::HTMLTokenizer(DOM::DocumentPtr *_doc, KHTMLView *_view)
{
    view = _view;
    buffer = 0;
    scriptCode = 0;
    scriptCodeSize = scriptCodeMaxSize = 0;
    charsets = KGlobal::charsets();
    parser = new KHTMLParser(_view, _doc);
    cachedScript = 0;
    m_executingScript = 0;
    onHold = false;

    reset();
}

HTMLTokenizer::HTMLTokenizer(DOM::DocumentPtr *_doc, DOM::DocumentFragmentImpl *i)
{
    view = 0;
    buffer = 0;
    scriptCode = 0;
    scriptCodeSize = scriptCodeMaxSize = 0;
    charsets = KGlobal::charsets();
    parser = new KHTMLParser( i, _doc );
    cachedScript = 0;
    m_executingScript = 0;
    onHold = false;

    reset();
}

void HTMLTokenizer::reset()
{
    assert(m_executingScript == 0);
    assert(onHold == false);
    if (cachedScript)
        cachedScript->deref(this);
    cachedScript = 0;

    if ( buffer )
        KHTML_DELETE_QCHAR_VEC(buffer);
    buffer = dest = 0;
    size = 0;

    if ( scriptCode )
        KHTML_DELETE_QCHAR_VEC(scriptCode);
    scriptCode = 0;
    scriptCodeSize = scriptCodeMaxSize = 0;

    currToken.reset();
}

void HTMLTokenizer::begin()
{
    m_executingScript = 0;
    onHold = false;
    reset();
    size = 254;
    buffer = KHTML_ALLOC_QCHAR_VEC( 255 );
    dest = buffer;
    tag = NoTag;
    pending = NonePending;
    discard = NoneDiscard;
    pre = false;
    prePos = 0;
    plaintext = 0;
    listing = false;
    processingInstruction = false;
    script = false;
    escaped = false;
    style = false;
    skipLF = false;
    select = false;
    comment = false;
    textarea = false;
    startTag = false;
    tquote = NoQuote;
    searchCount = 0;
    Entity = NoEntity;
    loadingExtScript = false;
    scriptSrc = "";
    pendingSrc = "";
    noMoreData = false;
    brokenComments = false;
}

void HTMLTokenizer::addListing(DOMStringIt list)
{
    bool old_pre = pre;
    // This function adds the listing 'list' as
    // preformatted text-tokens to the token-collection
    // thereby converting TABs.
    if(!style) pre = true;
    prePos = 0;

    while ( list.length() )
    {
        checkBuffer();

        if (skipLF && ( list[0] != '\n' ))
        {
            skipLF = false;
        }

        if (skipLF)
        {
            skipLF = false;
            ++list;
        }
        else if (( list[0] == '\n' ) || ( list[0] == '\r' ))
        {
            if (discard == LFDiscard)
            {
                // Ignore this LF
                discard = NoneDiscard; // We have discarded 1 LF
            }
            else
            {
                // Process this LF
                if (pending)
                    addPending();
                pending = LFPending;
            }
            /* Check for MS-DOS CRLF sequence */
            if (list[0] == '\r')
            {
                skipLF = true;
            }
            ++list;
        }
        else if (( list[0] == ' ' ) || ( list[0] == '\t'))
        {
            if (pending)
                addPending();
            if (list[0] == ' ')
                pending = SpacePending;
            else
                pending = TabPending;
            ++list;
        }
        else
        {
            discard = NoneDiscard;
            if (pending)
                addPending();

            prePos++;
            *dest++ = list[0];
            ++list;
        }

    }

    if ((pending == SpacePending) || (pending == TabPending))
    {
        addPending();
    }
    pending = NonePending;

    processToken();
    prePos = 0;

    pre = old_pre;
}

void HTMLTokenizer::parseListing( DOMStringIt &src)
{
    // We are inside a <script>, <style>, <textarea> . Look for the end tag
    // which is either </script>, </style> , </textarea> or -->
    // otherwise print out every received character
    if (Entity) {
        checkScriptBuffer();
        QChar *scriptCodeDest = scriptCode+scriptCodeSize;
        parseEntity(src,scriptCodeDest);
        scriptCodeSize = scriptCodeDest-scriptCode;
    }


#ifdef TOKEN_DEBUG
    kdDebug( 6036 ) << "HTMLTokenizer::parseListing()" << endl;
#endif

    bool doScriptExec = false;
    while ( src.length() )
    {
        // do we need to enlarge the buffer?
        checkBuffer();

        // Allocate memory to store the script. We will write maximal
        // 10 characers.
        checkScriptBuffer();

        char ch = src[0].latin1();
        if ( script && !escaped && ch == '\\' )
            escaped = true;

        if ( (!script || tquote == NoQuote) && !escaped && ( ch == '>' ) && ( searchFor[ searchCount ] == '>'))
        {
            ++src;
            searchCount = 0;
            scriptCode[ scriptCodeSize ] = 0;
            scriptCode[ scriptCodeSize + 1 ] = 0;
            if (script)
            {
                if (!scriptSrc.isEmpty()) {
                    // forget what we just got; load from src url instead
                    cachedScript = parser->doc()->docLoader()->requestScript(scriptSrc, parser->doc()->baseURL(), scriptSrcCharset);
                }
                else {
#ifdef TOKEN_DEBUG
                    kdDebug( 6036 ) << "---START SCRIPT---" << endl;
                    kdDebug( 6036 ) << QString(scriptCode, scriptCodeSize) << endl;
                    kdDebug( 6036 ) << "---END SCRIPT---" << endl;
#endif
                    // Parse scriptCode containing <script> info
                    doScriptExec = true;
                }
                processToken();
            }
            else if (style)
            {
#ifdef TOKEN_DEBUG
                kdDebug( 6036 ) << "---START STYLE---" << endl;
                kdDebug( 6036 ) << QString(scriptCode, scriptCodeSize) << endl;
                kdDebug( 6036 ) << "---END STYLE---" << endl;
#endif
                // just add it. The style element will get a DOM::TextImpl passed, which it will
                // convert into a StyleSheet.
                addListing(DOMStringIt(scriptCode, scriptCodeSize));
            }
            else
            {
                //
                // Add scriptcode to the buffer
                addListing(DOMStringIt(scriptCode, scriptCodeSize));
            }
            if(script)
                currToken.id = ID_SCRIPT + ID_CLOSE_TAG;
            else if(style)
                currToken.id = ID_STYLE + ID_CLOSE_TAG;
            else if (textarea)
                currToken.id = ID_TEXTAREA + ID_CLOSE_TAG;
            else
                currToken.id = ID_LISTING + ID_CLOSE_TAG;
            processToken();
            if (cachedScript) {
//                 qDebug( "cachedscript extern!" );
//                 qDebug( "src: *%s*", QString( src.current(), src.length() ).latin1() );
//                 qDebug( "pending: *%s*", pendingSrc.latin1() );
                pendingSrc.prepend( QString(src.current(), src.length() ) );
                _src = QString::null;
                src = DOMStringIt(_src);
                scriptCodeSize = 0;
                cachedScript->ref(this);
                // will be 0 if script was already loaded and ref() executed it
                if (cachedScript) {
                    loadingExtScript = true;
                }
            }
            else if (view && doScriptExec && javascript && !parser->skipMode()) {
                pendingSrc.prepend( QString( src.current(), src.length() ) ); // deep copy - again
                _src = QString::null;
                src = DOMStringIt( _src );
                m_executingScript++;
                script = false;
                QString exScript( scriptCode, scriptCodeSize ); // deep copy
                scriptCodeSize = 0;
                view->part()->executeScript(exScript);
                script = true;
                m_executingScript--;
            }
            script = style = listing = textarea = false;
            scriptCodeSize = 0;
            if ( !m_executingScript && !loadingExtScript )
                addPendingSource();

            return; // Finished parsing script/style/listing
        }
        // Find out wether we see an end tag without looking at
        // any other then the current character, since further characters
        // may still be on their way thru the web!
        else if ( searchCount > 0 )
        {
            const QChar& cmp = src[0];
            // be tolerant: skip spaces before the ">", i.e "</script >"
            if (!escaped &&  cmp.isSpace() && searchFor[searchCount].latin1() == '>')
            {
                ++src;
            }
            else if (!escaped && searchFor[searchCount] != QChar::null && cmp.lower() == searchFor[ searchCount ] )
            {
                searchBuffer[ searchCount++ ] = cmp;
                ++src;
            }
            // We were wrong => print all buffered characters and the current one;
            else
            {
                searchBuffer[ searchCount ] = 0;
                DOMStringIt pit(searchBuffer,searchCount);
                while (pit.length()) {
                    if (textarea && pit[0] == '&') {
                        QChar *scriptCodeDest = scriptCode+scriptCodeSize;
                        ++pit;
                        parseEntity(pit,scriptCodeDest,true);
                        scriptCodeSize = scriptCodeDest-scriptCode;
                    }
                    else {
                        scriptCode[ scriptCodeSize++ ] = pit[0];
                        ++pit;
                    }
                }
                searchCount = 0;
            }
        }
        // Is this perhaps the start of the </script> or </style> tag?
        else if ( !escaped && ( ch == '<' || ch == '-' ) )
        {
            searchCount = 1;
            searchBuffer[ 0 ] = src[0];
            ++src;
        }
        else
        {
            if (textarea && !escaped && ch == '&') {
                QChar *scriptCodeDest = scriptCode+scriptCodeSize;
                ++src;
                parseEntity(src,scriptCodeDest,true);
                scriptCodeSize = scriptCodeDest-scriptCode;
            }
            else {
                if ( !escaped ) {
                    if(script && ch == '\"')
                        tquote = (tquote == NoQuote) ? DoubleQuote : ((tquote == SingleQuote) ? SingleQuote : NoQuote);
                    else if(script && ch == '\'')
                        tquote = (tquote == NoQuote) ? SingleQuote : (tquote == DoubleQuote) ? DoubleQuote : NoQuote;
                    else if (script && tquote != NoQuote && (ch == '\r' || ch == '\n'))
                        tquote = NoQuote;
                }

                scriptCode[ scriptCodeSize++ ] = src[0];
                ++src;
                escaped = false;
            }
        }
    }
}

void HTMLTokenizer::parseScript(DOMStringIt &src)
{
    parseListing(src);
}
void HTMLTokenizer::parseStyle(DOMStringIt &src)
{
    parseListing(src);
}

void HTMLTokenizer::parseComment(DOMStringIt &src)
{
#ifdef TOKEN_DEBUG
    kdDebug( 6036 ) << "HTMLTokenizer::parseComment()" << endl;
#endif

    checkScriptBuffer(src.length());
    while ( src.length() )
    {
        if (src->unicode() == '>' &&
            ( brokenComments ||
              ( scriptCodeSize >= 2 && scriptCode[scriptCodeSize-2] == '-' &&
                scriptCode[scriptCodeSize-1] == '-' ) ) )
        {
            ++src;
#ifdef COMMENTS_IN_DOM
            checkScriptBuffer();
            scriptCode[ scriptCodeSize ] = 0;
            scriptCode[ scriptCodeSize + 1 ] = 0;
            currToken.id = ID_COMMENT;
            addListing(DOMStringIt(scriptCode, scriptCodeSize - 2));
            processToken();
            currToken.id = ID_COMMENT + ID_CLOSE_TAG;
            processToken();
#endif
            comment = false;
            scriptCodeSize = 0;
            return; // Finished parsing comment
        }

        scriptCode[ scriptCodeSize++ ] = src[0];
        ++src;
    }
}

void HTMLTokenizer::parseProcessingInstruction(DOMStringIt &src)
{
    while ( src.length() )
    {
        char chbegin = src[0].latin1();
        if(chbegin == '\'') {
            tquote = tquote == SingleQuote ? NoQuote : SingleQuote;
        }
        else if(chbegin == '\"') {
            tquote = tquote == DoubleQuote ? NoQuote : DoubleQuote;
        }
        // Look for '?>'
        // some crappy sites omit the "?" before it, so
        // we look for an unquoted '>' instead. (IE compatible)
        else if ( !tquote && chbegin == '>' )
        {
            // We got a '?>' sequence
            processingInstruction = false;
            ++src;
            discard=LFDiscard;
            return; // Finished parsing comment!
        }
        ++src;
    }
}

void HTMLTokenizer::parseText(DOMStringIt &src)
{
    while ( src.length() )
    {
        // do we need to enlarge the buffer?
        checkBuffer();

        // ascii is okay because we only do ascii comparisons
        char chbegin = src[0].latin1();

        if (skipLF && ( chbegin != '\n' ))
        {
            skipLF = false;
        }

        if (skipLF)
        {
            skipLF = false;
            ++src;
        }
        else if (( chbegin == '\n' ) || ( chbegin == '\r' ))
        {
            processToken();

            /* Check for MS-DOS CRLF sequence */
            if (chbegin == '\r')
            {
                skipLF = true;
            }
            ++src;
        }
        else
        {
            *dest++ = src[0];
            ++src;
        }
    }
}


void HTMLTokenizer::parseEntity(DOMStringIt &src, QChar *&dest, bool start)
{
    if( start )
    {
        cBufferPos = 0;
        Entity = SearchEntity;
    }

    while( src.length() )
    {
        ushort cc = src[0].unicode();
        switch(Entity) {
        case NoEntity:
            return;

            break;
        case SearchEntity:
            if(cc == '#') {
                cBuffer[cBufferPos++] = cc;
                ++src;
                Entity = NumericSearch;
            }
            else
                Entity = EntityName;

            break;

        case NumericSearch:
            if(cc == 'x' || cc == 'X') {
                cBuffer[cBufferPos++] = cc;
                ++src;
                Entity = Hexadecimal;
            }
            else if(cc >= '0' && cc <= '9')
                Entity = Decimal;
            else
                Entity = SearchSemicolon;

            break;

        case Hexadecimal:
        {
            int uc = EntityChar.unicode();
            int ll = kMin(src.length(), 9-cBufferPos);
            while(ll--) {
                QChar csrc(src[0].lower());
                cc = csrc.cell();

                if(csrc.row() || !((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f'))) {
                    Entity = SearchSemicolon;
                    break;
                }
                uc = uc*16 + (cc - ( cc < 'a' ? '0' : 'a' - 10));
                cBuffer[cBufferPos++] = cc;
                ++src;
            }
            EntityChar = QChar(uc);
            if(cBufferPos == 9) Entity = SearchSemicolon;
            break;
        }
        case Decimal:
        {
            int uc = EntityChar.unicode();
            int ll = kMin(src.length(), 9-cBufferPos);
            while(ll--) {
                cc = src[0].cell();

                if(src[0].row() || !(cc >= '0' && cc <= '9')) {
                    Entity = SearchSemicolon;
                    break;
                }

                uc = uc * 10 + (cc - '0');
                cBuffer[cBufferPos++] = cc;
                ++src;
            }
            EntityChar = QChar(uc);
            if(cBufferPos == 9)  Entity = SearchSemicolon;
            break;
        }
        case EntityName:
        {
            int ll = kMin(src.length(), 9-cBufferPos);
            while(ll--) {
                QChar csrc = src[0];
                cc = csrc.cell();

                if(csrc.row() || !((cc >= 'a' && cc <= 'z') ||
                                   (cc >= '0' && cc <= '9') || (cc >= 'A' && cc <= 'Z'))) {
                    Entity = SearchSemicolon;
                    break;
                }

                cBuffer[cBufferPos++] = cc;
                ++src;
            }
            if(cBufferPos == 9) Entity = SearchSemicolon;
            if(Entity == SearchSemicolon) {
                if(cBufferPos > 1) {
                    const entity *e = findEntity(cBuffer, cBufferPos);
                    if(e)
                        EntityChar = e->code;

                    // be IE compatible
                    if(tag && EntityChar.unicode() > 255 && src[0] != ';')
                        EntityChar = QChar::null;
                }
            }
            else
                break;
        }
        case SearchSemicolon:

            //kdDebug( 6036 ) << "ENTITY " << EntityChar.unicode() << ", " << res << endl;

            fixUpChar(EntityChar);

            if ( EntityChar != QChar::null ) {
                checkBuffer();
                // Just insert it
                *dest++ = EntityChar;
                if (pre)
                    prePos++;
                if (src[0] == ';')
                    ++src;
            } else {
#ifdef TOKEN_DEBUG
                kdDebug( 6036 ) << "unknown entity!" << endl;
#endif

                checkBuffer(10);
                // ignore the sequence, add it to the buffer as plaintext
                *dest++ = '&';
                for(unsigned int i = 0; i < cBufferPos; i++)
                    dest[i] = cBuffer[i];
                dest += cBufferPos;
                Entity = NoEntity;
                if (pre)
                    prePos += cBufferPos+1;
            }

            EntityChar = QChar::null;
            Entity = NoEntity;
            return;
        };
    }
}

void HTMLTokenizer::parseTag(DOMStringIt &src)
{
    if (Entity)
        parseEntity(src,dest);

    while ( src.length() )
    {
        checkBuffer();
        char curchar = src[0].latin1();
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
        int l = 0;
        while(l < src.length() && (*(src.current()+l)).latin1() != '>')
            l++;
        qDebug("src is now: *%s*, tquote: %d",
               QConstString((QChar*)src.current(), l).string().latin1(), tquote);
#endif
        switch(tag) {
        case NoTag:
        {
            return;
        }
        case TagName:
        {
#if defined(TOKEN_DEBUG) &&  TOKEN_DEBUG > 1
            qDebug("TagName");
#endif
            if (searchCount > 0)
            {
                if (src[0] == commentStart[searchCount])
                {
                    searchCount++;
                    if (searchCount == 4)
                    {
#ifdef TOKEN_DEBUG
                        kdDebug( 6036 ) << "Found comment" << endl;
#endif
                        // Found '<!--' sequence
                        ++src;
                        dest = buffer; // ignore the previous part of this tag
                        comment = true;
                        tag = NoTag;
                        parseComment(src);

                        return; // Finished parsing tag!
                    }
                    // cuts of high part, is okay
                    cBuffer[cBufferPos++] = src->cell();
                    ++src;
                    break;
                }
                else
                    searchCount = 0; // Stop looking for '<!--' sequence
            }

            bool finish = false;
            unsigned int ll = kMin(src.length(), CBUFLEN-cBufferPos);
            while(ll--) {
                ushort curchar = *src;
                if(curchar <= ' ' || curchar == '>') {
                    finish = true;
                    break;
                }
                // this is a nasty performance trick. will work for the A-Z
                // characters, but not for others. if it contains one,
                // we fail anyway
                char cc = curchar;
                cBuffer[cBufferPos++] = cc | 0x20;
                ++src;
            }

            // Disadvantage: we add the possible rest of the tag
            // as attribute names. ### judge if this causes problems
            if(finish || CBUFLEN == cBufferPos) {
                bool beginTag;
                const char* ptr = cBuffer;
                unsigned int len = cBufferPos;
                cBuffer[cBufferPos] = '\0';
                if ((cBufferPos > 0) && (*ptr == '/'))
                {
                    // End Tag
                    beginTag = false;
                    ptr++;
                    len--;
                }
                else
                    // Start Tag
                    beginTag = true;
                // limited xhtml support. Accept empty xml tags like <br/>
                if((len > 1) && (*(ptr+len-1) == '/')) len--;

                uint tagID = khtml::getTagID(ptr, len);
                if (!tagID) {
#ifdef TOKEN_DEBUG
                    QCString tmp(cBuffer, cBufferPos+1);
                    kdDebug( 6036 ) << "Unknown tag: \"" << tmp.data() << "\"" << endl;
#endif
                    dest = buffer;
                }
                else
                {
#ifdef TOKEN_DEBUG
                    QCString tmp(cBuffer, cBufferPos+1);
                    kdDebug( 6036 ) << "found tag id=" << tagID << ": " << tmp.data() << endl;
#endif
                    currToken.id = beginTag ? tagID : tagID + ID_CLOSE_TAG;
                    dest = buffer;
                }
                tag = SearchAttribute;
                cBufferPos = 0;
            }
            break;
        }
        case SearchAttribute:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("SearchAttribute");
#endif
            bool atespace = false;
            ushort curchar;
            while(src.length()) {
                curchar = *src;
                if(curchar > ' ') {
                    if(curchar == '>')
                        tag = SearchEnd;
                    else if(atespace && (curchar == '\'' || curchar == '"'))
                    {
                        tag = SearchValue;
                        *dest++ = 0;
                        attrName = QString::null;
                    }
                    else
                        tag = AttributeName;

                    cBufferPos = 0;
                    break;
                }
                atespace = true;
                ++src;
            }
            break;
        }
        case AttributeName:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("AttributeName");
#endif
            ushort curchar;
            int ll = kMin(src.length(), CBUFLEN-cBufferPos);

            while(ll--) {
                curchar = *src;
                if(curchar <= '>') {
                    if(curchar <= ' ' || curchar == '=' || curchar == '>') {
                        unsigned int a;
                        if(cBufferPos) {
                            cBuffer[cBufferPos] = '\0';
                            a = khtml::getAttrID(cBuffer, cBufferPos);
                        }
                        else {
                            a = 0;
                            // ugh, expensive!!
                            attrName = QString::fromLatin1(QCString(cBuffer, cBufferPos+1).data());
                        }

                        dest = buffer;
                        *dest++ = a;
#ifdef TOKEN_DEBUG
                        if (!a || (cBufferPos && *cBuffer == '!'))
                            kdDebug( 6036 ) << "Unknown attribute: *" << QCString(cBuffer, cBufferPos+1).data() << "*" << endl;
                        else
                            kdDebug( 6036 ) << "Known attribute: " << QCString(cBuffer, cBufferPos+1).data() << endl;
#endif
                        tag = SearchEqual;
                        break;
                    }
                }
                cBuffer[cBufferPos++] = (char) curchar | 0x20;
                ++src;
            }
            if(cBufferPos == CBUFLEN) tag = SearchEqual;
            break;
        }
        case SearchEqual:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("SearchEqual");
#endif
            while(src.length()) {
                if(curchar >= '=') {
                    if(curchar == '=') {
#ifdef TOKEN_DEBUG
                        kdDebug(6036) << "found equal" << endl;
#endif
                        tag = SearchValue;
                        ++src;
                    }
                    else {
                        AttrImpl* a;
                        if(*buffer)
                            a = new AttrImpl(parser->docPtr(), (int)*buffer);
                        else
                            a = new AttrImpl(parser->docPtr(), attrName);

                        a->setValue("");
                        currToken.insertAttr(a);

                        dest = buffer;
                        tag = SearchAttribute;
                    }
                    break;
                }
                ++src;
                curchar = src->unicode();
            }
            break;
        }
        case SearchValue:
        {
            while(src.length()) {
                if(curchar > ' ') {
                    if(curchar == '\'' || curchar == '\"') {
                        tquote = curchar == '\"' ? DoubleQuote : SingleQuote;
                        tag = QuotedValue;
                        ++src;
                    } else
                        tag = Value;

                    break;
                }
                ++src;
                curchar = src->unicode();
            }
            break;
        }
        case QuotedValue:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("QuotedValue");
#endif
            while(src.length()) {
                checkBuffer();

                if(curchar <= '\'') {
                    // ### attributes like '&{blaa....};' are supposed to be treated as jscript.
                    if ( curchar == '&' )
                    {
                        ++src;
                        parseEntity(src, dest, true);
                        break;
                    }
                    else if ( (tquote == SingleQuote && curchar == '\'') ||
                              (tquote == DoubleQuote && curchar == '\"') )
                    {
                        // end of attribute
                        AttrImpl* a;

                        if(*buffer)
                            a = new AttrImpl(parser->docPtr(), (int)*buffer);
                        else
                            a = new AttrImpl(parser->docPtr(), DOMString(attrName));

                        if(a->attrId || !attrName.isNull())
                        {
                            // some <input type=hidden> rely on trailing spaces. argh
                            while(dest > buffer+1 && (*(dest-1) == '\n' || *(dest-1) == '\r'))
                                dest--; // remove trailing newlines
                            a->setValue(DOMString(buffer+1, dest-buffer-1));
                            currToken.insertAttr(a);
                        }
                        else {
                            // hmm, suboptimal, but happens seldom
                            delete a;
                            a = 0;
                        }

                        dest = buffer;
                        tag = SearchAttribute;
                        tquote = NoQuote;
                        ++src;
                        break;
                    }
                }
                *dest++ = src[0];
                ++src;
                curchar = src->unicode();
            }
            break;
        }
        case Value:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
            qDebug("Value");
#endif
            while(src.length()) {
                checkBuffer();

                if(curchar <= '>') {
                    // parse Entities
                    if ( curchar == '&' )
                    {
                        ++src;
                        parseEntity(src, dest, true);
                        break;
                    }
                    if ( curchar <= ' ' || curchar == '>')
                    {
                        // no quotes. Every space means end of value
                        AttrImpl* a;
                        if(*buffer)
                            a = new AttrImpl(parser->docPtr(), (int)*buffer);
                        else
                            a = new AttrImpl(parser->docPtr(), DOMString(attrName));

                        a->setValue(DOMString(buffer+1, dest-buffer-1));
                        currToken.insertAttr(a);

                        dest = buffer;
                        tag = SearchAttribute;
                        break;
                    }
                }

                *dest++ = src[0];
                ++src;
                curchar = src->unicode();
            }
            break;
        }
        case SearchEnd:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("SearchEnd");
#endif
            while(src.length()) {
                if(curchar == '>')
                    break;

                ++src;
                curchar = src->unicode();
            }
            if(curchar != '>') break;

            searchCount = 0; // Stop looking for '<!--' sequence
            tag = NoTag;
            tquote = NoQuote;
            ++src;

            if ( !currToken.id ) //stop if tag is unknown
                return;

            uint tagID = currToken.id;
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 0
            kdDebug( 6036 ) << "appending Tag: " << tagID << endl;
#endif
            bool beginTag = tagID < ID_CLOSE_TAG;

            if(!beginTag)
                tagID -= ID_CLOSE_TAG;
            else if ( beginTag && tagID == ID_SCRIPT ) {
                AttrImpl* a = 0;
                scriptSrc = scriptSrcCharset = "";
                if ( currToken.attrs ) {
                    if ( ( a = currToken.attrs->getIdItem( ATTR_SRC ) ) )
                        scriptSrc = khtml::parseURL( a->value() ).string();
                    if ( ( a = currToken.attrs->getIdItem( ATTR_CHARSET ) ) )
                        scriptSrcCharset = a->value().string().stripWhiteSpace();
                    a = currToken.attrs->getIdItem( ATTR_LANGUAGE );
                }
                javascript = true;
                if( a ) {
                    QString lang = a->value().string();
                    lang = lang.lower();
                    if( !lang.contains("javascript") &&
                        !lang.contains("ecmascript") &&
                        !lang.contains("jscript") )
                        javascript = false;
                } else {
                    if( currToken.attrs )
                        a = currToken.attrs->getIdItem(ATTR_TYPE);
                    if( a ) {
                        QString lang = a->value().string();
                        lang = lang.lower();
                        if( !lang.contains("javascript") &&
                            !lang.contains("ecmascript") &&
                            !lang.contains("jscript") )
                            javascript = false;
                    }
                }
            }

            processToken();

            // we have to take care to close the pre block in
            // case we encounter an unallowed element....
            if(pre && beginTag && !DOM::checkChild(ID_PRE, tagID)) {
                kdDebug(6036) << " not allowed in <pre> " << (int)tagID << endl;
                pre = false;
            }

            if ( tagID == ID_PRE )
            {
                prePos = 0;
                pre = beginTag;
            }
            else if ( tagID == ID_TEXTAREA )
            {
                if(beginTag) {
                    listing = true;
                    textarea = true;
                    searchCount = 0;
                    searchFor = textareaEnd;
                    parseListing(src);
                }
            }
            else if ( tagID == ID_SCRIPT )
            {
                if (beginTag)
                {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                    kdDebug( 6036 ) << "start of script, token->id = " << currToken.id << endl;
#endif
                    script = true;
                    searchCount = 0;
                    searchFor = scriptEnd;
                    tquote = NoQuote;
                    parseScript(src);
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                    kdDebug( 6036 ) << "end of script" << endl;
#endif
                }
            }
            else if ( tagID == ID_STYLE )
            {
                if (beginTag)
                {
                    style = true;
                    searchCount = 0;
                    searchFor = styleEnd;
                    parseStyle(src);
                }
            }
            else if ( tagID == ID_LISTING )
            {
                if (beginTag)
                {
                    listing = true;
                    searchCount = 0;
                    searchFor = listingEnd;
                    parseListing(src);
                }
            }
            else if ( tagID == ID_SELECT )
            {
                select = beginTag;
            }

            if ( pending ) {
                if ( !parser->noSpaces() ) addPending();
                discard = AllDiscard;
                pending = NonePending;
            }
            return; // Finished parsing tag!
        }
        } // end switch
    }
    return;
}

void HTMLTokenizer::addPending()
{
    if ( select)
    {
        *dest++ = ' ';
    }
    else if ( textarea )
    {
        if (pending == LFPending)
            *dest++ = '\n';
        else
            *dest++ = ' ';
    }
    else if ( pre )
    {
        int p;

        switch (pending)
        {
        case SpacePending:
            // Insert a breaking space
            *dest++ = QChar(' ');
            prePos++;
            break;

        case LFPending:
            *dest = '\n';
            dest++;
            prePos = 0;
            break;

        case TabPending:
            p = TAB_SIZE - ( prePos % TAB_SIZE );
            for ( int x = 0; x < p; x++ )
            {
                *dest = QChar(' ');
                dest++;
            }
            prePos += p;
            break;

        default:
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
            kdDebug( 6036 ) << "Assertion failed: pending = " << (int) pending << endl;
#endif
            break;
        }
    }
    else
    {
        *dest++ = ' ';
    }

    pending = NonePending;
}

#if 0
void HTMLTokenizer::setPlainText()
{
    if (!plaintext)
    {
       // Do this only once!
       plaintext = true;
       currToken.id = ID_PLAIN;
       processToken();
       dest = buffer;
    }
}
#endif

void HTMLTokenizer::write( const QString &str, bool appendData )
{
    // we have to make this function reentrant. This is needed, because some
    // script code could call document.write(), which would add something here.
#ifdef TOKEN_DEBUG
    kdDebug( 6036 ) << "Tokenizer::write(\"" << str << "\"," << appendData << ")" << endl;
#endif

    if ( !buffer )
        return;

    if ( loadingExtScript || ( m_executingScript && appendData ) ) {
        // don't parse; we will do this later
        pendingSrc += str;
        return;
    }

    if ( onHold ) {
        QString rest = QString( src.current(), src.length() );
        rest += str;
        _src = rest;
        return;
    }
    else
        _src = str;

    src = DOMStringIt(_src);

    if (plaintext)
        parseText(src);
    else if (comment)
        parseComment(src);
    else if (script)
        parseScript(src);
    else if (style)
        parseStyle(src);
    else if (listing)
        parseListing(src);
    else if (processingInstruction)
        parseProcessingInstruction(src);
    else if (tag)
        parseTag(src);
    else if (Entity)
        parseEntity(src, dest);

    while ( src.length() )
    {
        // do we need to enlarge the buffer?
        checkBuffer();

        ushort cc = src[0].unicode();

//         if(!startTag && cc != '<') {
//             ++src;
//             continue;
//         }

        if (skipLF && (cc != '\n'))
            skipLF = false;

        if (skipLF)
        {
            skipLF = false;
            ++src;
        }
        else if ( startTag )
        {
            startTag = false;

            switch(cc) {
            case '/':
                break;
            case '!':
            {
                // <!-- comment -->
                searchCount = 1; // Look for '<!--' sequence to start comment

                break;
            }

            case '?':
            {
                // xml processing instruction
                processingInstruction = true;
                tquote = NoQuote;
                parseProcessingInstruction(src);
                continue;

                break;
            }

            default:
            {

                if( ((cc >= 'a') && (cc <= 'z')) || ((cc >= 'A') && (cc <= 'Z')))
                {
                    // Start of a Start-Tag
                }
                else
                {
                    // Invalid tag
                    // Add as is
                    if (pending)
                        addPending();
                    *dest = '<';
                    dest++;
                    continue;
                }
            }
            }; // end case

            processToken();

            cBufferPos = 0;
            tag = TagName;
            parseTag(src);
        }
        else if ( cc == '&' )
        {
            ++src;

            discard = NoneDiscard;
            if (pending)
                addPending();
            parseEntity(src, dest, true);
        }
        else if ( cc == '<')
        {
            ++src;
            startTag = true;
            discard = NoneDiscard;
        }
        else if (( cc == '\n' ) || ( cc == '\r' ))
        {
            if ( pre || textarea)
            {
                if (discard == LFDiscard || discard == AllDiscard)
                {
                    // Ignore this LF
                    discard = NoneDiscard; // We have discarded 1 LF
                }
                else
                {
                    // Process this LF
                    if (pending)
                        addPending();
                    pending = LFPending;
                }
            }
            else
            {
                if (discard == LFDiscard)
                {
                    // Ignore this LF
                    discard = NoneDiscard; // We have discarded 1 LF
                }
                else if(discard == AllDiscard)
                {
                }
                else
                {
                    // Process this LF
                    if (pending == NonePending)
                        pending = LFPending;
                }
            }
            /* Check for MS-DOS CRLF sequence */
            if (cc == '\r')
            {
                skipLF = true;
            }
            ++src;
        }
        else if (( cc == ' ' ) || ( cc == '\t' ))
        {
            if ( pre || textarea)
            {
                if (pending)
                    addPending();
                if (cc == ' ')
                    pending = SpacePending;
                else
                    pending = TabPending;
            }
            else
            {
                if(discard == SpaceDiscard)
                    discard = NoneDiscard;
                else if(discard == AllDiscard)
                { }
                else
                    pending = SpacePending;
            }
            ++src;
        }
        else
        {
            if (pending)
                addPending();

            discard = NoneDiscard;
            if ( pre )
            {
                prePos++;
            }
            unsigned char row = src[0].row();
            if ( row > 0x05 && row < 0x10 || row > 0xfd )
                    currToken.complexText = true;
            *dest = src[0];
            fixUpChar( *dest );
            ++dest;
            ++src;
        }
    }
    _src = QString();
    if (noMoreData && !loadingExtScript && !m_executingScript )
        end(); // this actually causes us to be deleted
}

void HTMLTokenizer::end()
{
    if ( buffer == 0 ) {
        emit finishedParsing();
        return;
    }

    processToken();

    if(buffer)
        KHTML_DELETE_QCHAR_VEC(buffer);

    if(scriptCode)
        KHTML_DELETE_QCHAR_VEC(scriptCode);

    scriptCode = 0;
    scriptCodeSize = scriptCodeMaxSize = 0;
    buffer = 0;
    emit finishedParsing();
}

void HTMLTokenizer::finish()
{
    // do this as long as we don't find matching comment ends
    while(comment && scriptCode && scriptCodeSize)
    {
        // we've found an unmatched comment start
        brokenComments = true;
        checkScriptBuffer();
        scriptCode[ scriptCodeSize ] = 0;
        scriptCode[ scriptCodeSize + 1 ] = 0;
        int pos = QConstString(scriptCode, scriptCodeSize).string().find('>');
        QString food;
        food.setUnicode(scriptCode+pos+1, scriptCodeSize-pos-1); // deep copy
        KHTML_DELETE_QCHAR_VEC(scriptCode);
        scriptCode = 0;
        scriptCodeSize = scriptCodeMaxSize = 0;
        script = style = listing = comment = textarea = false;
        scriptCodeSize = 0;
        write(food, true);
    }
    // this indicates we will not recieve any more data... but if we are waiting on
    // an external script to load, we can't finish parsing until that is done
    noMoreData = true;
    if (!loadingExtScript && !m_executingScript && !onHold)
        end(); // this actually causes us to be deleted
}

void HTMLTokenizer::processToken()
{
    if ( dest > buffer )
    {
#ifdef TOKEN_DEBUG
        if(currToken.id && currToken.id != ID_COMMENT)
            assert(0);
#endif
        if ( currToken.complexText ) {
            // ### we do too much QString copying here, but better here than in RenderText...
            // anyway have to find a better solution in the long run (lars)
            QString s = QConstString(buffer, dest-buffer).string();
            s.compose();
            currToken.text = new DOMStringImpl( s.unicode(), s.length() );
            currToken.text->ref();
        } else {
            currToken.text = new DOMStringImpl( buffer, dest - buffer );
            currToken.text->ref();
        }
        if (currToken.id != ID_COMMENT)
            currToken.id = ID_TEXT;
    }
    else if(!currToken.id) {
        currToken.reset();
        return;
    }

    dest = buffer;

#ifdef TOKEN_PRINT
    QString name = getTagName(currToken.id).string();
    QString text = currToken.text.string();

    kdDebug( 6036 ) << "Token --> " << name << "   id = " << currToken.id << endl;
    if(currToken.text != 0)
        kdDebug( 6036 ) << "text: \"" << text << "\"" << endl;
#else
#ifdef TOKEN_DEBUG
    QString name = getTagName(currToken.id).string();
    QString text;
    if(currToken.text)
        text = QConstString(currToken.text->s, currToken.text->l).string();

    kdDebug( 6036 ) << "Token --> " << name << "   id = " << currToken.id << endl;
    if(!text.isNull())
        kdDebug( 6036 ) << "text: \"" << text << "\"" << endl;
    int l = currToken.attrs ? currToken.attrs->length() : 0;
    if(l>0)
    {
        int i = 0;
        kdDebug( 6036 ) << "Attributes: " << l << endl;
        while(i<l)
        {
            AttrImpl* c = static_cast<AttrImpl*>(currToken.attrs->item(i));
            kdDebug( 6036 ) << "    " << c->attrId << " " << c->name().string()
                            << "=\"" << c->value().string() << "\"" << endl;
            i++;
        }
    }
    kdDebug( 6036 ) << endl;
#endif
#endif
    // pass the token over to the parser, the parser DOES NOT delete the token
    parser->parseToken(&currToken);

    currToken.reset();
}


HTMLTokenizer::~HTMLTokenizer()
{
    reset();
    delete parser;
}


void HTMLTokenizer::enlargeBuffer(int len)
{
    int newsize = kMax(size*2, size+len);
    int oldoffs = (dest - buffer);
//    qDebug("size: %d, newsize: %d, len: %d, oldoffs: %d", size, newsize, len, oldoffs);

    buffer = (QChar*)realloc(buffer, newsize*sizeof(QChar));
    dest = buffer + oldoffs;
    size = newsize;
}

void HTMLTokenizer::enlargeScriptBuffer(int len)
{
    int newsize = kMax(scriptCodeMaxSize*2, scriptCodeMaxSize+len);
    scriptCode = (QChar*)realloc(scriptCode, newsize*sizeof(QChar));
    scriptCodeMaxSize = newsize;
}

void HTMLTokenizer::notifyFinished(CachedObject *finishedObj)
{
    if (view && finishedObj == cachedScript) {
#ifdef TOKEN_DEBUG
        kdDebug( 6036 ) << "Finished loading an external script" << endl;
#endif
        loadingExtScript = false;
        DOMString scriptSource = cachedScript->script();
#ifdef TOKEN_DEBUG
        kdDebug( 6036 ) << "External script is:" << endl << scriptSource.string() << endl;
#endif
        cachedScript->deref(this);
        cachedScript = 0;

//         pendingSrc.prepend( QString( src.current(), src.length() ) ); // deep copy - again
        _src = QString::null;
        src = DOMStringIt( _src );
        m_executingScript++;
        view->part()->executeScript(scriptSource.string());
        m_executingScript--;
        // 'script' is true when we are called synchronously from
        // parseScript(). In that case parseScript() will take care
        // of 'scriptOutput'.
         if ( !script ) {
//              qDebug( "adding pending output! *%s*", pendingSrc.latin1() );

             QString rest = pendingSrc;
             pendingSrc = "";
             write(rest, false);
         }
    }
}

void HTMLTokenizer::addPendingSource()
{
//    kdDebug( 6036 ) << "adding pending Output to parsed string" << endl;
    QString newStr = QString(src.current(), src.length());
//     qDebug( "src: %s", newStr.latin1() );
//     qDebug( "pendingSrc: %s", pendingSrc.latin1() );
    newStr += pendingSrc;
    _src = newStr;
    src = DOMStringIt(_src);
    pendingSrc = "";
}

void HTMLTokenizer::setOnHold(bool _onHold)
{
    if (onHold == _onHold) return;
    onHold = _onHold;
    if (!onHold)
        write( _src, true );
}

#include "htmltokenizer.moc"
