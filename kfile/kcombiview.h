/*  -*- c++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KCOMBIVIEW_H
#define _KCOMBIVIEW_H

#include "kfileview.h"
#include <qsplitter.h>
#include <klocale.h>

class KFileIconView;
class QIconViewItem;

/**
 * This view is designed to combine two KFileViews into one widget, to show
 * directories on the left side and files on the right side.
 * 
 * Methods like selectedItems() to query status _only_ work on the right side,
 * i.e. on the files.
 */
class KCombiView : public QSplitter,
		   public KFileView
{
    Q_OBJECT

public:
    KCombiView( QWidget *parent, const char *name);
    virtual ~KCombiView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();

    virtual void updateView( bool );
    virtual void updateView(const KFileViewItem*);

    void setRight(KFileView *view);

    virtual void setSelectMode( KFileView::SelectionMode sm );

    virtual bool isSelected( const KFileViewItem * ) const;
    virtual void insertSorted(KFileViewItem *first, uint counter);
    virtual void clear();
    virtual void clearSelection();

protected:
    virtual void insertItem( KFileViewItem *i );
    virtual void highlightItem(const KFileViewItem *);

private:
    KFileIconView *left;

protected:
    KFileView *right;

protected slots:
    void activatedMenu(const KFileViewItem*);
    void selectDir(const KFileViewItem*);
    void highlightFile(const KFileViewItem*);
    void selectFile(const KFileViewItem*);

};

#endif
