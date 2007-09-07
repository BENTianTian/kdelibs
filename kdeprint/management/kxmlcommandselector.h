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

#ifndef KXMLCOMMANDSELECTOR_H
#define KXMLCOMMANDSELECTOR_H

#include <kdeprint_export.h>

#include <QtGui/QWidget>
#include <QtCore/QStringList>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;
class KDialog;

class KDEPRINT_MANAGEMENT_EXPORT KXmlCommandSelector : public QWidget
{
    Q_OBJECT
public:
    explicit KXmlCommandSelector(bool canBeNull = true, QWidget *parent = 0, KDialog *dlg = 0);

    void setCommand(const QString&);
    QString command() const;

protected:
    void loadCommands();

protected Q_SLOTS:
    void slotAddCommand();
    void slotEditCommand();
    void slotBrowse();
    void slotCommandSelected(int);
    void slotHelpCommand();
    void slotXmlCommandToggled(bool);

Q_SIGNALS:
    void commandValid(bool);

private:
    QComboBox *m_cmd;
    QLineEdit *m_line;
    QCheckBox *m_usefilter;
    QStringList m_cmdlist;
    QLabel  *m_shortinfo;
    QPushButton *m_helpbtn;
    QString m_help;
};

#endif
