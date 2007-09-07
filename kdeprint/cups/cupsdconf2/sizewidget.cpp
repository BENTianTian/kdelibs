/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <kdeprint@swing.be>
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

#include "sizewidget.h"

#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QLayout>
#include <QtCore/QRegExp>
#include <klocale.h>

SizeWidget::SizeWidget(QWidget *parent)
        : QWidget(parent)
{
    m_size = new QSpinBox(this);
    m_size->setRange(0, 9999);
    m_size->setSingleStep(1);
    m_unit = new QComboBox(this);

    m_unit->addItem(i18n("KB"));
    m_unit->addItem(i18n("MB"));
    m_unit->addItem(i18n("GB"));
    m_unit->addItem(i18n("Tiles"));
    m_unit->setCurrentIndex(1);
    m_size->setSpecialValueText(i18n("Unlimited"));

    QHBoxLayout *l0 = new QHBoxLayout(this);
    l0->setMargin(0);
    l0->setSpacing(5);
    l0->addWidget(m_size, 1);
    l0->addWidget(m_unit, 0);
}

void SizeWidget::setSizeString(const QString& sz)
{
    const int pos = sz.indexOf(QRegExp("\\D"));   // first non-digit char, or -1.
    m_size->setValue(sz.left(pos).toInt());
    int idx = 1;
    if (pos != -1) {
        switch (sz[pos].toLatin1()) {
        case 'k': idx = 0; break;
        default:
        case 'm': idx = 1; break;
        case 'g': idx = 2; break;
        case 't': idx = 3; break;
        }
    }
    m_unit->setCurrentIndex(idx);
}

QString SizeWidget::sizeString() const
{
    QString result = QString::number(m_size->value());
    switch (m_unit->currentIndex()) {
    case 0: result.append("k"); break;
    case 1: result.append("m"); break;
    case 2: result.append("g"); break;
    case 3: result.append("t"); break;
    }
    return result;
}

void SizeWidget::setValue(int value)
{
    m_size->setValue(value);
    m_unit->setCurrentIndex(1);
}

int SizeWidget::value() const
{
    return m_size->value();
}
