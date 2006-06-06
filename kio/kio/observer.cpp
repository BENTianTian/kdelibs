/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

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

#include <assert.h>

#include <kdebug.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kurl.h>

#include "jobclasses.h"
#include "observer.h"

#include "uiserveriface.h"

#include "passdlg.h"
#include "slavebase.h"
#include <kmessagebox.h>
#include <ksslinfodlg.h>
#include <ksslcertdlg.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>
#include <klocale.h>

using namespace KIO;

Observer * Observer::s_pObserver = 0L;

const int KDEBUG_OBSERVER = 7007; // Should be 7028

Observer::Observer()
{
    QDBus::sessionBus().registerObject("/KIO/Observer", this, QDBusConnection::ExportSlots);

    if ( !QDBus::sessionBus().busService()->nameHasOwner( "org.kde.kio.uiserver" ) )
    {
        kDebug(KDEBUG_OBSERVER) << "Starting kio_uiserver" << endl;
        QString error;
        int ret = KToolInvocation::startServiceByDesktopPath( "kio_uiserver.desktop",
                                                             QStringList(), &error );
        if ( ret > 0 )
        {
            kError() << "Couldn't start kio_uiserver from kio_uiserver.desktop: " << error << endl;
        } else
            kDebug(KDEBUG_OBSERVER) << "startServiceByDesktopPath returned " << ret << endl;

    }
    if ( !QDBus::sessionBus().busService()->nameHasOwner( "org.kde.kio.uiserver" ) )
        kDebug(KDEBUG_OBSERVER) << "The application kio_uiserver is STILL NOT REGISTERED" << endl;
    else
        kDebug(KDEBUG_OBSERVER) << "kio_uiserver registered" << endl;

#ifndef Q_OS_WIN
    m_uiserver = QDBus::sessionBus().findInterface<org::kde::KIO::UIServer>("org.kde.kio.uiserver",
                                                                            "/UIServer");
#endif
}

int Observer::newJob( KIO::Job * job, bool showProgress )
{
    // Tell the UI Server about this new job, and give it the application id
    // at the same time
    int progressId = m_uiserver->newJob( QDBus::sessionBus().baseService(), showProgress );

    // Keep the result in a dict
    m_dctJobs.insert( progressId, job );

    return progressId;
}

void Observer::jobFinished( int progressId )
{
    m_uiserver->jobFinished( progressId );
    m_dctJobs.remove( progressId );
}

void Observer::killJob( int progressId )
{
    KIO::Job * job = m_dctJobs.value( progressId );
    if (!job)
    {
        kWarning() << "Can't find job to kill ! There is no job with progressId=" << progressId << " in this process" << endl;
        return;
    }
    job->kill( false /* not quietly */ );
}

QVariantMap Observer::metadata( int progressId )
{
    KIO::Job * job = m_dctJobs.value( progressId );
    assert(job);
    MetaData data = job->metaData();
    QVariantMap map;
    for (MetaData::ConstIterator it = data.constBegin(); it != data.constEnd(); ++it)
        map.insert(it.key(), it.value());
    return map;
}

void Observer::slotTotalSize( KJob* job, qulonglong size )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalSize " << job << " " << KIO::number(size) << endl;
  m_uiserver->totalSize( job->progressId(), size );
}

void Observer::slotTotalFiles( KIO::Job* job, unsigned long files )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalFiles " << job << " " << files << endl;
  m_uiserver->totalFiles( job->progressId(), files );
}

void Observer::slotTotalDirs( KIO::Job* job, unsigned long dirs )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalDirs " << job << " " << dirs << endl;
  m_uiserver->totalDirs( job->progressId(), dirs );
}

void Observer::slotProcessedSize( KJob* job, qulonglong size )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedSize " << job << " " << job->progressId() << " " << KIO::number(size) << endl;
  m_uiserver->processedSize( job->progressId(), size );
}

void Observer::slotProcessedFiles( KIO::Job* job, unsigned long files )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedFiles " << job << " " << files << endl;
  m_uiserver->processedFiles( job->progressId(), files );
}

void Observer::slotProcessedDirs( KIO::Job* job, unsigned long dirs )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedDirs " << job << " " << dirs << endl;
  m_uiserver->processedDirs( job->progressId(), dirs );
}

void Observer::slotSpeed( KIO::Job* job, unsigned long speed )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotSpeed " << job << " " << speed << endl;
  m_uiserver->speed( job->progressId(), speed );
}

void Observer::slotPercent( KJob* job, unsigned long percent )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotPercent " << job << " " << percent << endl;
  m_uiserver->percent( job->progressId(), percent );
}

void Observer::slotInfoMessage( KJob* job, const QString & msg )
{
  m_uiserver->infoMessage( job->progressId(), msg );
}

void Observer::slotCopying( KIO::Job* job, const KUrl& from, const KUrl& to )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotCopying " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->copying( job->progressId(), from.url(), to.url() );
}

void Observer::slotMoving( KIO::Job* job, const KUrl& from, const KUrl& to )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotMoving " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->moving( job->progressId(), from.url(), to.url() );
}

void Observer::slotDeleting( KIO::Job* job, const KUrl& url )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotDeleting " << job << " " << url.url() << endl;
  m_uiserver->deleting( job->progressId(), url.url() );
}

void Observer::slotTransferring( KIO::Job* job, const KUrl& url )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotTransferring " << job << " " << url.url() << endl;
  m_uiserver->transferring( job->progressId(), url.url() );
}

void Observer::slotCreatingDir( KIO::Job* job, const KUrl& dir )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotCreatingDir " << job << " " << dir.url() << endl;
  m_uiserver->creatingDir( job->progressId(), dir.url() );
}

void Observer::slotCanResume( KIO::Job* job, KIO::filesize_t offset )
{
  //kDebug(KDEBUG_OBSERVER) << "** Observer::slotCanResume " << job << " " << KIO::number(offset) << endl;
  m_uiserver->canResume( job->progressId(), offset );
}

void Observer::stating( KIO::Job* job, const KUrl& url )
{
  m_uiserver->stating( job->progressId(), url.url() );
}

void Observer::mounting( KIO::Job* job, const QString & dev, const QString & point )
{
  m_uiserver->mounting( job->progressId(), dev, point );
}

void Observer::unmounting( KIO::Job* job, const QString & point )
{
  m_uiserver->unmounting( job->progressId(), point );
}

bool Observer::openPassDlg( const QString& prompt, QString& user,
			    QString& pass, bool readOnly )
{
   AuthInfo info;
   info.prompt = prompt;
   info.username = user;
   info.password = pass;
   info.readOnly = readOnly;
   bool result = openPassDlg ( info );
   if ( result )
   {
     user = info.username;
     pass = info.password;
   }
   return result;
}

bool Observer::openPassDlg( KIO::AuthInfo& info )
{
    kDebug(KDEBUG_OBSERVER) << "Observer::openPassDlg: User= " << info.username
                             << ", Message= " << info.prompt << endl;
    int result = KIO::PasswordDialog::getNameAndPassword( info.username, info.password,
                                                          &info.keepPassword, info.prompt,
                                                          info.readOnly, info.caption,
                                                          info.comment, info.commentLabel );
    if ( result == QDialog::Accepted )
    {
        info.setModified( true );
        return true;
    }
    return false;
}

int Observer::messageBox( int progressId, int type, const QString &text,
                          const QString &caption, const QString &buttonYes,
                          const QString &buttonNo )
{
    return messageBox( progressId, type, text, caption, buttonYes, buttonNo, QString() );
}

int Observer::messageBox( int progressId, int type, const QString &text,
                          const QString &caption, const QString &buttonYes,
                          const QString &buttonNo, const QString &dontAskAgainName )
{
    kDebug() << "Observer::messageBox " << type << " " << text << " - " << caption << endl;
    int result = -1;
    KConfig *config = new KConfig("kioslaverc");
    KMessageBox::setDontShowAskAgainConfig(config);

    switch (type) {
        case KIO::SlaveBase::QuestionYesNo:
            result = KMessageBox::questionYesNo( 0L, // parent ?
                                               text, caption, buttonYes, buttonNo, dontAskAgainName );
            break;
        case KIO::SlaveBase::WarningYesNo:
            result = KMessageBox::warningYesNo( 0L, // parent ?
                                              text, caption, buttonYes, buttonNo, dontAskAgainName );
            break;
        case KIO::SlaveBase::WarningContinueCancel:
            result = KMessageBox::warningContinueCancel( 0L, // parent ?
                                              text, caption, buttonYes, dontAskAgainName );
            break;
        case KIO::SlaveBase::WarningYesNoCancel:
            result = KMessageBox::warningYesNoCancel( 0L, // parent ?
                                              text, caption, buttonYes, buttonNo, dontAskAgainName );
            break;
        case KIO::SlaveBase::Information:
            KMessageBox::information( 0L, // parent ?
                                      text, caption, dontAskAgainName );
            result = 1; // whatever
            break;
        case KIO::SlaveBase::SSLMessageBox:
        {
#ifdef __GNUC__
# warning FIXME This will never work
#endif
            QString observerAppId = caption; // hack, see slaveinterface.cpp
            // Contact the object "KIO::Observer" in the application <appId>
            // Yes, this could be the same application we are, but not necessarily.
            QDBusInterfacePtr observer( observerAppId, "/KIO/Observer", "org.kde.KIO.Observer" );

            QDBusReply<QVariantMap> reply =
                observer->call(QDBusInterface::UseEventLoop, "metadata", progressId );
            const QVariantMap &meta = reply;
            KSSLInfoDlg *kid = new KSSLInfoDlg(meta["ssl_in_use"].toString().toUpper()=="TRUE", 0L /*parent?*/, 0L, true);
            KSSLCertificate *x = KSSLCertificate::fromString(meta["ssl_peer_certificate"].toString().toLocal8Bit());
            if (x) {
               // Set the chain back onto the certificate
               QStringList cl = meta["ssl_peer_chain"].toString().split('\n', QString::SkipEmptyParts);
               Q3PtrList<KSSLCertificate> ncl;

               ncl.setAutoDelete(true);
               for (QStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
                  KSSLCertificate *y = KSSLCertificate::fromString((*it).toLocal8Bit());
                  if (y) ncl.append(y);
               }

               if (ncl.count() > 0)
                  x->chain().setChain(ncl);

               kid->setup( x,
                           meta["ssl_peer_ip"].toString(),
                           text, // the URL
                           meta["ssl_cipher"].toString(),
                           meta["ssl_cipher_desc"].toString(),
                           meta["ssl_cipher_version"].toString(),
                           meta["ssl_cipher_used_bits"].toInt(),
                           meta["ssl_cipher_bits"].toInt(),
                           KSSLCertificate::KSSLValidation(meta["ssl_cert_state"].toInt()));
               kDebug(7024) << "Showing SSL Info dialog" << endl;
               kid->exec();
               delete x;
               kDebug(7024) << "SSL Info dialog closed" << endl;
            } else {
               KMessageBox::information( 0L, // parent ?
                                         i18n("The peer SSL certificate appears to be corrupt."), i18n("SSL") );
            }
            // This doesn't have to get deleted.  It deletes on it's own.
            result = 1; // whatever
            break;
        }
        default:
            kWarning() << "Observer::messageBox: unknown type " << type << endl;
            result = 0;
            break;
    }
    KMessageBox::setDontShowAskAgainConfig(0);
    delete config;
    return result;
#if 0
    QByteArray data, replyData;
    DCOPCString replyType;
    QDataStream arg( data, QIODevice::WriteOnly );
    arg << progressId;
    arg << type;
    arg << text;
    arg << caption;
    arg << buttonYes;
    arg << buttonNo;
    if ( KApplication::dcopClient()->call( "kio_uiserver", "UIServer", "messageBox(int,int,QString,QString,QString,QString)", data, replyType, replyData, true )
        && replyType == "int" )
    {
        int result;
        QDataStream _reply_stream( replyData, QIODevice::ReadOnly );
        _reply_stream >> result;
        kDebug(KDEBUG_OBSERVER) << "Observer::messageBox got result " << result << endl;
        return result;
    }
    kDebug(KDEBUG_OBSERVER) << "Observer::messageBox call failed" << endl;
    return 0;
#endif
}

RenameDlg_Result Observer::open_RenameDlg( KIO::Job* job,
                                           const QString & caption,
                                           const QString& src, const QString & dest,
                                           RenameDlg_Mode mode, QString& newDest,
                                           KIO::filesize_t sizeSrc,
                                           KIO::filesize_t sizeDest,
                                           time_t ctimeSrc,
                                           time_t ctimeDest,
                                           time_t mtimeSrc,
                                           time_t mtimeDest
                                           )
{
  kDebug(KDEBUG_OBSERVER) << "Observer::open_RenameDlg job=" << job << endl;
  if (job)
    kDebug(KDEBUG_OBSERVER) << "                        progressId=" << job->progressId() << endl;
  // Hide existing dialog box if any
  if (job && job->progressId())
    m_uiserver->setJobVisible( job->progressId(), false );
  // We now do it in process => KDE4: move this code out of Observer (back to job.cpp), so that
  // opening the rename dialog doesn't start uiserver for nothing if progressId=0 (e.g. F2 in konq)
  RenameDlg_Result res = KIO::open_RenameDlg( caption, src, dest, mode,
                                               newDest, sizeSrc, sizeDest,
                                               ctimeSrc, ctimeDest, mtimeSrc,
                                               mtimeDest );
  if (job && job->progressId())
    m_uiserver->setJobVisible( job->progressId(), true );
  return res;
}

SkipDlg_Result Observer::open_SkipDlg( KIO::Job* job,
                                       bool _multi,
                                       const QString& _error_text )
{
  // Hide existing dialog box if any
  if (job && job->progressId())
      m_uiserver->setJobVisible( job->progressId(), false );
  // We now do it in process. So this method is a useless wrapper around KIO::open_RenameDlg.
  SkipDlg_Result res = KIO::open_SkipDlg( _multi, _error_text );
  if (job && job->progressId())
      m_uiserver->setJobVisible( job->progressId(), true );
  return res;
}

void Observer::virtual_hook( int, void* )
{ /* BASe::virtual_hook( id, data ); */ }

#include "observer.moc"
