/**
 * configdialog.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#include "configdialog.h"
#include "configwidget.h"

#include <klocale.h>

#include <q3vbox.h>

using namespace KSpell2;

class ConfigDialog::Private
{
public:
    ConfigWidget *ui;
};

ConfigDialog::ConfigDialog( Broker *broker, QWidget *parent )
    : KDialogBase( parent, "KSpell2ConfigDialog", true,
                   i18n( "KSpell2 Configuration" ),
                   KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Cancel,
                   KDialogBase::Ok, true )
{
    init( broker );
}

ConfigDialog::~ConfigDialog()
{
    delete d; d = 0;
}

void ConfigDialog::init( Broker *broker )
{
    d = new Private;
    Q3VBox *page = makeVBoxMainWidget();
    d->ui = new ConfigWidget( broker, page );
}

void ConfigDialog::slotOk()
{
    d->ui->save();
    accept();
}

void ConfigDialog::slotApply()
{
    d->ui->save();
}


#include "configdialog.moc"
