/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KTEXTEDIT_H
#define KTEXTEDIT_H

#include <kdeui_export.h>
#include <sonnet/highlighter.h>
#include <QtGui/QTextEdit>

/**
 * @short A KDE'ified QTextEdit
 *
 * This is just a little subclass of QTextEdit, implementing
 * some standard KDE features, like Cursor auto-hiding, configurable
 * wheelscrolling (fast-scroll or zoom) and deleting of entire
 * words with Ctrl-Backspace or Ctrl-Delete.
 *
 * Basic rule: whenever you want to use QTextEdit, use KTextEdit!
 *
 * @see QTextEdit
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KDEUI_EXPORT KTextEdit : public QTextEdit
{
    Q_OBJECT

  public:
    /**
     * Constructs a KTextEdit object. See QTextEdit::QTextEdit
     * for details.
     */
    explicit KTextEdit( const QString& text, QWidget *parent = 0 );

    /**
     * Constructs a KTextEdit object. See QTextEdit::QTextEdit
     * for details.
     */
    explicit KTextEdit( QWidget *parent = 0 );

    /**
     * Destroys the KTextEdit object.
     */
    ~KTextEdit();

    /**
     * Reimplemented to set a proper "deactivated" background color.
     */
    virtual void setReadOnly( bool readOnly );

    /**
     * Turns spell checking for this text edit on or off. Note that spell
     * checking is only available in read-writable KTextEdits.
     *
     * @see checkSpellingEnabled()
     * @see isReadOnly()
     * @see setReadOnly()
     */
    void setCheckSpellingEnabled( bool check );

    /**
     * Returns true if spell checking is enabled for this text edit.
     * Note that it even returns true if this is a read-only KTextEdit,
     * where spell checking is actually disabled.  By default spell checking is disabled.
     *
     @ see setCheckSpellingEnabled()
     */
    bool checkSpellingEnabled() const;

    void highlightWord( int length, int pos );

    void setSpellCheckingConfigFileName(const QString &);
    /**
     * Allow to create specific Highlighter
     */
    virtual void createHighlighter();

    Sonnet::Highlighter* highlighter() const;

    void setHighlighter(Sonnet::Highlighter *_highLighter);
  public Q_SLOTS:
    /**
     * Create a modal dialog to check the spelling.  This slot will not return
     * until spell checking has been completed.
     */
    void checkSpelling();

  protected:
    /**
     * Reimplemented to catch "delete word" key events.
     */
    virtual void keyPressEvent( QKeyEvent* );

    /**
     * Reimplemented to instantiate a KDictSpellingHighlighter, if
     * spellchecking is enabled.
     */
    virtual void focusInEvent( QFocusEvent* );

    /**
     * Reimplemented to allow fast-wheelscrolling with Ctrl-Wheel
     * or zoom.
     */
    virtual void wheelEvent( QWheelEvent* );

    /**
     * Deletes a word backwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordBack();

    /**
     * Deletes a word forwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordForward();

    /**
     * Reimplemented from QTextEdit to add spelling related items
     * when appropriate.
     */
    virtual void contextMenuEvent( QContextMenuEvent* );

  private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void slotSpellCheckDone( const QString& ) )

    Q_PRIVATE_SLOT( d, void spellCheckerMisspelling( const QString&, int ) )
    Q_PRIVATE_SLOT( d, void spellCheckerCorrected(const QString&, int,const QString&) )
    Q_PRIVATE_SLOT( d, void spellCheckerCanceled())
    Q_PRIVATE_SLOT( d, void spellCheckerAutoCorrect(const QString&,const QString&) )
    Q_PRIVATE_SLOT( d, void spellCheckerFinished() )
    Q_PRIVATE_SLOT( d, void toggleAutoSpellCheck() )
    Q_PRIVATE_SLOT( d, void slotAllowTab() )
    Q_PRIVATE_SLOT( d, void menuActivated( QAction* ) )
};

#endif // KTEXTEDIT_H
