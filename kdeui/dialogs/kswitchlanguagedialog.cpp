/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2007 Krzysztof Lichota (lichota@mimuw.edu.pl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "kswitchlanguagedialog.moc"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtCore/QMap>

#include <klanguagebutton.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kpushbutton.h>

struct LanguageRowData
{
    QLabel *label;
    KLanguageButton *languageButton;
    KPushButton *removeButton;
    
    void setRowWidgets(
        QLabel *label,
        KLanguageButton *languageButton,
        KPushButton *removeButton
        )
    {
        this->label = label;
        this->languageButton = languageButton;
        this->removeButton = removeButton;
    }
    
};

class KSwitchLanguageDialogPrivate
{
public:
    KSwitchLanguageDialogPrivate(KSwitchLanguageDialog *parent);
    
    KSwitchLanguageDialog *p; //parent class
    
    /**
        Fills language button with names of languages for which given application has translation.
    */
    void fillApplicationLanguages(KLanguageButton *button);
    
    /**
        Adds one button with language to widget.
    */
    void addLanguageButton(const QString & languageCode, bool primaryLanguage);
    
    /**
        Returns list of languages chosen for application or default languages is they are not set.
    */
    QStringList applicationLanguageList();
    
    QMap<KPushButton*, LanguageRowData> languageRows;
    QList<KLanguageButton*> languageButtons;
    QGridLayout *languagesLayout;
    QWidget *page;
};

/*************************** KSwitchLanguageDialog **************************/

KSwitchLanguageDialog::KSwitchLanguageDialog( QWidget *parent )
  : KDialog(parent),
    d(new KSwitchLanguageDialogPrivate(this))
{
    setCaption(i18n("Switch application language"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    showButtonSeparator(true);
    connect(this, SIGNAL(okClicked()), SLOT(slotOk()));

    d->page = new QWidget( this );
    setMainWidget(d->page);
    QVBoxLayout *topLayout = new QVBoxLayout( d->page );
    topLayout->setSpacing( spacingHint() );
    QLabel *label = new QLabel( i18n("Please choose language which should be used for this application:"), d->page );
    topLayout->addWidget( label );
    
    QHBoxLayout *languageHorizontalLayout = new QHBoxLayout();
    topLayout->addLayout(languageHorizontalLayout);
    
    d->languagesLayout = new QGridLayout();
    languageHorizontalLayout->addLayout(d->languagesLayout);
    languageHorizontalLayout->addStretch();
    
    QStringList defaultLanguages = d->applicationLanguageList();

    int count = defaultLanguages.count();
    for (int i = 0; i < count; ++i)
    {
        QString language = defaultLanguages[i];
        bool primaryLanguage = (i == 0);
        d->addLanguageButton(language, primaryLanguage);
    }
    
    if (!count)
    {
        d->addLanguageButton(KGlobal::locale()->defaultLanguage(), true);
    }
    
    QHBoxLayout *addButtonHorizontalLayout = new QHBoxLayout();
    topLayout->addLayout(addButtonHorizontalLayout);
    
    KPushButton *addLangButton = new KPushButton(i18n("Add fallback language"), d->page);
    addLangButton->setToolTip(i18n("Adds one more language which will be used if other translations do not contain proper translation"));
    connect(addLangButton, SIGNAL(clicked()), this, SLOT(slotAddLanguageButton()));
    addButtonHorizontalLayout->addWidget(addLangButton);
    addButtonHorizontalLayout->addStretch();
    
    topLayout->addStretch(10);
}

KSwitchLanguageDialog::~KSwitchLanguageDialog()
{
    delete d;
}

void KSwitchLanguageDialog::slotAddLanguageButton()
{
    //adding new button with en_US as it should always be present
    d->addLanguageButton("en_US", d->languageButtons.isEmpty());
}

void KSwitchLanguageDialog::removeButtonClicked()
{
    QObject const *signalSender = sender();
    if (!signalSender)
    {
        kError() << "KSwitchLanguageDialog::removeButtonClicked() called directly, not using signal" << endl;
        return;
    }
    
    KPushButton *removeButton = const_cast<KPushButton*>(::qobject_cast<const KPushButton*>(signalSender));
    if (!removeButton)
    {
        kError() << "KSwitchLanguageDialog::removeButtonClicked() called from something else than KPushButton" << endl;
        return;
    }
    
    QMap<KPushButton *, LanguageRowData>::iterator it = d->languageRows.find(removeButton);
    if (it == d->languageRows.end())
    {
        kError() << "KSwitchLanguageDialog::removeButtonClicked called from unknown KPushButton" << endl;
        return;
    }

    LanguageRowData languageRowData = it.value();
    
    d->languageButtons.removeAll(languageRowData.languageButton);
    
    languageRowData.label->deleteLater();
    languageRowData.languageButton->deleteLater();
    languageRowData.removeButton->deleteLater();
    d->languageRows.erase(it);
}

void KSwitchLanguageDialog::languageOnButtonChanged(const QString & languageCode)
{
    for ( int i = 0, count = d->languageButtons.count(); i < count; ++i )
    {
        KLanguageButton *languageButton = d->languageButtons[i];
#if 0
        if (languageButton->current() == languageCode)
        {
            //update all buttons which have matching id
            //might update buttons which were not changed, but well...
            languageButton->setText(KGlobal::locale()->languageCodeToName(languageCode));
        }
#endif
    }
}

void KSwitchLanguageDialog::slotOk()
{
    QString languageString;
    bool first = true;
 
    for ( int i = 0, count = d->languageButtons.count(); i < count; ++i )
    {
        KLanguageButton *languageButton = d->languageButtons[i];
        
        if (!first)
        {
            languageString += ':';
        }
        languageString += languageButton->current();
        first = false;
    }
    
    if (d->applicationLanguageList().join(":") != languageString)
    {
        //list is different from defaults or saved languages list
        KConfigGroup group(KGlobal::config(), "Locale");
    
        group.writeEntry("Language", languageString);
        group.sync();
        
        KMessageBox::information(
            this,
            i18n("Language for this application has been changed. The change will take effect upon next start of application"), //text
            i18n("Application language changed"), //caption
            "ApplicationLanguageChangedWarning" //dontShowAgainName
            );
    }

    accept();
}

/************************ KSwitchLanguageDialogPrivate ***********************/

KSwitchLanguageDialogPrivate::KSwitchLanguageDialogPrivate(
    KSwitchLanguageDialog *parent)
  : p(parent)
{
    //NOTE: do NOT use "p" in constructor, it is not fully constructed
}

void KSwitchLanguageDialogPrivate::fillApplicationLanguages(KLanguageButton *button)
{
    KLocale *locale = KGlobal::locale();
    QStringList allLanguages = locale->allLanguagesList();
    for ( int i = 0, count = allLanguages.count(); i < count; ++i )
    {
        QString languageCode = allLanguages[i];
        if (locale->isApplicationTranslatedInto(languageCode))
        {
            button->insertLanguage(languageCode);
        }
    }
}

QStringList KSwitchLanguageDialogPrivate::applicationLanguageList()
{
    KSharedConfigPtr config = KGlobal::config();
    QStringList languagesList;
    
    if (config->hasGroup("Locale"))
    {
        KConfigGroup group(config, "Locale");
        if (group.hasKey("Language"))
        {
            languagesList = group.readEntry("Language", QStringList(), ':');
        }
    }
    return languagesList.isEmpty() ? KGlobal::locale()->languageList() : languagesList;
}

void KSwitchLanguageDialogPrivate::addLanguageButton(const QString & languageCode, bool primaryLanguage)
{
    QString labelText = primaryLanguage ? i18n("Primary language:") : i18n("Fallback language:");
    
    KLanguageButton *languageButton = new KLanguageButton(page);
    
    fillApplicationLanguages(languageButton);
    
    languageButton->setCurrentItem(languageCode);
    
    QObject::connect(
        languageButton, 
        SIGNAL(activated(const QString &)), 
        p,
        SLOT(languageOnButtonChanged(const QString &))
        );
    
    LanguageRowData languageRowData;
    KPushButton *removeButton = 0;
    
    if (!primaryLanguage)
    {
        removeButton = new KPushButton(i18n("Remove"), page);
        
        QObject::connect(
            removeButton, 
            SIGNAL(clicked()),
            p,
            SLOT(removeButtonClicked())
            );
    }
        
    languageButton->setToolTip(primaryLanguage
          ? i18n("This is main application language which will be used first before any other languages")
          : i18n("This is language which will be used if any previous languages does not contain proper translation"));
    
    int numRows = languagesLayout->rowCount();
    
    QLabel *languageLabel = new QLabel(labelText, page);
    languagesLayout->addWidget( languageLabel, numRows + 1, 1, Qt::AlignLeft );
    languagesLayout->addWidget( languageButton, numRows + 1, 2, Qt::AlignLeft );
    
    if (!primaryLanguage)
    {
        languagesLayout->addWidget( removeButton, numRows + 1, 3, Qt::AlignLeft );
        languageRowData.setRowWidgets( languageLabel, languageButton, removeButton );
        removeButton->show();
    }
    
    languageRows.insert(removeButton, languageRowData);
    
    languageButtons.append(languageButton);
    languageButton->show();
    languageLabel->show();
}

