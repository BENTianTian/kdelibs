/**
 * dialog.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "dialog.h"
#include "ui_sonnetui.h"

#include "backgroundchecker.h"
#include "speller.h"
#include "filter_p.h"
#include "settings_p.h"

#include <kconfig.h>
#include <kguiitem.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtCore/QTimer>

namespace Sonnet
{

//to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class Dialog::Private
{
public:
    Ui_SonnetUi ui;
    QWidget *wdg;
    QString   originalBuffer;
    BackgroundChecker *checker;

    Word   currentWord;
    QMap<QString, QString> replaceAllMap;
    bool restart;//used when text is distributed across several qtextedits, eg in KAider
};

Dialog::Dialog( BackgroundChecker *checker,
                QWidget *parent )
    : KDialog( parent ),d(new Private)
{
    setModal( true );
    setCaption( i18nc("@title:window", "Check Spelling" ) );
    setButtons( Help | Cancel | User1 );
    setButtonGuiItem( User1, KGuiItem(i18nc("@action:button", "&Finished" )) );
    setDefaultButton( Cancel );
    showButtonSeparator( true );

    setDefaultButton( Cancel );
    d->checker = checker;

    initGui();
    initConnections();
    setMainWidget( d->wdg );
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::initConnections()
{
    connect( d->ui.m_addBtn, SIGNAL(clicked()),
             SLOT(slotAddWord()) );
    connect( d->ui.m_replaceBtn, SIGNAL(clicked()),
             SLOT(slotReplaceWord()) );
    connect( d->ui.m_replaceAllBtn, SIGNAL(clicked()),
             SLOT(slotReplaceAll()) );
    connect( d->ui.m_skipBtn, SIGNAL(clicked()),
             SLOT(slotSkip()) );
    connect( d->ui.m_skipAllBtn, SIGNAL(clicked()),
             SLOT(slotSkipAll()) );
    connect( d->ui.m_suggestBtn, SIGNAL(clicked()),
             SLOT(slotSuggest()) );
    connect( d->ui.m_language, SIGNAL(activated(const QString&)),
             SLOT(slotChangeLanguage(const QString&)) );
    connect( d->ui.m_suggestions, SIGNAL(itemClicked(QListWidgetItem*)),
	     SLOT(slotSelectionChanged(QListWidgetItem*)) );
    connect( d->checker, SIGNAL(misspelling(const QString&, int)),
             SIGNAL(misspelling(const QString&, int)) );
    connect( d->checker, SIGNAL(misspelling(const QString&, int)),
             SLOT(slotMisspelling(const QString&, int)) );
    connect( d->checker, SIGNAL(done()),
             SLOT(slotDone()) );
    connect( d->ui.m_suggestions, SIGNAL(itemDoubleClicked (QListWidgetItem *)),
             SLOT( slotReplaceWord() ) );
    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotFinished()) );
    connect( this, SIGNAL(cancelClicked()),this, SLOT(slotCancel()) );
    connect( d->ui.m_replacement, SIGNAL(returnPressed()), this, SLOT(slotReplaceWord()) );
    connect( d->ui.m_autoCorrect, SIGNAL(clicked()),
             SLOT(slotAutocorrect()) );
    // button use by kword/kpresenter
    // hide by default
    d->ui.m_autoCorrect->hide();
}

void Dialog::initGui()
{
    d->wdg = new QWidget(this);
    d->ui.setupUi(d->wdg);

    //d->ui.m_suggestions->setSorting( NONSORTINGCOLUMN );
    d->ui.m_language->clear();
    Speller speller = d->checker->speller();
    d->ui.m_language->insertItems(0, speller.availableLanguageNames());
    d->ui.m_language->setCurrentIndex(speller.availableLanguages().indexOf(
                                          speller.language()));
    d->restart=false;
}

void Dialog::activeAutoCorrect( bool _active )
{
    if ( _active )
        d->ui.m_autoCorrect->show();
    else
        d->ui.m_autoCorrect->hide();
}

void Dialog::slotAutocorrect()
{
    kDebug()<<"void Dialog::slotAutocorrect()\n";
    emit autoCorrect(d->currentWord.word, d->ui.m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotFinished()
{
    kDebug()<<"void Dialog::slotFinished() \n";
    emit stop();
    //FIXME: should we emit done here?
    emit done(d->checker->text());
    accept();
}

void Dialog::slotCancel()
{
    kDebug()<<"void Dialog::slotCancel() \n";
    emit cancel();
    reject();
}

QString Dialog::originalBuffer() const
{
    return d->originalBuffer;
}

QString Dialog::buffer() const
{
    return d->checker->text();
}

void Dialog::setBuffer( const QString& buf )
{
    d->originalBuffer = buf;
    //it is possible to change buffer inside slot connected to done() signal
    d->restart=true;
}


void Dialog::updateDialog( const QString& word )
{
    d->ui.m_unknownWord->setText( word );
    d->ui.m_contextLabel->setText( d->checker->currentContext() );
    QStringList suggs = d->checker->suggest( word );

    if (suggs.isEmpty())
        d->ui.m_replacement->clear();
    else
        d->ui.m_replacement->setText( suggs.first() );
    fillSuggestions( suggs );
}

void Dialog::show()
{
    kDebug()<<"Showing dialog";
    if (d->originalBuffer.isEmpty())
        d->checker->start();
    else
        d->checker->setText(d->originalBuffer);
}

void Dialog::slotAddWord()
{
   d->checker->addWordToPersonal(d->currentWord.word);
   d->checker->continueChecking();
}

void Dialog::slotReplaceWord()
{
    emit replace( d->currentWord.word, d->currentWord.start,
                  d->ui.m_replacement->text() );
    d->checker->replace(d->currentWord.start,
                        d->currentWord.word,
                        d->ui.m_replacement->text());
    d->checker->continueChecking();
}

void Dialog::slotReplaceAll()
{
    d->replaceAllMap.insert( d->currentWord.word,
                             d->ui.m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotSkip()
{
    d->checker->continueChecking();
}

void Dialog::slotSkipAll()
{
    //### do we want that or should we have a d->ignoreAll list?
    Speller speller = d->checker->speller();
    speller.addToPersonal(d->currentWord.word);
    d->checker->setSpeller(speller);
    d->checker->continueChecking();
}

void Dialog::slotSuggest()
{
    QStringList suggs = d->checker->suggest( d->ui.m_replacement->text() );
    fillSuggestions( suggs );
}

void Dialog::slotChangeLanguage( const QString& lang )
{
    Speller speller = d->checker->speller();
    d->checker->changeLanguage(
        speller.availableLanguages().at(
            speller.availableLanguageNames().indexOf(lang)));
    slotSuggest();
}

void Dialog::slotSelectionChanged( QListWidgetItem *item )
{
    d->ui.m_replacement->setText( item->text() );
}

void Dialog::fillSuggestions( const QStringList& suggs )
{
    d->ui.m_suggestions->clear();
    for ( QStringList::ConstIterator it = suggs.begin(); it != suggs.end(); ++it ) {
	    d->ui.m_suggestions->addItem(*it );
    }
}

void Dialog::slotMisspelling(const QString& word, int start)
{
    kDebug()<<"Dialog misspelling!!";
    d->currentWord = Word( word, start );
    if ( d->replaceAllMap.contains( word ) ) {
        d->ui.m_replacement->setText( d->replaceAllMap[ word ] );
        slotReplaceWord();
    } else {
        updateDialog( word );
    }
    KDialog::show();
}

void Dialog::slotDone()
{
    kDebug()<<"Dialog done!";
    QString buffer(d->originalBuffer);
    emit done(d->checker->text());
    if (d->restart)
    {
        d->checker->setText(d->originalBuffer);
        d->restart=false;
    }
    else
        accept();
}

}

#include "dialog.moc"
