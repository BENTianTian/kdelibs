/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "callstackdock.h"
#include "callstackdock.moc"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>

#include <kdebug.h>

#include "debugdocument.h"

CallStackDock::CallStackDock(QWidget *parent)
    : QDockWidget("Call Stack", parent)
{
    m_view = new QTableWidget(0, 2);
    m_view->setHorizontalHeaderLabels(QStringList() << "Call" << "Line");
    m_view->verticalHeader()->hide();
    m_view->setShowGrid(false);
    m_view->setAlternatingRowColors(true);

    setWidget(m_view);
}

CallStackDock::~CallStackDock()
{
}

void CallStackDock::displayStack(KJS::DebugDocument *document)
{
    QVector<KJS::CallStackEntry> entries = document->callStack();

    m_view->clearContents();
    m_view->setRowCount(entries.count());
    m_current = document;

    int row = 0;
    foreach (KJS::CallStackEntry entry, entries)
    {
        QTableWidgetItem *function = new QTableWidgetItem(QString("%1").arg(entry.name));
        m_view->setItem(row, 0, function);
        QTableWidgetItem *lineNumber = new QTableWidgetItem(QString("%1").arg(entry.lineNumber));
        m_view->setItem(row, 1, lineNumber);
        row++;
    }
    m_view->resizeColumnsToContents();
    m_view->resizeRowsToContents();
//    m_view->setColumnWidth(1, 20);
}

