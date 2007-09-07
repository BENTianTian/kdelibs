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

#ifndef KMLISTVIEW_H
#define KMLISTVIEW_H

#include <Qt3Support/Q3ListView>
#include <QtCore/QList>

class KMListViewItem;
class KMPrinter;

class KMListView : public Q3ListView
{
    Q_OBJECT
public:
    explicit KMListView(QWidget *parent = 0, const char *name = 0);
    ~KMListView();

    void setPrinterList(QList<KMPrinter*> *list);
    void setPrinter(const QString&);
    void setPrinter(KMPrinter*);

Q_SIGNALS:
    void rightButtonClicked(const QString&, const QPoint&);
    void printerSelected(const QString&);

protected Q_SLOTS:
    void slotRightButtonClicked(Q3ListViewItem*, const QPoint&, int);
    void slotSelectionChanged();
    void slotOnItem(Q3ListViewItem*);
    void slotOnViewport();

protected:
    KMListViewItem* findItem(KMPrinter*);
    KMListViewItem* findItem(const QString&);

private:
    QList<KMListViewItem*> m_items;
    KMListViewItem  *m_root, *m_classes, *m_printers, *m_specials;
};

#endif
