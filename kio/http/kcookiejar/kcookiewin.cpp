/*
This file is part of KDE

  Copyright (C) 2000 Waldo Bastian (bastian@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookie Dialogs
// $Id$

#include "kcookiewin.h"
#include "kcookiejar.h"

#include <klocale.h>
#include <kapp.h>
#include <kwin.h>
#include <kglobal.h>

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qgroupbox.h>
#include <qdatetime.h>

#include <qmessagebox.h>

// no need to include Xlib.h as long as we don't do that XSetTransientForHint
// call anyway
//#include <X11/Xlib.h> // for XSetTransientForHint() 
        
KCookieWin::KCookieWin(QWidget *parent, KHttpCookie *_cookie, KCookieJar *cookiejar) :
    KDialogBase( i18n("Cookie Alert"), 
		KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
		KDialogBase::Yes, KDialogBase::No, 
		parent, 
		"cookiealert", true, true,
                 i18n("&Accept"), i18n("&Reject"), i18n("&View >>")),
    cookie(_cookie)
{
    KWin::setState( winId(), NET::StaysOnTop );
#if 0
    if (cookie->windowId())
    {
       XSetTransientForHint( x11Display(), winId(), cookie->windowId());
    }
#endif
    // Always place the dialog on the current desktop.
    KWin::setOnDesktop(winId(), KWin::currentDesktop());

    QWidget *contents = new QWidget(this);

    layout = new QGridLayout(contents, 5, 5,
	KDialog::marginHint(), 
        KDialog::spacingHint());

    layout->setColStretch(0, 0);
    layout->setColStretch(1, 1);
    layout->setRowStretch(0, 0);
    layout->setRowStretch(1, 1);

    layout->addColSpacing(2, KDialog::spacingHint());
    layout->addColSpacing(3, KDialog::spacingHint());
    layout->addRowSpacing(3, KDialog::spacingHint());

    QLabel *icon = new QLabel( contents );
    icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()));
    layout->addMultiCellWidget(icon, 0, 2, 0, 0);

    QLabel *text1 = new QLabel( 
	i18n("You received a cookie from host:"),
	contents);
    layout->addWidget(text1, 0, 2, AlignLeft | AlignTop);

    QLabel *text2 = new QLabel( cookie->host(), contents);
    layout->addWidget(text2, 1, 2, AlignCenter);

    QLabel *text3 = new QLabel( i18n("Do you want to accept or reject this cookie?"), contents);
    layout->addWidget( text3, 2, 2, AlignLeft | AlignTop);

    QVButtonGroup *bg = new QVButtonGroup( i18n("Apply to:"), contents);
    bg->setExclusive( true );
    layout->addMultiCellWidget(bg , 4, 4, 0, 2);

    int defaultRadioButton = cookiejar->defaultRadioButton;

    rb1 = new QRadioButton( i18n("&This cookie only"), bg );
    rb1->adjustSize();
    if (defaultRadioButton == 0)
      rb1->setChecked( true );
     
    rb2 = new QRadioButton( i18n("All cookies from this &domain"), bg );
    rb2->adjustSize();
    if (defaultRadioButton == 1)
      rb2->setChecked( true );

    rb3 = new QRadioButton( i18n("All &cookies"), bg );
    rb3->adjustSize();
    if (defaultRadioButton == 2)
      rb3->setChecked( true );

    bg->adjustSize();

    setMainWidget(contents);
    enableButtonSeparator(false);

// new QLabel( i18n("Do you want to accept or reject this cookie?"), this, "_msg" );
}

void KCookieWin::slotCancel()
{
	QGroupBox *viewgroup = new QGroupBox(1, Qt::Horizontal, i18n("Cookie details"), mainWidget());
	QWidget *viewbox = new QWidget(viewgroup);

	QVBoxLayout *viewlayout = new QVBoxLayout(viewbox, KDialog::marginHint(),
				   KDialog::spacingHint());

	viewlayout->addWidget( new QLabel(i18n("Target Path: %1").arg(cookie->path()),
    	viewbox));
	// cookies can be large, truncate to avoid problems
	QString cookieval = cookie->value();
	cookieval.truncate(100);
	viewlayout->addWidget( new QLabel(i18n("Value: %1").arg(cookieval), viewbox));
	QDateTime cookiedate;
	cookiedate.setTime_t(cookie->expireDate());
	viewlayout->addWidget( new QLabel(i18n("Expires On: %1").arg(
		(cookie->expireDate()) ? KGlobal::locale()->formatDateTime(cookiedate) : "End of Session"),
    	viewbox));
	viewlayout->addWidget( new QLabel(i18n("Protocol Version: %1").arg(cookie->protocolVersion()), viewbox));
	viewlayout->addWidget( new QLabel(i18n("Is Secure: %1").arg(cookie->isSecure() ? "True" : "False"),
    	viewbox));

	viewgroup->show();
	layout->addMultiCellWidget(viewgroup, 0, 4, 4, 4);
	enableButtonCancel(false);
}

KCookieWin::~KCookieWin()
{
}

KCookieAdvice
KCookieWin::advice(KCookieJar *cookiejar)
{
   int result = exec();
   if (rb1->isChecked())
      cookiejar->defaultRadioButton = 0;
   if (rb2->isChecked())
      cookiejar->defaultRadioButton = 1;
   if (rb3->isChecked())
      cookiejar->defaultRadioButton = 2;

   if (result == Yes)
   {
      if (rb2->isChecked())
         cookiejar->setDomainAdvice( cookie, KCookieAccept);
      else if (rb3->isChecked())
         cookiejar->setGlobalAdvice( KCookieAccept );
      return KCookieAccept;
   }
   else
   {
      if (rb2->isChecked())
         cookiejar->setDomainAdvice( cookie, KCookieReject);
      else if (rb3->isChecked())
         cookiejar->setGlobalAdvice( KCookieReject );
      return KCookieReject;
   } 
   return KCookieReject; // Never reached
}

