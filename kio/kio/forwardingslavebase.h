/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin@ipsquad.net>

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

#ifndef _FORWARDING_SLAVE_BASE_H_
#define _FORWARDING_SLAVE_BASE_H_

#include <kio/slavebase.h>
#include <kio/jobclasses.h>

#include <QtCore/QObject>
#include <QtCore/QEventLoop>

namespace KIO
{

class ForwardingSlaveBasePrivate;

/**
 * This class should be used as a base for ioslaves acting as a
 * forwarder to other ioslaves. It has been designed to support only
 * local filesystem like ioslaves.
 *
 * If the resulting ioslave should be a simple proxy, you only need
 * to implement the ForwardingSlaveBase::rewriteUrl() method.
 *
 * For more advanced behavior, the classic ioslave methods should
 * be reimplemented, because their default behavior in this class
 * is to forward using the ForwardingSlaveBase::rewriteUrl() method.
 *
 * A possible code snippet for an advanced stat() behavior would look
 * like this in the child class:
 *
 * \code
 *     void ChildProtocol::stat(const KUrl &url)
 *     {
 *         bool is_special = false;
 *
 *         // Process the URL to see if it should have
 *         // a special treatment
 *
 *         if ( is_special )
 *         {
 *             // Handle the URL ourselves
 *             KIO::UDSEntry entry;
 *             // Fill entry with UDSAtom instances
 *             statEntry(entry);
 *             finished();
 *         }
 *         else
 *         {
 *             // Setup the ioslave internal state if
 *             // required by ChildProtocol::rewriteUrl()
 *             ForwardingSlaveBase::stat(url);
 *         }
 *     }
 * \endcode
 *
 * Of course in this case, you surely need to reimplement listDir()
 * and get() accordingly.
 *
 * If you want view on directories to be correctly refreshed when
 * something changes on a forwarded URL, you'll need a companion kded
 * module to emit the KDirNotify Files*() D-Bus signals.
 *
 * This class was initially used for media:/ ioslave. This ioslave code
 * and the MediaDirNotify class of its companion kded module can be a
 * good source of inspiration.
 *
 * @see ForwardingSlaveBase::rewriteUrl()
 * @author Kevin Ottens <ervin@ipsquad.net>
 */
class KIO_EXPORT ForwardingSlaveBase : public QObject, public SlaveBase
{
Q_OBJECT
public:
    ForwardingSlaveBase(const QByteArray &protocol,
                        const QByteArray &poolSocket,
                        const QByteArray &appSocket);
    virtual ~ForwardingSlaveBase();

    virtual void get(const KUrl &url);

    virtual void put(const KUrl &url, int permissions,
                     bool overwrite, bool resume);

    virtual void stat(const KUrl &url);

    virtual void mimetype(const KUrl &url);

    virtual void listDir(const KUrl &url);

    virtual void mkdir(const KUrl &url, int permissions);

    virtual void rename(const KUrl &src, const KUrl &dest, bool overwrite);

    virtual void symlink(const QString &target, const KUrl &dest,
                         bool overwrite);

    virtual void chmod(const KUrl &url, int permissions);

    virtual void setModificationTime(const KUrl& url, const QDateTime& mtime);

    virtual void copy(const KUrl &src, const KUrl &dest,
                      int permissions, bool overwrite);

    virtual void del(const KUrl &url, bool isfile);

protected:
    /**
     * Rewrite an url to it's forwarded counterpart. It should return
     * true if everything was ok, and false otherwise.
     *
     * If a problem is detected it's up to this method to trigger error()
     * before returning. Returning false silently cancel the current
     * slave operation.
     *
     * @param url The URL as given during the slave call
     * @param newURL The new URL to forward the slave call to
     * @return true if the given url could be correctly rewritten
     */
    virtual bool rewriteUrl(const KUrl &url, KUrl &newURL)=0;

    /**
     * Allow to modify a UDSEntry before it's sent to the ioslave enpoint.
     * This is the default implementation working in most case, but sometimes
     * you could make use of more forwarding black magic (for example
     * dynamically transform any desktop file into a fake directory...)
     *
     * @param entry the UDSEntry to post-process
     * @param listing indicate if this entry it created during a listDir
     *                operation
     */
    virtual void prepareUDSEntry(KIO::UDSEntry &entry,
                                 bool listing=false) const;

    /**
     * Return the URL being processed by the ioslave
     * Only access it inside prepareUDSEntry()
     */
    KUrl processedUrl() const;

    /**
     * Return the URL asked to the ioslave
     * Only access it inside prepareUDSEntry()
     */
    KUrl requestedUrl() const;

private:
    bool internalRewriteUrl(const KUrl &url, KUrl &newURL);

    void connectJob(Job *job);
    void connectSimpleJob(SimpleJob *job);
    void connectListJob(ListJob *job);
    void connectTransferJob(TransferJob *job);

private Q_SLOTS:
    // KIO::Job
    void slotResult(KJob *job);
    void slotWarning(KJob *job, const QString &msg);
    void slotInfoMessage(KJob *job, const QString &msg);
    void slotTotalSize(KJob *job, qulonglong size);
    void slotProcessedSize(KJob *job, qulonglong size);
    void slotSpeed(KJob *job, unsigned long bytesPerSecond);

    // KIO::SimpleJob subclasses
    void slotRedirection(KIO::Job *job, const KUrl &url);

    // KIO::ListJob
    void slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries);

    // KIO::TransferJob
    void slotData(KIO::Job *job, const QByteArray &data);
    void slotDataReq(KIO::Job *job, QByteArray &data);
    void slotMimetype (KIO::Job *job, const QString &type);
    void slotCanResume (KIO::Job *job, KIO::filesize_t offset);

private:
    ForwardingSlaveBasePrivate *const d;
};

}

#endif
