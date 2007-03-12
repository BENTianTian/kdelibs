/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2007 Olivier Goffart <ogoffart at kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kpassworddialog.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QTextDocument>
#include <QTimer>

#include <kcombobox.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <khbox.h>
#include <kdebug.h>
#include <kconfiggroup.h>

#include "ui_kpassworddialog.h"

/** @internal */
class KPasswordDialog::KPasswordDialogPrivate
{
public:
    KPasswordDialogPrivate(KPasswordDialog *q)
        : q(q),
          userEditCombo(0),
          pixmapLabel(0),
          commentRow(0)
    {}

    void actuallyAccept();
    void activated( const QString& userName );

    void init( const KPasswordDialogFlags& flags );

    KPasswordDialog *q;
    Ui_KPasswordDialog ui;
    QMap<QString,QString> knownLogins;
    KComboBox* userEditCombo;
    QLabel* pixmapLabel;
    unsigned int commentRow;
};

static QString qrichtextify( const QString& text )
{
    if ( text.isEmpty() || text[0] == '<' ) {
        return text;
    }

    QStringList lines = text.split('\n', QString::SkipEmptyParts);
    for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
    {
        *it = Qt::convertFromPlainText( *it, Qt::WhiteSpaceNormal );
    }

    return lines.join(QString());
}

KPasswordDialog::KPasswordDialog( QWidget* parent ,
                                  const KPasswordDialogFlags& flags,
                                  const KDialog::ButtonCodes otherButtons )
   : KDialog( parent ), d(new KPasswordDialogPrivate(this))
{
    setCaption( i18n("Password") );
    setButtons( Ok | Cancel | otherButtons );
    showButtonSeparator( true );
    setDefaultButton( Ok );
    d->init ( flags );
}



KPasswordDialog::~KPasswordDialog()
{
    delete d;
}

void KPasswordDialog::KPasswordDialogPrivate::init( const KPasswordDialogFlags& flags )
{
    ui.setupUi( q->mainWidget() );

    ui.errorMessage->setVisible( false );
    ui.errorMessage->setFrameShape( QFrame::StyledPanel );
    ui.errorMessage->setFrameShadow( QFrame::Plain );
    ui.errorMessage->setBackgroundRole( QPalette::Base );

    // Row 4: Username field
    if ( flags & KPasswordDialog::ShowUsernameLine ) {
        ui.userEdit->setFocus();
        QObject::connect( ui.userEdit, SIGNAL(returnPressed()), ui.passEdit, SLOT(setFocus()) );
    } else {
        ui.userNameLabel->hide();
        ui.userEdit->hide();
        ui.passEdit->setFocus();
    }

    if ( flags ^ KPasswordDialog::ShowKeepPassword )
    {
        ui.keepCheckBox->hide();
    }

    // Configure necessary key-bindings and connect necessary slots and signals
    QObject::connect( ui.passEdit, SIGNAL(returnPressed()), q, SLOT(accept()) );

    QRect desktop = KGlobalSettings::desktopGeometry(q->topLevelWidget());
    q->setFixedWidth(qMin(1000, qMax(400, desktop.width() / 4)));
    //q->setMinimumHeight( q->minimumSizeHint().height() );
    q->mainWidget()->setMinimumHeight( q->mainWidget()->sizeHint().height() );
}

void KPasswordDialog::setPixmap(const QPixmap &pixmap)
{
    if ( !d->pixmapLabel )
    {
        d->pixmapLabel = new QLabel( mainWidget() );
        d->pixmapLabel->setAlignment( Qt::AlignLeft | Qt::AlignTop );
        d->ui.hboxLayout->insertWidget( 0, d->pixmapLabel );
    }

    d->pixmapLabel->setPixmap( pixmap );
}

QPixmap KPasswordDialog::pixmap() const
{
    if ( !d->pixmapLabel ) {
        return QPixmap();
    }

    return *d->pixmapLabel->pixmap();
}


void KPasswordDialog::setUsername(const QString& user)
{
    d->ui.userEdit->setText(user);
    d->activated(user);
    if ( d->ui.userEdit->isVisibleTo( this ) )
    {
        d->ui.passEdit->setFocus();
    }
}


QString KPasswordDialog::username() const
{
    return d->ui.userEdit->text();
}

QString KPasswordDialog::password() const
{
    return d->ui.passEdit->text();
}

void KPasswordDialog::setKeepPassword( bool b )
{
    d->ui.keepCheckBox->setChecked( b );
}

bool KPasswordDialog::keepPassword() const
{
    return d->ui.keepCheckBox->isChecked();
}

void KPasswordDialog::addCommentLine( const QString& label,
                                      const QString& comment )
{
    QLabel* l = new QLabel(label, mainWidget());
    QLabel* c = new QLabel(qrichtextify(comment), mainWidget());

    d->ui.gridLayout->addWidget(l, d->commentRow, 0);
    d->ui.gridLayout->addWidget(c, d->commentRow, 1);
    ++d->commentRow;
    d->ui.gridLayout->addWidget(d->ui.userNameLabel, d->commentRow, 0);
    d->ui.gridLayout->addWidget(d->ui.userEdit, d->commentRow, 1);
    d->ui.gridLayout->addWidget(d->ui.passwordLabel, d->commentRow + 1, 0);
    d->ui.gridLayout->addWidget(d->ui.passEdit, d->commentRow + 1, 1);
    d->ui.gridLayout->addWidget(d->ui.keepCheckBox, d->commentRow + 2, 1);

    mainWidget()->setMinimumHeight( mainWidget()->sizeHint().height() );
}

void KPasswordDialog::showErrorMessage( const QString& message, const ErrorType type )
{
    d->ui.errorMessage->setVisible( true );
    d->ui.errorMessage->setText( message );
    mainWidget()->setMinimumHeight( mainWidget()->sizeHint().height() );

    QFont bold = font();
    bold.setBold( true );
    switch ( type ) {
        case PasswordError:
            d->ui.passwordLabel->setFont( bold );
            d->ui.passEdit->clear();
            d->ui.passEdit->setFocus();
            break;
        case UsernameError:
            if ( d->ui.userEdit->isVisibleTo( this ) )
            {
                d->ui.userNameLabel->setFont( bold );
                d->ui.userEdit->setFocus();
                break;
            }
        case FatalError:
            d->ui.userNameLabel->setEnabled( false );
            d->ui.userEdit->setEnabled( false );
            d->ui.passwordLabel->setEnabled( false );
            d->ui.passEdit->setEnabled( false );
            d->ui.keepCheckBox->setEnabled( false );
            enableButton( Ok, false );
            break;
        default:
            break;
    }
}

void KPasswordDialog::setPrompt(const QString& prompt)
{
    d->ui.prompt->setText( qrichtextify( prompt ) );
    mainWidget()->setMinimumHeight( mainWidget()->sizeHint().height() );
}

QString KPasswordDialog::prompt() const
{
    return d->ui.prompt->text();
}

void KPasswordDialog::setPassword(const QString &p)
{
    d->ui.passEdit->setText(p);
}

void KPasswordDialog::setUsernameReadOnly( bool readOnly )
{
    d->ui.userEdit->setReadOnly( readOnly );

    if ( readOnly && d->ui.userEdit->hasFocus() ) {
        d->ui.passEdit->setFocus();
    }
}

void KPasswordDialog::setKnownLogins( const QMap<QString, QString>& knownLogins )
{
    const int nr = knownLogins.count();
    if ( nr == 0 ) {
        return;
    }

    if ( nr == 1 ) {
        d->ui.userEdit->setText( knownLogins.begin().key() );
        setPassword( knownLogins.begin().value() );
        return;
    }

    Q_ASSERT( !d->ui.userEdit->isReadOnly() );
    if ( !d->userEditCombo ) {
        delete d->ui.userEdit;
        d->userEditCombo = new KComboBox( true, mainWidget() );
        d->ui.userEdit = d->userEditCombo->lineEdit();
//        QSize s = d->userEditCombo->sizeHint();
//        d->ui.userEditCombo->setFixedHeight( s.height() );
//        d->ui.userEditCombo->setMinimumWidth( s.width() );
        d->ui.userNameLabel->setBuddy( d->userEditCombo );
        d->ui.gridLayout->addWidget( d->userEditCombo, d->commentRow, 1 );
        connect( d->ui.userEdit, SIGNAL(returnPressed()), d->ui.passEdit, SLOT(setFocus()) );
    }

    d->knownLogins = knownLogins;
    d->userEditCombo->addItems( knownLogins.keys() );
    d->userEditCombo->setFocus();

    connect( d->userEditCombo, SIGNAL( activated( const QString& ) ),
             this, SLOT( activated( const QString& ) ) );
}

void KPasswordDialog::KPasswordDialogPrivate::activated( const QString& userName )
{
    QMap<QString, QString>::ConstIterator it = knownLogins.find( userName );
    if ( it != knownLogins.end() ) {
        q->setPassword( it.value() );
    }
}

void KPasswordDialog::accept()
{
    d->ui.errorMessage->setVisible( false );

    // reset the font in case we had an error previously
    d->ui.passwordLabel->setFont( font() );
    d->ui.userNameLabel->setFont( font() );

    // we do this to allow the error message, if any, to go away
    // checkPassword() may block for a period of time
    QTimer::singleShot( 0, this, SLOT(actuallyAccept()) );
}

void KPasswordDialog::KPasswordDialogPrivate::actuallyAccept()
{
    if ( !q->checkPassword() )
    {
        return;
    }

    bool keep = ui.keepCheckBox->isVisibleTo( q ) && ui.keepCheckBox->isChecked();
    emit q->gotPassword( q->password(), keep);

    if ( ui.userEdit->isVisibleTo( q ) ) {
        emit q->gotUsernameAndPassword( q->username(), q->password() , keep);
    }

    q->KDialog::accept();
}

bool KPasswordDialog::checkPassword()
{
    return true;
}

#include "kpassworddialog.moc"
