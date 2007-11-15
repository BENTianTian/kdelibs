/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>
   Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KNOTIFICATIONMANAGER_H
#define KNOTIFICATIONMANAGER_H

#include <knotification.h>

class KNotification;
class QPixmap;
class QStringList;

/**
 * @internal
 * @author Olivier Goffart
 */
class KNotificationManager : public QObject
{
Q_OBJECT
public: 
    static KNotificationManager* self();
    ~KNotificationManager();

    /**
     * send the dbus call to the knotify server
     */
    void notify(KNotification *n, const QPixmap& pix, const QStringList &action,
                        const KNotification::ContextList& contexts, const QString &appname);

    /**
     * send the close dcop call to the knotify server for the notification with the identifier @p id .
     * @param id the id of the notification
     */
    void close(int id);

    /**
     * remove the KNotification ptr from the internal map
     * To be called in the KNotification destructor
     */
    void remove(int id);

    /**
     * Insert the notification and it's id in the internal map
     */
    void insert(KNotification *n, int id);

    /**
     * update one notification text and pixmap and actions
     */
    void update(KNotification *n, int id);

    /**
     * re-emit the notification, eventually with new contexts
     */
    void reemit(KNotification *n, int id);

private Q_SLOTS:
    void notificationClosed( int id );
    void notificationActivated( int id,  int action);

private:
    struct Private;
    Private * const d;
    KNotificationManager();
};

#endif
