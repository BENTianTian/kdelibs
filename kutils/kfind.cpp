/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kfind.h"
#include "kfinddialog.h"
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlabel.h>
#include <qregexp.h>

// Create the dialog.
KFind::KFind(const QString &pattern, long options, QWidget *parent) :
    KDialogBase(parent, __FILE__, false,  // non-modal!
        i18n("Find"),
        User1 | Close,
        User1,
        false,
        i18n("&Yes"))
{
    m_options = options;
    setMainWidget( new QLabel( i18n("Find next '%1'").arg(pattern), this ) );
    init( pattern );
}

// Constructor for KReplace
KFind::KFind(const QString &pattern, const QString &, long options, QWidget *parent) :
    KDialogBase(parent, __FILE__, false,  // non-modal!
        i18n("Replace"),
        User3 | User2 | User1 | Close,
        User3,
        false,
        i18n("&All"), i18n("&Skip"), i18n("&Yes"))
{
    m_options = options;
    // setMainWidget done by KReplace
    init( pattern );
}

void KFind::init( const QString& pattern )
{
    m_cancelled = false;
    m_displayFinalDialog = true;
    m_matches = 0;
    if (m_options & KFindDialog::RegularExpression)
        m_regExp = new QRegExp(pattern, m_options & KFindDialog::CaseSensitive);
    else
        m_pattern = pattern;
    resize(minimumSize());
}

KFind::~KFind()
{
    if (m_displayFinalDialog && !m_matches && !m_cancelled)
        KMessageBox::information(parentWidget(), i18n("No match was found."));
}

void KFind::slotClose()
{
    m_matches++;
    m_cancelled = true;
    kapp->exit_loop();
}

void KFind::abort()
{
    slotClose();
}

bool KFind::find(const QString &text, const QRect &expose)
{
    if (m_options & KFindDialog::FindBackwards)
    {
        m_index = text.length();
    }
    else
    {
        m_index = 0;
    }
    m_text = text;
    m_expose = expose;
    do
    {
        // Find the next match.
        if (m_options & KFindDialog::RegularExpression)
            m_index = KFind::find(m_text, *m_regExp, m_index, m_options, &m_matchedLength);
        else
            m_index = KFind::find(m_text, m_pattern, m_index, m_options, &m_matchedLength);
        if (m_index != -1)
        {
            // Tell the world about the match we found, in case someone wants to
            // highlight it.
            if ( validateMatch( m_text, m_index, m_matchedLength ))
            {
                emit highlight(m_text, m_index, m_matchedLength, m_expose);
                show();
                kapp->enter_loop();
            }
            else
                m_index = m_index+m_matchedLength;
        }
    }
    while ((m_index != -1) && !m_cancelled);

    // Should the user continue?
    return !m_cancelled;
}

int KFind::find(const QString &text, const QString &pattern, int index, long options, int *matchedLength)
{
    // Handle regular expressions in the appropriate way.
    if (options & KFindDialog::RegularExpression)
    {
        QRegExp regExp(pattern, options & KFindDialog::CaseSensitive);

        return find(text, regExp, index, options, matchedLength);
    }

    bool caseSensitive = (options & KFindDialog::CaseSensitive);

    if (options & KFindDialog::WholeWordsOnly)
    {
        if (options & KFindDialog::FindBackwards)
        {
            // Backward search, until the beginning of the line...
            while (index >= 0)
            {
                // ...find the next match.
                index = text.findRev(pattern, index, caseSensitive);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                *matchedLength = pattern.length();
                if (isWholeWords(text, index, *matchedLength))
                    break;
                index--;
            }
        }
        else
        {
            // Forward search, until the end of the line...
            while (index < (int)text.length())
            {
                // ...find the next match.
                index = text.find(pattern, index, caseSensitive);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                *matchedLength = pattern.length();
                if (isWholeWords(text, index, *matchedLength))
                    break;
                index++;
            }
            if (index >= (int)text.length()) // end of line
                index = -1; // not found
        }
    }
    else
    {
        // Non-whole-word search.
        if (options & KFindDialog::FindBackwards)
        {
            index = text.findRev(pattern, index, caseSensitive);
        }
        else
        {
            index = text.find(pattern, index, caseSensitive);
        }
        if (index != -1)
        {
            *matchedLength = pattern.length();
        }
    }
    return index;
}

int KFind::find(const QString &text, const QRegExp &pattern, int index, long options, int *matchedLength)
{
    if (options & KFindDialog::WholeWordsOnly)
    {
        if (options & KFindDialog::FindBackwards)
        {
            // Backward search, until the beginning of the line...
            while (index >= 0)
            {
                // ...find the next match.
                index = text.findRev(pattern, index);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                //pattern.match(text, index, matchedLength, false);
                /*int pos =*/ pattern.search( text.mid(index) );
                *matchedLength = pattern.matchedLength();
                if (isWholeWords(text, index, *matchedLength))
                    break;
                index--;
            }
        }
        else
        {
            // Forward search, until the end of the line...
            while (index < (int)text.length())
            {
                // ...find the next match.
                index = text.find(pattern, index);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                //pattern.match(text, index, matchedLength, false);
                /*int pos =*/ pattern.search( text.mid(index) );
                *matchedLength = pattern.matchedLength();
                if (isWholeWords(text, index, *matchedLength))
                    break;
                index++;
            }
            if (index >= (int)text.length()) // end of line
                index = -1; // not found
        }
    }
    else
    {
        // Non-whole-word search.
        if (options & KFindDialog::FindBackwards)
        {
            index = text.findRev(pattern, index);
        }
        else
        {
            index = text.find(pattern, index);
        }
        if (index != -1)
        {
            //pattern.match(text, index, matchedLength, false);
            /*int pos =*/ pattern.search( text.mid(index) );
            *matchedLength = pattern.matchedLength();
        }
    }
    return index;
}

bool KFind::isInWord(QChar ch)
{
    return ch.isLetter() || ch.isDigit() || ch == '_';
}

bool KFind::isWholeWords(const QString &text, int starts, int matchedLength)
{
    if ((starts == 0) || (!isInWord(text[starts - 1])))
    {
        int ends = starts + matchedLength;

        if ((ends == (int)text.length()) || (!isInWord(text[ends])))
            return true;
    }
    return false;
}

// Yes.
void KFind::slotUser1()
{
    m_matches++;
    if (m_options & KFindDialog::FindBackwards)
        m_index--;
    else
        m_index++;
    kapp->exit_loop();
}

bool KFind::shouldRestart( bool forceAsking ) const
{
    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if ( !forceAsking && (m_options & KFindDialog::FromCursor) == 0 )
        return false;
    QString message;
    if ( numMatches() )
        message = i18n("1 match found.\n", "%1 matches found.\n").arg( numMatches() );
    else
        message = i18n("No matches found.\n");
    // Hope this word puzzle is ok, it's a different sentence
    message += i18n("Do you want to restart search at the beginning?");

    int ret = KMessageBox::questionYesNo( parentWidget(), message );
    bool yes = ( ret == KMessageBox::Yes );
    if ( yes )
        const_cast<KFind*>(this)->m_options &= ~KFindDialog::FromCursor; // clear FromCursor option
    return yes;
}

#include "kfind.moc"

