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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "configdialog.h"
#include "configwidget.h"

#include <klocale.h>

#include <kvbox.h>

using namespace Sonnet;

class ConfigDialog::Private
{
public:
    ConfigWidget *ui;
};

ConfigDialog::ConfigDialog( Loader::Ptr loader, QWidget *parent )
    : KDialog( parent ),d(new Private)
{
    setObjectName( "SonnetConfigDialog" );
    setModal( true );
    setCaption( i18n( "Sonnet Configuration" ) );
    setButtons( Ok | Apply | Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );

    init( loader );
}

ConfigDialog::~ConfigDialog()
{
    delete d;
}

void ConfigDialog::init( Loader::Ptr loader )
{
    d->ui = new ConfigWidget( loader, this );
    setMainWidget( d->ui );
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()), this, SLOT(slotApply()));
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
