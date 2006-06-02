/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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
#undef QT3_SUPPORT
#include "passdlg.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QTextDocument>
#include <QTextLayout>


#include <kcombobox.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <khbox.h>

using namespace KIO;

struct PasswordDialog::PasswordDialogPrivate
{
    QGridLayout *layout;
    QLineEdit* userEdit;
    KLineEdit* passEdit;
    QLabel* userNameLabel;
    QLabel* prompt;
    QCheckBox* keepCheckBox;
    QMap<QString,QString> knownLogins;
    KComboBox* userEditCombo;
    KHBox* userNameHBox;

    bool keep;
    short unsigned int nRow;
};

PasswordDialog::PasswordDialog( const QString& prompt, const QString& user,
                                bool enableKeep, bool modal, QWidget* parent )
               :KDialog( parent, i18n("Password"), Ok|Cancel ),d(new PasswordDialogPrivate)
{
    setModal( modal );
    enableButtonSeparator( true );
    setDefaultButton( Ok );
    init ( prompt, user, enableKeep );
}

PasswordDialog::~PasswordDialog()
{
    delete d;
}

void PasswordDialog::init( const QString& prompt, const QString& user,
                           bool enableKeep  )
{
    QWidget *main = new QWidget;
    setMainWidget( main );

    d->keep = false;
    d->nRow = 0;
    d->keepCheckBox = 0;

    KConfigGroup cg( KGlobal::config(), "Passwords" );

    d->layout = new QGridLayout( main );
    d->layout->setSpacing( spacingHint() );
    d->layout->setMargin( marginHint() );
    d->layout->addItem(new QSpacerItem(5,0),0,1); //addColSpacing(1, 5);

    // Row 0: pixmap  prompt
    QLabel* lbl;
    QPixmap pix( KGlobal::iconLoader()->loadIcon( "password", K3Icon::NoGroup, K3Icon::SizeHuge, 0, 0, true));
    if ( !pix.isNull() )
    {
        lbl = new QLabel( main );
        lbl->setPixmap( pix );
        lbl->setAlignment( Qt::AlignLeft|Qt::AlignVCenter );
        lbl->setFixedSize( lbl->sizeHint() );
        d->layout->addWidget( lbl, 0, 0, Qt::AlignLeft );
    }
    d->prompt = new QLabel( main );
    d->prompt->setAlignment( Qt::AlignLeft|Qt::AlignVCenter);
    d->prompt->setWordWrap( true);
    d->layout->addWidget( d->prompt, 0, 2, Qt::AlignLeft );
    if ( prompt.isEmpty() )
        setPrompt( i18n( "You need to supply a username and a password" ) );
    else
        setPrompt( prompt );

    // Row 1: Row Spacer
    d->layout->addItem(new QSpacerItem(0,7),1,0); //addRowSpacing( 1, 7 );

    // Row 2-3: Reserved for an additional comment

    // Row 4: Username field
    d->userNameLabel = new QLabel( i18n("&Username:"), main );
    d->userNameLabel->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    d->userNameLabel->setFixedSize( d->userNameLabel->sizeHint() );
    d->userNameHBox = new KHBox( main );

    d->userEdit = new KLineEdit( d->userNameHBox );
    QSize s = d->userEdit->sizeHint();
    d->userEdit->setFixedHeight( s.height() );
    d->userEdit->setMinimumWidth( s.width() );
    d->userNameLabel->setBuddy( d->userEdit );
    d->layout->addWidget( d->userNameLabel, 4, 0 );
    d->layout->addWidget( d->userNameHBox, 4, 2 );

    // Row 5: Row spacer
    d->layout->addItem(new QSpacerItem(0,4),5,0); //addRowSpacing( 5, 4 );

    // Row 6: Password field
    lbl = new QLabel( i18n("&Password:"), main );
    lbl->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    lbl->setFixedSize( lbl->sizeHint() );
    KHBox* hbox = new KHBox( main );
    d->passEdit = new KLineEdit( hbox );
    if ( cg.readEntry("EchoMode", "OneStar") == "NoEcho" )
        d->passEdit->setEchoMode( QLineEdit::NoEcho );
    else
        d->passEdit->setEchoMode( QLineEdit::Password );
    s = d->passEdit->sizeHint();
    d->passEdit->setFixedHeight( s.height() );
    d->passEdit->setMinimumWidth( s.width() );
    lbl->setBuddy( d->passEdit );
    d->layout->addWidget( lbl, 6, 0 );
    d->layout->addWidget( hbox, 6, 2 );

    if ( enableKeep )
    {
        // Row 7: Add spacer
        d->layout->addItem(new QSpacerItem(0,4),7,0); //addRowSpacing( 7, 4 );
        // Row 8: Keep Password
        hbox = new KHBox( main );
        d->keepCheckBox = new QCheckBox( i18n("&Keep password"), hbox );
        d->keepCheckBox->setFixedSize( d->keepCheckBox->sizeHint() );
        d->keep = cg.readEntry("Keep", false);
        d->keepCheckBox->setChecked( d->keep );
        connect(d->keepCheckBox, SIGNAL(toggled( bool )), SLOT(slotKeep( bool )));
        d->layout->addWidget( hbox, 8, 2 );
    }

    // Configure necessary key-bindings and connect necessar slots and signals
    connect( d->userEdit, SIGNAL(returnPressed()), d->passEdit, SLOT(setFocus()) );
    connect( d->passEdit, SIGNAL(returnPressed()), SLOT(slotOk()) );

    if ( !user.isEmpty() )
    {
        d->userEdit->setText( user );
        d->passEdit->setFocus();
    }
    else
        d->userEdit->setFocus();

    d->userEditCombo = 0;
//    setFixedSize( sizeHint() );
}

QString PasswordDialog::username() const
{
    return d->userEdit->text();
}

QString PasswordDialog::password() const
{
    return d->passEdit->text();
}

void PasswordDialog::setKeepPassword( bool b )
{
    if ( d->keepCheckBox )
        d->keepCheckBox->setChecked( b );
}

bool PasswordDialog::keepPassword() const
{
    return d->keep;
}

static void calculateLabelSize(QLabel *label)
{
    QString qt_text = label->text();

    QRect rect;
    QRect d = KGlobalSettings::desktopGeometry(label->topLevelWidget());
    QTextLayout newLayout(qt_text, label->font());

    qreal curWidth = d.width() / 4;
    int leading = QFontMetrics(label->font()).leading();
    int lineCount = -1;

    do {
        qreal new_width = (curWidth * 0.9);
        qreal maxWidth = 0.;
        int cur_height = 0;

        // save current size
        rect = newLayout.boundingRect().toRect();
        // do layout calculation
        newLayout.beginLayout();
        while (1) {
            QTextLine line = newLayout.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(new_width);
            cur_height += leading;
            line.setPosition(QPoint(0, cur_height));
            cur_height += (int)(line.height()+0.5);
            maxWidth = qMax(maxWidth, line.naturalTextWidth());
        }
        newLayout.endLayout();
        if( lineCount == -1 ) {
            lineCount = newLayout.lineCount();;
            rect = newLayout.boundingRect().toRect();
            if( lineCount == 1 )
                rect.setWidth( (int)maxWidth );
        }
        curWidth = rect.width();
    } while( newLayout.lineCount() > 1 &&
              newLayout.lineCount() == lineCount );

    label->setFixedSize(rect.width(), rect.height()+leading);
}

void PasswordDialog::addCommentLine( const QString& label,
                                     const QString comment )
{
    if (d->nRow > 0)
        return;

    QWidget *main = mainWidget();

    QLabel* lbl = new QLabel( label, main);
    lbl->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    lbl->setFixedSize( lbl->sizeHint() );
    d->layout->addWidget( lbl, d->nRow+2, 0, Qt::AlignLeft );
    lbl = new QLabel( comment, main);
    lbl->setAlignment( Qt::AlignVCenter|Qt::AlignLeft);
    lbl->setWordWrap(true);
    calculateLabelSize(lbl);
    d->layout->addWidget( lbl, d->nRow+2, 2, Qt::AlignLeft );
    d->layout->addItem(new QSpacerItem(0,10),3,0); //addRowSpacing( 3, 10 ); // Add a spacer
    d->nRow++;
}

void PasswordDialog::slotKeep( bool keep )
{
    d->keep = keep;
}

static QString qrichtextify( const QString& text )
{
  if ( text.isEmpty() || text[0] == '<' )
    return text;

  QStringList lines = text.split('\n', QString::SkipEmptyParts);
  for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
  {
    *it = Qt::convertFromPlainText( *it, Qt::WhiteSpaceNormal );
  }

  return lines.join(QString());
}

void PasswordDialog::setPrompt(const QString& prompt)
{
    QString text = qrichtextify(prompt);
    d->prompt->setText(text);
    calculateLabelSize(d->prompt);
}

void PasswordDialog::setPassword(const QString &p)
{
    d->passEdit->setText(p);
}

void PasswordDialog::setUserReadOnly( bool readOnly )
{
    d->userEdit->setReadOnly( readOnly );
    if ( readOnly && d->userEdit->hasFocus() )
        d->passEdit->setFocus();
}

void PasswordDialog::setKnownLogins( const QMap<QString, QString>& knownLogins )
{
    const int nr = knownLogins.count();
    if ( nr == 0 )
        return;
    if ( nr == 1 ) {
        d->userEdit->setText( knownLogins.begin().key() );
        setPassword( knownLogins.begin().value() );
        return;
    }

    Q_ASSERT( !d->userEdit->isReadOnly() );
    if ( !d->userEditCombo ) {
        delete d->userEdit;
        d->userEditCombo = new KComboBox( true, d->userNameHBox );
        d->userEdit = d->userEditCombo->lineEdit();
        QSize s = d->userEditCombo->sizeHint();
        d->userEditCombo->setFixedHeight( s.height() );
        d->userEditCombo->setMinimumWidth( s.width() );
        d->userNameLabel->setBuddy( d->userEditCombo );
        d->layout->addWidget( d->userNameHBox, 4, 2 );
    }

    d->knownLogins = knownLogins;
    d->userEditCombo->addItems( knownLogins.keys() );
    d->userEditCombo->setFocus();

    connect( d->userEditCombo, SIGNAL( activated( const QString& ) ),
             this, SLOT( slotActivated( const QString& ) ) );
}

void PasswordDialog::slotActivated( const QString& userName )
{
    QMap<QString, QString>::ConstIterator it = d->knownLogins.find( userName );
    if ( it != d->knownLogins.end() )
        setPassword( it.value() );
}


int PasswordDialog::getNameAndPassword( QString& user, QString& pass, bool* keep,
                                        const QString& prompt, bool readOnly,
                                        const QString& caption,
                                        const QString& comment,
                                        const QString& label )
{
    PasswordDialog* dlg;
    if( keep )
        dlg = new PasswordDialog( prompt, user, (*keep) );
    else
        dlg = new PasswordDialog( prompt, user );

    if ( !caption.isEmpty() )
        dlg->setPlainCaption( caption );
    else
        dlg->setPlainCaption( i18n("Authorization Dialog") );

    if ( !comment.isEmpty() )
        dlg->addCommentLine( label, comment );

    if ( readOnly )
        dlg->setUserReadOnly( readOnly );

    int ret = dlg->exec();
    if ( ret == Accepted )
    {
        user = dlg->username();
        pass = dlg->password();
        if ( keep ) { (*keep) = dlg->keepPassword(); }
    }
    delete dlg;
    return ret;
 }


#include "passdlg.moc"
