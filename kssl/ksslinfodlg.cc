/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
 * Copyright (C) 2000 Malte Starostik <malte.starostik@t-online.de>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "ksslinfodlg.h"

#include <kssl.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qlabel.h>
#include <qscrollview.h>
#include <qfile.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kprocess.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kurllabel.h>
//#include <kstddirs.h>
//#include <krun.h>

class KSSLInfoDlg::KSSLInfoDlgPrivate {
private:
    friend class KSSLInfoDlg;
    bool m_secCon;
    QGridLayout *m_layout;
};

KSSLInfoDlg::KSSLInfoDlg(bool secureConnection, QWidget *parent, const char *name)
 : KDialog(parent, name, false, Qt::WDestructiveClose), d(new KSSLInfoDlgPrivate) {
    d->m_secCon = secureConnection;
    d->m_layout = new QGridLayout(this, 4, 3, KDialog::marginHint(), KDialog::spacingHint());
    d->m_layout->setColStretch(1, 1);
    d->m_layout->setColStretch(2, 1);
    QLabel *pixmap = new QLabel(this);
    d->m_layout->addWidget(pixmap, 0, 0);
    QLabel *info = new QLabel(this);
    d->m_layout->addWidget(info, 0, 1);
    if (KSSL::doesSSLWork()) {
        if (d->m_secCon) {
            pixmap->setPixmap(BarIcon("lock"));
            info->setText(i18n("Current connection is secured with SSL."));
        } else {
            pixmap->setPixmap(BarIcon("unlock"));
            info->setText(i18n("Current connection is not secured with SSL."));
        }
    } else {
        pixmap->setPixmap(BarIcon("unlock"));
        info->setText(i18n("SSL support is not available in this build of KDE."));
    }

    QPushButton *button = new QPushButton(i18n("Close"), this);
    connect(button, SIGNAL(clicked()), SLOT(close()));
    d->m_layout->addWidget(button, 3, 1);

    button = new QPushButton(i18n("Cryptography Configuration..."), this);
    connect(button, SIGNAL(clicked()), SLOT(launchConfig()));
    d->m_layout->addWidget(button, 3, 2);

    setCaption(i18n("KDE SSL Information"));
}


KSSLInfoDlg::~KSSLInfoDlg() {
    delete d;
}

void KSSLInfoDlg::launchConfig() {
KShellProcess p;
  p << "kcmshell" << "crypto";
  p.start(KProcess::DontCare);
}

void KSSLInfoDlg::setup(const QString& peername, const QString& issuer,
                        const QString& ip, const QString& url,
                        const QString& cipher, const QString& cipherdesc,
                        const QString& sslversion, int usedbits, int bits,
                        KSSLCertificate::KSSLValidation certState,
                        const QString& goodFrom, const QString& goodUntil) {
// Needed to put the GUI stuff here to get the layouting right
    QGridLayout *layout = new QGridLayout(2, 2, KDialog::spacingHint());
    layout->addWidget(new QLabel(i18n("Peer Certificate:"), this), 0, 0);
    layout->addWidget(buildCertInfo(peername), 1, 0);
    layout->addWidget(new QLabel(i18n("Issuer:"), this), 0, 1);
    layout->addWidget(buildCertInfo(issuer), 1, 1);
    d->m_layout->addMultiCell(layout, 1, 1, 0, 2);

    layout = new QGridLayout(7, 2, KDialog::spacingHint());
    layout->setColStretch(1, 1);
    layout->addWidget(new QLabel(i18n("IP Address:"), this), 0, 0);
    layout->addWidget(new QLabel(ip, this), 0, 1);
    layout->addWidget(new QLabel(i18n("URL:"), this), 1, 0);
    KURLLabel *urlLabel = new KURLLabel(url, url, this);
    layout->addWidget(urlLabel, 1, 1);
    connect(urlLabel, SIGNAL(leftClickedURL(const QString &)), SLOT(urlClicked(const QString &)));
    layout->addWidget(new QLabel(i18n("Certificate State:"), this), 2, 0);

    QLabel *csl;
    QPalette cspl;
    switch(certState) {
    case KSSLCertificate::Ok:
      layout->addWidget(csl = new QLabel(i18n("Certificate is valid from %1 until %1.").arg(goodFrom).arg(goodUntil), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
      csl->setPalette(cspl);
    break;
    case KSSLCertificate::NoCARoot:
      layout->addWidget(csl = new QLabel(i18n("Certificate signing authority root files could not be found so the certificate is not verified."), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
      csl->setPalette(cspl);
    break;
    case KSSLCertificate::InvalidCA:
      layout->addWidget(csl = new QLabel(i18n("Certificate signing authority is unknown or invalid."), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
      csl->setPalette(cspl);
    break;
    case KSSLCertificate::SelfSigned:
      layout->addWidget(csl = new QLabel(i18n("Certificate is self signed and thus may not be trustworthy."), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
      csl->setPalette(cspl);
    break;
    case KSSLCertificate::Expired:
      layout->addWidget(csl = new QLabel(i18n("Certificate has expired."), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
      csl->setPalette(cspl);
    break;
    case KSSLCertificate::Revoked:
      layout->addWidget(csl = new QLabel(i18n("Certificate has been revoked."), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
      csl->setPalette(cspl);
    break;
    default:
      layout->addWidget(csl = new QLabel(i18n("Certificate is not valid."), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
      csl->setPalette(cspl);
    break;
    }
    update();

    layout->addWidget(new QLabel(i18n("Cipher in Use:"), this), 3, 0);
    layout->addWidget(new QLabel(cipher, this), 3, 1);
    layout->addWidget(new QLabel(i18n("Details:"), this), 4, 0);
    layout->addWidget(new QLabel(cipherdesc.simplifyWhiteSpace(), this), 4, 1);
    layout->addWidget(new QLabel(i18n("SSL Version:"), this), 5, 0);
    layout->addWidget(new QLabel(sslversion, this), 5, 1);
    layout->addWidget(new QLabel(i18n("Cipher Strength:"), this), 6, 0);
    layout->addWidget(new QLabel(i18n("%1 bits used of a %1 bit cipher").arg(usedbits).arg(bits), this), 6, 1);
    d->m_layout->addMultiCell(layout, 2, 2, 0, 2);
}

QScrollView *KSSLInfoDlg::buildCertInfo(const QString &certName) {
    KSSLX509Map cert(certName);
    QString tmp;
    QScrollView *result = new QScrollView(this);
    result->viewport()->setBackgroundMode(QWidget::PaletteButton);
    QFrame *frame = new QFrame(result);
    QGridLayout *grid = new QGridLayout(frame, 1, 2, KDialog::marginHint(), KDialog::spacingHint());
    grid->setAutoAdd(true);
    if (!(tmp = cert.getValue("O")).isEmpty()) {
        new QLabel(i18n("Organization:"), frame);
        new QLabel(tmp, frame);
    }
    if (!(tmp = cert.getValue("OU")).isEmpty()) {
        new QLabel(i18n("Organizational Unit:"), frame);
        new QLabel(tmp, frame);
    }
    if (!(tmp = cert.getValue("L")).isEmpty()) {
        new QLabel(i18n("Locality:"), frame);
        new QLabel(tmp, frame);
    }
    if (!(tmp = cert.getValue("ST")).isEmpty()) {
        new QLabel(i18n("State:"), frame);
        new QLabel(tmp, frame);
    }
    if (!(tmp = cert.getValue("C")).isEmpty()) {
        new QLabel(i18n("Country:"), frame);
        new QLabel(tmp, frame);
    }
    if (!(tmp = cert.getValue("CN")).isEmpty()) {
        new QLabel(i18n("Common Name:"), frame);
        new QLabel(tmp, frame);
    }
    if (!(tmp = cert.getValue("Email")).isEmpty()) {
        new QLabel(i18n("EMail:"), frame);
        KURLLabel *mail = new KURLLabel(tmp, tmp, frame);
        connect(mail, SIGNAL(leftClickedURL(const QString &)), SLOT(mailClicked(const QString &)));
    }
    result->addChild(frame);
    return result;
}

void KSSLInfoDlg::urlClicked(const QString &url) {
    kapp->invokeBrowser(url);
}

void KSSLInfoDlg::mailClicked(const QString &url) {
    kapp->invokeMailer(url, QString::null);
}

#include "ksslinfodlg.moc"

