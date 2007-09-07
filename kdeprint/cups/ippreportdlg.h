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

#ifndef IPPREPORTDLG_H
#define IPPREPORTDLG_H

#include <kdialog.h>

class IppRequest;
class KTextEdit;

#ifdef __GNUC__
#warning rename class or remove from global namespace
#endif
class IppReportDlg : public KDialog
{
    Q_OBJECT
public:
    explicit IppReportDlg(QWidget *parent = 0, const char *name = 0);
    static void report(IppRequest *req, int group, const QString& caption = QString());

protected Q_SLOTS:
    void slotUser1();

private:
    KTextEdit *m_edit;
};

#endif
