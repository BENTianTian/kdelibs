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

#ifndef KREPLACE_H
#define KREPLACE_H

#include "kfind.h"

/**
 * @short A generic implementation of the "replace" function.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 * @sect Detail
 *
 * This class includes prompt handling etc. Also provides some
 * static functions which can be used to create custom behaviour
 * instead of using the class directly.
 *
 * @sect Example
 *
 * To use the class to implement a complete replace feature:
 *
 * <pre>
 *
 *  // This creates a replace-on-prompt dialog if needed.
 *  dialog = new KReplace(find, replace, options);
 *
 *  // Connect signals to code which handles highlighting
 *  // of found text, and on-the-fly replacement.
 *  QObject::connect(
 *      dialog, SIGNAL( highlight( const QString &, int, int, const QRect & ) ),
 *      this, SLOT( highlight( const QString &, int, int, const QRect & ) ) );
 *  QObject::connect(
 *      dialog, SIGNAL( replace( const QString &, int, int, const QRect & ) ),
 *      this, SLOT( replace( const QString &, int, int, const QRect & ) ) );
 *
 *  for (text chosen by option SelectedText and in a direction set by FindBackwards)
 *  {
 *      dialog->replace()
 *  }
 *  delete dialog;
 *
 * </pre>
 */
class KReplace :
    public KFind
{
    Q_OBJECT

public:

    /** Will create a prompt dialog and use it as needed. */
    KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent = 0);
    virtual ~KReplace();

    /**
     * Return the number of replacements made (i.e. the number of times
     * the @ref replace signal was emitted).
     * Can be used in a dialog box to tell the user how many replacements were made.
     * The final dialog does so already, unless you used setDisplayFinalDialog(false).
     */
    int numReplacements() const { return m_replacements; }

    /**
     * Call this to reset the numMatches & numReplacements counts.
     * Can be useful if reusing the same KReplace for different operations,
     * or when restarting from the beginning of the document.
     */
    virtual void resetCounts();

    /**
     * Walk the text fragment (e.g. kwrite line, kspread cell) looking for matches.
     * For each match, if prompt-on-replace is specified, emits the expose() signal
     * and displays the prompt-for-replace dialog before doing the replace.
     *
     * @param text The text fragment to modify.
     * @param exposeOnReplace The region to expose
     * @return False if the user elected to discontinue the replace.
     */
    bool replace(QString &text, const QRect &expose);

    /**
     * Search the given string, replaces with the given replacement string,
     * and returns whether a match was found. If one is,
     * the replacement string length is also returned.
     *
     * A performance optimised version of the function is provided for use
     * with regular expressions.
     *
     * @param text The string to search.
     * @param pattern The pattern to look for.
     * @param replacement The replacement string to insert into the text.
     * @param index The starting index into the string.
     * @param options The options to use.
     * @param replacedLength Output parameter, contains the length of the replaced string.
     * Not always the same as replacement.length(), when backreferences are used.
     * @return The index at which a match was found, or -1 if no match was found.
     */
    static int replace( QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength );
    static int replace( QString &text, const QRegExp &pattern, const QString &replacement, int index, long options, int *replacedLength );

    /**
     * Returns true if we should restart the search from scratch.
     * Can ask the user, or return false (if we already searched/replaced the
     * whole document without the PromptOnReplace option).
     *
     * @param forceAsking set to true if the user modified the document during the
     * search. In that case it makes sense to restart the search again.
     */
    virtual bool shouldRestart( bool forceAsking = false ) const;

signals:

    /**
     * Connect to this slot to implement updating of replaced text during the replace
     * operation.
     *
     * Extra care must be taken to properly implement the "no prompt-on-replace" case.
     * For instance @ref highlight isn't emitted in that case (some code might rely on it),
     * and for performance reasons one should repaint after replace() ONLY if
     * prompt-on-replace was selected.
     *
     * @param text The text, in which the replacement has already been done.
     * @param replacementIndex Starting index of the matched substring
     * @param replacedLength Length of the replacement string
     * @param matchedLength Length of the matched string
     * @param expose The rectangle to expose
     */
    void replace(const QString &text, int replacementIndex, int replacedLength, int matchedLength, const QRect &expose);

private:
    void doReplace();

    QString m_replacement;
    unsigned m_replacements;

    static int replace( QString &text, const QString &replacement, int index, int length );

    // Binary compatible extensibility.
    class KReplacePrivate;
    KReplacePrivate *d;

private slots:

    virtual void slotUser1();   // All
    virtual void slotUser2();   // Skip
    virtual void slotUser3();   // Yes
};
#endif
