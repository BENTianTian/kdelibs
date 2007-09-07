/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "portdialog.h"
#include "cupsdconf.h"

#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <klocale.h>

PortDialog::PortDialog(QWidget *parent, const char *name)
        : KDialog(parent)
{
    setObjectName(name);
    setModal(true);
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    showButtonSeparator(true);

    QWidget *dummy = new QWidget(this);
    setMainWidget(dummy);
    address_ = new QLineEdit(dummy);
    port_ = new QSpinBox(dummy);
    port_->setRange(0, 9999);
    port_->setSingleStep(1);
    port_->setValue(631);
    usessl_ = new QCheckBox(i18n("Use SSL encryption"), dummy);

    QLabel *l1 = new QLabel(i18n("Address:"), dummy);
    QLabel *l2 = new QLabel(i18n("Port:"), dummy);

    QVBoxLayout *m1 = new QVBoxLayout(dummy);
    m1->setMargin(0);
    m1->setSpacing(10);
    QGridLayout *m2 = new QGridLayout();
    m1->addLayout(m2);
    m2->setMargin(0);
    m2->setSpacing(5);
    m2->addWidget(l1, 0, 0, Qt::AlignRight);
    m2->addWidget(l2, 1, 0, Qt::AlignRight);
    m2->addWidget(usessl_, 2, 2, 0, 1);
    m2->addWidget(address_, 0, 1);
    m2->addWidget(port_, 1, 1);

    setCaption(i18n("Listen To"));
    resize(250, 100);
}

QString PortDialog::listenString()
{
    QString s;
    if (usessl_->isChecked())
        s.append("SSLListen ");
    else
        s.append("Listen ");
    if (!address_->text().isEmpty())
        s.append(address_->text());
    else
        s.append("*");
    s.append(":").append(port_->text());
    return s;
}

void PortDialog::setInfos(CupsdConf *conf)
{
    address_->setWhatsThis(conf->comments_.toolTip("address"));
    port_->setWhatsThis(conf->comments_.toolTip("port"));
    usessl_->setWhatsThis(conf->comments_.toolTip("usessl"));
}

QString PortDialog::newListen(QWidget *parent, CupsdConf *conf)
{
    PortDialog dlg(parent);
    dlg.setInfos(conf);
    if (dlg.exec()) {
        return dlg.listenString();
    }
    return QString();
}

QString PortDialog::editListen(const QString& s, QWidget *parent, CupsdConf *conf)
{
    PortDialog dlg(parent);
    dlg.setInfos(conf);
    int p = s.indexOf(' ');
    if (p != -1) {
        dlg.usessl_->setChecked(s.left(p).startsWith("SSL"));
        QString addr = s.mid(p + 1).trimmed();
        int p1 = addr.indexOf(':');
        if (p1 == -1) {
            dlg.address_->setText(addr);
            dlg.port_->setValue(631);
        } else {
            dlg.address_->setText(addr.left(p1));
            dlg.port_->setValue(addr.mid(p1 + 1).toInt());
        }
    }
    if (dlg.exec()) {
        return dlg.listenString();
    }
    return QString();
}
