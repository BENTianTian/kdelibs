/* This file is part of the KDE libraries
    Copyright (C) Stephan Kulow <coolo@kde.org>

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

#ifndef KFILEFILTER_H
#define KFILEFILTER_H

#include <qstringlist.h>

#include <kcombobox.h>

class KFileFilter : public KComboBox
{
    Q_OBJECT

 public:
    KFileFilter(QWidget *parent= 0, const char *name= 0);
    ~KFileFilter();

    void setFilter(const QString& filter);
    QString currentFilter() const;

 protected:
    QStringList filters;

 signals:
    void filterChanged();
};

#endif
