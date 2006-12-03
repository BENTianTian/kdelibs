/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef OPENWITHDIALOG_P_H
#define OPENWITHDIALOG_P_H

#include <k3listview.h>

/**
 * @internal
 */
class KAppTreeListItem : public Q3ListViewItem
{
    bool parsed;
    bool directory;
    QString path;
    QString exec;

protected:
    int compare(Q3ListViewItem *i, int col, bool ascending ) const;
    QString key(int column, bool ascending) const;

    void init(const QPixmap& pixmap, bool parse, bool dir, const QString &_path, const QString &exec);

public:
    KAppTreeListItem( K3ListView* parent, const QString & name, const QPixmap& pixmap,
                      bool parse, bool dir, const QString &p, const QString &c );
    KAppTreeListItem( Q3ListViewItem* parent, const QString & name, const QPixmap& pixmap,
                      bool parse, bool dir, const QString &p, const QString &c );
    bool isDirectory();

protected:
    virtual void activate();
    virtual void setOpen( bool o );

    friend class KApplicationTree;
};


/**
 * @internal
 */
class KApplicationTree : public K3ListView
{
    Q_OBJECT
public:
    KApplicationTree( QWidget *parent );

    /**
     * Add a group of .desktop/.kdelnk entries
     */
    void addDesktopGroup( const QString &relPath, KAppTreeListItem *item = 0 );

    bool isDirSel();

protected:
    void resizeEvent( QResizeEvent *_ev );
    KAppTreeListItem* currentitem;
    void cleanupTree();

public Q_SLOTS:
    void slotItemHighlighted(Q3ListViewItem* i);
    void slotSelectionChanged(Q3ListViewItem* i);

Q_SIGNALS:
    void selected( const QString& _name, const QString& _exec );
    void highlighted( const QString& _name, const  QString& _exec );
};

#endif
