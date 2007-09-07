/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#include "kkeysequencewidget.h"
#include "kkeysequencewidget_p.h"

#include <config.h>

#include "kkeyserver.h"
#include "kiconloader.h"

//TODO: remove unneeded includes
#include <QtGui/QCursor>
#include <QtGui/qdrawutil.h>
#include <QtGui/QPainter>
#include <QPolygon>
#include <QStyle>
#include <QKeyEvent>
#include <QTimer>
#include <QHBoxLayout>
#include <QToolButton>
#include <QApplication>

#include <kdebug.h>
#include <kglobalaccel.h>
#include <kicon.h>
#include <klocale.h>


#if 0
#ifdef Q_WS_X11
#define XK_XKB_KEYS
#define XK_MISCELLANY
#include <X11/Xlib.h>	// For x11Event()
#include <X11/keysymdef.h> // For XK_...

#ifdef KeyPress
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyRelease
#undef KeyPress
#undef FocusOut
#undef FocusIn
#endif // KeyPress
#endif // Q_WS_X11*/

//static const char* psTemp[] = {
//  I18N_NOOP("Primary"), I18N_NOOP("Alternate"), I18N_NOOP("Multi-Key")
//};
#endif

/***********************************************************************/
/* KKeySequenceWidget                                                  */
/*                                                                     */
/* Initially added by Mark Donohoe <donohoe@kde.org>                   */
/*                                                                     */
/***********************************************************************/

class KKeySequenceWidgetPrivate
{
public:
	KKeySequenceWidgetPrivate(KKeySequenceWidget *q): q(q) {}

	void init();

	static QKeySequence appendToSequence(const QKeySequence& seq, int keyQt);
	static bool isOkWhenModifierless(int keyQt);

	void updateShortcutDisplay();
	void startRecording();
//private slot
	void doneRecording(bool validate = true);
	
//members
	KKeySequenceWidget *const q;
	QHBoxLayout *layout;
	KKeySequenceButton *keyButton;
	QToolButton *clearButton;

	QKeySequence keySequence;
	QKeySequence oldKeySequence;
	QTimer modifierlessTimeout;
	bool allowModifierless;
	uint nKey;
	uint modifierKeys;
	bool isRecording;
	bool denyValidation;
};


KKeySequenceWidget::KKeySequenceWidget(QWidget *parent)
 : QWidget(parent),
   d(new KKeySequenceWidgetPrivate(this))
{
	d->init();
	connect(d->keyButton, SIGNAL(clicked()), this, SLOT(captureKeySequence()));
	connect(d->clearButton, SIGNAL(clicked()), this, SLOT(clearKeySequence()));
	connect(&d->modifierlessTimeout, SIGNAL(timeout()), this, SLOT(doneRecording()));
	//TODO: how to adopt style changes at runtime?
	/*QFont modFont = d->clearButton->font();
	modFont.setStyleHint(QFont::TypeWriter);
	d->clearButton->setFont(modFont);*/
	d->updateShortcutDisplay();
}


void KKeySequenceWidgetPrivate::init()
{
	isRecording = false;
	allowModifierless = false;
	
	layout = new QHBoxLayout(q);

	keyButton = new KKeySequenceButton(this, q);
	keyButton->setFocusPolicy(Qt::StrongFocus);
	keyButton->setIcon(KIcon("configure"));
	layout->addWidget(keyButton);

	clearButton = new QToolButton(q);
	layout->addWidget(clearButton);

	if (qApp->isLeftToRight())
		clearButton->setIcon(KIcon("clear-left"));
	else
		clearButton->setIcon(KIcon("locationbar-erase"));
}


KKeySequenceWidget::~KKeySequenceWidget ()
{
	delete d;
}


void KKeySequenceWidget::setModifierlessAllowed(bool allow)
{
	d->allowModifierless = allow;
}


bool KKeySequenceWidget::isModifierlessAllowed()
{
	return d->allowModifierless;
}


void KKeySequenceWidget::setClearButtonShown(bool show)
{
	d->clearButton->setVisible(show);
}


//slot
void KKeySequenceWidget::captureKeySequence()
{
	d->startRecording();
}


QKeySequence KKeySequenceWidget::keySequence() const
{
	return d->keySequence;
}


//slot
void KKeySequenceWidget::setKeySequence(const QKeySequence &seq, Validation validate)
{
	d->oldKeySequence = d->keySequence;
	d->keySequence = seq;
	d->doneRecording(validate == Validate);
}


//slot
void KKeySequenceWidget::clearKeySequence()
{
	setKeySequence(QKeySequence());
}


//slot
void KKeySequenceWidget::denyValidation()
{
	d->denyValidation = true;
}


void KKeySequenceButton::setText(const QString &text)
{
	QPushButton::setText(text);
	//setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}


void KKeySequenceWidgetPrivate::startRecording()
{
	nKey = 0;
	modifierKeys = 0;
	oldKeySequence = keySequence;
	keySequence = QKeySequence();
	isRecording = true;
	keyButton->grabKeyboard();
	keyButton->setDown(true);
	updateShortcutDisplay();
}


void KKeySequenceWidgetPrivate::doneRecording(bool validate)
{
	modifierlessTimeout.stop();
	isRecording = false;
	keyButton->releaseKeyboard();
	keyButton->setDown(false);
	if (keySequence != oldKeySequence) {
		if (validate) {
			denyValidation = false;
			emit q->validationHook(keySequence);
			if (denyValidation)
				keySequence = oldKeySequence;
		}
	}
	updateShortcutDisplay();
	if (keySequence != oldKeySequence)
		emit q->keySequenceChanged(keySequence);
}


void KKeySequenceWidgetPrivate::updateShortcutDisplay()
{
	//empty string if no non-modifier was pressed
	QString s = keySequence.toString();
	s.replace('&', QLatin1String("&&"));

	if (isRecording) {
		// Display modifiers for the first key in the QKeySequence
		if (nKey == 0) {
			if (modifierKeys) {
				if (modifierKeys & Qt::META)  s += KKeyServer::modToStringUser(Qt::META) + '+';
#if defined(Q_WS_MAC)
                if (modifierKeys & Qt::ALT)   s += KKeyServer::modToStringUser(Qt::ALT) + '+';
				if (modifierKeys & Qt::CTRL)  s += KKeyServer::modToStringUser(Qt::CTRL) + '+';
#elif defined(Q_WS_X11)
				if (modifierKeys & Qt::CTRL)  s += KKeyServer::modToStringUser(Qt::CTRL) + '+';
				if (modifierKeys & Qt::ALT)   s += KKeyServer::modToStringUser(Qt::ALT) + '+';
#endif
				if (modifierKeys & Qt::SHIFT) s += KKeyServer::modToStringUser(Qt::SHIFT) + '+';
			} else
				s = i18nc("What the user inputs now will be taken as the new shortcut", "Input");
		}
		//make it clear that input is still going on
		s.append(" ...");
	}

	if (s.isEmpty())
		s = "None";

	s.prepend(' ');
	s.append(' ');
	keyButton->setText(s);
}


KKeySequenceButton::~KKeySequenceButton()
{
}


//prevent Qt from special casing Tab and Backtab
bool KKeySequenceButton::event (QEvent* e)
{
	if (d->isRecording && e->type() == QEvent::KeyPress) {
		keyPressEvent(static_cast<QKeyEvent *>(e));
		return true;
	}

	return QPushButton::event(e);
}


void KKeySequenceButton::keyPressEvent(QKeyEvent *e)
{
	int keyQt = e->key();
	uint newModifiers = e->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);

	//don't have the return or space key appear as first key of the sequence when they
	//were pressed to start editing - catch and them and imitate their effect
	if (!d->isRecording && ((keyQt == Qt::Key_Return || keyQt == Qt::Key_Space))) {
		d->startRecording();
		d->modifierKeys = newModifiers;
		d->updateShortcutDisplay();
		return;
	}

	if (!d->isRecording) {
		QPushButton::keyPressEvent(e);
		return;
	}

	if (d->nKey == 0)
		d->modifierKeys = newModifiers;

	switch(keyQt) {
	case Qt::Key_AltGr: //or else we get unicode salad
		return;
	case Qt::Key_Shift:
	case Qt::Key_Control:
	case Qt::Key_Alt:
	case Qt::Key_Meta:
	case Qt::Key_Menu: //unused (yes, but why?)
		// If we are editing the first key in the sequence,
		// display modifier keys which are held down
		if(d->nKey == 0)
			d->updateShortcutDisplay();
		break;
	default:
		//Shift is not a modifier in the sense of Ctrl/Alt/WinKey
		if (!(d->modifierKeys & ~Qt::SHIFT)
		    && !KKeySequenceWidgetPrivate::isOkWhenModifierless(keyQt)) {
			if (!d->allowModifierless)
				return;
			else
				d->modifierlessTimeout.start(600);
		}

		if (keyQt) {
			if (d->nKey == 0) {
				d->keySequence =
				  KKeySequenceWidgetPrivate::appendToSequence(d->keySequence,
				                                              keyQt | d->modifierKeys);
			} else {
				d->keySequence =
				  KKeySequenceWidgetPrivate::appendToSequence(d->keySequence, keyQt);
			}

			d->nKey++;
			if (d->nKey >= 4) {
				d->doneRecording();
				return;
			}
			d->updateShortcutDisplay();
		}
	}
}


void KKeySequenceButton::keyReleaseEvent(QKeyEvent *e)
{
	if (!d->isRecording) {
		QPushButton::keyReleaseEvent(e);
		return;
	}

	uint newModifiers = e->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);

	//if a modifier that belongs to the shortcut was released...
	if ((newModifiers & d->modifierKeys) < d->modifierKeys) {
		if (d->nKey == 0) {
			d->modifierKeys = newModifiers;
			d->updateShortcutDisplay();
		} else
			d->doneRecording();
	}
}


//static
QKeySequence KKeySequenceWidgetPrivate::appendToSequence(const QKeySequence& seq, int keyQt)
{
	switch (seq.count()) {
	case 0:
		return QKeySequence(keyQt);
	case 1:
		return QKeySequence(seq[0], keyQt);
	case 2:
		return QKeySequence(seq[0], seq[1], keyQt);
	case 3:
		return QKeySequence(seq[0], seq[1], seq[2], keyQt);
	default:
		return seq;
	}
}


//static
bool KKeySequenceWidgetPrivate::isOkWhenModifierless(int keyQt)
{
	//this whole function is a hack, but especially the first line of code
	if (QKeySequence(keyQt).toString().length() == 1)
		return false;

	switch (keyQt) {
	case Qt::Key_Return:
	case Qt::Key_Space:
	case Qt::Key_Tab:
	case Qt::Key_Backtab: //does this ever happen?
	case Qt::Key_Backspace:
	case Qt::Key_Delete:
		return false;
	default:
		return true;
	}
}

#include "kkeysequencewidget.moc"
#include "kkeysequencewidget_p.moc"
