/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Ellis Whitehead <ellis@kde.org>

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

#include "kshortcutdialog.h"

#ifdef Q_WS_X11
	#define XK_XKB_KEYS
	#define XK_MISCELLANY
	#include <X11/Xlib.h>	// For x11Event()
	#include <X11/keysymdef.h> // For XK_...

	#ifdef KeyPress
		const int XKeyPress = KeyPress;
		const int XKeyRelease = KeyRelease;
		const int XFocusOut = FocusOut;
		const int XFocusIn = FocusIn;
		#undef KeyRelease
		#undef KeyPress
		#undef FocusOut
		#undef FocusIn
	#endif
#endif

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qradiobutton.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kkeynative.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

KShortcutDialog::KShortcutDialog( const KShortcut& shortcut, bool bQtShortcut, QWidget* parent, const char* name )
: KIShortcutDialog( parent, name )
{
	m_bQtShortcut = bQtShortcut;

	m_bGrab = false;
	m_iSeq = 0;
	m_iKey = 0;
	m_ptxtCurrent = 0;
	m_bRecording = false;
	m_mod = 0;

	m_btnClearPrimary->setPixmap( SmallIcon( "locationbar_erase" ) );
	m_btnClearAlternate->setPixmap( SmallIcon( "locationbar_erase" ) );

	KGuiItem ok = KStdGuiItem::ok();
	ok.setText( i18n( "OK" ) );
	m_btnOK->setGuiItem( ok );

	KGuiItem cancel = KStdGuiItem::cancel();
	cancel.setText( i18n( "Cancel" ) );
	m_btnCancel->setGuiItem( cancel );
	m_btnCancel_2->setGuiItem( cancel );

	m_frameMore->hide();
	setShortcut( shortcut );
	resize( 0, 0 );

	#ifdef Q_WS_X11
	kapp->installX11EventFilter( this );	// Allow button to capture X Key Events.
	#endif
}

KShortcutDialog::~KShortcutDialog()
{
}

void KShortcutDialog::setShortcut( const KShortcut & shortcut )
{
	m_shortcut = shortcut;
	updateShortcutDisplay();
}

void KShortcutDialog::updateShortcutDisplay()
{
	QString s[2] = { m_shortcut.seq(0).toString(), m_shortcut.seq(1).toString() };

	if( m_bRecording ) {
		m_ptxtCurrent->setDefault( true );
		m_ptxtCurrent->setFocus();

		// Display modifiers for the first key in the KKeySequence
		if( m_iKey == 0 ) {
			if( m_mod ) {
				QString keyModStr;
				if( m_mod & KKey::WIN )   keyModStr += KKey::modFlagLabel(KKey::WIN) + "+";
				if( m_mod & KKey::ALT )   keyModStr += KKey::modFlagLabel(KKey::ALT) + "+";
				if( m_mod & KKey::CTRL )  keyModStr += KKey::modFlagLabel(KKey::CTRL) + "+";
				if( m_mod & KKey::SHIFT ) keyModStr += KKey::modFlagLabel(KKey::SHIFT) + "+";
				s[m_iSeq] = keyModStr;
	}
		}
		// When in the middle of entering multi-key shortcuts,
		//  add a "," to the end of the displayed shortcut.
		else
			s[m_iSeq] += ",";
	}
	else {
		m_txtPrimary->setDefault( false );
		m_txtAlternate->setDefault( false );
		this->setFocus();
	}

	m_txtShortcut->setText( s[0] );
	m_txtPrimary->setText( s[0] );
	m_txtAlternate->setText( s[1] );

	// Determine the enable state of the 'Less' button
	bool bLessOk;
	// If there is no shortcut defined,
	if( m_shortcut.count() == 0 )
		bLessOk = true;
	// If there is a single shortcut defined, and it is not a multi-key shortcut,
	else if( m_shortcut.count() == 1 && m_shortcut.seq(0).count() <= 1 )
		bLessOk = true;
	// Otherwise, we have an alternate shortcut or multi-key shortcut(s).
	else
		bLessOk = false;
	m_btnLess->setEnabled( bLessOk );
}

void KShortcutDialog::slotShowMore()
{
	m_bRecording = false;
	m_iSeq = 0;
	m_iKey = 0;

	m_frameMore->show();
	m_frameLess->hide();
	//m_btnPrimary->setFocus();
	this->setFocus();
	m_btnPrimary->setChecked( true );
	slotSelectPrimary();
}

void KShortcutDialog::slotShowLess()
{
	m_bRecording = false;
	m_iSeq = 0;
	m_iKey = 0;

	m_ptxtCurrent = m_txtShortcut;
	m_frameMore->hide();
	m_frameLess->show();
	m_txtShortcut->setDefault( true );
	m_txtShortcut->setFocus();
	m_btnMultiKey->setChecked( false );

	// FIXME: why doesn't this work?  I want the dialog to try to set it's minimum size.
	resize( 1, 1 );
}

void KShortcutDialog::slotSelectPrimary()
{
	m_bRecording = false;
	m_iSeq = 0;
	m_iKey = 0;
	m_ptxtCurrent = m_txtPrimary;
	this->setFocus();
	updateShortcutDisplay();
}
        
void KShortcutDialog::slotSelectAlternate()
{
	m_bRecording = false;
	m_iSeq = 1;
	m_iKey = 0;
	m_ptxtCurrent = m_txtAlternate;
	this->setFocus();
	updateShortcutDisplay();
}

void KShortcutDialog::slotClearPrimary()
{
	m_shortcut.setSeq( 0, KKeySequence() );
	m_btnPrimary->setChecked( true );
	slotSelectPrimary();
}

void KShortcutDialog::slotClearAlternate()
{
	if( m_shortcut.count() == 2 )
		m_shortcut.init( m_shortcut.seq(0) );
	m_btnAlternate->setChecked( true );
	slotSelectAlternate();
}

void KShortcutDialog::slotMultiKeyMode( bool bOn )
{
	// If turning off multi-key mode during a recording,
	if( !bOn && m_bRecording ) {
		m_bRecording = false;
	m_iKey = 0;
		updateShortcutDisplay();
	}
}

void KShortcutDialog::showEvent( QShowEvent * pEvent )
{
	//kdDebug(125) << "KShortcutDialog::showEvent" << endl;
	if( m_shortcut.count() == 1 )
		slotShowLess();
	else
		slotShowMore();

	KDialog::showEvent( pEvent );
}

// TODO: delete me
void KShortcutDialog::hideEvent( QHideEvent * pEvent )
{
	//kdDebug(125) << "KShortcutDialog::hideEvent" << endl;
	KDialog::hideEvent( pEvent );
}

// TODO: delete me
void KShortcutDialog::paintEvent( QPaintEvent * pEvent )
{
	//kdDebug(125) << "KShortcutDialog::paintEvent" << endl;
	KDialog::paintEvent( pEvent );
}

#ifdef Q_WS_X11
bool KShortcutDialog::x11Event( XEvent *pEvent )
{
	switch( pEvent->type ) {
		case XKeyPress:
			x11KeyPressEvent( pEvent );
			return true;
		case XKeyRelease:
			x11KeyReleaseEvent( pEvent );
				return true;
		case XFocusIn:
			if (!m_bGrab) {
				//kdDebug(125) << "FocusIn and Grab!" << endl;
				grabKeyboard();
				m_bGrab = true;
			}
			//else
			//	kdDebug(125) << "FocusIn" << endl;
			break;
		case XFocusOut:
			if (m_bGrab) {
				//kdDebug(125) << "FocusOut and Ungrab!" << endl;
				releaseKeyboard();
				m_bGrab = false;
			}
			//else
			//	kdDebug(125) << "FocusOut" << endl;
			break;
		default:
			//kdDebug(125) << "x11Event->type = " << pEvent->type << endl;
			break;
	}
	return QWidget::x11Event( pEvent );
}

static uint getModsFromModX( uint keyModX )
{
	uint mod = 0;
	if( keyModX & KKeyNative::modX(KKey::SHIFT) ) mod += KKey::SHIFT;
	if( keyModX & KKeyNative::modX(KKey::CTRL) )  mod += KKey::CTRL;
	if( keyModX & KKeyNative::modX(KKey::ALT) )   mod += KKey::ALT;
	if( keyModX & KKeyNative::modX(KKey::WIN) )   mod += KKey::WIN;
	return mod;
}

static bool convertSymXToMod( uint keySymX, uint* pmod )
{
	switch( keySymX ) {
		// Don't allow setting a modifier key as an accelerator.
		// Also, don't release the focus yet.  We'll wait until
		//  we get a 'normal' key.
		case XK_Shift_L:   case XK_Shift_R:   *pmod = KKey::SHIFT; break;
		case XK_Control_L: case XK_Control_R: *pmod = KKey::CTRL; break;
		case XK_Alt_L:     case XK_Alt_R:     *pmod = KKey::ALT; break;
		// FIXME: check whether the Meta or Super key are for the Win modifier
		case XK_Meta_L:    case XK_Meta_R:
		case XK_Super_L:   case XK_Super_R:   *pmod = KKey::WIN; break;
		case XK_Hyper_L:   case XK_Hyper_R:
		case XK_Mode_switch:
		case XK_Num_Lock:
		case XK_Caps_Lock:
			break;
		default:
			return false;
				}
	return true;
}

void KShortcutDialog::x11KeyPressEvent( XEvent* pEvent )
{
	KKeyNative keyNative( pEvent );
	uint keyModX = keyNative.mod();
	uint keySymX = keyNative.sym();

	m_mod = getModsFromModX( keyModX );

	if( keySymX ) {
		m_bRecording = true;

		uint mod = 0;
		if( convertSymXToMod( keySymX, &mod ) ) {
			if( mod )
				m_mod |= mod;
		}
		else
			keyPressed( KKey(keyNative) );
	}
	updateShortcutDisplay();
}

void KShortcutDialog::x11KeyReleaseEvent( XEvent* pEvent )
{
	// We're only interested in the release of modifier keys,
	//  and then only when it's for the first key in a sequence.
	if( m_bRecording && m_iKey == 0 ) {
		KKeyNative keyNative( pEvent );
		uint keyModX = keyNative.mod();
		uint keySymX = keyNative.sym();

		m_mod = getModsFromModX( keyModX );

		uint mod = 0;
		if( convertSymXToMod( keySymX, &mod ) && mod ) {
			m_mod &= ~mod;
			if( !m_mod )
				m_bRecording = false;
		}
		updateShortcutDisplay();
	}
}

#endif // QT_WS_X11

void KShortcutDialog::keyPressed( KKey key )
{
	kdDebug(125) << "keyPressed: " << key.toString() << endl;

	key.simplify();
	if( m_bQtShortcut ) {
		key = key.keyCodeQt();
		if( key.isNull() ) {
			// TODO: message box about key not able to be used as application shortcut
		}
	}

	KKeySequence seq;
	if( m_iKey == 0 )
		seq = key;
	else {
		// Remove modifiers
		key.init( key.sym(), 0 );
		seq = m_shortcut.seq( m_iSeq );
		seq.setKey( m_iKey, key );
	}

	m_shortcut.setSeq( m_iSeq, seq );

	m_mod = 0;
	if( m_btnMultiKey->isChecked() && m_iKey < KKeySequence::MAX_KEYS - 1 )
		m_iKey++;
	else {
		m_iKey = 0;
		m_bRecording = false;
	}

	updateShortcutDisplay();

	if( !m_btnMultiKey->isChecked() )
		accept();
}

#include "kshortcutdialog.moc"
