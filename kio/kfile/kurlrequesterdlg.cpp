/* This file is part of the KDE libraries
    Copyright (C) 2000 Wilco Greven <greven@kde.org>

    library is free software; you can redistribute it and/or
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


#include <sys/stat.h>
#include <unistd.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qtoolbutton.h>

#include <kfiledialog.h>
#include <kglobal.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <krecentdocument.h>
#include <kurl.h>
#include <kurlrequester.h>

#include "kurlrequesterdlg.h"


KUrlRequesterDlg::KUrlRequesterDlg( const QString& urlName, QWidget *parent)
    :   KDialog( parent )
{
  setButtons( Ok | Cancel | User1 );
  setButtonGuiItem( User1, KStandardGuiItem::clear() );

  initDialog(i18n( "Location:" ), urlName);
}

KUrlRequesterDlg::KUrlRequesterDlg( const QString& urlName, const QString& _text, QWidget *parent)
    :   KDialog( parent )
{
  setButtons( Ok | Cancel | User1 );
  setButtonGuiItem( User1, KStandardGuiItem::clear() );

  initDialog(_text, urlName);
}

KUrlRequesterDlg::~KUrlRequesterDlg()
{
}

void KUrlRequesterDlg::initDialog(const QString &text,const QString &urlName)
{
  setDefaultButton(Ok);
  showButtonSeparator(true);
   QFrame *plainPage=new QFrame(this);
   setMainWidget(plainPage);
   QVBoxLayout * topLayout = new QVBoxLayout( plainPage );
   topLayout->setMargin( 0 );
   topLayout->setSpacing( spacingHint() );

    QLabel * label = new QLabel( text , plainPage );
    topLayout->addWidget( label );

    urlRequester_ = new KUrlRequester( urlName, plainPage);
    urlRequester_->setMinimumWidth( urlRequester_->sizeHint().width() * 3 );
    topLayout->addWidget( urlRequester_ );
    urlRequester_->setFocus();
    connect( urlRequester_->lineEdit(), SIGNAL(textChanged(const QString&)),
             SLOT(slotTextChanged(const QString&)) );
    bool state = !urlName.isEmpty();
    enableButtonOk( state );
    enableButton( KDialog::User1, state );
    /*
    KFile::Mode mode = static_cast<KFile::Mode>( KFile::File |
            KFile::ExistingOnly );
	urlRequester_->setMode( mode );
    */
    connect( this, SIGNAL( user1Clicked() ), SLOT( slotClear() ) );
    resize(minimumSize());
}

void KUrlRequesterDlg::slotTextChanged(const QString & text)
{
    bool state = !text.trimmed().isEmpty();
    enableButtonOk( state );
    enableButton( KDialog::User1, state );
}

void KUrlRequesterDlg::slotClear()
{
    urlRequester_->clear();
}

KUrl KUrlRequesterDlg::selectedUrl() const
{
    if ( result() == QDialog::Accepted )
        return urlRequester_->url();
    else
        return KUrl();
}


KUrl KUrlRequesterDlg::getUrl(const QString& dir, QWidget *parent,
                              const QString& caption)
{
    KUrlRequesterDlg dlg(dir, parent);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.exec();

    const KUrl& url = dlg.selectedUrl();
    if (url.isValid())
        KRecentDocument::add(url);

    return url;
}

KFileDialog * KUrlRequesterDlg::fileDialog()
{
    return urlRequester_->fileDialog();
}

KUrlRequester * KUrlRequesterDlg::urlRequester()
{
    return urlRequester_;
}

#include "kurlrequesterdlg.moc"

// vim:ts=4:sw=4:tw=78
