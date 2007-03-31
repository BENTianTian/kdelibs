/**
 * configdialog.h
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
#ifndef KSPELL_CONFIGDIALOG_H
#define KSPELL_CONFIGDIALOG_H

#include <kdialog.h>
#include "loader.h"

namespace KSpell2
{
    class Loader;
    class KDEUI_EXPORT ConfigDialog : public KDialog
    {
        Q_OBJECT
    public:
        ConfigDialog( Loader::Ptr loader,
                      QWidget *parent );
        ~ConfigDialog();

    protected Q_SLOTS:
        virtual void slotOk();
        virtual void slotApply();

    private:
        void init( Loader::Ptr loader );
    private:
        class Private;
        Private* const d;
    };
}

#endif
