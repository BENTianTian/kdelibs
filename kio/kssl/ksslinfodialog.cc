/* This file is part of the KDE project
 *
 * Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2000 Malte Starostik <malte@kde.org>
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
 */

#include "ksslinfodialog.h"

#include <kssl.h>

#include <QtGui/QFrame>
#include <Qt3Support/Q3ScrollView>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/Q_PID>

#include <kapplication.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <ksqueezedtextlabel.h>
#include <kstandardguiitem.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

#include "ksslcertificate.h"
#include "ksslcertchain.h"
#include "ksslsigners.h"


class KSSLInfoDialog::KSSLInfoDialogPrivate {
    private:
        friend class KSSLInfoDialog;
        bool m_secCon;
        QGridLayout *m_layout;
        KComboBox *_chain;
        KSSLCertificate *_cert;
        KSSLCertificate::KSSLValidationList _cert_ksvl;

        bool inQuestion;

        QLabel *_serialNum;
        QLabel *_csl;
        QLabel *_validFrom;
        QLabel *_validUntil;
        QLabel *_digest;

        QLabel *pixmap;
        QLabel *info;

        KSSLCertBox *_subject, *_issuer;
};



KSSLInfoDialog::KSSLInfoDialog(bool secureConnection, QWidget *parent, const char *name, bool modal)
	: KDialog(parent), d(new KSSLInfoDialogPrivate) {
        setObjectName(name);
        setModal(modal);
        setAttribute(Qt::WA_DeleteOnClose);
        QVBoxLayout *topLayout = new QVBoxLayout(this);
        topLayout->setMargin(KDialog::marginHint());
        topLayout->setSpacing(KDialog::spacingHint());
        d->m_secCon = secureConnection;
        d->m_layout = new QGridLayout();
        topLayout->addLayout(d->m_layout);
        d->m_layout->setSpacing(KDialog::spacingHint());
        d->m_layout->setColumnStretch(1, 1);
        d->m_layout->setColumnStretch(2, 1);

        d->pixmap = new QLabel(this);
        d->m_layout->addWidget(d->pixmap, 0, 0);

        d->info = new QLabel(this);
        d->m_layout->addWidget(d->info, 0, 1);

        if (KSSL::doesSSLWork()) {
            if (d->m_secCon) {
                d->pixmap->setPixmap(BarIcon("encrypted"));
                d->info->setText(i18n("Current connection is secured with SSL."));
            } else {
                d->pixmap->setPixmap(BarIcon("decrypted"));
                d->info->setText(i18n("Current connection is not secured with SSL."));
            }
        } else {
            d->pixmap->setPixmap(BarIcon("decrypted"));
            d->info->setText(i18n("SSL support is not available in this build of KDE."));
        }
        d->m_layout->addItem(new QSpacerItem(0, 50), 0, 0); // give minimum height to look better

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(KDialog::spacingHint());
        buttonLayout->addStretch( 1 );
        topLayout->addLayout( buttonLayout );

        KPushButton *button;

        if (KSSL::doesSSLWork()) {
            button = new KPushButton(KGuiItem(i18n("C&ryptography Configuration..."),"configure"), this);
            connect(button, SIGNAL(clicked()), SLOT(launchConfig()));
            buttonLayout->addWidget( button );
        }

        button = new KPushButton(KStandardGuiItem::close(), this);
        connect(button, SIGNAL(clicked()), SLOT(close()));
        buttonLayout->addWidget( button );

        button->setFocus();

        setCaption(i18n("KDE SSL Information"));
        d->inQuestion = false;
    }


KSSLInfoDialog::~KSSLInfoDialog() {
    delete d;
}

void KSSLInfoDialog::launchConfig() {
    QProcess::startDetached("kcmshell", QStringList() << "crypto");
}


void KSSLInfoDialog::setSecurityInQuestion(bool isIt) {
    d->inQuestion = isIt;
    if (KSSL::doesSSLWork())
        if (isIt) {
            d->pixmap->setPixmap(BarIcon("halfencrypted"));
            if (d->m_secCon) {
                d->info->setText(i18n("The main part of this document is secured with SSL, but some parts are not."));
            } else {
                d->info->setText(i18n("Some of this document is secured with SSL, but the main part is not."));
            }
        } else {
            if (d->m_secCon) {
                d->pixmap->setPixmap(BarIcon("encrypted"));
                d->info->setText(i18n("Current connection is secured with SSL."));
            } else {
                d->pixmap->setPixmap(BarIcon("decrypted"));
                d->info->setText(i18n("Current connection is not secured with SSL."));
            }
        }
}


void KSSLInfoDialog::setup( KSSL & ssl, const QString & ip, const QString & url )
{
    setup(
            &ssl.peerInfo().getPeerCertificate(),
            ip,
            url,
            ssl.connectionInfo().getCipher(),
            ssl.connectionInfo().getCipherDescription(),
            ssl.connectionInfo().getCipherVersion(),
            ssl.connectionInfo().getCipherUsedBits(),
            ssl.connectionInfo().getCipherBits(),
            ssl.peerInfo().getPeerCertificate().validate()
         );
}

void KSSLInfoDialog::setup(KSSLCertificate *cert,
        const QString& ip, const QString& url,
        const QString& cipher, const QString& cipherdesc,
        const QString& sslversion, int usedbits, int bits,
        KSSLCertificate::KSSLValidation /*certState*/) {
    // Needed to put the GUI stuff here to get the layouting right

    d->_cert = cert;

    QGridLayout *layout = new QGridLayout();

    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(new QLabel(i18n("Chain:"), this), 0, 0);
    d->_chain = new KComboBox(this);
    layout->addWidget(d->_chain, 1, 0, 1, 2);
    connect(d->_chain, SIGNAL(activated(int)), this, SLOT(slotChain(int)));

    d->_chain->clear();

    if (cert->chain().isValid() && cert->chain().depth() > 1) {
        d->_chain->setEnabled(true);
        d->_chain->addItem(i18n("0 - Site Certificate"));
        int cnt = 0;
        Q3PtrList<KSSLCertificate> cl = cert->chain().getChain();
        for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
            KSSLX509Map map(c->getSubject());
            QString id;
            id = map.getValue("CN");
            if (id.length() == 0)
                id = map.getValue("O");
            if (id.length() == 0)
                id = map.getValue("OU");
            d->_chain->addItem(QString::number(++cnt)+" - "+id);
        }
        d->_chain->setCurrentIndex(0);
    } else d->_chain->setEnabled(false);

    layout->addWidget(new QLabel(i18n("Peer certificate:"), this), 2, 0);
    layout->addWidget(d->_subject = static_cast<KSSLCertBox*>(buildCertInfo(cert->getSubject())), 3, 0);
    layout->addWidget(new QLabel(i18n("Issuer:"), this), 2, 1);
    layout->addWidget(d->_issuer = static_cast<KSSLCertBox*>(buildCertInfo(cert->getIssuer())), 3, 1);
    d->m_layout->addLayout(layout, 1, 0, 1, 3);

    layout = new QGridLayout();
    layout->setSpacing(KDialog::spacingHint());
    layout->setColumnStretch(1, 1);
    QLabel *ipl = new QLabel(i18n("IP address:"), this);
    layout->addWidget(ipl, 0, 0);
    if (ip.isEmpty()) {
        ipl->hide();
    }
    layout->addWidget(ipl = new QLabel(ip, this), 0, 1);
    if (ip.isEmpty()) {
        ipl->hide();
    }
    layout->addWidget(new QLabel(i18n("URL:"), this), 1, 0);
    KSqueezedTextLabel *urlLabel = new KSqueezedTextLabel(url, this);
    layout->addWidget(urlLabel, 1, 1);
    layout->addWidget(new QLabel(i18n("Certificate state:"), this), 2, 0);

    layout->addWidget(d->_csl = new QLabel("", this), 2, 1);

    update();

    layout->addWidget(new QLabel(i18n("Valid from:"), this), 3, 0);
    layout->addWidget(d->_validFrom = new QLabel("", this), 3, 1);
    layout->addWidget(new QLabel(i18n("Valid until:"), this), 4, 0);
    layout->addWidget(d->_validUntil = new QLabel("", this), 4, 1);

    layout->addWidget(new QLabel(i18n("Serial number:"), this), 5, 0);
    layout->addWidget(d->_serialNum = new QLabel("", this), 5, 1);
    layout->addWidget(new QLabel(i18n("MD5 digest:"), this), 6, 0);
    layout->addWidget(d->_digest = new QLabel("", this), 6, 1);

    layout->addWidget(new QLabel(i18n("Cipher in use:"), this), 7, 0);
    layout->addWidget(new QLabel(cipher, this), 7, 1);
    layout->addWidget(new QLabel(i18n("Details:"), this), 8, 0);
    layout->addWidget(new QLabel(cipherdesc.simplified(), this), 8, 1);
    layout->addWidget(new QLabel(i18n("SSL version:"), this), 9, 0);
    layout->addWidget(new QLabel(sslversion, this), 9, 1);
    layout->addWidget(new QLabel(i18n("Cipher strength:"), this), 10, 0);
    layout->addWidget(new QLabel(i18n("%1 bits used of a %2 bit cipher", usedbits, bits), this), 10, 1);
    d->m_layout->addLayout(layout, 2, 0, 1, 3);

    displayCert(cert);
}

void KSSLInfoDialog::setCertState(const QString &errorNrs)
{
    d->_cert_ksvl.clear();
    QStringList errors = errorNrs.split(':', QString::SkipEmptyParts);
    for(QStringList::ConstIterator it = errors.begin();
            it != errors.end(); ++it)
    {
        d->_cert_ksvl << (KSSLCertificate::KSSLValidation) (*it).toInt();
    }
}

void KSSLInfoDialog::displayCert(KSSLCertificate *x) {
    QPalette cspl;

    d->_serialNum->setText(x->getSerialNumber());

    cspl = d->_validFrom->palette();
    if (x->getQDTNotBefore() > QDateTime::currentDateTime().toUTC())
        cspl.setColor(QPalette::Foreground, QColor(196,33,21));
    else cspl.setColor(QPalette::Foreground, QColor(42,153,59));
    d->_validFrom->setPalette(cspl);
    d->_validFrom->setText(x->getNotBefore());

    cspl = d->_validUntil->palette();
    if (x->getQDTNotAfter() < QDateTime::currentDateTime().toUTC())
        cspl.setColor(QPalette::Foreground, QColor(196,33,21));
    else cspl.setColor(QPalette::Foreground, QColor(42,153,59));
    d->_validUntil->setPalette(cspl);
    d->_validUntil->setText(x->getNotAfter());

    cspl = palette();

    KSSLCertificate::KSSLValidation ksv;
    KSSLCertificate::KSSLValidationList ksvl;
    if ((x == d->_cert) && !d->_cert_ksvl.isEmpty()) {
        ksvl = d->_cert_ksvl;
        ksv = ksvl.first();
    } else {
        if (x == d->_cert)
            ksvl = d->_cert->validateVerbose(KSSLCertificate::SSLServer);
        else
            ksvl = d->_cert->validateVerbose(KSSLCertificate::SSLServer, x);

        if (ksvl.isEmpty())
            ksvl << KSSLCertificate::Ok;

        ksv = ksvl.first();

        if (ksv == KSSLCertificate::SelfSigned) {
            if (x->getQDTNotAfter() > QDateTime::currentDateTime().toUTC() &&
                    x->getQDTNotBefore() < QDateTime::currentDateTime().toUTC()) {
                if (KSSLSigners().useForSSL(*x))
                    ksv = KSSLCertificate::Ok;
            } else {
                ksv = KSSLCertificate::Expired;
            }
        }
    }

    if (ksv == KSSLCertificate::Ok) {
        cspl.setColor(QPalette::Foreground, QColor(42,153,59));
    } else if (ksv != KSSLCertificate::Irrelevant) {
        cspl.setColor(QPalette::Foreground, QColor(196,33,21));
    }
    d->_csl->setPalette(cspl);

    QString errorStr;
    for(KSSLCertificate::KSSLValidationList::ConstIterator it = ksvl.begin();
            it != ksvl.end(); ++it) {
        if (!errorStr.isEmpty())
            errorStr.append(QChar('\n'));
        errorStr += KSSLCertificate::verifyText(*it);
    }

    d->_csl->setText(errorStr);
    d->_csl->setMinimumSize(d->_csl->sizeHint());

    d->_subject->setValues(x->getSubject());
    d->_issuer->setValues(x->getIssuer());

    d->_digest->setText(x->getMD5DigestText());
}


void KSSLInfoDialog::slotChain(int x) {
    if (x == 0) {
        displayCert(d->_cert);
    } else {
        Q3PtrList<KSSLCertificate> cl = d->_cert->chain().getChain();
        cl.setAutoDelete(true);
        for (int i = 0; i < x-1; i++)
            cl.remove((unsigned int)0);
        KSSLCertificate thisCert = *(cl.at(0));
        cl.remove((unsigned int)0);
        thisCert.chain().setChain(cl);
        displayCert(&thisCert);
    }
}


KSSLCertBox *KSSLInfoDialog::certInfoWidget(QWidget *parent, const QString &certName, QWidget *mailCatcher) {
    KSSLCertBox *result = new KSSLCertBox(parent);
    if (!certName.isEmpty()) {
        result->setValues(certName, mailCatcher);
    }
    return result;
}


KSSLCertBox::KSSLCertBox(QWidget *parent, const char *name, Qt::WFlags f)
: Q3ScrollView(parent, name, f)
{
    _frame = 0L;
    setBackgroundRole(QPalette::Button);
    setValues(QString(), 0L);
}


void KSSLCertBox::setValues(const QString &certName, QWidget *mailCatcher) {
    if (_frame) {
        removeChild(_frame);
        delete _frame;
    }

    if (certName.isEmpty()) {
        _frame = new QFrame(this);
        addChild(_frame);
        viewport()->setBackgroundRole(_frame->backgroundRole());
        _frame->show();
        updateScrollBars();
        show();
        return;
    }

    KSSLX509Map cert(certName);
    QString tmp;
    viewport()->setBackgroundRole(QPalette::Button);
    _frame = new QFrame(this);
    QGridLayout *grid = new QGridLayout(_frame);
    grid->setMargin(KDialog::marginHint());
    grid->setSpacing(KDialog::spacingHint());

    int row = 0;
    QLabel *label = 0L;
    if (!(tmp = cert.getValue("O")).isEmpty()) {
        label = new QLabel(i18n("Organization:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( new QLabel(tmp, _frame), row, 0 );
        row++;
    }
    if (!(tmp = cert.getValue("OU")).isEmpty()) {
        label = new QLabel(i18n("Organizational unit:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( new QLabel(tmp, _frame), row, 0 );
        row++;
    }
    if (!(tmp = cert.getValue("L")).isEmpty()) {
        label = new QLabel(i18n("Locality:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( new QLabel(tmp, _frame), row, 0 );
        row++;
    }
    if (!(tmp = cert.getValue("ST")).isEmpty()) {
        label = new QLabel(i18nc("Federal State","State:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( new QLabel(tmp, _frame), row, 0 );
        row++;
    }
    if (!(tmp = cert.getValue("C")).isEmpty()) {
        label = new QLabel(i18n("Country:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( new QLabel(tmp, _frame), row, 0 );
        row++;
    }
    if (!(tmp = cert.getValue("CN")).isEmpty()) {
        label = new QLabel(i18n("Common name:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( new QLabel(tmp, _frame), row, 0 );
        row++;
    }
    if (!(tmp = cert.getValue("Email")).isEmpty()) {
        label = new QLabel(i18n("Email:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        if (mailCatcher) {
            KUrlLabel *mail = new KUrlLabel(tmp, tmp, _frame);
            connect(mail, SIGNAL(leftClickedURL(const QString &)), mailCatcher, SLOT(mailClicked(const QString &)));
        } else {
            label = new QLabel(tmp, _frame);
        }
        grid->addWidget( label, row, 0 );
    }
    if (label && viewport()) {
        viewport()->setBackgroundRole(label->backgroundRole());
    }
    addChild(_frame);
    updateScrollBars();
    _frame->show();
    show();
}


Q3ScrollView *KSSLInfoDialog::buildCertInfo(const QString &certName) {
    return KSSLInfoDialog::certInfoWidget(this, certName, this);
}

void KSSLInfoDialog::urlClicked(const QString &url) {
    KToolInvocation::invokeBrowser(url);
}

void KSSLInfoDialog::mailClicked(const QString &url) {
    KToolInvocation::invokeMailer(url, QString());
}

#include "ksslinfodialog.moc"
// vim: ts=4 sw=4 et
