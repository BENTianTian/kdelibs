/*  This file is part of the KDE project
    Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
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

#ifndef KWIDGETJOBTRACKER_P_H
#define KWIDGETJOBTRACKER_P_H

#include <QWidget>
#include <QMap>
#include <QTime>

#include <kdebug.h>
#include <kurl.h>

class KPushButton;
class QCheckBox;
class KLineEdit;
class QLabel;
class QProgressBar;


class KWidgetJobTracker::Private
{
public:
    Private(QWidget *parent)
        : parent(parent) { }

    ~Private() {
    }

    class ProgressWidget;

    QWidget *parent;
    QMap<KJob*, ProgressWidget*> progressWidget;
};


class KWidgetJobTracker::Private::ProgressWidget
    : public QWidget
{
    Q_OBJECT

public:
    ProgressWidget(KJob *job, KWidgetJobTracker *object, QWidget *parent)
        : q(object), job(job), totalSize(0), totalFiles(0), totalDirs(0),
          processedSize(0), processedDirs(0), processedFiles(0),
          keepOpenChecked(false), cancelClose(0), openFile(0), openLocation(0),
          keepOpenCheck(0), pauseButton(0), sourceEdit(0), destEdit(0),
          progressLabel(0), destInvite(0), speedLabel(0), sizeLabel(0),
          resumeLabel(0), progressBar(0), suspendedProperty(false)
    {
        init(parent);
    }

    ~ProgressWidget()
    {
    }

    KWidgetJobTracker *const q;
    KJob *const job;

    qulonglong totalSize;
    qulonglong totalFiles;
    qulonglong totalDirs;
    qulonglong processedSize;
    qulonglong processedDirs;
    qulonglong processedFiles;

    bool keepOpenChecked;
    QString caption;

    KPushButton *cancelClose;
    KPushButton *openFile;
    KPushButton *openLocation;
    QCheckBox   *keepOpenCheck;
    KUrl        location;
    QTime       startTime;
    KPushButton *pauseButton;
    KLineEdit *sourceEdit;
    KLineEdit *destEdit;
    QLabel *progressLabel;
    QLabel *sourceInvite;
    QLabel *destInvite;
    QLabel *speedLabel;
    QLabel *sizeLabel;
    QLabel *resumeLabel;
    QProgressBar *progressBar;

    bool suspendedProperty;

    void init(QWidget *parent);
    void showTotals();
    void setDestVisible(bool visible);
    void checkDestination(const KUrl &dest);
    bool keepOpen() const;

public Q_SLOTS:
    virtual void infoMessage(const QString &plain, const QString &rich);
    virtual void description(const QString &title,
                             const QPair<QString, QString> &field1,
                             const QPair<QString, QString> &field2);
    virtual void totalAmount(KJob::Unit unit, qulonglong amount);
    virtual void processedAmount(KJob::Unit unit, qulonglong amount);
    virtual void percent(unsigned long percent);
    virtual void speed(unsigned long value);
    virtual void slotClean();
    virtual void suspended();
    virtual void resumed();

    //TODO: Misses canResume()

protected:
    void closeEvent(QCloseEvent *event);

private Q_SLOTS:
    void _k_keepOpenToggled(bool);
    void _k_openFile();
    void _k_openLocation();
    void _k_pauseResumeClicked();
    void _k_stop();
};


#endif // KWIDGETJOBTRACKER_P_H
