/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer at kde.org>

   Parts of this class have been take from the KAboutApplication class, which was
   Copyright (C) 2000 Waldo Bastian (bastian@kde.org) and Espen Sand (espen@kde.org)

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

#include "kaboutapplicationdialog.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QTabWidget>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <ktextbrowser.h>
#include <ktitlewidget.h>

class KAboutApplicationDialog::Private
{
public:
    Private(KAboutApplicationDialog *parent)
        : q(parent),
          aboutData(0)
    {}

    void _k_showLicense();

    KAboutApplicationDialog *q;

    const KAboutData *aboutData;
};

KAboutApplicationDialog::KAboutApplicationDialog(const KAboutData *aboutData, QWidget *parent)
  : KDialog(parent),
    d(new Private(this))
{
    setPlainCaption(i18n("About %1", aboutData->programName()));
    setButtons(KDialog::Close);
    setDefaultButton(KDialog::Close);
    setModal(false);

    if (aboutData == 0)
        aboutData = KGlobal::mainComponent().aboutData();

    d->aboutData = aboutData;

    if (!aboutData) {
        QLabel *errorLabel = new QLabel(i18n("No information available.<br />"
                                             "The supplied KAboutData object does not exist."), this);
        setMainWidget(errorLabel);
        return;
    }

    KTitleWidget *titleWidget = new KTitleWidget(this);

    titleWidget->setPixmap(qApp->windowIcon().pixmap(64, 64), KTitleWidget::ImageLeft);
    if (aboutData->programLogo().canConvert<QPixmap>())
        titleWidget->setPixmap(aboutData->programLogo().value<QPixmap>(), KTitleWidget::ImageLeft);
    else if (aboutData->programLogo().canConvert<QImage>())
        titleWidget->setPixmap(QPixmap::fromImage(aboutData->programLogo().value<QImage>()), KTitleWidget::ImageLeft);

    titleWidget->setText(i18n("<font size=\"5\">%1</font><br><b>Version %2</b><br>Using KDE %3</html>", aboutData->programName(),
                         aboutData->version(), QString(KDE_VERSION_STRING)));

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);

    QString aboutPageText = aboutData->shortDescription() + "<br />";

    if (!aboutData->otherText().isEmpty())
        aboutPageText += "<br />" + aboutData->otherText() + "<br />";

    if (!aboutData->copyrightStatement().isEmpty())
        aboutPageText += "<br />" + aboutData->copyrightStatement() + "<br />";

    if (!aboutData->homepage().isEmpty())
        aboutPageText += "<br />" + QString("<a href=\"%1\">%1</a>").arg(aboutData->homepage()) + "<br />";

    QLabel *aboutLabel = new QLabel;
    aboutLabel->setWordWrap(true);
    aboutLabel->setOpenExternalLinks(true);
    aboutLabel->setText(aboutPageText.replace('\n', "<br />"));

    QVBoxLayout *aboutLayout = new QVBoxLayout;
    aboutLayout->addStretch();
    aboutLayout->addWidget(aboutLabel);

    if (!aboutData->license().isEmpty()) {
        QLabel *showLicenseLabel = new QLabel;
        showLicenseLabel->setText(QString("<a href=\"%1\">%1</a>").arg(i18n("License: %1",
                                                                            aboutData->licenseName(KAboutData::FullName))));
        connect(showLicenseLabel, SIGNAL(linkActivated(QString)), this, SLOT(_k_showLicense()));

        aboutLayout->addWidget(showLicenseLabel);
    }

    aboutLayout->addStretch();

    QWidget *aboutWidget = new QWidget(this);
    aboutWidget->setLayout(aboutLayout);

    tabWidget->addTab(aboutWidget, i18n("&About"));

    int authorCount = aboutData->authors().count();
    if (authorCount) {
        QString authorPageText;

        QString authorPageTitle = authorCount == 1 ? i18n("A&uthor") : i18n("A&uthors");

        if (!aboutData->customAuthorTextEnabled() || !aboutData->customAuthorRichText().isEmpty()) {
            if (!aboutData->customAuthorTextEnabled()) {
                if (aboutData->bugAddress().isEmpty() || aboutData->bugAddress() == "submit@bugs.kde.org")
                    authorPageText = i18n("Please use <a href=\"http://bugs.kde.org\">http://bugs.kde.org</a> to report bugs.\n");
                else {
                    if(aboutData->authors().count() == 1 && (aboutData->authors().first().emailAddress() == aboutData->bugAddress())) {
                        authorPageText = i18n("Please report bugs to <a href=\"mailto:%1\">%2</a>.\n",
                                              aboutData->authors().first().emailAddress(),
                                              aboutData->authors().first().emailAddress());
                    }
                    else {
                        authorPageText = i18n("Please report bugs to <a href=\"mailto:%1\">%2</a>.\n",
                                              aboutData->bugAddress(), aboutData->bugAddress());
                    }
                }
            }
            else
                authorPageText = aboutData->customAuthorRichText();
        }

        authorPageText += "<br />";

        QList<KAboutPerson> lst = aboutData->authors();
        for (int i = 0; i < lst.size(); ++i) {
            authorPageText += QString("<br />%1<br />").arg(lst.at(i).name());
            if (!lst.at(i).emailAddress().isEmpty())
                authorPageText += QString("&nbsp;&nbsp;<a href=\"mailto:%1\">%1</a><br />").arg(lst.at(i).emailAddress());
            if (!lst.at(i).webAddress().isEmpty())
                authorPageText += QString("&nbsp;&nbsp;<a href=\"%3\">%3</a><br />").arg(lst.at(i).webAddress());
            if (!lst.at(i).task().isEmpty())
                authorPageText += QString("&nbsp;&nbsp;%4<br />").arg(lst.at(i).task());
        }

        KTextBrowser *authorTextBrowser = new KTextBrowser;
        authorTextBrowser->setFrameStyle(QFrame::NoFrame);
        authorTextBrowser->setHtml(authorPageText);
        tabWidget->addTab(authorTextBrowser, authorPageTitle);
    }

    int creditsCount = aboutData->credits().count();
    if (creditsCount) {
        QString creditsPageText;

        QList<KAboutPerson> lst = aboutData->credits();
        for (int i = 0; i < lst.size(); ++i) {
            creditsPageText += QString("<br />%1<br />").arg(lst.at(i).name());
            if (!lst.at(i).emailAddress().isEmpty())
                creditsPageText += QString("&nbsp;&nbsp;<a href=\"mailto:%1\">%1</a><br />").arg(lst.at(i).emailAddress());
            if (!lst.at(i).webAddress().isEmpty())
                creditsPageText += QString("&nbsp;&nbsp;<a href=\"%3\">%3</a><br />").arg(lst.at(i).webAddress());
            if (!lst.at(i).task().isEmpty())
                creditsPageText += QString("&nbsp;&nbsp;%4<br />").arg(lst.at(i).task());
        }

        KTextBrowser *creditsTextBrowser = new KTextBrowser;
        creditsTextBrowser->setFrameStyle(QFrame::NoFrame);
        creditsTextBrowser->setHtml(creditsPageText);
        tabWidget->addTab(creditsTextBrowser, i18n("&Thanks To"));
    }

    const QList<KAboutPerson> translatorList = aboutData->translators();

    if(translatorList.count() > 0) {
        QString translatorPageText = QString();

        QList<KAboutPerson>::ConstIterator it;
        for(it = translatorList.begin(); it != translatorList.end(); ++it) {
            translatorPageText += QString("<br />%1<br />").arg((*it).name());
            if (!(*it).emailAddress().isEmpty())
                translatorPageText += QString("&nbsp;&nbsp;<a href=\"mailto:%1\">%1</a><br />").arg((*it).emailAddress());
        }

        translatorPageText += KAboutData::aboutTranslationTeam();

        KTextBrowser *translatorTextBrowser = new KTextBrowser;
        translatorTextBrowser->setFrameStyle(QFrame::NoFrame);
        translatorTextBrowser->setHtml(translatorPageText);
        tabWidget->addTab(translatorTextBrowser, i18n("T&ranslation"));
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(tabWidget);
    mainLayout->setMargin(0);

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);
}

KAboutApplicationDialog::~KAboutApplicationDialog()
{
    delete d;
}

void KAboutApplicationDialog::Private::_k_showLicense()
{
    KDialog *dialog = new KDialog(q);

    dialog->setCaption(i18n("License Agreement"));
    dialog->setButtons(KDialog::Close);
    dialog->setDefaultButton(KDialog::Close);

    QFont font = KGlobalSettings::fixedFont();
    QFontMetrics metrics(font);

    KTextBrowser *licenseBrowser = new KTextBrowser;
    licenseBrowser->setFont(font);
    licenseBrowser->setLineWrapMode(QTextEdit::NoWrap);
    licenseBrowser->setText(aboutData->license());

    dialog->setMainWidget(licenseBrowser);

    // try to set up the dialog such that the full width of the
    // document is visible without horizontal scroll-bars being required
    qreal idealWidth = licenseBrowser->document()->idealWidth() + (2 * dialog->marginHint())
        + licenseBrowser->verticalScrollBar()->width() * 2;

    // try to allow enough height for a reasonable number of lines to be shown
    int idealHeight = metrics.height() * 30;

    dialog->setInitialSize(dialog->sizeHint().expandedTo(QSize((int)idealWidth,idealHeight)));
    dialog->show();
}

#include "kaboutapplicationdialog.moc"
