/* This file is part of the KDE project
 *
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SAFESITE_H
#define SAFESITE_H

#include <kio/kio_export.h>
#include <kurl.h>

namespace SafeSite {

class Service;

class KIO_EXPORT ServiceInfo {
public:
    ServiceInfo();
    ~ServiceInfo();
    const QString& name() const;
    const KUrl& url() const;
    const QString& details() const;

    ServiceInfo& operator=(const ServiceInfo&);

protected:
    friend class Service;
    QString _name;
    KUrl _url;
    QString _details;
    class ServiceInfoPrivate;
    ServiceInfoPrivate * const d;
};

class KIO_EXPORT Report : public QObject {
    friend class Agent;
    friend class Service;
    Q_OBJECT
public:
    Report();
    ~Report();

    enum Result { Working = 0, Unknown = 1, Error = 2, KnownPhishing = 3, Indeterminate = 4, KnownGood = 5, Inconsistent = 6, Aborted = 7 };

    Result result(const QString& service = QString()) const;
    QString extendedResultText(const QString& service = QString()) const;
    QString report(const QString& service = QString()) const;
    QMap<QString, QString> metaData(const QString& service = QString()) const;
    QStringList services() const;
    ServiceInfo serviceInfo(const QString& service);

    void abort();

    const KUrl& url() const;

protected Q_SLOTS:
    void serviceReported(Service *s);

private Q_SLOTS:
    void next();

Q_SIGNALS:
    void done();
    void serviceReported(const QString& service);

protected:
    bool run();
    KUrl _url;

private:
    class ReportPrivate;
    ReportPrivate * const d;
};

class KIO_EXPORT Agent {
public:
    static Report *obtainReport(const KUrl& url);
    static QStringList services();
    static ServiceInfo serviceInfo(const QString& service);
private:
    Agent();
    ~Agent();
};

}

#endif

