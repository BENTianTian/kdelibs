/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   Re-designed for KDE 2.x by
   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qclipboard.h>

#include <kcursor.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kcompletionbox.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kiconloader.h>

#include "klineedit.h"
#include "klineedit.moc"


class KLineEdit::KLineEditPrivate
{
public:
    KLineEditPrivate() {
      hasReference= false;
      grabReturnKeyEvents= false;
      handleURLDrops= true;
      origCursorPos = -1;
      completionBox= 0;
      popupMenu= 0;
      subMenu= 0;
    }
    int origCursorPos;
    bool hasReference;
    bool handleURLDrops;    
    bool grabReturnKeyEvents;
   
    QString prevText;
    QPopupMenu* subMenu;
    QPopupMenu* popupMenu;
    KCompletionBox *completionBox;   
};


KLineEdit::KLineEdit( const QString &string, QWidget *parent, const char *name )
    : QLineEdit( string, parent, name )
{
    init();
}

KLineEdit::KLineEdit( QWidget *parent, const char *name )
    : QLineEdit( parent, name )
{
    init();
}

KLineEdit::~KLineEdit ()
{
    delete d->popupMenu;
    delete d->completionBox;
    delete d;
}

void KLineEdit::init()
{
    d = new KLineEditPrivate;
    // Enable the context menu by default.
    setContextMenuEnabled( true );
    KCursor::setAutoHideCursor( this, true, true );
    installEventFilter( this );
}

void KLineEdit::setCompletionMode( KGlobalSettings::Completion mode )
{
    KGlobalSettings::Completion oldMode = completionMode();
    if ( oldMode != mode && oldMode == KGlobalSettings::CompletionPopup )
        hideCompletionBox();

    // If the widgets echo mode is not Normal, no completion
    // feature will be enabled even if one is requested.
    if ( echoMode() != QLineEdit::Normal )
        mode = KGlobalSettings::CompletionNone; // Override the request.

    KCompletionBase::setCompletionMode( mode );
}

void KLineEdit::setCompletedText( const QString& t, bool marked )
{
    if ( marked )
    {
        int curpos = (d->origCursorPos != -1) ?
                          d->origCursorPos : text().length();
        validateAndSet( t, curpos, curpos, t.length() );
    }
    else
    {
      if ( t != text() ) // no need to flicker
      {
        int curpos = t.length();
        validateAndSet( t, curpos, curpos, curpos );
      }
    }
}

void KLineEdit::setCompletedText( const QString& text )
{
    KGlobalSettings::Completion mode = completionMode();
    bool marked = ( mode == KGlobalSettings::CompletionAuto ||
                    mode == KGlobalSettings::CompletionMan ||
                    mode == KGlobalSettings::CompletionPopup );
    setCompletedText( text, marked );
}

void KLineEdit::rotateText( KCompletionBase::KeyBindingType type )
{
    KCompletion* comp = compObj();
    if ( comp &&
       (type == KCompletionBase::PrevCompletionMatch ||
        type == KCompletionBase::NextCompletionMatch ) )
    {
       QString input = (type == KCompletionBase::PrevCompletionMatch) ? comp->previousMatch() : comp->nextMatch();
       // Skip rotation if previous/next match is null or the same tex
       if ( input.isNull() || input == displayText() )
        return;
       setCompletedText( input, hasMarkedText() );
    }
}

void KLineEdit::makeCompletion( const QString& txt )
{
    KCompletion *comp = compObj();
    if ( !comp )
       return;  // No completion object...

    QString match = comp->makeCompletion( txt );
    KGlobalSettings::Completion mode = completionMode();
    if ( mode == KGlobalSettings::CompletionPopup ||
         mode == KGlobalSettings::CompletionShell )
    {
       if ( match.isNull() )
        hideCompletionBox();
       else
        setCompletedItems( comp->allMatches() );
    }
    else
    {
        // All other completion modes
        // If no match or the same match, simply return without completing.
        if ( match.isNull() || match == txt )
            return;
        bool marked = ( mode == KGlobalSettings::CompletionAuto ||
                        mode == KGlobalSettings::CompletionMan );
        setCompletedText( match, marked );
    }
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    // Filter key-events if EchoMode is normal &
    // completion mode is not set to CompletionNone
    if ( echoMode() == QLineEdit::Normal &&
         completionMode() != KGlobalSettings::CompletionNone )
    {
        KeyBindingMap keys = getKeyBindings();
        KGlobalSettings::Completion mode = completionMode();

        if ( mode == KGlobalSettings::CompletionAuto ||
             mode == KGlobalSettings::CompletionMan )
        {
            QString keycode = e->text();
            if ( !keycode.isNull() && keycode.unicode()->isPrint() )
            {
                QLineEdit::keyPressEvent ( e );
                QString txt = text();
                int len = txt.length();
                if ( !hasMarkedText() && len && cursorPosition() == len )
                {
                    kdDebug() << "Auto Completion: " << txt << endl;
                    if ( emitSignals() )
                        emit completion( txt );
                    if ( handleSignals() )
                        makeCompletion( txt );
                    e->accept();
                }
                return;
            }
        }
        else if ( mode == KGlobalSettings::CompletionPopup )
        {
            QLineEdit::keyPressEvent ( e );
            QString keycode = e->text();
            QString txt = text();
            int key = e->key();

            if (((!keycode.isNull() && keycode.unicode()->isPrint())||
                (key == Qt::Key_Delete || key == Qt::Key_BackSpace)) &&
                txt != d->prevText)
            {
                kdDebug() << "Popup Completion: " << txt << endl;
                if ( emitSignals() )
                    emit completion(txt);
                if ( handleSignals() )
                    makeCompletion(txt);
                e->accept();
                d->prevText = txt;
            }
            return;
        }

        else if ( mode == KGlobalSettings::CompletionShell )
        {
            // Handles completion.
            int key = ( keys[TextCompletion] == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : keys[TextCompletion];
            if ( KStdAccel::isEqual( e, key ) )
            {
                // Emit completion if the completion mode is CompletionShell
                // and the cursor is at the end of the string.
                QString txt = text();
                int len = txt.length();
                if ( cursorPosition() == len && len != 0 )
                {
                    kdDebug() << "Shell Completion: " << txt << endl;
                    if ( emitSignals() )
                        emit completion( txt );
                    if ( handleSignals() )
                        makeCompletion( txt );
                    return;
                }
            }
        }

        // handle rotation
        if ( mode != KGlobalSettings::CompletionNone )
        {
            // Handles previous match
            int key = ( keys[PrevCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::PrevCompletion) : keys[PrevCompletionMatch];
            if ( KStdAccel::isEqual( e, key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::PrevCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::PrevCompletionMatch );
                return;
            }
            // Handles next match
            key = ( keys[NextCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::NextCompletion) : keys[NextCompletionMatch];
            if ( KStdAccel::isEqual( e, key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::NextCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::NextCompletionMatch );
                return;
            }
        }
    }
    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent ( e );
}

void KLineEdit::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::RightButton )
    {
        // Return if popup menu is not enabled !!
        if ( !m_bEnableMenu )
            return;

        if ( !d->popupMenu )
        {
            d->popupMenu = contextMenuInternal();
            d->hasReference = false;
        }

        initPopup();

        KGlobalSettings::Completion oldMode = completionMode();
        int result = d->popupMenu->exec( e->globalPos() );
        if ( !d->hasReference )
        {
            delete d->popupMenu;
            d->popupMenu = 0;
        }

        if ( result == Cut )
            cut();
        else if ( result == Copy )
            copy();
        else if ( result == Paste )
            paste();
        else if ( result == Clear )
            clear();
        else if ( result == Unselect )
            deselect();
        else if ( result == SelectAll )
            selectAll();
        else if ( result == Default )
            setCompletionMode( KGlobalSettings::completionMode() );
        else if ( result == NoCompletion )
            setCompletionMode( KGlobalSettings::CompletionNone );
        else if ( result == AutoCompletion )
            setCompletionMode( KGlobalSettings::CompletionAuto );
        else if ( result == SemiAutoCompletion )
            setCompletionMode( KGlobalSettings::CompletionMan );
        else if ( result == ShellCompletion )
            setCompletionMode( KGlobalSettings::CompletionShell );
        else if ( result == PopupCompletion )
            setCompletionMode( KGlobalSettings::CompletionPopup );

        if ( oldMode != completionMode() )
        {
            if ( oldMode == KGlobalSettings::CompletionPopup )
                hideCompletionBox();
            emit completionModeChanged( completionMode() );
        }
        return;
    }
    QLineEdit::mousePressEvent( e );
}

void KLineEdit::initPopup()
{
    if( compObj() )
    {
        d->subMenu->clear();

        d->subMenu->insertItem( i18n("None"), NoCompletion );
        d->subMenu->insertItem( i18n("Manual"), ShellCompletion );
        d->subMenu->insertItem( i18n("Popup"), PopupCompletion );
        d->subMenu->insertItem( i18n("Automatic"), AutoCompletion );
        d->subMenu->insertItem( i18n("Short Automatic"), SemiAutoCompletion );

        KGlobalSettings::Completion mode = completionMode();
        d->subMenu->setItemChecked( NoCompletion,
                                    mode == KGlobalSettings::CompletionNone );
        d->subMenu->setItemChecked( ShellCompletion,
                                    mode == KGlobalSettings::CompletionShell );
        d->subMenu->setItemChecked( PopupCompletion,
                                    mode == KGlobalSettings::CompletionPopup );
        d->subMenu->setItemChecked( AutoCompletion,
                                    mode == KGlobalSettings::CompletionAuto );
        d->subMenu->setItemChecked( SemiAutoCompletion,
                                    mode == KGlobalSettings::CompletionMan );
        if ( mode != KGlobalSettings::completionMode() )
        {
            d->subMenu->insertSeparator();
            d->subMenu->insertItem( i18n("Default"), Default );
        }
    }

    bool flag = ( echoMode() == Normal && !isReadOnly() );
    bool allMarked = (markedText().length() == text().length());
    d->popupMenu->setItemEnabled( Cut, flag && hasMarkedText() );
    d->popupMenu->setItemEnabled( Copy, flag && hasMarkedText() );
    d->popupMenu->setItemEnabled( Paste, flag &&
                                 !QApplication::clipboard()->text().isEmpty());
    d->popupMenu->setItemEnabled( Clear, flag && !text().isEmpty() );
    d->popupMenu->setItemEnabled( Unselect, hasMarkedText() );
    d->popupMenu->setItemEnabled( SelectAll, flag && !allMarked );
}

void KLineEdit::dropEvent(QDropEvent *e)
{
    KURL::List urlList;
    if( d->handleURLDrops && KURLDrag::decode( e, urlList ) )
    {
        QString dropText;
        KURL::List::ConstIterator it;
        for( it = urlList.begin() ; it != urlList.end() ; ++it )
        {
            if(!dropText.isEmpty())
                dropText+=' ';

            dropText += (*it).prettyURL();
        }

        if( !text().isEmpty() )
            dropText=' '+dropText;

        end(false);
        insert(dropText);

        e->accept();
    }
    else
    {
        QLineEdit::dropEvent(e);
    }
}

bool KLineEdit::eventFilter( QObject* o, QEvent* ev )
{
    if( o == this )
    {
        KCursor::autoHideEventFilter( this, ev );
        if( ev->type() == QEvent::KeyPress )
        {
            QKeyEvent *e = static_cast<QKeyEvent *>( ev );
            int key = e->key();
            if( (e->state() == Qt::NoButton) &&
                (key == Qt::Key_Return || key == Qt::Key_Enter) )
            {
                emit QLineEdit::returnPressed();
                emit returnPressed( displayText() );

                bool trap = d->completionBox && d->completionBox->isVisible();
                if ( trap )
                    hideCompletionBox();
                // don't go to QLineEdit::eventFilter!
                return  d->grabReturnKeyEvents || trap;
            }
        }
    }
    return QLineEdit::eventFilter( o, ev );
}


void KLineEdit::setURLDropsEnabled(bool enable)
{
    d->handleURLDrops=enable;
}

bool KLineEdit::isURLDropsEnabled() const
{
    return d->handleURLDrops;
}

void KLineEdit::setTrapReturnKey( bool grab )
{
    d->grabReturnKeyEvents = grab;
}

bool KLineEdit::trapReturnKey() const
{
    return d->grabReturnKeyEvents;
}

void KLineEdit::setURL( const KURL& url )
{
    QString text = url.prettyURL();
    int curpos = text.length();
    validateAndSet( text, curpos, curpos, curpos  );
}

void KLineEdit::makeCompletionBox()
{
    if ( d->completionBox )
        return;

    d->completionBox = new KCompletionBox( this, "completion box" );
    connect( d->completionBox, SIGNAL( highlighted( const QString& )),
                 SLOT( setCompletedText( const QString& )));
    connect( d->completionBox, SIGNAL( userCancelled( const QString& ) ),
                 SLOT( setText(const QString&) ) );

}

// FIXME: make pure virtual in KCompletionBase!
void KLineEdit::setCompletedItems( const QStringList& items )
{
    if ( completionMode() == KGlobalSettings::CompletionPopup ||
         completionMode() == KGlobalSettings::CompletionShell )
    {
        if ( !items.isEmpty() )
        {
            if ( !d->completionBox )
                makeCompletionBox();

            d->origCursorPos = cursorPosition();
            d->completionBox->setCancelledText( text() );
            d->completionBox->clear();
            d->completionBox->insertStringList( items );
            d->completionBox->popup();
        }
        else
            hideCompletionBox();
    }
    else {
        if ( !items.isEmpty() ) // fallback
            setCompletedText( items.first() );
    }
}

// ### merge these two for 3.0
KCompletionBox * KLineEdit::completionBox()
{
    makeCompletionBox();
    return d->completionBox;
}

KCompletionBox * KLineEdit::completionBox( bool create )
{
    if ( create )
	makeCompletionBox();
	
    return d->completionBox;
}

void KLineEdit::setCompletionObject( KCompletion* comp, bool hsig )
{
    KCompletion *oldComp = completionObject( false, false ); // don't create!
    if ( oldComp && handleSignals() )
        disconnect( oldComp, SIGNAL( matches( const QStringList& )),
                    this, SLOT( setCompletedItems( const QStringList& )));
    if ( comp && hsig )
      connect( comp, SIGNAL( matches( const QStringList& )),
               this, SLOT( setCompletedItems( const QStringList& )));

    KCompletionBase::setCompletionObject( comp, hsig );
}

QPopupMenu* KLineEdit::contextMenu()
{
    d->hasReference = true;
    return contextMenuInternal();
}

QPopupMenu* KLineEdit::contextMenuInternal()
{
    if( !d->popupMenu )
    {
        d->popupMenu = new QPopupMenu( this );
        d->popupMenu->insertItem( SmallIconSet("editcut"), i18n( "Cut" ), Cut );
        d->popupMenu->insertItem( SmallIconSet("editcopy"), i18n( "Copy" ), Copy );
        d->popupMenu->insertItem( SmallIconSet("editpaste"), i18n( "Paste" ), Paste );
        d->popupMenu->insertItem( SmallIconSet("editclear"), i18n( "Clear" ), Clear );
        // Create and insert the completion sub-menu iff a completion object is present.
        if ( compObj() )
        {
            d->subMenu = new QPopupMenu( d->popupMenu );
            d->popupMenu->insertSeparator();
            d->popupMenu->insertItem( SmallIconSet("completion"), i18n("Completion"), d->subMenu );
        }
        d->popupMenu->insertSeparator();
        d->popupMenu->insertItem( i18n( "Unselect" ), Unselect );
        d->popupMenu->insertItem( i18n( "Select All" ), SelectAll );
    }
    return d->popupMenu;
}

void KLineEdit::hideCompletionBox()
{
    if ( d->completionBox )
    {
        d->origCursorPos = -1;
        d->completionBox->hide();
        d->completionBox->clear();
        d->prevText = QString::null;
    }
}

// QWidget::create() turns off mouse-Tracking which would break auto-hiding
void KLineEdit::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
    QLineEdit::create( id, initializeWindow, destroyOldWindow );
    KCursor::setAutoHideCursor( this, true, true );
}
