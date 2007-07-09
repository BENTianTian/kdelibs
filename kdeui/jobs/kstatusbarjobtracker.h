/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef KSTATUSBARJOBTRACKER_H
#define KSTATUSBARJOBTRACKER_H

#include <kabstractwidgetjobtracker.h>

/**
 * This class implements a job tracker with a widget suited for embedding in a
 * status bar.
 */
class KDEUI_EXPORT KStatusBarJobTracker : public KAbstractWidgetJobTracker
{
    Q_OBJECT

public:
    /**
     * Creates a new KStatusBarJobTracker
     *
     * @param parent the parent of this object and of the widget displaying the job progresses
     * @param button true to display a stop button allowing to kill the job, false otherwise
     */
    explicit KStatusBarJobTracker(QWidget *parent = 0, bool button = true);

    /**
     * Destroys a KStatusBarJobTracker
     */
    virtual ~KStatusBarJobTracker();

    /**
     * Register a new job in this tracker.
     *
     * @param job the job to register
     */
    virtual void registerJob(KJob *job);

    /**
     * The widget associated to this tracker.
     *
     * @return the widget displaying the job progresses
     */
    virtual QWidget *widget();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

protected Q_SLOTS:
    virtual void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    virtual void percent(KJob *job, unsigned long percent);
    virtual void speed(KJob *job, unsigned long value);
    virtual void slotClean();

private:
    class Private;
    Private *const d;
};

#endif
