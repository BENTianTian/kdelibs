/* This file is part of the KDE libraries
   Copyright (C) 1997 Henner Zeller <zeller@think.de>
   Copyright (C) 1997 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef kurlcompletion_h
#define kurlcompletion_h

#include <qstring.h>
#include <qstrlist.h>
#include <qobject.h>

/**
 * URL completion helper. To use this, connect
 * it to a @ref KLineEdit widget, which emits signals for completion
 * (Tab/Ctrl-S) and rotation (Ctrl-D).
 *
 * Limitations: Currently only file completion is supported.
 *
 * @see KLineEdit
 * @author Henner Zeller <zeller@think.de>, based on code by Torben Weis.
 * @version $Id$
 */
class KURLCompletion : public QObject
{
    Q_OBJECT

public:
    KURLCompletion( const QString& dir = QString::null);
    ~KURLCompletion();

public slots:
    void make_completion ();
    void make_rotation ();
    void edited (const QString&);
    
signals:
    void setText (const QString&);
	
private:
    bool is_fileurl (QString &url, bool &ambigous_beginning) const;
    void GetList (QString dir, QString & match);
    bool CompleteDir (QString &dir);
    QString directory;

    bool new_guess;
    bool completed_dir;
    bool self_update;
    QString the_text;
    QStrList possibilityList;
    QString guess;
    QString qual_dir;

    class KURLCompletionPrivate;
    KURLCompletionPrivate *d;
};

#endif
