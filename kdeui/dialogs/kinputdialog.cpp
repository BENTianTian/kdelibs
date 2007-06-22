/*
  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

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

#include "kinputdialog.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QDoubleValidator>

#include <kguiitem.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <klistwidget.h>
#include <kstandardguiitem.h>
#include <ktextedit.h>
#include <kcompletion.h>

class KInputDialogPrivate
{
  public:
    KInputDialogPrivate(KInputDialog *q);

    void slotEditTextChanged( const QString& );
    void slotUpdateButtons( const QString& );

    KInputDialog *q;
    QLabel *m_label;
    KLineEdit *m_lineEdit;
    KIntSpinBox *m_intSpinBox;
    KDoubleSpinBox *m_doubleSpinBox;
    KComboBox *m_comboBox;
    KListWidget *m_listBox;
    KTextEdit *m_textEdit;
};

KInputDialogPrivate::KInputDialogPrivate(KInputDialog *q)
    : q(q), m_label( 0L ), m_lineEdit( 0L ), m_intSpinBox( 0L ),
      m_doubleSpinBox( 0L ), m_comboBox( 0L )
{
}

KInputDialog::KInputDialog( const QString &caption, const QString &label,
    const QString &value, QWidget *parent,
    QValidator *validator, const QString &mask )
    : KDialog( parent ),
    d( new KInputDialogPrivate(this) )
{
  setCaption( caption );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  setModal(true);

  QWidget *frame = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( frame );
    layout->setMargin(0);
  layout->setSpacing( spacingHint() );

  d->m_label = new QLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_lineEdit = new KLineEdit( value, frame );
  d->m_lineEdit->setClearButtonShown( true );
  layout->addWidget( d->m_lineEdit );

  d->m_lineEdit->setFocus();
  d->m_label->setBuddy( d->m_lineEdit );

  layout->addStretch();

  if ( validator )
    d->m_lineEdit->setValidator( validator );

  if ( !mask.isEmpty() )
    d->m_lineEdit->setInputMask( mask );

  connect( d->m_lineEdit, SIGNAL( textChanged( const QString & ) ),
      SLOT( slotEditTextChanged( const QString & ) ) );

  setMainWidget(frame);
  d->slotEditTextChanged( value );
  setMinimumWidth( 350 );
}

KInputDialog::KInputDialog( const QString &caption, const QString &label,
    const QString &value, QWidget *parent )
    : KDialog( parent ),
    d( new KInputDialogPrivate(this) )
{
  setCaption( caption );
  setButtons( Ok | Cancel | User1 );
  setButtonGuiItem( User1, KStandardGuiItem::clear() );
  setDefaultButton( Ok );
  showButtonSeparator( false );
  setModal(true);
  QWidget *frame = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( frame );
    layout->setMargin(0);
  layout->setSpacing( spacingHint() );

  d->m_label = new QLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_textEdit = new KTextEdit( frame );
  d->m_textEdit->insertPlainText( value );
  layout->addWidget( d->m_textEdit, 10 );

  d->m_textEdit->setFocus();
  d->m_label->setBuddy( d->m_textEdit );

  connect( this, SIGNAL( user1Clicked() ), d->m_textEdit, SLOT( clear() ) );
  connect( this, SIGNAL( user1Clicked() ), d->m_textEdit, SLOT( setFocus() ) );
  setMainWidget(frame);
  setMinimumWidth( 400 );
}

KInputDialog::KInputDialog( const QString &caption, const QString &label,
    int value, int minValue, int maxValue, int step, int base,
    QWidget *parent )
    : KDialog( parent ),
    d( new KInputDialogPrivate(this) )
{
  setCaption( caption );
  setButtons( Ok | Cancel );
  showButtonSeparator( true );
  setModal(true);

  QWidget *frame = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( frame );
  layout->setSpacing( spacingHint() );

  d->m_label = new QLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_intSpinBox = new KIntSpinBox( minValue, maxValue, step, value, frame,
      base);
  layout->addWidget( d->m_intSpinBox );

  layout->addStretch();
    layout->setMargin(0);

  d->m_intSpinBox->setFocus();
  setMainWidget(frame);
  setMinimumWidth( 300 );
}

KInputDialog::KInputDialog( const QString &caption, const QString &label,
    double value, double minValue, double maxValue, double step, int decimals,
    QWidget *parent )
    : KDialog( parent ),
    d( new KInputDialogPrivate(this) )
{
  setCaption( caption );
  setButtons( Ok | Cancel );
  showButtonSeparator( true );
  setModal(true);

  QWidget *frame = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( frame );
  layout->setSpacing( spacingHint() );

  d->m_label = new QLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_doubleSpinBox = new KDoubleSpinBox( minValue, maxValue, step, value, frame,
      decimals);
  layout->addWidget( d->m_doubleSpinBox );

  layout->addStretch();
    layout->setMargin(0);

  d->m_doubleSpinBox->setFocus();
  setMainWidget(frame);
  setMinimumWidth( 300 );
}

KInputDialog::KInputDialog( const QString &caption, const QString &label,
    const QStringList &list, int current, bool editable, QWidget *parent )
    : KDialog( parent ),
    d( new KInputDialogPrivate(this) )
{
  setCaption( caption );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  setModal(true);

  QWidget *frame = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( frame );
  layout->setSpacing( spacingHint() );

  d->m_label = new QLabel( label, frame );
  layout->addWidget( d->m_label );

  if ( editable )
  {
    d->m_comboBox = new KComboBox( editable, frame );
    d->m_lineEdit = new KLineEdit( frame );
    d->m_lineEdit->setClearButtonShown( true );
    d->m_comboBox->setLineEdit( d->m_lineEdit );
    d->m_comboBox->insertItems( 0, list );
    d->m_comboBox->setCurrentIndex( current );
    layout->addWidget( d->m_comboBox );

    connect( d->m_comboBox, SIGNAL( editTextChanged( const QString & ) ),
      SLOT( slotUpdateButtons( const QString & ) ) );
    d->slotUpdateButtons( d->m_comboBox->currentText() );
    d->m_comboBox->setFocus();
  } else {
    d->m_listBox = new KListWidget( frame );
    d->m_listBox->addItems( list );
    d->m_listBox->setCurrentRow(current);
    layout->addWidget( d->m_listBox, 10 );
    connect( d->m_listBox, SIGNAL( executed( QListWidgetItem * ) ),
      SLOT( accept() ) );
    /*connect( d->m_listBox, SIGNAL( returnPressed( QListWidgetItem * ) ),
      SLOT( accept() ) );*/

    d->m_listBox->setFocus();
  }

  layout->addStretch();
    layout->setMargin(0);
  setMainWidget(frame);
  setMinimumWidth( 320 );
}

KInputDialog::KInputDialog( const QString &caption, const QString &label,
    const QStringList &list, const QStringList &select, bool multiple,
    QWidget *parent )
    : KDialog( parent ),
    d( new KInputDialogPrivate(this) )
{
  setCaption( caption );
  setButtons( Ok | Cancel );
  showButtonSeparator( true );
  setModal(true);

  QWidget *frame = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout( frame );
  layout->setSpacing( spacingHint() );

  d->m_label = new QLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_listBox = new KListWidget( frame );
  d->m_listBox->addItems( list );
  layout->addWidget( d->m_listBox );

  if ( multiple )
  {
    d->m_listBox->setSelectionMode( QAbstractItemView::ExtendedSelection );

    for ( QStringList::ConstIterator it=select.begin(); it!=select.end(); ++it )
    {
      QList<QListWidgetItem*> matches = d->m_listBox->findItems( *it , Qt::MatchCaseSensitive|Qt::MatchExactly);
      if ( !matches.isEmpty() )
        d->m_listBox->setCurrentItem( matches.first() );
    }
  }
  else
  {
    connect( d->m_listBox, SIGNAL( executed( QListWidgetItem * ) ),
      SLOT( accept() ) );
    /*connect( d->m_listBox, SIGNAL( returnPressed( QListWidgetItem * ) ),
      SLOT( accept() ) );*/

    if(!select.isEmpty())
    {
    	QString text = select.first();
    
    	QList<QListWidgetItem*> matches = d->m_listBox->findItems(text,Qt::MatchCaseSensitive|Qt::MatchExactly);
    	if ( !matches.isEmpty() )
      		d->m_listBox->setCurrentItem( matches.first() );
    }
  }

  d->m_listBox->setFocus();

  layout->addStretch();
    layout->setMargin(0);
  setMainWidget(frame);
  setMinimumWidth( 320 );
}

KInputDialog::~KInputDialog()
{
  delete d;
}


QString KInputDialog::getText( const QString &caption,
    const QString &label, const QString &value, bool *ok, QWidget *parent,
    QValidator *validator, const QString &mask,
    const QString &whatsThis,const QStringList &completionList )
{
  KInputDialog dlg( caption, label, value, parent, validator, mask );

  if( !whatsThis.isEmpty() )
    dlg.lineEdit()->setWhatsThis(whatsThis );

  if (!completionList.isEmpty())
  {
    KCompletion *comp=dlg.lineEdit()->completionObject();
    for(QStringList::const_iterator it=completionList.constBegin();it!=completionList.constEnd();++it)
      comp->addItem(*it);
  }

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  QString result;
  if ( _ok )
    result = dlg.lineEdit()->text();

  // A validator may explicitly allow leading and trailing whitespace
  if ( !validator )
    result = result.trimmed();

  return result;
}

QString KInputDialog::getMultiLineText( const QString &caption,
    const QString &label, const QString &value, bool *ok,
    QWidget *parent )
{
  KInputDialog dlg( caption, label, value, parent );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  QString result;
  if ( _ok )
    result = dlg.textEdit()->toPlainText();

  return result;
}

int KInputDialog::getInteger( const QString &caption, const QString &label,
    int value, int minValue, int maxValue, int step, int base, bool *ok,
    QWidget *parent )
{
  KInputDialog dlg( caption, label, value, minValue,
    maxValue, step, base, parent );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  int result=0;
  if ( _ok )
    result = dlg.intSpinBox()->value();

  return result;
}

int KInputDialog::getInteger( const QString &caption, const QString &label,
    int value, int minValue, int maxValue, int step, bool *ok,
    QWidget *parent )
{
  return getInteger( caption, label, value, minValue, maxValue, step,
    10, ok, parent );
}

double KInputDialog::getDouble( const QString &caption, const QString &label,
    double value, double minValue, double maxValue, double step, int decimals,
    bool *ok, QWidget *parent )
{
  KInputDialog dlg( caption, label, value, minValue,
    maxValue, step, decimals, parent );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  double result=0;
  if ( _ok )
    result = dlg.doubleSpinBox()->value();

  return result;
}

double KInputDialog::getDouble( const QString &caption, const QString &label,
    double value, double minValue, double maxValue, int decimals,
    bool *ok, QWidget *parent )
{
  return getDouble( caption, label, value, minValue, maxValue, 0.1, decimals,
    ok, parent );
}

QString KInputDialog::getItem( const QString &caption, const QString &label,
    const QStringList &list, int current, bool editable, bool *ok,
    QWidget *parent )
{
  KInputDialog dlg( caption, label, list, current,
    editable, parent );
  if ( !editable)
  {
      connect( dlg.listBox(),  SIGNAL(executed( QListWidgetItem *)), &dlg, SLOT( accept()));
  }
  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  QString result;
  if ( _ok ) {
    
    if ( editable )
      result = dlg.comboBox()->currentText();
    else
      result = dlg.listBox()->currentItem()->text();
  }

  return result;
}

QStringList KInputDialog::getItemList( const QString &caption,
    const QString &label, const QStringList &list, const QStringList &select,
    bool multiple, bool *ok, QWidget *parent )
{
  KInputDialog dlg( caption, label, list, select,
    multiple, parent );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  QStringList result;
  if ( _ok )
  {
    for (int i=0 ; i < dlg.listBox()->count() ; i++) {

      QListWidgetItem* item = dlg.listBox()->item(i);

      if ( item->isSelected() )
        result.append( item->text() );
    }
  }

  return result;
}

void KInputDialogPrivate::slotEditTextChanged( const QString &text )
{
  bool on;
  if ( m_lineEdit->validator() ) {
    QString str = m_lineEdit->text();
    int index = m_lineEdit->cursorPosition();
    on = ( m_lineEdit->validator()->validate( str, index )
      == QValidator::Acceptable );
  } else {
    on = !text.trimmed().isEmpty();
  }

  q->enableButton( KDialog::Ok, on );
}

void KInputDialogPrivate::slotUpdateButtons( const QString &text )
{
  q->enableButton( KDialog::Ok, !text.isEmpty() );
}

KLineEdit *KInputDialog::lineEdit() const
{
  return d->m_lineEdit;
}

KIntSpinBox *KInputDialog::intSpinBox() const
{
  return d->m_intSpinBox;
}

KDoubleSpinBox *KInputDialog::doubleSpinBox() const
{
  return d->m_doubleSpinBox;
}

KComboBox *KInputDialog::comboBox() const
{
  return d->m_comboBox;
}

KListWidget *KInputDialog::listBox() const
{
  return d->m_listBox;
}

KTextEdit *KInputDialog::textEdit() const
{
  return d->m_textEdit;
}

#include "kinputdialog.moc"

/* vim: set ai et sw=2 ts=2
*/
