/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KACTIVELABEL_H
#define KACTIVELABEL_H

#include <ktextbrowser.h>
#include <kdelibs_export.h>

class KActiveLabelPrivate;
 /**
  * Label with support for selection and clickable links.
  *
  * @author Waldo Bastian (bastian@kde.org)
  */
class KDEUI_EXPORT KActiveLabel : public KTextBrowser
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * It connects the "anchorClicked(const QUrl &)" signal to the
     * "openLink(const QUrl &)" slot. You will need to disconnect
     * this if you want to process anchorClicked() yourself.
     */
    KActiveLabel(QWidget * parent);

    /**
     * Constructor.
     *
     * It connects the "anchorClicked(const QUrl &)" signal to the
     * "openLink(const QUrl &)" slot. You will need to disconnect
     * this if you want to process anchorClicked() yourself.
     */
    KActiveLabel(const QString & text, QWidget * parent);

    virtual ~KActiveLabel();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    virtual void virtual_hook( int id, void* data );
    virtual void focusInEvent( QFocusEvent* fe );
    virtual void focusOutEvent( QFocusEvent* fe );
    virtual void keyPressEvent ( QKeyEvent * e );
    virtual bool event(QEvent *e);

public Q_SLOTS:
    virtual void setSource(const QUrl &name);

private:
    KActiveLabelPrivate *const d;
};

#endif
