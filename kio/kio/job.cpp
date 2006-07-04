/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
                       Waldo Bastian <bastian@kde.org>

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kio/job.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <assert.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
extern "C" {
#include <pwd.h>
#include <grp.h>
}
#include <qtimer.h>
#include <qfile.h>

#include <kapplication.h>
#include <kauthorized.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kde_file.h>

#include <errno.h>

#include "jobuidelegate.h"
#include "kmimetype.h"
#include "slave.h"
#include "scheduler.h"
#include "kdirwatch.h"
#include "kmimemagic.h"
#include "kprotocolinfo.h"
#include "kprotocolmanager.h"

#include "kio/observer.h"

#include "kssl/ksslcsessioncache.h"

#include <kdirnotify.h>
#include <ktempfile.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif

using namespace KIO;

//this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
#define REPORT_TIMEOUT 200

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( &packedArgs, QIODevice::WriteOnly ); stream

class Job::JobPrivate
{
public:
    JobPrivate() : m_interactive( true ), m_parentJob( 0L ), m_extraFlags(0)
                   {}

    bool m_interactive;
    // Maybe we could use the QObject parent/child mechanism instead
    // (requires a new ctor, and moving the ctor code to some init()).
    Job* m_parentJob;
    int m_extraFlags;
};

Job::Job(bool showProgressInfo) : KJob(0), m_speedTimer(0), d( new JobPrivate )
{
    setUiDelegate( new JobUiDelegate( showProgressInfo ) );
}

Job::~Job()
{
    delete m_speedTimer;
    delete d;
}

JobUiDelegate *Job::ui() const
{
    return static_cast<JobUiDelegate*>( uiDelegate() );
}

int& Job::extraFlags()
{
    return d->m_extraFlags;
}

void Job::addSubjob(Job *job, bool inheritMetaData)
{
    //kDebug(7007) << "addSubjob(" << job << ") this = " << this << endl;
    m_subjobs.append(job);

    connect( job, SIGNAL(result(KJob*)),
             SLOT(slotResult(KJob*)) );

    // Forward information from that subjob.
    connect( job, SIGNAL(speed( KIO::Job*, unsigned long )),
             SLOT(slotSpeed(KIO::Job*, unsigned long)) );

    connect( job, SIGNAL(infoMessage( KJob*, const QString &, const QString & )),
             SLOT(slotInfoMessage(KJob*, const QString &)) );

    if (inheritMetaData)
       job->mergeMetaData(m_outgoingMetaData);

    job->ui()->setWindow( m_window );
}

void Job::removeSubjob( KJob *jobBase, bool mergeMetaData )
{
    KIO::Job *job = dynamic_cast<KIO::Job*>( jobBase );

    if ( job == 0 )
    {
        return;
    }

    //kDebug(7007) << "removeSubjob(" << job << ") this = " << this << "  subjobs = " << m_subjobs.count() << endl;
    // Merge metadata from subjob
    if ( mergeMetaData )
        m_incomingMetaData += job->metaData();
    m_subjobs.removeAll(job);
}

void Job::emitSpeed( unsigned long bytes_per_second )
{
  //kDebug(7007) << "Job " << this << " emitSpeed " << bytes_per_second << endl;
  if ( !m_speedTimer )
  {
    m_speedTimer = new QTimer(this);
    connect( m_speedTimer, SIGNAL( timeout() ), SLOT( slotSpeedTimeout() ) );
  }
  emit speed( this, bytes_per_second );
  m_speedTimer->start( 5000 );   // 5 seconds interval should be enough
}

bool Job::doKill()
{
  kDebug(7007) << "Job::kill this=" << this << " " << metaObject()->className() << " progressId()=" << progressId() << endl;
  // kill all subjobs, without triggering their result slot
  QList<Job *>::const_iterator it = m_subjobs.begin();
  const QList<Job *>::const_iterator end = m_subjobs.end();
  for ( ; it != end ; ++it )
    (*it)->kill( KJob::Quietly );
  m_subjobs.clear();

  return true;
}

void Job::slotResult( KJob *job )
{
    // Did job have an error ?
    if ( job->error() && !error() )
    {
        // Store it in the parent only if first error
        setError( job->error() );
        setErrorText( job->errorText() );
    }
    removeSubjob(job);
    if ( m_subjobs.isEmpty() )
        emitResult();
}

void Job::slotSpeed( KIO::Job*, unsigned long speed )
{
  //kDebug(7007) << "Job::slotSpeed " << speed << endl;
  emitSpeed( speed );
}

void Job::slotInfoMessage( KJob*, const QString & msg )
{
  emit infoMessage( this, msg );
}

void Job::slotSpeedTimeout()
{
  //kDebug(7007) << "slotSpeedTimeout()" << endl;
  // send 0 and stop the timer
  // timer will be restarted only when we receive another speed event
  emit speed( this, 0 );
  m_speedTimer->stop();
}

//Job::errorString is implemented in global.cpp

bool Job::isInteractive() const
{
  return uiDelegate() != 0;
}

void Job::setParentJob(Job* job)
{
  Q_ASSERT(d->m_parentJob == 0L);
  Q_ASSERT(job);
  d->m_parentJob = job;
}

Job* Job::parentJob() const
{
  return d->m_parentJob;
}

MetaData Job::metaData() const
{
    return m_incomingMetaData;
}

QString Job::queryMetaData(const QString &key)
{
    if (!m_incomingMetaData.contains(key))
       return QString();
    return m_incomingMetaData[key];
}

void Job::setMetaData( const KIO::MetaData &_metaData)
{
    m_outgoingMetaData = _metaData;
}

void Job::addMetaData( const QString &key, const QString &value)
{
    m_outgoingMetaData.insert(key, value);
}

void Job::addMetaData( const QMap<QString,QString> &values)
{
    QMap<QString,QString>::const_iterator it = values.begin();
    for(;it != values.end(); ++it)
      m_outgoingMetaData.insert(it.key(), it.value());
}

void Job::mergeMetaData( const QMap<QString,QString> &values)
{
    QMap<QString,QString>::const_iterator it = values.begin();
    for(;it != values.end(); ++it)
        // there's probably a faster way
        if ( !m_outgoingMetaData.contains( it.key() ) )
            m_outgoingMetaData.insert( it.key(), it.value() );
}

MetaData Job::outgoingMetaData() const
{
    return m_outgoingMetaData;
}


SimpleJob::SimpleJob(const KUrl& url, int command, const QByteArray &packedArgs,
                     bool showProgressInfo )
  : Job(showProgressInfo), m_slave(0), m_packedArgs(packedArgs),
    m_url(url), m_command(command), m_totalSize(0)
{
    if (!m_url.isValid())
    {
        setError( ERR_MALFORMED_URL );
        setErrorText( m_url.url() );
        QTimer::singleShot(0, this, SLOT(slotFinished()) );
        return;
    }


    if (m_url.hasSubUrl())
    {
       KUrl::List list = KUrl::split(m_url);
       list.removeLast();
       m_subUrl = KUrl::join(list);
       //kDebug(7007) << "New URL = "  << m_url.url() << endl;
       //kDebug(7007) << "Sub URL = "  << m_subUrl.url() << endl;
    }

    Scheduler::doJob(this);
}

void SimpleJob::start()
{

}

bool SimpleJob::doKill()
{
    Scheduler::cancelJob( this ); // deletes the slave if not 0
    m_slave = 0; // -> set to 0
    return true;
}

void SimpleJob::putOnHold()
{
    Q_ASSERT( m_slave );
    if ( m_slave )
    {
        Scheduler::putSlaveOnHold(this, m_url);
        m_slave = 0;
    }
    kill( Quietly );
}

void SimpleJob::removeOnHold()
{
    Scheduler::removeSlaveOnHold();
}

SimpleJob::~SimpleJob()
{
    if (m_slave) // was running
    {
        kDebug(7007) << "SimpleJob::~SimpleJob: Killing running job in destructor!"  << endl;
#if 0
        m_slave->kill();
        Scheduler::jobFinished( this, m_slave ); // deletes the slave
#endif
        Scheduler::cancelJob( this );
        m_slave = 0; // -> set to 0
    }
}

void SimpleJob::start(Slave *slave)
{
    m_slave = slave;

    connect( m_slave, SIGNAL( error( int , const QString & ) ),
             SLOT( slotError( int , const QString & ) ) );

    connect( m_slave, SIGNAL( warning( const QString & ) ),
             SLOT( slotWarning( const QString & ) ) );

    connect( m_slave, SIGNAL( infoMessage( const QString & ) ),
             SLOT( slotInfoMessage( const QString & ) ) );

    connect( m_slave, SIGNAL( connected() ),
             SLOT( slotConnected() ) );

    connect( m_slave, SIGNAL( finished() ),
             SLOT( slotFinished() ) );

    if ((extraFlags() & EF_TransferJobDataSent) == 0)
    {
        connect( m_slave, SIGNAL( totalSize( KIO::filesize_t ) ),
                 SLOT( slotTotalSize( KIO::filesize_t ) ) );

        connect( m_slave, SIGNAL( processedSize( KIO::filesize_t ) ),
                 SLOT( slotProcessedSize( KIO::filesize_t ) ) );

        connect( m_slave, SIGNAL( speed( unsigned long ) ),
                 SLOT( slotSpeed( unsigned long ) ) );
    }

    connect( slave, SIGNAL( needProgressId() ),
             SLOT( slotNeedProgressId() ) );

    connect( slave, SIGNAL(metaData( const KIO::MetaData& ) ),
             SLOT( slotMetaData( const KIO::MetaData& ) ) );

    if (ui()->window())
    {
       QString id;
       addMetaData("window-id", id.setNum((ulong)ui()->window()->winId()));
    }

    QString sslSession = KSSLCSessionCache::getSessionForURL(m_url);
    if ( !sslSession.isNull() )
    {
        addMetaData("ssl_session_id", sslSession);
    }

    if (!isInteractive())
    {
        addMetaData("no-auth-prompt", "true");
    }

    if (!outgoingMetaData().isEmpty())
    {
       KIO_ARGS << outgoingMetaData();
       slave->send( CMD_META_DATA, packedArgs );
    }

    if (!m_subUrl.isEmpty())
    {
       KIO_ARGS << m_subUrl;
       m_slave->send( CMD_SUBURL, packedArgs );
    }

    m_slave->send( m_command, m_packedArgs );
}

void SimpleJob::slaveDone()
{
   if (!m_slave) return;
   disconnect(m_slave); // Remove all signals between slave and job
   Scheduler::jobFinished( this, m_slave );
   m_slave = 0;
}

void SimpleJob::slotFinished( )
{
    // Return slave to the scheduler
    slaveDone();

    if (!hasSubjobs())
    {
        if ( !error() && (m_command == CMD_MKDIR || m_command == CMD_RENAME ) )
        {
            if ( m_command == CMD_MKDIR )
            {
                KUrl urlDir( url() );
                urlDir.setPath( urlDir.directory() );
                org::kde::KDirNotify::emitFilesAdded( urlDir.url() );
            }
            else /*if ( m_command == CMD_RENAME )*/
            {
                KUrl src, dst;
                QDataStream str( m_packedArgs );
                str >> src >> dst;
                if ( src.directory() == dst.directory() ) // For the user, moving isn't renaming. Only renaming is.
                    org::kde::KDirNotify::emitFileRenamed( src.url(), dst.url() );
            }
        }
        emitResult();
    }
}

void SimpleJob::slotError( int err, const QString & errorText )
{
    setError( err );
    setErrorText( errorText );
    if ((error() == ERR_UNKNOWN_HOST) && m_url.host().isEmpty())
       setErrorText( QString() );
    // error terminates the job
    slotFinished();
}

void SimpleJob::slotWarning( const QString & errorText )
{
    emit warning( this, errorText );
}

void SimpleJob::slotInfoMessage( const QString & msg )
{
    emit infoMessage( this, msg );
}

void SimpleJob::slotConnected()
{
    emit connected( this );
}

void SimpleJob::slotNeedProgressId()
{
    if ( !progressId() )
        setProgressId( Observer::self()->newJob( this, false ) );
    m_slave->setProgressId( progressId() );
}

void SimpleJob::slotTotalSize( KIO::filesize_t size )
{
    if (size > m_totalSize)
    {
        m_totalSize = size;
        emit totalSize( this, size );
    }
}

void SimpleJob::slotProcessedSize( KIO::filesize_t size )
{
    //kDebug(7007) << "SimpleJob::slotProcessedSize " << KIO::number(size) << endl;
    setProcessedSize(size);
    emit processedSize( this, size );
    if ( size > m_totalSize ) {
        slotTotalSize(size); // safety
    }
    emitPercent( size, m_totalSize );
}

void SimpleJob::slotSpeed( unsigned long speed )
{
    //kDebug(7007) << "SimpleJob::slotSpeed( " << speed << " )" << endl;
    emitSpeed( speed );
}

void SimpleJob::slotMetaData( const KIO::MetaData &_metaData)
{
    m_incomingMetaData += _metaData;
}

void SimpleJob::storeSSLSessionFromJob(const KUrl &m_redirectionURL) {
    QString sslSession = queryMetaData("ssl_session_id");

    if ( !sslSession.isNull() ) {
	    const KUrl &queryURL = m_redirectionURL.isEmpty()?m_url:m_redirectionURL;
	    KSSLCSessionCache::putSessionForURL(queryURL, sslSession);
    }
}

//////////
MkdirJob::MkdirJob( const KUrl& url, int command,
                    const QByteArray &packedArgs, bool showProgressInfo )
    : SimpleJob(url, command, packedArgs, showProgressInfo)
{
}

void MkdirJob::start(Slave *slave)
{
    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    SimpleJob::start(slave);
}

// Slave got a redirection request
void MkdirJob::slotRedirection( const KUrl &url)
{
     kDebug(7007) << "MkdirJob::slotRedirection(" << url << ")" << endl;
     if (!KAuthorized::authorizeURLAction("redirect", m_url, url))
     {
       kWarning(7007) << "MkdirJob: Redirection from " << m_url << " to " << url << " REJECTED!" << endl;
       setError( ERR_ACCESS_DENIED );
       setErrorText( url.prettyUrl() );
       return;
     }
     m_redirectionURL = url; // We'll remember that when the job finishes
     if (m_url.hasUser() && !url.hasUser() && (m_url.host().toLower() == url.host().toLower()))
        m_redirectionURL.setUser(m_url.user()); // Preserve user
     // Tell the user that we haven't finished yet
     emit redirection(this, m_redirectionURL);
}

void MkdirJob::slotFinished()
{
    if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid())
    {
        // Return slave to the scheduler
        SimpleJob::slotFinished();
    } else {
        //kDebug(7007) << "MkdirJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        KUrl dummyUrl;
        int permissions;
        QDataStream istream( m_packedArgs );
        istream >> dummyUrl >> permissions;

        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        m_packedArgs.truncate(0);
        QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
        stream << m_url << permissions;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

SimpleJob *KIO::mkdir( const KUrl& url, int permissions )
{
    //kDebug(7007) << "mkdir " << url << endl;
    KIO_ARGS << url << permissions;
    return new MkdirJob(url, CMD_MKDIR, packedArgs, false);
}

SimpleJob *KIO::rmdir( const KUrl& url )
{
    //kDebug(7007) << "rmdir " << url << endl;
    KIO_ARGS << url << qint8(false); // isFile is false
    return new SimpleJob(url, CMD_DEL, packedArgs, false);
}

SimpleJob *KIO::chmod( const KUrl& url, int permissions )
{
    //kDebug(7007) << "chmod " << url << endl;
    KIO_ARGS << url << permissions;
    return new SimpleJob(url, CMD_CHMOD, packedArgs, false);
}

SimpleJob *KIO::rename( const KUrl& src, const KUrl & dest, bool overwrite )
{
    //kDebug(7007) << "rename " << src << " " << dest << endl;
    KIO_ARGS << src << dest << (qint8) overwrite;
    return new SimpleJob(src, CMD_RENAME, packedArgs, false);
}

SimpleJob *KIO::symlink( const QString& target, const KUrl & dest, bool overwrite, bool showProgressInfo )
{
    //kDebug(7007) << "symlink target=" << target << " " << dest << endl;
    KIO_ARGS << target << dest << (qint8) overwrite;
    return new SimpleJob(dest, CMD_SYMLINK, packedArgs, showProgressInfo);
}

SimpleJob *KIO::special(const KUrl& url, const QByteArray & data, bool showProgressInfo)
{
    //kDebug(7007) << "special " << url << endl;
    return new SimpleJob(url, CMD_SPECIAL, data, showProgressInfo);
}

SimpleJob *KIO::mount( bool ro, const QByteArray& fstype, const QString& dev, const QString& point, bool showProgressInfo )
{
    KIO_ARGS << int(1) << qint8( ro ? 1 : 0 )
             << QString::fromLatin1(fstype) << dev << point;
    SimpleJob *job = special( KUrl("file:/"), packedArgs, showProgressInfo );
    if ( showProgressInfo )
         Observer::self()->mounting( job, dev, point );
    return job;
}

SimpleJob *KIO::unmount( const QString& point, bool showProgressInfo )
{
    KIO_ARGS << int(2) << point;
    SimpleJob *job = special( KUrl("file:/"), packedArgs, showProgressInfo );
    if ( showProgressInfo )
         Observer::self()->unmounting( job, point );
    return job;
}



//////////

StatJob::StatJob( const KUrl& url, int command,
                  const QByteArray &packedArgs, bool showProgressInfo )
    : SimpleJob(url, command, packedArgs, showProgressInfo),
    m_bSource(true), m_details(2)
{
}

void StatJob::start(Slave *slave)
{
    m_outgoingMetaData.insert( "statSide", m_bSource ? "source" : "dest" );
    m_outgoingMetaData.insert( "details", QString::number(m_details) );

    connect( slave, SIGNAL( statEntry( const KIO::UDSEntry& ) ),
             SLOT( slotStatEntry( const KIO::UDSEntry & ) ) );
    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    SimpleJob::start(slave);
}

void StatJob::slotStatEntry( const KIO::UDSEntry & entry )
{
    //kDebug(7007) << "StatJob::slotStatEntry" << endl;
    m_statResult = entry;
}

// Slave got a redirection request
void StatJob::slotRedirection( const KUrl &url)
{
     kDebug(7007) << "StatJob::slotRedirection(" << url << ")" << endl;
     if (!KAuthorized::authorizeURLAction("redirect", m_url, url))
     {
       kWarning(7007) << "StatJob: Redirection from " << m_url << " to " << url << " REJECTED!" << endl;
       setError( ERR_ACCESS_DENIED );
       setErrorText( url.prettyUrl() );
       return;
     }
     m_redirectionURL = url; // We'll remember that when the job finishes
     if (m_url.hasUser() && !url.hasUser() && (m_url.host().toLower() == url.host().toLower()))
        m_redirectionURL.setUser(m_url.user()); // Preserve user
     // Tell the user that we haven't finished yet
     emit redirection(this, m_redirectionURL);
}

void StatJob::slotFinished()
{
    if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid())
    {
        // Return slave to the scheduler
        SimpleJob::slotFinished();
    } else {
        //kDebug(7007) << "StatJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        m_packedArgs.truncate(0);
        QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

void StatJob::slotMetaData( const KIO::MetaData &_metaData) {
    SimpleJob::slotMetaData(_metaData);
    storeSSLSessionFromJob(m_redirectionURL);
}

StatJob *KIO::stat(const KUrl& url, bool showProgressInfo)
{
    // Assume sideIsSource. Gets are more common than puts.
    return stat( url, true, 2, showProgressInfo );
}

StatJob *KIO::stat(const KUrl& url, bool sideIsSource, short int details, bool showProgressInfo)
{
    kDebug(7007) << "stat " << url << endl;
    KIO_ARGS << url;
    StatJob * job = new StatJob(url, CMD_STAT, packedArgs, showProgressInfo );
    job->setSide( sideIsSource );
    job->setDetails( details );
    if ( showProgressInfo )
      Observer::self()->stating( job, url );
    return job;
}

SimpleJob *KIO::http_update_cache( const KUrl& url, bool no_cache, time_t expireDate)
{
    assert( (url.protocol() == "http") || (url.protocol() == "https") );
    // Send http update_cache command (2)
    KIO_ARGS << (int)2 << url << no_cache << qlonglong(expireDate);
    SimpleJob * job = new SimpleJob( url, CMD_SPECIAL, packedArgs, false );
    Scheduler::scheduleJob(job);
    return job;
}

//////////

TransferJob::TransferJob( const KUrl& url, int command,
                          const QByteArray &packedArgs,
                          const QByteArray &_staticData,
                          bool showProgressInfo)
    : SimpleJob(url, command, packedArgs, showProgressInfo), staticData( _staticData)
{
    m_suspended = false;
    m_errorPage = false;
    m_subJob = 0L;
    if ( showProgressInfo )
        Observer::self()->slotTransferring( this, url );
}

// Slave sends data
void TransferJob::slotData( const QByteArray &_data)
{
    if(m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error())
      emit data( this, _data);
}

// Slave got a redirection request
void TransferJob::slotRedirection( const KUrl &url)
{
     kDebug(7007) << "TransferJob::slotRedirection(" << url << ")" << endl;
     if (!KAuthorized::authorizeURLAction("redirect", m_url, url))
     {
       kWarning(7007) << "TransferJob: Redirection from " << m_url << " to " << url << " REJECTED!" << endl;
       return;
     }

    // Some websites keep redirecting to themselves where each redirection
    // acts as the stage in a state-machine. We define "endless redirections"
    // as 5 redirections to the same URL.
    if (m_redirectionList.count(url) > 5)
    {
       kDebug(7007) << "TransferJob::slotRedirection: CYCLIC REDIRECTION!" << endl;
       setError( ERR_CYCLIC_LINK );
       setErrorText( m_url.prettyUrl() );
    }
    else
    {
       m_redirectionURL = url; // We'll remember that when the job finishes
       if (m_url.hasUser() && !url.hasUser() && (m_url.host().toLower() == url.host().toLower()))
          m_redirectionURL.setUser(m_url.user()); // Preserve user
       m_redirectionList.append(url);
       m_outgoingMetaData["ssl_was_in_use"] = m_incomingMetaData["ssl_in_use"];
       // Tell the user that we haven't finished yet
       emit redirection(this, m_redirectionURL);
    }
}

void TransferJob::slotFinished()
{
   //kDebug(7007) << "TransferJob::slotFinished(" << this << ", " << m_url << ")" << endl;
    if (m_redirectionURL.isEmpty() || !m_redirectionURL.isValid())
        SimpleJob::slotFinished();
    else {
        //kDebug(7007) << "TransferJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        // Honour the redirection
        // We take the approach of "redirecting this same job"
        // Another solution would be to create a subjob, but the same problem
        // happens (unpacking+repacking)
        staticData.truncate(0);
        m_incomingMetaData.clear();
        if (queryMetaData("cache") != "reload")
            addMetaData("cache","refresh");
        m_suspended = false;
        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        // The very tricky part is the packed arguments business
        QString dummyStr;
        KUrl dummyUrl;
        QDataStream istream( m_packedArgs );
        switch( m_command ) {
            case CMD_GET: {
                m_packedArgs.truncate(0);
                QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
                stream << m_url;
                break;
            }
            case CMD_PUT: {
                int permissions;
                qint8 iOverwrite, iResume;
                istream >> dummyUrl >> iOverwrite >> iResume >> permissions;
                m_packedArgs.truncate(0);
                QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
                stream << m_url << iOverwrite << iResume << permissions;
                break;
            }
            case CMD_SPECIAL: {
                int specialcmd;
                istream >> specialcmd;
                if (specialcmd == 1) // HTTP POST
                {
                   addMetaData("cache","reload");
                   m_packedArgs.truncate(0);
                   QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
                   stream << m_url;
                   m_command = CMD_GET;
                }
                break;
            }
        }

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

void TransferJob::setAsyncDataEnabled(bool enabled)
{
    if (enabled)
       extraFlags() |= EF_TransferJobAsync;
    else
       extraFlags() &= ~EF_TransferJobAsync;
}

void TransferJob::sendAsyncData(const QByteArray &dataForSlave)
{
    if (extraFlags() & EF_TransferJobNeedData)
    {
       m_slave->send( MSG_DATA, dataForSlave );
       if (extraFlags() & EF_TransferJobDataSent)
       {
           KIO::filesize_t size = processedSize()+dataForSlave.size();
           setProcessedSize(size);
           emit processedSize( this, size );
           if ( size > m_totalSize ) {
               slotTotalSize(size); // safety
           }
           emitPercent( size, m_totalSize );
       }
    }

    extraFlags() &= ~EF_TransferJobNeedData;
}

void TransferJob::setReportDataSent(bool enabled)
{
    if (enabled)
       extraFlags() |= EF_TransferJobDataSent;
    else
       extraFlags() &= ~EF_TransferJobDataSent;
}

bool TransferJob::reportDataSent()
{
    return (extraFlags() & EF_TransferJobDataSent);
}


// Slave requests data
void TransferJob::slotDataReq()
{
    QByteArray dataForSlave;

    extraFlags() |= EF_TransferJobNeedData;

    if (!staticData.isEmpty())
    {
       dataForSlave = staticData;
       staticData = QByteArray();
    }
    else
    {
       emit dataReq( this, dataForSlave);

       if (extraFlags() & EF_TransferJobAsync)
          return;
    }

    static const int max_size = 14 * 1024 * 1024;
    if (dataForSlave.size() > max_size)
    {
       kDebug(7007) << "send " << dataForSlave.size() / 1024 / 1024 << "MB of data in TransferJob::dataReq. This needs to be splitted, which requires a copy. Fix the application.\n";
       staticData = QByteArray(dataForSlave.data() + max_size ,  dataForSlave.size() - max_size);
       dataForSlave.truncate(max_size);
    }

    sendAsyncData(dataForSlave);

    if (m_subJob)
    {
       // Bitburger protocol in action
       suspend(); // Wait for more data from subJob.
       m_subJob->resume(); // Ask for more!
    }
}

void TransferJob::slotMimetype( const QString& type )
{
    m_mimetype = type;
    emit mimetype( this, m_mimetype);
}


void TransferJob::suspend()
{
    m_suspended = true;
    if (m_slave)
       m_slave->suspend();
}

void TransferJob::resume()
{
    m_suspended = false;
    if (m_slave)
       m_slave->resume();
}

void TransferJob::start(Slave *slave)
{
    assert(slave);
    connect( slave, SIGNAL( data( const QByteArray & ) ),
             SLOT( slotData( const QByteArray & ) ) );

    connect( slave, SIGNAL( dataReq() ),
             SLOT( slotDataReq() ) );

    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    connect( slave, SIGNAL(mimeType( const QString& ) ),
             SLOT( slotMimetype( const QString& ) ) );

    connect( slave, SIGNAL(errorPage() ),
             SLOT( slotErrorPage() ) );

    connect( slave, SIGNAL( needSubURLData() ),
             SLOT( slotNeedSubURLData() ) );

    connect( slave, SIGNAL(canResume( KIO::filesize_t ) ),
             SLOT( slotCanResume( KIO::filesize_t ) ) );

    if (slave->suspended())
    {
       m_mimetype = "unknown";
       // WABA: The slave was put on hold. Resume operation.
       slave->resume();
    }

    SimpleJob::start(slave);
    if (m_suspended)
       slave->suspend();
}

void TransferJob::slotNeedSubURLData()
{
    // Job needs data from subURL.
    m_subJob = KIO::get( m_subUrl, false, false);
    suspend(); // Put job on hold until we have some data.
    connect(m_subJob, SIGNAL( data(KIO::Job*,const QByteArray &)),
            SLOT( slotSubURLData(KIO::Job*,const QByteArray &)));
    addSubjob(m_subJob);
}

void TransferJob::slotSubURLData(KIO::Job*, const QByteArray &data)
{
    // The Alternating Bitburg protocol in action again.
    staticData = data;
    m_subJob->suspend(); // Put job on hold until we have delivered the data.
    resume(); // Activate ourselves again.
}

void TransferJob::slotMetaData( const KIO::MetaData &_metaData) {
    SimpleJob::slotMetaData(_metaData);
    storeSSLSessionFromJob(m_redirectionURL);
}

void TransferJob::slotErrorPage()
{
    m_errorPage = true;
}

void TransferJob::slotCanResume( KIO::filesize_t offset )
{
    emit canResume(this, offset);
}

void TransferJob::slotResult( KJob *job)
{
   // This can only be our suburl.
   assert(job == m_subJob);
   // Did job have an error ?
   if ( job->error() )
   {
      setError( job->error() );
      setErrorText( job->errorText() );

      emitResult();
      return;
   }

   if (job == m_subJob)
   {
      m_subJob = 0; // No action required
      resume(); // Make sure we get the remaining data.
   }
   removeSubjob(job);
}

TransferJob *KIO::get( const KUrl& url, bool reload, bool showProgressInfo )
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    TransferJob * job = new TransferJob( url, CMD_GET, packedArgs, QByteArray(), showProgressInfo );
    if (reload)
       job->addMetaData("cache", "reload");
    return job;
}

class PostErrorJob : public TransferJob
{
public:

  PostErrorJob(int _error, const QString& url, const QByteArray &packedArgs, const QByteArray &postData, bool showProgressInfo)
      : TransferJob(KUrl(), CMD_SPECIAL, packedArgs, postData, showProgressInfo)
  {
    setError( _error );
    setErrorText( url );
  }

};

TransferJob *KIO::http_post( const KUrl& url, const QByteArray &postData, bool showProgressInfo )
{
    int _error = 0;

    // filter out some malicious ports
    static const int bad_ports[] = {
        1,   // tcpmux
        7,   // echo
        9,   // discard
        11,   // systat
        13,   // daytime
        15,   // netstat
        17,   // qotd
        19,   // chargen
        20,   // ftp-data
        21,   // ftp-cntl
        22,   // ssh
        23,   // telnet
        25,   // smtp
        37,   // time
        42,   // name
        43,   // nicname
        53,   // domain
        77,   // priv-rjs
        79,   // finger
        87,   // ttylink
        95,   // supdup
        101,  // hostriame
        102,  // iso-tsap
        103,  // gppitnp
        104,  // acr-nema
        109,  // pop2
        110,  // pop3
        111,  // sunrpc
        113,  // auth
        115,  // sftp
        117,  // uucp-path
        119,  // nntp
        123,  // NTP
        135,  // loc-srv / epmap
        139,  // netbios
        143,  // imap2
        179,  // BGP
        389,  // ldap
        512,  // print / exec
        513,  // login
        514,  // shell
        515,  // printer
        526,  // tempo
        530,  // courier
        531,  // Chat
        532,  // netnews
        540,  // uucp
        556,  // remotefs
        587,  // sendmail
        601,  //
        989,  // ftps data
        990,  // ftps
        992,  // telnets
        993,  // imap/SSL
        995,  // pop3/SSL
        1080, // SOCKS
        2049, // nfs
        4045, // lockd
        6000, // x11
        6667, // irc
        0};
    for (int cnt=0; bad_ports[cnt]; ++cnt)
        if (url.port() == bad_ports[cnt])
        {
            _error = KIO::ERR_POST_DENIED;
            break;
        }

    if( _error )
    {
	static bool override_loaded = false;
	static QList< int >* overriden_ports = NULL;
	if( !override_loaded )
	{
	    KConfig cfg( "kio_httprc", true );
	    overriden_ports = new QList< int >;
	    *overriden_ports = cfg.readEntry( "OverriddenPorts", QList<int>() );
	    override_loaded = true;
	}
	for( QList< int >::ConstIterator it = overriden_ports->begin();
	     it != overriden_ports->end();
	     ++it )
	    if( overriden_ports->contains( url.port()))
		_error = 0;
    }

    // filter out non https? protocols
    if ((url.protocol() != "http") && (url.protocol() != "https" ))
        _error = KIO::ERR_POST_DENIED;

    bool redirection = false;
    KUrl _url(url);
    if (_url.path().isEmpty())
    {
      redirection = true;
      _url.setPath("/");
    }

    if (!_error && !KAuthorized::authorizeURLAction("open", KUrl(), _url))
        _error = KIO::ERR_ACCESS_DENIED;

    // if request is not valid, return an invalid transfer job
    if (_error)
    {
        KIO_ARGS << (int)1 << url;
        TransferJob * job = new PostErrorJob(_error, url.prettyUrl(), packedArgs, postData, showProgressInfo);
        return job;
    }

    // Send http post command (1), decoded path and encoded query
    KIO_ARGS << (int)1 << _url;
    TransferJob * job = new TransferJob( _url, CMD_SPECIAL,
                                         packedArgs, postData, showProgressInfo );

    if (redirection)
      QTimer::singleShot(0, job, SLOT(slotPostRedirection()) );

    return job;
}

// http post got redirected from http://host to http://host/ by TransferJob
// We must do this redirection ourselves because redirections by the
// slave change post jobs into get jobs.
void TransferJob::slotPostRedirection()
{
    kDebug(7007) << "TransferJob::slotPostRedirection(" << m_url << ")" << endl;
    // Tell the user about the new url.
    emit redirection(this, m_url);
}


TransferJob *KIO::put( const KUrl& url, int permissions,
                  bool overwrite, bool resume, bool showProgressInfo )
{
    KIO_ARGS << url << qint8( overwrite ? 1 : 0 ) << qint8( resume ? 1 : 0 ) << permissions;
    TransferJob * job = new TransferJob( url, CMD_PUT, packedArgs, QByteArray(), showProgressInfo );
    return job;
}

//////////

StoredTransferJob::StoredTransferJob(const KUrl& url, int command,
                                     const QByteArray &packedArgs,
                                     const QByteArray &_staticData,
                                     bool showProgressInfo)
    : TransferJob( url, command, packedArgs, _staticData, showProgressInfo ),
      m_uploadOffset( 0 )
{
    connect( this, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             SLOT( slotStoredData( KIO::Job *, const QByteArray & ) ) );
    connect( this, SIGNAL( dataReq( KIO::Job *, QByteArray & ) ),
             SLOT( slotStoredDataReq( KIO::Job *, QByteArray & ) ) );
}

void StoredTransferJob::setData( const QByteArray& arr )
{
    Q_ASSERT( m_data.isNull() ); // check that we're only called once
    Q_ASSERT( m_uploadOffset == 0 ); // no upload started yet
    m_data = arr;
}

void StoredTransferJob::slotStoredData( KIO::Job *, const QByteArray &data )
{
  // check for end-of-data marker:
  if ( data.size() == 0 )
    return;
  unsigned int oldSize = m_data.size();
  m_data.resize( oldSize + data.size() );
  memcpy( m_data.data() + oldSize, data.data(), data.size() );
}

void StoredTransferJob::slotStoredDataReq( KIO::Job *, QByteArray &data )
{
  // Inspired from kmail's KMKernel::byteArrayToRemoteFile
  // send the data in 64 KB chunks
  const int MAX_CHUNK_SIZE = 64*1024;
  int remainingBytes = m_data.size() - m_uploadOffset;
  if( remainingBytes > MAX_CHUNK_SIZE ) {
    // send MAX_CHUNK_SIZE bytes to the receiver (deep copy)
    data = QByteArray( m_data.data() + m_uploadOffset, MAX_CHUNK_SIZE );
    m_uploadOffset += MAX_CHUNK_SIZE;
    //kDebug() << "Sending " << MAX_CHUNK_SIZE << " bytes ("
    //                << remainingBytes - MAX_CHUNK_SIZE << " bytes remain)\n";
  } else {
    // send the remaining bytes to the receiver (deep copy)
    data = QByteArray( m_data.data() + m_uploadOffset, remainingBytes );
    m_data = QByteArray();
    m_uploadOffset = 0;
    //kDebug() << "Sending " << remainingBytes << " bytes\n";
  }
}

StoredTransferJob *KIO::storedGet( const KUrl& url, bool reload, bool showProgressInfo )
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    StoredTransferJob * job = new StoredTransferJob( url, CMD_GET, packedArgs, QByteArray(), showProgressInfo );
    if (reload)
       job->addMetaData("cache", "reload");
    return job;
}

StoredTransferJob *KIO::storedPut( const QByteArray& arr, const KUrl& url, int permissions,
                                   bool overwrite, bool resume, bool showProgressInfo )
{
    KIO_ARGS << url << qint8( overwrite ? 1 : 0 ) << qint8( resume ? 1 : 0 ) << permissions;
    StoredTransferJob * job = new StoredTransferJob( url, CMD_PUT, packedArgs, QByteArray(), showProgressInfo );
    job->setData( arr );
    return job;
}

//////////

MimetypeJob::MimetypeJob( const KUrl& url, int command,
                  const QByteArray &packedArgs, bool showProgressInfo )
    : TransferJob(url, command, packedArgs, QByteArray(), showProgressInfo)
{
}

void MimetypeJob::start(Slave *slave)
{
    TransferJob::start(slave);
}


void MimetypeJob::slotFinished( )
{
    //kDebug(7007) << "MimetypeJob::slotFinished()" << endl;
    if ( error() == KIO::ERR_IS_DIRECTORY )
    {
        // It is in fact a directory. This happens when HTTP redirects to FTP.
        // Due to the "protocol doesn't support listing" code in KRun, we
        // assumed it was a file.
        kDebug(7007) << "It is in fact a directory!" << endl;
        m_mimetype = QString::fromLatin1("inode/directory");
        emit TransferJob::mimetype( this, m_mimetype );
        setError( 0 );
    }
    if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error() )
    {
        // Return slave to the scheduler
        TransferJob::slotFinished();
    } else {
        //kDebug(7007) << "MimetypeJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        staticData.truncate(0);
        m_suspended = false;
        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        m_packedArgs.truncate(0);
        QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

MimetypeJob *KIO::mimetype(const KUrl& url, bool showProgressInfo )
{
    KIO_ARGS << url;
    MimetypeJob * job = new MimetypeJob(url, CMD_MIMETYPE, packedArgs, showProgressInfo);
    if ( showProgressInfo )
      Observer::self()->stating( job, url );
    return job;
}

//////////////////////////

DirectCopyJob::DirectCopyJob( const KUrl& url, int command,
                              const QByteArray &packedArgs, bool showProgressInfo )
    : SimpleJob(url, command, packedArgs, showProgressInfo)
{
}

void DirectCopyJob::start( Slave* slave )
{
    connect( slave, SIGNAL(canResume( KIO::filesize_t ) ),
             SLOT( slotCanResume( KIO::filesize_t ) ) );
    SimpleJob::start(slave);
}

void DirectCopyJob::slotCanResume( KIO::filesize_t offset )
{
    emit canResume(this, offset);
}

//////////////////////////


class FileCopyJob::FileCopyJobPrivate
{
public:
    KIO::filesize_t m_sourceSize;
    time_t m_modificationTime;
    SimpleJob *m_delJob;
};

/*
 * The FileCopyJob works according to the famous Bayern
 * 'Alternating Bitburger Protocol': we either drink a beer or we
 * we order a beer, but never both at the same time.
 * Tranlated to io-slaves: We alternate between receiving a block of data
 * and sending it away.
 */
FileCopyJob::FileCopyJob( const KUrl& src, const KUrl& dest, int permissions,
                          bool move, bool overwrite, bool resume, bool showProgressInfo)
    : Job(showProgressInfo), m_src(src), m_dest(dest),
      m_permissions(permissions), m_move(move), m_overwrite(overwrite), m_resume(resume),
      m_totalSize(0),d(new FileCopyJobPrivate)
{
   if (showProgressInfo && !move)
      Observer::self()->slotCopying( this, src, dest );
   else if (showProgressInfo && move)
      Observer::self()->slotMoving( this, src, dest );

    //kDebug(7007) << "FileCopyJob::FileCopyJob()" << endl;
    m_moveJob = 0;
    m_copyJob = 0;
    m_getJob = 0;
    m_putJob = 0;
    d->m_delJob = 0;
    d->m_sourceSize = (KIO::filesize_t) -1;
    d->m_modificationTime = static_cast<time_t>( -1 );
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void FileCopyJob::slotStart()
{
   if ( m_move )
   {
      // The if() below must be the same as the one in startBestCopyMethod
      if ((m_src.protocol() == m_dest.protocol()) &&
          (m_src.host() == m_dest.host()) &&
          (m_src.port() == m_dest.port()) &&
          (m_src.user() == m_dest.user()) &&
          (m_src.pass() == m_dest.pass()) &&
          !m_src.hasSubUrl() && !m_dest.hasSubUrl())
      {
         startRenameJob(m_src);
         return;
      }
      else if (m_src.isLocalFile() && KProtocolManager::canRenameFromFile(m_dest))
      {
         startRenameJob(m_dest);
         return;
      }
      else if (m_dest.isLocalFile() && KProtocolManager::canRenameToFile(m_src))
      {
         startRenameJob(m_src);
         return;
      }
      // No fast-move available, use copy + del.
   }
   startBestCopyMethod();
}

void FileCopyJob::startBestCopyMethod()
{
   if ((m_src.protocol() == m_dest.protocol()) &&
       (m_src.host() == m_dest.host()) &&
       (m_src.port() == m_dest.port()) &&
       (m_src.user() == m_dest.user()) &&
       (m_src.pass() == m_dest.pass()) &&
       !m_src.hasSubUrl() && !m_dest.hasSubUrl())
   {
      startCopyJob();
   }
   else if (m_src.isLocalFile() && KProtocolManager::canCopyFromFile(m_dest))
   {
      startCopyJob(m_dest);
   }
   else if (m_dest.isLocalFile() && KProtocolManager::canCopyToFile(m_src))
   {
      startCopyJob(m_src);
   }
   else
   {
      startDataPump();
   }
}

FileCopyJob::~FileCopyJob()
{
    delete d;
}


void FileCopyJob::setSourceSize( KIO::filesize_t size )
{
    d->m_sourceSize = size;
    if (size != (KIO::filesize_t) -1)
       m_totalSize = size;
}

void FileCopyJob::setModificationTime( time_t mtime )
{
    d->m_modificationTime = mtime;
}

void FileCopyJob::startCopyJob()
{
    startCopyJob(m_src);
}

void FileCopyJob::startCopyJob(const KUrl &slave_url)
{
    //kDebug(7007) << "FileCopyJob::startCopyJob()" << endl;
    KIO_ARGS << m_src << m_dest << m_permissions << (qint8) m_overwrite;
    m_copyJob = new DirectCopyJob(slave_url, CMD_COPY, packedArgs, false);
    addSubjob( m_copyJob );
    connectSubjob( m_copyJob );
    connect( m_copyJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)),
             SLOT( slotCanResume(KIO::Job *, KIO::filesize_t)));
}

void FileCopyJob::startRenameJob(const KUrl &slave_url)
{
    KIO_ARGS << m_src << m_dest << (qint8) m_overwrite;
    m_moveJob = new SimpleJob(slave_url, CMD_RENAME, packedArgs, false);
    addSubjob( m_moveJob );
    connectSubjob( m_moveJob );
}

void FileCopyJob::connectSubjob( SimpleJob * job )
{
    connect( job, SIGNAL(totalSize( KJob*, qulonglong )),
             this, SLOT( slotTotalSize(KJob*, qulonglong)) );

    connect( job, SIGNAL(processedSize( KJob*, qulonglong )),
             this, SLOT( slotProcessedSize(KJob*, qulonglong)) );

    connect( job, SIGNAL(percent( KJob*, unsigned long )),
             this, SLOT( slotPercent(KJob*, unsigned long)) );

}

void FileCopyJob::slotProcessedSize( KJob *, qulonglong size )
{
    setProcessedSize(size);
    emit processedSize( this, size );
    if ( size > m_totalSize ) {
        slotTotalSize( this, size ); // safety
    }
    emitPercent( size, m_totalSize );
}

void FileCopyJob::slotTotalSize( KJob*, qulonglong size )
{
    if (size > m_totalSize)
    {
        m_totalSize = size;
        emit totalSize( this, m_totalSize );
    }
}

void FileCopyJob::slotPercent( KJob*, unsigned long pct )
{
    if ( pct > percent() )
    {
        setPercent( pct );
        emit percent( this, percent() );
    }
}

void FileCopyJob::startDataPump()
{
    //kDebug(7007) << "FileCopyJob::startDataPump()" << endl;

    m_canResume = false;
    m_resumeAnswerSent = false;
    m_getJob = 0L; // for now
    m_putJob = put( m_dest, m_permissions, m_overwrite, m_resume, false /* no GUI */);
    //kDebug(7007) << "FileCopyJob: m_putJob = " << m_putJob << " m_dest=" << m_dest << endl;
    if ( d->m_modificationTime != static_cast<time_t>( -1 ) ) {
        QDateTime dt; dt.setTime_t( d->m_modificationTime );
        m_putJob->addMetaData( "modified", dt.toString( Qt::ISODate ) );
    }

    // The first thing the put job will tell us is whether we can
    // resume or not (this is always emitted)
    connect( m_putJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)),
             SLOT( slotCanResume(KIO::Job *, KIO::filesize_t)));
    connect( m_putJob, SIGNAL(dataReq(KIO::Job *, QByteArray&)),
             SLOT( slotDataReq(KIO::Job *, QByteArray&)));
    addSubjob( m_putJob );
}

void FileCopyJob::slotCanResume( KIO::Job* job, KIO::filesize_t offset )
{
    if ( job == m_putJob || job == m_copyJob )
    {
        //kDebug(7007) << "FileCopyJob::slotCanResume from PUT job. offset=" << KIO::number(offset) << endl;
        if (offset)
        {
            RenameDlg_Result res = R_RESUME;

            if (!KProtocolManager::autoResume() && !m_overwrite)
            {
                QString newPath;
                KIO::Job* job = ( !progressId() && parentJob() ) ? parentJob() : this;
                // Ask confirmation about resuming previous transfer
                res = Observer::self()->open_RenameDlg(
                      job, i18n("File Already Exists"),
                      m_src.url(),
                      m_dest.url(),
                      (RenameDlg_Mode) (M_OVERWRITE | M_RESUME | M_NORENAME), newPath,
                      d->m_sourceSize, offset );
            }

            if ( res == R_OVERWRITE || m_overwrite )
              offset = 0;
            else if ( res == R_CANCEL )
            {
                if ( job == m_putJob )
                    m_putJob->kill( Quietly );
                else
                    m_copyJob->kill( Quietly );
                setError( ERR_USER_CANCELED );
                emitResult();
                return;
            }
        }
        else
            m_resumeAnswerSent = true; // No need for an answer

        if ( job == m_putJob )
        {
            m_getJob = get( m_src, false, false /* no GUI */ );
            //kDebug(7007) << "FileCopyJob: m_getJob = " << m_getJob << endl;
            m_getJob->addMetaData( "errorPage", "false" );
            m_getJob->addMetaData( "AllowCompressedPage", "false" );
            // Set size in subjob. This helps if the slave doesn't emit totalSize.
            if ( d->m_sourceSize != (KIO::filesize_t)-1 )
                m_getJob->slotTotalSize( d->m_sourceSize );
            if (offset)
            {
                //kDebug(7007) << "Setting metadata for resume to " << (unsigned long) offset << endl;
                // TODO KDE4: rename to seek or offset and document it
                // This isn't used only for resuming, but potentially also for extracting (#72302).
                m_getJob->addMetaData( "resume", KIO::number(offset) );

                // Might or might not get emitted
                connect( m_getJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)),
                         SLOT( slotCanResume(KIO::Job *, KIO::filesize_t)));
            }
            m_putJob->slave()->setOffset( offset );

            m_putJob->suspend();
            addSubjob( m_getJob );
            connectSubjob( m_getJob ); // Progress info depends on get
            m_getJob->resume(); // Order a beer

            connect( m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)),
                     SLOT( slotData(KIO::Job *, const QByteArray&)));
        }
        else // copyjob
        {
            m_copyJob->slave()->sendResumeAnswer( offset != 0 );
        }
    }
    else if ( job == m_getJob )
    {
        // Cool, the get job said ok, we can resume
        m_canResume = true;
        //kDebug(7007) << "FileCopyJob::slotCanResume from the GET job -> we can resume" << endl;

        m_getJob->slave()->setOffset( m_putJob->slave()->offset() );
    }
    else
        kWarning(7007) << "FileCopyJob::slotCanResume from unknown job=" << job
                        << " m_getJob=" << m_getJob << " m_putJob=" << m_putJob << endl;
}

void FileCopyJob::slotData( KIO::Job * , const QByteArray &data)
{
   //kDebug(7007) << "FileCopyJob::slotData" << endl;
   //kDebug(7007) << " data size : " << data.size() << endl;
   assert(m_putJob);
   if (!m_putJob) return; // Don't crash
   m_getJob->suspend();
   m_putJob->resume(); // Drink the beer
   m_buffer = data;

   // On the first set of data incoming, we tell the "put" slave about our
   // decision about resuming
   if (!m_resumeAnswerSent)
   {
       m_resumeAnswerSent = true;
       //kDebug(7007) << "FileCopyJob::slotData (first time) -> send resume answer " << m_canResume << endl;
       m_putJob->slave()->sendResumeAnswer( m_canResume );
   }
}

void FileCopyJob::slotDataReq( KIO::Job * , QByteArray &data)
{
   //kDebug(7007) << "FileCopyJob::slotDataReq" << endl;
   if (!m_resumeAnswerSent && !m_getJob)
   {
       // This can't happen (except as a migration bug on 12/10/2000)
       setError( ERR_INTERNAL );
       setErrorText( "'Put' job didn't send canResume or 'Get' job didn't send data!" );
       m_putJob->kill( Quietly );
       emitResult();
       return;
   }
   if (m_getJob)
   {
      m_getJob->resume(); // Order more beer
      m_putJob->suspend();
   }
   data = m_buffer;
   m_buffer = QByteArray();
}

void FileCopyJob::slotResult( KJob *job)
{
   //kDebug(7007) << "FileCopyJob this=" << this << " ::slotResult(" << job << ")" << endl;
   // Did job have an error ?
   if ( job->error() )
   {
      if ((job == m_moveJob) && (job->error() == ERR_UNSUPPORTED_ACTION))
      {
         m_moveJob = 0;
         startBestCopyMethod();
         removeSubjob(job);
         return;
      }
      else if ((job == m_copyJob) && (job->error() == ERR_UNSUPPORTED_ACTION))
      {
         m_copyJob = 0;
         startDataPump();
         removeSubjob(job);
         return;
      }
      else if (job == m_getJob)
      {
        m_getJob = 0L;
        if (m_putJob)
          m_putJob->kill( Quietly );
      }
      else if (job == m_putJob)
      {
        m_putJob = 0L;
        if (m_getJob)
          m_getJob->kill( Quietly );
      }
      setError( job->error() );
      setErrorText( job->errorText() );
      emitResult();
      return;
   }

   if (job == m_moveJob)
   {
      m_moveJob = 0; // Finished
   }

   if (job == m_copyJob)
   {
      m_copyJob = 0;
      if (m_move)
      {
         d->m_delJob = file_delete( m_src, false/*no GUI*/ ); // Delete source
         addSubjob(d->m_delJob);
      }
   }

   if (job == m_getJob)
   {
      m_getJob = 0; // No action required
      if (m_putJob)
         m_putJob->resume();
   }

   if (job == m_putJob)
   {
      //kDebug(7007) << "FileCopyJob: m_putJob finished " << endl;
      m_putJob = 0;
      if (m_getJob)
      {
         kWarning(7007) << "WARNING ! Get still going on..." << endl;
         m_getJob->resume();
      }
      if (m_move)
      {
         d->m_delJob = file_delete( m_src, false/*no GUI*/ ); // Delete source
         addSubjob(d->m_delJob);
      }
   }

   if (job == d->m_delJob)
   {
      d->m_delJob = 0; // Finished
   }
   removeSubjob(job);
   if ( !hasSubjobs() )
       emitResult();
}

FileCopyJob *KIO::file_copy( const KUrl& src, const KUrl& dest, int permissions,
                             bool overwrite, bool resume, bool showProgressInfo)
{
   return new FileCopyJob( src, dest, permissions, false, overwrite, resume, showProgressInfo );
}

FileCopyJob *KIO::file_move( const KUrl& src, const KUrl& dest, int permissions,
                             bool overwrite, bool resume, bool showProgressInfo)
{
   return new FileCopyJob( src, dest, permissions, true, overwrite, resume, showProgressInfo );
}

SimpleJob *KIO::file_delete( const KUrl& src, bool showProgressInfo)
{
    KIO_ARGS << src << qint8(true); // isFile
    return new SimpleJob(src, CMD_DEL, packedArgs, showProgressInfo );
}

//////////

ListJob::ListJob(const KUrl& u, bool showProgressInfo, bool _recursive, const QString &_prefix, bool _includeHidden) :
    SimpleJob(u, CMD_LISTDIR, QByteArray(), showProgressInfo),
    recursive(_recursive), includeHidden(_includeHidden), prefix(_prefix), m_processedEntries(0)
{
    // We couldn't set the args when calling the parent constructor,
    // so do it now.
    QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
    stream << u;
}

void ListJob::slotListEntries( const KIO::UDSEntryList& list )
{
    // Emit progress info (takes care of emit processedSize and percent)
    m_processedEntries += list.count();
    slotProcessedSize( m_processedEntries );

    if (recursive) {
        UDSEntryList::ConstIterator it = list.begin();
        const UDSEntryList::ConstIterator end = list.end();

        for (; it != end; ++it) {

            const UDSEntry& entry = *it;

            KUrl itemURL;
            const UDSEntry::ConstIterator end2 = entry.end();
            UDSEntry::ConstIterator it2 = entry.find( KIO::UDS_URL );
            if ( it2 != end2 )
                itemURL = it2.value().toString();
            else { // no URL, use the name
                itemURL = url();
                itemURL.addPath( entry.stringValue( KIO::UDS_NAME ) );
            }

            if (entry.isDir() && !entry.isLink()) {
                const QString filename = itemURL.fileName();
                // skip hidden dirs when listing if requested
                if (filename != ".." && filename != "." && (includeHidden || filename[0] != '.')) {
                    ListJob *job = new ListJob(itemURL,
                                               false /*no progress info!*/,
                                               true /*recursive*/,
                                               prefix + filename + '/',
                                               includeHidden);
                    Scheduler::scheduleJob(job);
                    connect(job, SIGNAL(entries( KIO::Job *,
                                                 const KIO::UDSEntryList& )),
                            SLOT( gotEntries( KIO::Job*,
                                              const KIO::UDSEntryList& )));
                    addSubjob(job);
                }
            }
        }
    }

    // Not recursive, or top-level of recursive listing : return now (send . and .. as well)
    // exclusion of hidden files also requires the full sweep, but the case for full-listing
    // a single dir is probably common enough to justify the shortcut
    if (prefix.isNull() && includeHidden) {
        emit entries(this, list);
    } else {
        // cull the unwanted hidden dirs and/or parent dir references from the listing, then emit that
        UDSEntryList newlist;

        UDSEntryList::const_iterator it = list.begin();
        const UDSEntryList::const_iterator end = list.end();
        for (; it != end; ++it) {

            // Modify the name in the UDSEntry
            UDSEntry newone = *it;
            const QString filename = newone.stringValue( KIO::UDS_NAME );
            // Avoid returning entries like subdir/. and subdir/.., but include . and .. for
            // the toplevel dir, and skip hidden files/dirs if that was requested
            if (  (prefix.isNull() || (filename != ".." && filename != ".") )
                  && (includeHidden || (filename[0] != '.') )  )
            {
                // ## Didn't find a way to use the iterator instead of re-doing a key lookup
                newone.insert( KIO::UDS_NAME, prefix + filename );
                newlist.append(newone);
            }
        }

        emit entries(this, newlist);
    }
}

void ListJob::gotEntries(KIO::Job *, const KIO::UDSEntryList& list )
{
    // Forward entries received by subjob - faking we received them ourselves
    emit entries(this, list);
}

void ListJob::slotResult( KJob * job )
{
    // If we can't list a subdir, the result is still ok
    // This is why we override Job::slotResult() - to skip error checking
    removeSubjob( job );
    if ( !hasSubjobs() )
        emitResult();
}

void ListJob::slotRedirection( const KUrl & url )
{
     if (!KAuthorized::authorizeURLAction("redirect", m_url, url))
     {
       kWarning(7007) << "ListJob: Redirection from " << m_url << " to " << url << " REJECTED!" << endl;
       return;
     }
    m_redirectionURL = url; // We'll remember that when the job finishes
    if (m_url.hasUser() && !url.hasUser() && (m_url.host().toLower() == url.host().toLower()))
        m_redirectionURL.setUser(m_url.user()); // Preserve user
    emit redirection( this, m_redirectionURL );
}

void ListJob::slotFinished()
{
    // Support for listing archives as directories
    if ( error() == KIO::ERR_IS_FILE && m_url.isLocalFile() ) {
        KMimeType::Ptr ptr = KMimeType::findByURL( m_url, 0, true, true );
        if ( ptr ) {
            QString proto = ptr->property("X-KDE-LocalProtocol").toString();
            if ( !proto.isEmpty() ) {
                m_redirectionURL = m_url;
                m_redirectionURL.setProtocol( proto );
                setError( 0 );
                emit redirection(this,m_redirectionURL);
            }
        }
    }
    if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error() ) {
        // Return slave to the scheduler
        SimpleJob::slotFinished();
    } else {

        //kDebug(7007) << "ListJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        m_packedArgs.truncate(0);
        QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

void ListJob::slotMetaData( const KIO::MetaData &_metaData) {
    SimpleJob::slotMetaData(_metaData);
    storeSSLSessionFromJob(m_redirectionURL);
}

ListJob *KIO::listDir( const KUrl& url, bool showProgressInfo, bool includeHidden )
{
    ListJob * job = new ListJob(url, showProgressInfo,false,QString(),includeHidden);
    return job;
}

ListJob *KIO::listRecursive( const KUrl& url, bool showProgressInfo, bool includeHidden )
{
    ListJob * job = new ListJob(url, showProgressInfo, true,QString(),includeHidden);
    return job;
}

void ListJob::setUnrestricted(bool unrestricted)
{
    if (unrestricted)
       extraFlags() |= EF_ListJobUnrestricted;
    else
       extraFlags() &= ~EF_ListJobUnrestricted;
}

void ListJob::start(Slave *slave)
{
    if (!KAuthorized::authorizeURLAction("list", m_url, m_url) && !(extraFlags() & EF_ListJobUnrestricted))
    {
        setError( ERR_ACCESS_DENIED );
        setErrorText( m_url.url() );
        QTimer::singleShot(0, this, SLOT(slotFinished()) );
        return;
    }
    connect( slave, SIGNAL( listEntries( const KIO::UDSEntryList& )),
             SLOT( slotListEntries( const KIO::UDSEntryList& )));
    connect( slave, SIGNAL( totalSize( KIO::filesize_t ) ),
             SLOT( slotTotalSize( KIO::filesize_t ) ) );
    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    SimpleJob::start(slave);
}

class CopyJob::CopyJobPrivate
{
public:
    CopyJobPrivate() {
        m_defaultPermissions = false;
        m_bURLDirty = false;
    }
    // This is the dest URL that was initially given to CopyJob
    // It is copied into m_dest, which can be changed for a given src URL
    // (when using the RENAME dialog in slotResult),
    // and which will be reset for the next src URL.
    KUrl m_globalDest;
    // The state info about that global dest
    CopyJob::DestinationState m_globalDestinationState;
    // See setDefaultPermissions
    bool m_defaultPermissions;
    // Whether URLs changed (and need to be emitted by the next slotReport call)
    bool m_bURLDirty;
    // Used after copying all the files into the dirs, to set mtime (TODO: and permissions?)
    // after the copy is done
    QLinkedList<CopyInfo> m_directoriesCopied;
};

CopyJob::CopyJob( const KUrl::List& src, const KUrl& dest, CopyMode mode, bool asMethod, bool showProgressInfo )
  : Job(showProgressInfo), m_mode(mode), m_asMethod(asMethod),
    destinationState(DEST_NOT_STATED), state(STATE_STATING),
    m_totalSize(0), m_processedSize(0), m_fileProcessedSize(0),
    m_processedFiles(0), m_processedDirs(0),
    m_srcList(src), m_currentStatSrc(m_srcList.begin()),
    m_bCurrentOperationIsLink(false), m_bSingleFileCopy(false), m_bOnlyRenames(mode==Move),
    m_dest(dest), m_bAutoSkip( false ), m_bOverwriteAll( false ),
    m_conflictError(0), m_reportTimer(0),d(new CopyJobPrivate)
{
    d->m_globalDest = dest;
    d->m_globalDestinationState = destinationState;

    if ( showProgressInfo ) {
        connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );

        connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );
    }
    QTimer::singleShot(0, this, SLOT(slotStart()));
    /**
       States:
       STATE_STATING for the dest
       STATE_STATING for each src url (statNextSrc)
            for each: if dir -> STATE_LISTING (filling 'dirs' and 'files')
            but if direct rename possible: STATE_RENAMING instead.
       STATE_CREATING_DIRS (createNextDir, iterating over 'dirs')
            if conflict: STATE_CONFLICT_CREATING_DIRS
       STATE_COPYING_FILES (copyNextFile, iterating over 'files')
            if conflict: STATE_CONFLICT_COPYING_FILES
       STATE_DELETING_DIRS (deleteNextDir) (if moving)
       STATE_SETTING_DIR_ATTRIBUTES (setNextDirAttribute, iterating over d->m_directoriesCopied)
       done.
    */
}

CopyJob::~CopyJob()
{
    delete d;
}

void CopyJob::slotStart()
{
    /**
       We call the functions directly instead of using signals.
       Calling a function via a signal takes approx. 65 times the time
       compared to calling it directly (at least on my machine). aleXXX
    */
    m_reportTimer = new QTimer(this);

    connect(m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
    m_reportTimer->start(REPORT_TIMEOUT);

    // Stat the dest
    KIO::Job * job = KIO::stat( m_dest, false, 2, false );
    //kDebug(7007) << "CopyJob:stating the dest " << m_dest << endl;
    addSubjob(job);
}

// For unit test purposes
KIO_EXPORT bool kio_resolve_local_urls = true;

void CopyJob::slotResultStating( KJob *job )
{
    //kDebug(7007) << "CopyJob::slotResultStating" << endl;
    // Was there an error while stating the src ?
    if (job->error() && destinationState != DEST_NOT_STATED )
    {
        KUrl srcurl = ((SimpleJob*)job)->url();
        if ( !srcurl.isLocalFile() )
        {
            // Probably : src doesn't exist. Well, over some protocols (e.g. FTP)
            // this info isn't really reliable (thanks to MS FTP servers).
            // We'll assume a file, and try to download anyway.
            kDebug(7007) << "Error while stating source. Activating hack" << endl;
            removeSubjob( job );
            assert ( !hasSubjobs() ); // We should have only one job at a time ...
            struct CopyInfo info;
            info.permissions = (mode_t) -1;
            info.mtime = (time_t) -1;
            info.ctime = (time_t) -1;
            info.size = (KIO::filesize_t)-1;
            info.uSource = srcurl;
            info.uDest = m_dest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
                info.uDest.addPath( srcurl.fileName() );

            files.append( info );
            statNextSrc();
            return;
        }
        // Local file. If stat fails, the file definitely doesn't exist.
        Job::slotResult( job ); // will set the error and emit result(this)
        return;
    }

    // Keep copy of the stat result
    const UDSEntry entry = static_cast<StatJob*>(job)->statResult();
    const QString sLocalPath = entry.stringValue( KIO::UDS_LOCAL_PATH );
    const bool isDir = entry.isDir();

    if ( destinationState == DEST_NOT_STATED )
        // we were stating the dest
    {
        if (job->error())
            destinationState = DEST_DOESNT_EXIST;
        else {
            // Treat symlinks to dirs as dirs here, so no test on isLink
            destinationState = isDir ? DEST_IS_DIR : DEST_IS_FILE;
            //kDebug(7007) << "CopyJob::slotResultStating dest is dir:" << bDir << endl;
        }
        const bool isGlobalDest = m_dest == d->m_globalDest;
        if ( isGlobalDest )
            d->m_globalDestinationState = destinationState;

        if ( !sLocalPath.isEmpty() && kio_resolve_local_urls ) {
            m_dest = KUrl();
            m_dest.setPath(sLocalPath);
            if ( isGlobalDest )
                d->m_globalDest = m_dest;
        }

        removeSubjob( job );
        assert ( !hasSubjobs() );

        // After knowing what the dest is, we can start stat'ing the first src.
        statCurrentSrc();
        return;
    }

    // Is it a file or a dir ?
    const QString sName = entry.stringValue( KIO::UDS_NAME );

    // We were stating the current source URL
    m_currentDest = m_dest; // used by slotEntries
    // Create a dummy list with it, for slotEntries
    UDSEntryList lst;
    lst.append(entry);

    // There 6 cases, and all end up calling slotEntries(job, lst) first :
    // 1 - src is a dir, destination is a directory,
    // slotEntries will append the source-dir-name to the destination
    // 2 - src is a dir, destination is a file, ERROR (done later on)
    // 3 - src is a dir, destination doesn't exist, then it's the destination dirname,
    // so slotEntries will use it as destination.

    // 4 - src is a file, destination is a directory,
    // slotEntries will append the filename to the destination.
    // 5 - src is a file, destination is a file, m_dest is the exact destination name
    // 6 - src is a file, destination doesn't exist, m_dest is the exact destination name
    // Tell slotEntries not to alter the src url
    m_bCurrentSrcIsDir = false;
    slotEntries(static_cast<KIO::Job*>( job ), lst);

    KUrl srcurl;
    if (!sLocalPath.isEmpty())
        srcurl.setPath(sLocalPath);
    else
        srcurl = ((SimpleJob*)job)->url();

    removeSubjob( job );
    assert ( !hasSubjobs() ); // We should have only one job at a time ...

    if ( isDir
         // treat symlinks as files (no recursion)
         && !entry.isLink()
         && m_mode != Link ) // No recursion in Link mode either.
    {
        //kDebug(7007) << " Source is a directory " << endl;

        m_bCurrentSrcIsDir = true; // used by slotEntries
        if ( destinationState == DEST_IS_DIR ) // (case 1)
        {
            if ( !m_asMethod )
            {
                // Use <desturl>/<directory_copied> as destination, from now on
                QString directory = srcurl.fileName();
                if ( !sName.isEmpty() && KProtocolManager::fileNameUsedForCopying( srcurl ) == KProtocolInfo::Name )
                {
                    directory = sName;
                }
                m_currentDest.addPath( directory );
            }
        }
        else if ( destinationState == DEST_IS_FILE ) // (case 2)
        {
            setError( ERR_IS_FILE );
            setErrorText( m_dest.prettyUrl() );
            emitResult();
            return;
        }
        else // (case 3)
        {
            // otherwise dest is new name for toplevel dir
            // so the destination exists, in fact, from now on.
            // (This even works with other src urls in the list, since the
            //  dir has effectively been created)
            destinationState = DEST_IS_DIR;
            if ( m_dest == d->m_globalDest )
                d->m_globalDestinationState = destinationState;
        }

        startListing( srcurl );
    }
    else
    {
        //kDebug(7007) << " Source is a file (or a symlink), or we are linking -> no recursive listing " << endl;
        statNextSrc();
    }
}

void CopyJob::slotReport()
{
    // If showProgressInfo was set, progressId() is > 0.
    Observer * observer = progressId() ? Observer::self() : 0L;
    switch (state) {
        case STATE_COPYING_FILES:
            emit processedFiles( this, m_processedFiles );
            if (observer) observer->slotProcessedFiles(this, m_processedFiles);
            if (d->m_bURLDirty)
            {
                // Only emit urls when they changed. This saves time, and fixes #66281
                d->m_bURLDirty = false;
                if (m_mode==Move)
                {
                    if (observer) observer->slotMoving( this, m_currentSrcURL, m_currentDestURL);
                    emit moving( this, m_currentSrcURL, m_currentDestURL);
                }
                else if (m_mode==Link)
                {
                    if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL ); // we don't have a slotLinking
                    emit linking( this, m_currentSrcURL.path(), m_currentDestURL );
                }
                else
                {
                    if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL );
                    emit copying( this, m_currentSrcURL, m_currentDestURL );
                }
            }
            break;

        case STATE_CREATING_DIRS:
            if (observer) observer->slotProcessedDirs( this, m_processedDirs );
            emit processedDirs( this, m_processedDirs );
            if (d->m_bURLDirty)
            {
                d->m_bURLDirty = false;
                emit creatingDir( this, m_currentDestURL );
                if (observer) observer->slotCreatingDir( this, m_currentDestURL);
            }
            break;

        case STATE_STATING:
        case STATE_LISTING:
            if (d->m_bURLDirty)
            {
                d->m_bURLDirty = false;
                if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL );
            }
            emit totalSize( this, m_totalSize );
            emit totalFiles( this, files.count() );
            emit totalDirs( this, dirs.count() );
            break;

        default:
            break;
    }
}

void CopyJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    UDSEntryList::ConstIterator it = list.begin();
    UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it) {
        const UDSEntry& entry = *it;
        struct CopyInfo info;
        info.permissions = entry.numberValue( KIO::UDS_ACCESS, -1 );
        info.mtime = (time_t) entry.numberValue( KIO::UDS_MODIFICATION_TIME, -1 );
        info.ctime = (time_t) entry.numberValue( KIO::UDS_CREATION_TIME, -1 );
        info.size = (KIO::filesize_t) entry.numberValue( KIO::UDS_SIZE, -1 );
        if ( info.size != (KIO::filesize_t) -1 )
            m_totalSize += info.size;

        // recursive listing, displayName can be a/b/c/d
        const QString displayName = entry.stringValue( KIO::UDS_NAME );
        const QString urlStr = entry.stringValue( KIO::UDS_URL );
        KUrl url;
        if ( !urlStr.isEmpty() )
            url = urlStr;
        QString localPath = entry.stringValue( KIO::UDS_LOCAL_PATH );
        const bool isDir = entry.isDir();
        info.linkDest = entry.stringValue( KIO::UDS_LINK_DEST );

        if (displayName != ".." && displayName != ".")
        {
            bool hasCustomURL = !url.isEmpty() || !localPath.isEmpty();
            if( !hasCustomURL ) {
                // Make URL from displayName
                url = ((SimpleJob *)job)->url();
                if ( m_bCurrentSrcIsDir ) { // Only if src is a directory. Otherwise uSource is fine as is
                    //kDebug(7007) << "adding path " << displayName << endl;
                    url.addPath( displayName );
                }
            }
            //kDebug(7007) << "displayName=" << displayName << " url=" << url << endl;
            if (!localPath.isEmpty() && kio_resolve_local_urls) {
                url = KUrl();
                url.setPath(localPath);
            }

            info.uSource = url;
            info.uDest = m_currentDest;
            //kDebug(7007) << " uSource=" << info.uSource << " uDest(1)=" << info.uDest << endl;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR &&
                 // "copy/move as <foo>" means 'foo' is the dest for the base srcurl
                 // (passed here during stating) but not its children (during listing)
                 ( ! ( m_asMethod && state == STATE_STATING ) ) )
            {
                QString destFileName;
                if ( hasCustomURL &&
                     KProtocolManager::fileNameUsedForCopying( url ) == KProtocolInfo::FromURL ) {
                    //destFileName = url.fileName(); // Doesn't work for recursive listing
                    // Count the number of prefixes used by the recursive listjob
                    int numberOfSlashes = displayName.count( '/' ); // don't make this a find()!
                    QString path = url.path();
                    int pos = 0;
                    for ( int n = 0; n < numberOfSlashes + 1; ++n ) {
                        pos = path.lastIndexOf( '/', pos - 1 );
                        if ( pos == -1 ) { // error
                            kWarning(7007) << "kioslave bug: not enough slashes in UDS_URL " << path << " - looking for " << numberOfSlashes << " slashes" << endl;
                            break;
                        }
                    }
                    if ( pos >= 0 ) {
                        destFileName = path.mid( pos + 1 );
                    }

                } else { // destination filename taken from UDS_NAME
                    destFileName = displayName;
                }

                // Here we _really_ have to add some filename to the dest.
                // Otherwise, we end up with e.g. dest=..../Desktop/ itself.
                // (This can happen when dropping a link to a webpage with no path)
                if ( destFileName.isEmpty() )
                    destFileName = KIO::encodeFileName( info.uSource.prettyUrl() );

                //kDebug(7007) << " adding destFileName=" << destFileName << endl;
                info.uDest.addPath( destFileName );
            }
            //kDebug(7007) << " uDest(2)=" << info.uDest << endl;
            //kDebug(7007) << " " << info.uSource << " -> " << info.uDest << endl;
            if ( info.linkDest.isEmpty() && isDir && m_mode != Link ) // Dir
            {
                dirs.append( info ); // Directories
                if (m_mode == Move)
                    dirsToRemove.append( info.uSource );
            }
            else {
                files.append( info ); // Files and any symlinks
            }
        }
    }
}

void CopyJob::skipSrc()
{
    m_dest = d->m_globalDest;
    destinationState = d->m_globalDestinationState;
    ++m_currentStatSrc;
    skip( m_currentSrcURL );
    statCurrentSrc();
}

void CopyJob::statNextSrc()
{
    /* Revert to the global destination, the one that applies to all source urls.
     * Imagine you copy the items a b and c into /d, but /d/b exists so the user uses "Rename" to put it in /foo/b instead.
     * m_dest is /foo/b for b, but we have to revert to /d for item c and following.
     */
    m_dest = d->m_globalDest;
    destinationState = d->m_globalDestinationState;
    ++m_currentStatSrc;
    statCurrentSrc();
}

void CopyJob::statCurrentSrc()
{
    if ( m_currentStatSrc != m_srcList.end() )
    {
        m_currentSrcURL = (*m_currentStatSrc);
        d->m_bURLDirty = true;
        if ( m_mode == Link )
        {
            // Skip the "stating the source" stage, we don't need it for linking
            m_currentDest = m_dest;
            struct CopyInfo info;
            info.permissions = -1;
            info.mtime = (time_t) -1;
            info.ctime = (time_t) -1;
            info.size = (KIO::filesize_t)-1;
            info.uSource = m_currentSrcURL;
            info.uDest = m_currentDest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
            {
                if (
                    (m_currentSrcURL.protocol() == info.uDest.protocol()) &&
                    (m_currentSrcURL.host() == info.uDest.host()) &&
                    (m_currentSrcURL.port() == info.uDest.port()) &&
                    (m_currentSrcURL.user() == info.uDest.user()) &&
                    (m_currentSrcURL.pass() == info.uDest.pass()) )
                {
                    // This is the case of creating a real symlink
                    info.uDest.addPath( m_currentSrcURL.fileName() );
                }
                else
                {
                    // Different protocols, we'll create a .desktop file
                    // We have to change the extension anyway, so while we're at it,
                    // name the file like the URL
                    info.uDest.addPath( KIO::encodeFileName( m_currentSrcURL.prettyUrl() )+".desktop" );
                }
            }
            files.append( info ); // Files and any symlinks
            statNextSrc(); // we could use a loop instead of a recursive call :)
            return;
        }
        else if ( m_mode == Move && (
                // Don't go renaming right away if we need a stat() to find out the destination filename
                KProtocolManager::fileNameUsedForCopying( m_currentSrcURL ) == KProtocolInfo::FromURL ||
                destinationState != DEST_IS_DIR || m_asMethod )
            )
        {
           // If moving, before going for the full stat+[list+]copy+del thing, try to rename
           // The logic is pretty similar to FileCopyJob::slotStart()
           if ( (m_currentSrcURL.protocol() == m_dest.protocol()) &&
              (m_currentSrcURL.host() == m_dest.host()) &&
              (m_currentSrcURL.port() == m_dest.port()) &&
              (m_currentSrcURL.user() == m_dest.user()) &&
              (m_currentSrcURL.pass() == m_dest.pass()) )
           {
              startRenameJob( m_currentSrcURL );
              return;
           }
           else if ( m_currentSrcURL.isLocalFile() && KProtocolManager::canRenameFromFile( m_dest ) )
           {
              startRenameJob( m_dest );
              return;
           }
           else if ( m_dest.isLocalFile() && KProtocolManager::canRenameToFile( m_currentSrcURL ) )
           {
              startRenameJob( m_currentSrcURL );
              return;
           }
        }

        // if the file system doesn't support deleting, we do not even stat
        if (m_mode == Move && !KProtocolManager::supportsDeleting(m_currentSrcURL)) {
            QPointer<CopyJob> that = this;
            emit warning( this, buildErrorString(ERR_CANNOT_DELETE, m_currentSrcURL.prettyUrl()) );
            if (that)
                statNextSrc(); // we could use a loop instead of a recursive call :)
            return;
        }

        // Stat the next src url
        Job * job = KIO::stat( m_currentSrcURL, true, 2, false );
        //kDebug(7007) << "KIO::stat on " << m_currentSrcURL << endl;
        state = STATE_STATING;
        addSubjob(job);
        m_currentDestURL=m_dest;
        m_bOnlyRenames = false;
        d->m_bURLDirty = true;
    }
    else
    {
        // Finished the stat'ing phase
        // First make sure that the totals were correctly emitted
        state = STATE_STATING;
        d->m_bURLDirty = true;
        slotReport();
        if (!dirs.isEmpty())
           emit aboutToCreate( this, dirs );
        if (!files.isEmpty())
           emit aboutToCreate( this, files );
        // Check if we are copying a single file
        m_bSingleFileCopy = ( files.count() == 1 && dirs.isEmpty() );
        // Then start copying things
        state = STATE_CREATING_DIRS;
        createNextDir();
    }
}

void CopyJob::startRenameJob( const KUrl& slave_url )
{
    KUrl dest = m_dest;
    // Append filename or dirname to destination URL, if allowed
    if ( destinationState == DEST_IS_DIR && !m_asMethod )
        dest.addPath( m_currentSrcURL.fileName() );
    kDebug(7007) << "This seems to be a suitable case for trying to rename before stat+[list+]copy+del" << endl;
    state = STATE_RENAMING;

    struct CopyInfo info;
    info.permissions = -1;
    info.mtime = (time_t) -1;
    info.ctime = (time_t) -1;
    info.size = (KIO::filesize_t)-1;
    info.uSource = m_currentSrcURL;
    info.uDest = dest;
    QList<CopyInfo> files;
    files.append(info);
    emit aboutToCreate( this, files );

    KIO_ARGS << m_currentSrcURL << dest << (qint8) false /*no overwrite*/;
    SimpleJob * newJob = new SimpleJob(slave_url, CMD_RENAME, packedArgs, false);
    Scheduler::scheduleJob(newJob);
    addSubjob( newJob );
    if ( m_currentSrcURL.directory() != dest.directory() ) // For the user, moving isn't renaming. Only renaming is.
        m_bOnlyRenames = false;
}

void CopyJob::startListing( const KUrl & src )
{
    state = STATE_LISTING;
    d->m_bURLDirty = true;
    ListJob * newjob = listRecursive( src, false );
    newjob->setUnrestricted(true);
    connect(newjob, SIGNAL(entries( KIO::Job *,
                                    const KIO::UDSEntryList& )),
            SLOT( slotEntries( KIO::Job*,
                               const KIO::UDSEntryList& )));
    addSubjob( newjob );
}

void CopyJob::skip( const KUrl & sourceUrl )
{
    // If this is one if toplevel sources,
    // remove it from m_srcList, for a correct FilesRemoved() signal
    //kDebug(7007) << "CopyJob::skip: looking for " << sourceUrl << endl;
    m_srcList.removeAll( sourceUrl );
    dirsToRemove.removeAll( sourceUrl );
}

bool CopyJob::shouldOverwrite( const QString& path ) const
{
    if ( m_bOverwriteAll )
        return true;
    QStringList::ConstIterator sit = m_overwriteList.begin();
    for( ; sit != m_overwriteList.end(); ++sit )
        if ( path.startsWith( *sit ) )
            return true;
    return false;
}

bool CopyJob::shouldSkip( const QString& path ) const
{
    QStringList::ConstIterator sit = m_skipList.begin();
    for( ; sit != m_skipList.end(); ++sit )
        if ( path.startsWith( *sit ) )
            return true;
    return false;
}

void CopyJob::slotResultCreatingDirs( KJob * job )
{
    // The dir we are trying to create:
    QList<CopyInfo>::Iterator it = dirs.begin();
    // Was there an error creating a dir ?
    if ( job->error() )
    {
        m_conflictError = job->error();
        if ( (m_conflictError == ERR_DIR_ALREADY_EXIST)
             || (m_conflictError == ERR_FILE_ALREADY_EXIST) ) // can't happen?
        {
            KUrl oldURL = ((SimpleJob*)job)->url();
            // Should we skip automatically ?
            if ( m_bAutoSkip ) {
                // We don't want to copy files in this directory, so we put it on the skip list
              m_skipList.append( oldURL.path( KUrl::AddTrailingSlash ) );
                skip( oldURL );
                dirs.erase( it ); // Move on to next dir
            } else {
                // Did the user choose to overwrite already?
                const QString destFile = (*it).uDest.path();
                if ( shouldOverwrite( destFile ) ) { // overwrite => just skip
                    emit copyingDone( this, ( *it ).uSource, ( *it ).uDest, true /* directory */, false /* renamed */ );
                    dirs.erase( it ); // Move on to next dir
                } else {
                    if ( !isInteractive() ) {
                        Job::slotResult( job ); // will set the error and emit result(this)
                        return;
                    }

                    assert( ((SimpleJob*)job)->url().url() == (*it).uDest.url() );
                    removeSubjob( job );
                    assert ( !hasSubjobs() ); // We should have only one job at a time ...

                    // We need to stat the existing dir, to get its last-modification time
                    KUrl existingDest( (*it).uDest );
                    SimpleJob * newJob = KIO::stat( existingDest, false, 2, false );
                    Scheduler::scheduleJob(newJob);
                    kDebug(7007) << "KIO::stat for resolving conflict on " << existingDest << endl;
                    state = STATE_CONFLICT_CREATING_DIRS;
                    addSubjob(newJob);
                    return; // Don't move to next dir yet !
                }
            }
        }
        else
        {
            // Severe error, abort
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
    }
    else // no error : remove from list, to move on to next dir
    {
        //this is required for the undo feature
        emit copyingDone( this, (*it).uSource, (*it).uDest, true, false );
        d->m_directoriesCopied.append( *it );
        dirs.erase( it );
    }

    m_processedDirs++;
    //emit processedDirs( this, m_processedDirs );
    removeSubjob( job );
    assert( !hasSubjobs() ); // We should have only one job at a time ...
    createNextDir();
}

void CopyJob::slotResultConflictCreatingDirs( KJob * job )
{
    // We come here after a conflict has been detected and we've stated the existing dir

    // The dir we were trying to create:
    QList<CopyInfo>::Iterator it = dirs.begin();

    const UDSEntry entry = ((KIO::StatJob*)job)->statResult();

    // Its modification time:
    const time_t destmtime = (time_t) entry.numberValue( KIO::UDS_MODIFICATION_TIME, -1 );
    const time_t destctime = (time_t) entry.numberValue( KIO::UDS_CREATION_TIME, -1 );

    const KIO::filesize_t destsize = entry.numberValue( KIO::UDS_SIZE );
    const QString linkDest = entry.stringValue( KIO::UDS_LINK_DEST );

    removeSubjob( job );
    assert ( !hasSubjobs() ); // We should have only one job at a time ...

    // Always multi and skip (since there are files after that)
    RenameDlg_Mode mode = (RenameDlg_Mode)( M_MULTI | M_SKIP );
    // Overwrite only if the existing thing is a dir (no chance with a file)
    if ( m_conflictError == ERR_DIR_ALREADY_EXIST )
    {
        if( (*it).uSource == (*it).uDest ||
            ((*it).uSource.protocol() == (*it).uDest.protocol() &&
              (*it).uSource.path( KUrl::RemoveTrailingSlash ) == linkDest) )
          mode = (RenameDlg_Mode)( mode | M_OVERWRITE_ITSELF);
        else
          mode = (RenameDlg_Mode)( mode | M_OVERWRITE );
    }

    QString existingDest = (*it).uDest.path();
    QString newPath;
    if (m_reportTimer)
        m_reportTimer->stop();
    RenameDlg_Result r = Observer::self()->open_RenameDlg( this, i18n("Folder Already Exists"),
                                         (*it).uSource.url(),
                                         (*it).uDest.url(),
                                         mode, newPath,
                                         (*it).size, destsize,
                                         (*it).ctime, destctime,
                                         (*it).mtime, destmtime );
    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT);
    switch ( r ) {
        case R_CANCEL:
            setError( ERR_USER_CANCELED );
            emitResult();
            return;
        case R_RENAME:
        {
          QString oldPath = (*it).uDest.path( KUrl::AddTrailingSlash );
            KUrl newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit renamed( this, (*it).uDest, newUrl ); // for e.g. kpropsdlg

            // Change the current one and strip the trailing '/'
            (*it).uDest.setPath( newUrl.path( KUrl::RemoveTrailingSlash ) );
            newPath = newUrl.path( KUrl::AddTrailingSlash ); // With trailing slash
            QList<CopyInfo>::Iterator renamedirit = it;
            ++renamedirit;
            // Change the name of subdirectories inside the directory
            for( ; renamedirit != dirs.end() ; ++renamedirit )
            {
                QString path = (*renamedirit).uDest.path();
                if ( path.startsWith( oldPath ) ) {
                    QString n = path;
                    n.replace( 0, oldPath.length(), newPath );
                    kDebug(7007) << "dirs list: " << (*renamedirit).uSource.path()
                                  << " was going to be " << path
                                  << ", changed into " << n << endl;
                    (*renamedirit).uDest.setPath( n );
                }
            }
            // Change filenames inside the directory
            QList<CopyInfo>::Iterator renamefileit = files.begin();
            for( ; renamefileit != files.end() ; ++renamefileit )
            {
                QString path = (*renamefileit).uDest.path();
                if ( path.startsWith( oldPath ) ) {
                    QString n = path;
                    n.replace( 0, oldPath.length(), newPath );
                    kDebug(7007) << "files list: " << (*renamefileit).uSource.path()
                                  << " was going to be " << path
                                  << ", changed into " << n << endl;
                    (*renamefileit).uDest.setPath( n );
                }
            }
            if (!dirs.isEmpty())
                emit aboutToCreate( this, dirs );
            if (!files.isEmpty())
                emit aboutToCreate( this, files );
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            m_skipList.append( existingDest );
            skip( (*it).uSource );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        case R_OVERWRITE:
            m_overwriteList.append( existingDest );
            emit copyingDone( this, ( *it ).uSource, ( *it ).uDest, true /* directory */, false /* renamed */ );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        case R_OVERWRITE_ALL:
            m_bOverwriteAll = true;
            emit copyingDone( this, ( *it ).uSource, ( *it ).uDest, true /* directory */, false /* renamed */ );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        default:
            assert( 0 );
    }
    state = STATE_CREATING_DIRS;
    //emit processedDirs( this, m_processedDirs );
    createNextDir();
}

void CopyJob::createNextDir()
{
    KUrl udir;
    if ( !dirs.isEmpty() )
    {
        // Take first dir to create out of list
        QList<CopyInfo>::Iterator it = dirs.begin();
        // Is this URL on the skip list or the overwrite list ?
        while( it != dirs.end() && udir.isEmpty() )
        {
            const QString dir = (*it).uDest.path();
            if ( shouldSkip( dir ) ) {
                dirs.erase( it );
                it = dirs.begin();
            } else
                udir = (*it).uDest;
        }
    }
    if ( !udir.isEmpty() ) // any dir to create, finally ?
    {
        // Create the directory - with default permissions so that we can put files into it
        // TODO : change permissions once all is finished; but for stuff coming from CDROM it sucks...
        KIO::SimpleJob *newjob = KIO::mkdir( udir, -1 );
        Scheduler::scheduleJob(newjob);

        m_currentDestURL = udir;
        d->m_bURLDirty = true;

        addSubjob(newjob);
        return;
    }
    else // we have finished creating dirs
    {
        emit processedDirs( this, m_processedDirs ); // make sure final number appears
        if (progressId()) Observer::self()->slotProcessedDirs( this, m_processedDirs );

        state = STATE_COPYING_FILES;
        m_processedFiles++; // Ralf wants it to start at 1, not 0
        copyNextFile();
    }
}

void CopyJob::slotResultCopyingFiles( KJob * job )
{
    // The file we were trying to copy:
    QList<CopyInfo>::Iterator it = files.begin();
    if ( job->error() )
    {
        // Should we skip automatically ?
        if ( m_bAutoSkip )
        {
            skip( (*it).uSource );
            m_fileProcessedSize = (*it).size;
            files.erase( it ); // Move on to next file
        }
        else
        {
            if ( !isInteractive() ) {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            m_conflictError = job->error(); // save for later
            // Existing dest ?
            if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
                 || ( m_conflictError == ERR_DIR_ALREADY_EXIST )
                 || ( m_conflictError == ERR_IDENTICAL_FILES ) )
            {
                removeSubjob( job );
                assert ( !hasSubjobs() );
                // We need to stat the existing file, to get its last-modification time
                KUrl existingFile( (*it).uDest );
                SimpleJob * newJob = KIO::stat( existingFile, false, 2, false );
                Scheduler::scheduleJob(newJob);
                kDebug(7007) << "KIO::stat for resolving conflict on " << existingFile << endl;
                state = STATE_CONFLICT_COPYING_FILES;
                addSubjob(newJob);
                return; // Don't move to next file yet !
            }
            else
            {
                if ( m_bCurrentOperationIsLink && qobject_cast<KIO::DeleteJob*>( job ) )
                {
                    // Very special case, see a few lines below
                    // We are deleting the source of a symlink we successfully moved... ignore error
                    m_fileProcessedSize = (*it).size;
                    files.erase( it );
                } else {
                    // Go directly to the conflict resolution, there is nothing to stat
                    slotResultConflictCopyingFiles( job );
                    return;
                }
            }
        }
    } else // no error
    {
        // Special case for moving links. That operation needs two jobs, unlike others.
        if ( m_bCurrentOperationIsLink && m_mode == Move
             && !qobject_cast<KIO::DeleteJob *>( job ) // Deleting source not already done
             )
        {
            removeSubjob( job );
            assert ( !hasSubjobs() );
            // The only problem with this trick is that the error handling for this del operation
            // is not going to be right... see 'Very special case' above.
            KIO::Job * newjob = KIO::del( (*it).uSource, false /*don't shred*/, false /*no GUI*/ );
            addSubjob( newjob );
            return; // Don't move to next file yet !
        }

        if ( m_bCurrentOperationIsLink )
        {
            QString target = ( m_mode == Link ? (*it).uSource.path() : (*it).linkDest );
            //required for the undo feature
            emit copyingLinkDone( this, (*it).uSource, target, (*it).uDest );
        }
        else
            //required for the undo feature
            emit copyingDone( this, (*it).uSource, (*it).uDest, false, false );
        // remove from list, to move on to next file
        files.erase( it );
    }
    m_processedFiles++;

    // clear processed size for last file and add it to overall processed size
    m_processedSize += m_fileProcessedSize;
    m_fileProcessedSize = 0;

    //kDebug(7007) << files.count() << " files remaining" << endl;

    removeSubjob( job, true ); // merge metadata
    assert( !hasSubjobs() ); // We should have only one job at a time ...
    copyNextFile();
}

void CopyJob::slotResultConflictCopyingFiles( KJob * job )
{
    // We come here after a conflict has been detected and we've stated the existing file
    // The file we were trying to create:
    QList<CopyInfo>::Iterator it = files.begin();

    RenameDlg_Result res;
    QString newPath;

    if (m_reportTimer)
        m_reportTimer->stop();

    if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
         || ( m_conflictError == ERR_DIR_ALREADY_EXIST )
         || ( m_conflictError == ERR_IDENTICAL_FILES ) )
    {
        // Its modification time:
        const UDSEntry entry = ((KIO::StatJob*)job)->statResult();

        const time_t destmtime = (time_t) entry.numberValue( KIO::UDS_MODIFICATION_TIME, -1 );
        const time_t destctime = (time_t) entry.numberValue( KIO::UDS_CREATION_TIME, -1 );
        const KIO::filesize_t destsize = entry.numberValue( KIO::UDS_SIZE );
        const QString linkDest = entry.stringValue( KIO::UDS_LINK_DEST );

        // Offer overwrite only if the existing thing is a file
        // If src==dest, use "overwrite-itself"
        RenameDlg_Mode mode;
        bool isDir = true;

        if( m_conflictError == ERR_DIR_ALREADY_EXIST )
            mode = (RenameDlg_Mode) 0;
        else
        {
            if ( (*it).uSource == (*it).uDest  ||
                 ((*it).uSource.protocol() == (*it).uDest.protocol() &&
                   (*it).uSource.path( KUrl::RemoveTrailingSlash ) == linkDest) )
                mode = M_OVERWRITE_ITSELF;
            else
                mode = M_OVERWRITE;
            isDir = false;
        }

        if ( m_bSingleFileCopy )
            mode = (RenameDlg_Mode) ( mode | M_SINGLE );
        else
            mode = (RenameDlg_Mode) ( mode | M_MULTI | M_SKIP );

        res = Observer::self()->open_RenameDlg( this, !isDir ?
                                i18n("File Already Exists") : i18n("Already Exists as Folder"),
                                (*it).uSource.url(),
                                (*it).uDest.url(),
                                mode, newPath,
                              (*it).size, destsize,
                              (*it).ctime, destctime,
                              (*it).mtime, destmtime );

    }
    else
    {
        if ( job->error() == ERR_USER_CANCELED )
            res = R_CANCEL;
        else if ( !isInteractive() ) {
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
        else
        {
            SkipDlg_Result skipResult = Observer::self()->open_SkipDlg( this, files.count() > 1,
                                                                        job->errorString() );

            // Convert the return code from SkipDlg into a RenameDlg code
            res = ( skipResult == S_SKIP ) ? R_SKIP :
                         ( skipResult == S_AUTO_SKIP ) ? R_AUTO_SKIP :
                                        R_CANCEL;
        }
    }

    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT);

    removeSubjob( job );
    assert ( !hasSubjobs() );
    switch ( res ) {
        case R_CANCEL:
            setError( ERR_USER_CANCELED );
            emitResult();
            return;
        case R_RENAME:
        {
            KUrl newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit renamed( this, (*it).uDest, newUrl ); // for e.g. kpropsdlg
            (*it).uDest = newUrl;

            QList<CopyInfo> files;
            files.append(*it);
            emit aboutToCreate( this, files );
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            // Move on to next file
            skip( (*it).uSource );
            m_processedSize += (*it).size;
            files.erase( it );
            m_processedFiles++;
            break;
       case R_OVERWRITE_ALL:
            m_bOverwriteAll = true;
            break;
        case R_OVERWRITE:
            // Add to overwrite list, so that copyNextFile knows to overwrite
            m_overwriteList.append( (*it).uDest.path() );
            break;
        default:
            assert( 0 );
    }
    state = STATE_COPYING_FILES;
    //emit processedFiles( this, m_processedFiles );
    copyNextFile();
}

void CopyJob::copyNextFile()
{
    bool bCopyFile = false;
    //kDebug(7007) << "CopyJob::copyNextFile()" << endl;
    // Take the first file in the list
    QList<CopyInfo>::Iterator it = files.begin();
    // Is this URL on the skip list ?
    while (it != files.end() && !bCopyFile)
    {
        const QString destFile = (*it).uDest.path();
        bCopyFile = !shouldSkip( destFile );
        if ( !bCopyFile ) {
            files.erase( it );
            it = files.begin();
        }
    }

    if (bCopyFile) // any file to create, finally ?
    {
        // Do we set overwrite ?
        bool bOverwrite;
        const QString destFile = (*it).uDest.path();
        kDebug(7007) << "copying " << destFile << endl;
        if ( (*it).uDest == (*it).uSource )
            bOverwrite = false;
        else
            bOverwrite = shouldOverwrite( destFile );

        m_bCurrentOperationIsLink = false;
        KIO::Job * newjob = 0L;
        if ( m_mode == Link )
        {
            //kDebug(7007) << "Linking" << endl;
            if (
                ((*it).uSource.protocol() == (*it).uDest.protocol()) &&
                ((*it).uSource.host() == (*it).uDest.host()) &&
                ((*it).uSource.port() == (*it).uDest.port()) &&
                ((*it).uSource.user() == (*it).uDest.user()) &&
                ((*it).uSource.pass() == (*it).uDest.pass()) )
            {
                // This is the case of creating a real symlink
                KIO::SimpleJob *newJob = KIO::symlink( (*it).uSource.path(), (*it).uDest, bOverwrite, false /*no GUI*/ );
                newjob = newJob;
                Scheduler::scheduleJob(newJob);
                //kDebug(7007) << "CopyJob::copyNextFile : Linking target=" << (*it).uSource.path() << " link=" << (*it).uDest << endl;
                //emit linking( this, (*it).uSource.path(), (*it).uDest );
                m_bCurrentOperationIsLink = true;
                m_currentSrcURL=(*it).uSource;
                m_currentDestURL=(*it).uDest;
                d->m_bURLDirty = true;
                //Observer::self()->slotCopying( this, (*it).uSource, (*it).uDest ); // should be slotLinking perhaps
            } else {
                //kDebug(7007) << "CopyJob::copyNextFile : Linking URL=" << (*it).uSource << " link=" << (*it).uDest << endl;
                if ( (*it).uDest.isLocalFile() )
                {
                    // if the source is a devices url, handle it a littlebit special

                    QString path = (*it).uDest.path();
                    //kDebug(7007) << "CopyJob::copyNextFile path=" << path << endl;
                    QFile f( path );
                    if ( f.open( QIODevice::ReadWrite ) )
                    {
                        f.close();
                        KSimpleConfig config( path );
                        config.setDesktopGroup();
                        KUrl url = (*it).uSource;
                        url.setPass( "" );
                        config.writePathEntry( QString::fromLatin1("URL"), url.url() );
                        config.writeEntry( QString::fromLatin1("Name"), url.url() );
                        config.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("Link") );
                        QString protocol = (*it).uSource.protocol();
                        if ( protocol == QString::fromLatin1("ftp") )
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("ftp") );
                        else if ( protocol == QString::fromLatin1("http") )
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("www") );
                        else if ( protocol == QString::fromLatin1("info") )
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("info") );
                        else if ( protocol == QString::fromLatin1("mailto") )   // sven:
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("kmail") ); // added mailto: support
                        else
                            config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("unknown") );
                        config.sync();
                        files.erase( it );
                        m_processedFiles++;
                        //emit processedFiles( this, m_processedFiles );
                        copyNextFile();
                        return;
                    }
                    else
                    {
                        kDebug(7007) << "CopyJob::copyNextFile ERR_CANNOT_OPEN_FOR_WRITING" << endl;
                        setError( ERR_CANNOT_OPEN_FOR_WRITING );
                        setErrorText( (*it).uDest.path() );
                        emitResult();
                        return;
                    }
                } else {
                    // Todo: not show "link" on remote dirs if the src urls are not from the same protocol+host+...
                    setError( ERR_CANNOT_SYMLINK );
                    setErrorText( (*it).uDest.prettyUrl() );
                    emitResult();
                    return;
                }
            }
        }
        else if ( !(*it).linkDest.isEmpty() &&
                  ((*it).uSource.protocol() == (*it).uDest.protocol()) &&
                  ((*it).uSource.host() == (*it).uDest.host()) &&
                  ((*it).uSource.port() == (*it).uDest.port()) &&
                  ((*it).uSource.user() == (*it).uDest.user()) &&
                  ((*it).uSource.pass() == (*it).uDest.pass()))
            // Copying a symlink - only on the same protocol/host/etc. (#5601, downloading an FTP file through its link),
        {
            KIO::SimpleJob *newJob = KIO::symlink( (*it).linkDest, (*it).uDest, bOverwrite, false /*no GUI*/ );
            Scheduler::scheduleJob(newJob);
            newjob = newJob;
            //kDebug(7007) << "CopyJob::copyNextFile : Linking target=" << (*it).linkDest << " link=" << (*it).uDest << endl;
            //emit linking( this, (*it).linkDest, (*it).uDest );
            m_currentSrcURL = KUrl( (*it).linkDest );
            m_currentDestURL=(*it).uDest;
            d->m_bURLDirty = true;
            //Observer::self()->slotCopying( this, m_currentSrcURL, (*it).uDest ); // should be slotLinking perhaps
            m_bCurrentOperationIsLink = true;
            // NOTE: if we are moving stuff, the deletion of the source will be done in slotResultCopyingFiles
        } else if (m_mode == Move) // Moving a file
        {
            KIO::FileCopyJob * moveJob = KIO::file_move( (*it).uSource, (*it).uDest, (*it).permissions, bOverwrite, false, false/*no GUI*/ );
            moveJob->setSourceSize( (*it).size );
            newjob = moveJob;
            //kDebug(7007) << "CopyJob::copyNextFile : Moving " << (*it).uSource << " to " << (*it).uDest << endl;
            //emit moving( this, (*it).uSource, (*it).uDest );
            m_currentSrcURL=(*it).uSource;
            m_currentDestURL=(*it).uDest;
            d->m_bURLDirty = true;
            //Observer::self()->slotMoving( this, (*it).uSource, (*it).uDest );
        }
        else // Copying a file
        {
            // If source isn't local and target is local, we ignore the original permissions
            // Otherwise, files downloaded from HTTP end up with -r--r--r--
            bool remoteSource = !KProtocolManager::supportsListing((*it).uSource);
            int permissions = (*it).permissions;
            if ( d->m_defaultPermissions || ( remoteSource && (*it).uDest.isLocalFile() ) )
                permissions = -1;
            KIO::FileCopyJob * copyJob = KIO::file_copy( (*it).uSource, (*it).uDest, permissions, bOverwrite, false, false/*no GUI*/ );
            copyJob->setParentJob( this ); // in case of rename dialog
            copyJob->setSourceSize( (*it).size );
            copyJob->setModificationTime( (*it).mtime );
            newjob = copyJob;
            //kDebug(7007) << "CopyJob::copyNextFile : Copying " << (*it).uSource << " to " << (*it).uDest << endl;
            m_currentSrcURL=(*it).uSource;
            m_currentDestURL=(*it).uDest;
            d->m_bURLDirty = true;
        }
        addSubjob(newjob);
        connect( newjob, SIGNAL( processedSize( KJob*, qulonglong ) ),
                 this, SLOT( slotProcessedSize( KJob*, qulonglong ) ) );
        connect( newjob, SIGNAL( totalSize( KJob*, qulonglong ) ),
                 this, SLOT( slotTotalSize( KJob*, qulonglong ) ) );
    }
    else
    {
        // We're done
        //kDebug(7007) << "copyNextFile finished" << endl;
        deleteNextDir();
    }
}

void CopyJob::deleteNextDir()
{
    if ( m_mode == Move && !dirsToRemove.isEmpty() ) // some dirs to delete ?
    {
        state = STATE_DELETING_DIRS;
        d->m_bURLDirty = true;
        // Take first dir to delete out of list - last ones first !
        KUrl::List::Iterator it = --dirsToRemove.end();
        SimpleJob *job = KIO::rmdir( *it );
        Scheduler::scheduleJob(job);
        dirsToRemove.erase(it);
        addSubjob( job );
    }
    else
    {
        // This step is done, move on
        setNextDirAttribute();
    }
}

void CopyJob::setNextDirAttribute()
{
    if ( !d->m_directoriesCopied.isEmpty() )
    {
        state = STATE_SETTING_DIR_ATTRIBUTES;
#ifdef Q_OS_UNIX
        // TODO KDE4: this should use a SlaveBase method, but we have none yet in KDE3.
        QLinkedList<CopyInfo>::Iterator it = d->m_directoriesCopied.begin();
        for ( ; it != d->m_directoriesCopied.end() ; ++it ) {
            const KUrl& url = (*it).uDest;
            if ( url.isLocalFile() && (*it).mtime != (time_t)-1 ) {
                const QByteArray path = QFile::encodeName( url.path() );
                KDE_struct_stat statbuf;
                if (KDE_lstat(path, &statbuf) == 0) {
                    struct utimbuf utbuf;
                    utbuf.actime = statbuf.st_atime; // access time, unchanged
                    utbuf.modtime = (*it).mtime; // modification time
                    utime( path, &utbuf );
                }

            }
        }
#endif
        d->m_directoriesCopied.clear();
    }

    // No "else" here, since the above is a simple sync loop

    {
        // Finished - tell the world
        if ( !m_bOnlyRenames )
        {
            KUrl url( d->m_globalDest );
            if ( d->m_globalDestinationState != DEST_IS_DIR || m_asMethod )
                url.setPath( url.directory() );
            //kDebug(7007) << "KDirNotify'ing FilesAdded " << url << endl;
            org::kde::KDirNotify::emitFilesAdded( url.url() );

            if ( m_mode == Move && !m_srcList.isEmpty() ) {
                //kDebug(7007) << "KDirNotify'ing FilesRemoved " << m_srcList.toStringList() << endl;
                org::kde::KDirNotify::emitFilesRemoved( m_srcList.toStringList() );
            }
        }
        if (m_reportTimer)
            m_reportTimer->stop();
        --m_processedFiles; // undo the "start at 1" hack
        slotReport(); // display final numbers, important if progress dialog stays up

        emitResult();
    }
}

void CopyJob::slotProcessedSize( KJob*, qulonglong data_size )
{
  //kDebug(7007) << "CopyJob::slotProcessedSize " << data_size << endl;
  m_fileProcessedSize = data_size;
  setProcessedSize(m_processedSize + m_fileProcessedSize);

  if ( m_processedSize + m_fileProcessedSize > m_totalSize )
  {
    m_totalSize = m_processedSize + m_fileProcessedSize;
    //kDebug(7007) << "Adjusting m_totalSize to " << m_totalSize << endl;
    emit totalSize( this, m_totalSize ); // safety
  }
  //kDebug(7007) << "emit processedSize " << (unsigned long) (m_processedSize + m_fileProcessedSize) << endl;
  emit processedSize( this, m_processedSize + m_fileProcessedSize );
  emitPercent( m_processedSize + m_fileProcessedSize, m_totalSize );
}

void CopyJob::slotTotalSize( KJob*, qulonglong size )
{
  //kDebug(7007) << "slotTotalSize: " << size << endl;
  // Special case for copying a single file
  // This is because some protocols don't implement stat properly
  // (e.g. HTTP), and don't give us a size in some cases (redirection)
  // so we'd rather rely on the size given for the transfer
  if ( m_bSingleFileCopy && size > m_totalSize)
  {
    //kDebug(7007) << "slotTotalSize: updating totalsize to " << size << endl;
    m_totalSize = size;
    emit totalSize( this, size );
  }
}

void CopyJob::slotResultDeletingDirs( KJob * job )
{
    if (job->error())
    {
        // Couldn't remove directory. Well, perhaps it's not empty
        // because the user pressed Skip for a given file in it.
        // Let's not display "Could not remove dir ..." for each of those dir !
    }
    removeSubjob( job );
    assert ( !hasSubjobs() );
    deleteNextDir();
}

#if 0 // TODO KDE4
void CopyJob::slotResultSettingDirAttributes( KJob * job )
{
    if (job->error())
    {
        // Couldn't set directory attributes. Ignore the error, it can happen
        // with inferior file systems like VFAT.
        // Let's not display warnings for each dir like "cp -a" does.
    }
    subjobs.remove( job );
    assert ( subjobs.isEmpty() );
    setNextDirAttribute();
}
#endif

void CopyJob::slotResultRenaming( KJob* job )
{
    int err = job->error();
    const QString errText = job->errorText();
    removeSubjob( job, true ); // merge metadata
    assert ( !hasSubjobs() );
    // Determine dest again
    KUrl dest = m_dest;
    if ( destinationState == DEST_IS_DIR && !m_asMethod )
        dest.addPath( m_currentSrcURL.fileName() );
    if ( err )
    {
        // Direct renaming didn't work. Try renaming to a temp name,
        // this can help e.g. when renaming 'a' to 'A' on a VFAT partition.
        // In that case it's the _same_ dir, we don't want to copy+del (data loss!)
      if ( m_currentSrcURL.isLocalFile() && m_currentSrcURL.url(KUrl::RemoveTrailingSlash) != dest.url(KUrl::RemoveTrailingSlash) &&
           m_currentSrcURL.url(KUrl::RemoveTrailingSlash).toLower() == dest.url(KUrl::RemoveTrailingSlash).toLower() &&
             ( err == ERR_FILE_ALREADY_EXIST ||
               err == ERR_DIR_ALREADY_EXIST ||
               err == ERR_IDENTICAL_FILES ) )
        {
            kDebug(7007) << "Couldn't rename directly, dest already exists. Detected special case of lower/uppercase renaming in same dir, try with 2 rename calls" << endl;
            QByteArray _src( QFile::encodeName(m_currentSrcURL.path()) );
            QByteArray _dest( QFile::encodeName(dest.path()) );
            KTempFile tmpFile( m_currentSrcURL.directory(KUrl::ObeyTrailingSlash) );
            QByteArray _tmp( QFile::encodeName(tmpFile.name()) );
            kDebug(7007) << "CopyJob::slotResult KTempFile status:" << tmpFile.status() << " using " << _tmp << " as intermediary" << endl;
            tmpFile.unlink();
            if ( ::rename( _src, _tmp ) == 0 )
            {
                if ( !QFile::exists( _dest ) && ::rename( _tmp, _dest ) == 0 )
                {
                    kDebug(7007) << "Success." << endl;
                    err = 0;
                }
                else
                {
                    // Revert back to original name!
                    if ( ::rename( _tmp, _src ) != 0 ) {
                        kError(7007) << "Couldn't rename " << tmpFile.name() << " back to " << _src << " !" << endl;
                        // Severe error, abort
                        Job::slotResult( job ); // will set the error and emit result(this)
                        return;
                    }
                }
            }
        }
    }
    if ( err )
    {
        // This code is similar to CopyJob::slotResultConflictCopyingFiles
        // but here it's about the base src url being moved/renamed
        // (*m_currentStatSrc) and its dest (m_dest), not about a single file.
        // It also means we already stated the dest, here.
        // On the other hand we haven't stated the src yet (we skipped doing it
        // to save time, since it's not necessary to rename directly!)...

        Q_ASSERT( m_currentSrcURL == *m_currentStatSrc );

        // Existing dest?
        if ( ( err == ERR_DIR_ALREADY_EXIST ||
               err == ERR_FILE_ALREADY_EXIST ||
               err == ERR_IDENTICAL_FILES )
             && isInteractive() )
        {
            if (m_reportTimer)
                m_reportTimer->stop();

            // Should we skip automatically ?
            if ( m_bAutoSkip ) {
                // Move on to next file
                skipSrc();
                return;
            } else if ( m_bOverwriteAll ) {
                ; // nothing to do, stat+copy+del will overwrite
            } else {
                QString newPath;
                // If src==dest, use "overwrite-itself"
                RenameDlg_Mode mode = (RenameDlg_Mode)
                                      ( ( m_currentSrcURL == dest ) ? M_OVERWRITE_ITSELF : M_OVERWRITE );

                if ( m_srcList.count() > 1 )
                    mode = (RenameDlg_Mode) ( mode | M_MULTI | M_SKIP );
                else
                    mode = (RenameDlg_Mode) ( mode | M_SINGLE );

                // we lack mtime info for both the src (not stated)
                // and the dest (stated but this info wasn't stored)
                // Let's do it for local files, at least
                KIO::filesize_t sizeSrc = (KIO::filesize_t) -1;
                KIO::filesize_t sizeDest = (KIO::filesize_t) -1;
                time_t ctimeSrc = (time_t) -1;
                time_t ctimeDest = (time_t) -1;
                time_t mtimeSrc = (time_t) -1;
                time_t mtimeDest = (time_t) -1;

                KDE_struct_stat stat_buf;
                if ( m_currentSrcURL.isLocalFile() &&
                    KDE_stat(QFile::encodeName(m_currentSrcURL.path()), &stat_buf) == 0 ) {
                    sizeSrc = stat_buf.st_size;
                    ctimeSrc = stat_buf.st_ctime;
                    mtimeSrc = stat_buf.st_mtime;
                }
                if ( dest.isLocalFile() &&
                    KDE_stat(QFile::encodeName(dest.path()), &stat_buf) == 0 ) {
                    sizeDest = stat_buf.st_size;
                    ctimeDest = stat_buf.st_ctime;
                    mtimeDest = stat_buf.st_mtime;
                }

                RenameDlg_Result r = Observer::self()->open_RenameDlg(
                    this,
                    err != ERR_DIR_ALREADY_EXIST ? i18n("File Already Exists") : i18n("Already Exists as Folder"),
                    m_currentSrcURL.url(),
                    dest.url(),
                    mode, newPath,
                    sizeSrc, sizeDest,
                    ctimeSrc, ctimeDest,
                    mtimeSrc, mtimeDest );
                if (m_reportTimer)
                    m_reportTimer->start(REPORT_TIMEOUT);

                switch ( r )
                {
                case R_CANCEL:
                {
                    setError( ERR_USER_CANCELED );
                    emitResult();
                    return;
                }
                case R_RENAME:
                {
                    // Set m_dest to the chosen destination
                    // This is only for this src url; the next one will revert to d->m_globalDest
                    m_dest.setPath( newPath );
                    KIO::Job* job = KIO::stat( m_dest, false, 2, false );
                    state = STATE_STATING;
                    destinationState = DEST_NOT_STATED;
                    addSubjob(job);
                    return;
                }
                case R_AUTO_SKIP:
                    m_bAutoSkip = true;
                    // fall through
                case R_SKIP:
                    // Move on to next file
                    skipSrc();
                    return;
                case R_OVERWRITE_ALL:
                    m_bOverwriteAll = true;
                    break;
                case R_OVERWRITE:
                    // Add to overwrite list
                    // Note that we add dest, not m_dest.
                    // This ensures that when moving several urls into a dir (m_dest),
                    // we only overwrite for the current one, not for all.
                    // When renaming a single file (m_asMethod), it makes no difference.
                    kDebug(7007) << "adding to overwrite list: " << dest.path() << endl;
                    m_overwriteList.append( dest.path() );
                    break;
                default:
                    //assert( 0 );
                    break;
                }
            }
        } else if ( err != KIO::ERR_UNSUPPORTED_ACTION ) {
            kDebug(7007) << "Couldn't rename " << m_currentSrcURL << " to " << dest << ", aborting" << endl;
            setError( err );
            setErrorText( errText );
            emitResult();
            return;
        }
        kDebug(7007) << "Couldn't rename " << m_currentSrcURL << " to " << dest << ", reverting to normal way, starting with stat" << endl;
        //kDebug(7007) << "KIO::stat on " << m_currentSrcURL << endl;
        KIO::Job* job = KIO::stat( m_currentSrcURL, true, 2, false );
        state = STATE_STATING;
        addSubjob(job);
        m_bOnlyRenames = false;
    }
    else
    {
        //kDebug(7007) << "Renaming succeeded, move on" << endl;
        emit copyingDone( this, *m_currentStatSrc, dest, true, true );
        statNextSrc();
    }
}

void CopyJob::slotResult( KJob *job )
{
    //kDebug(7007) << "CopyJob::slotResult() state=" << (int) state << endl;
    // In each case, what we have to do is :
    // 1 - check for errors and treat them
    // 2 - removeSubjob(job);
    // 3 - decide what to do next

    switch ( state ) {
        case STATE_STATING: // We were trying to stat a src url or the dest
            slotResultStating( job );
            break;
        case STATE_RENAMING: // We were trying to do a direct renaming, before even stat'ing
        {
            slotResultRenaming( job );
            break;
        }
        case STATE_LISTING: // recursive listing finished
            //kDebug(7007) << "totalSize: " << (unsigned int) m_totalSize << " files: " << files.count() << " dirs: " << dirs.count() << endl;
            // Was there an error ?
            if (job->error())
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            removeSubjob( job );
            assert ( !hasSubjobs() );

            statNextSrc();
            break;
        case STATE_CREATING_DIRS:
            slotResultCreatingDirs( job );
            break;
        case STATE_CONFLICT_CREATING_DIRS:
            slotResultConflictCreatingDirs( job );
            break;
        case STATE_COPYING_FILES:
            slotResultCopyingFiles( job );
            break;
        case STATE_CONFLICT_COPYING_FILES:
            slotResultConflictCopyingFiles( job );
            break;
        case STATE_DELETING_DIRS:
            slotResultDeletingDirs( job );
            break;
        case STATE_SETTING_DIR_ATTRIBUTES: // TODO KDE4
            assert( 0 );
            //slotResultSettingDirAttributes( job );
            break;
        default:
            assert( 0 );
    }
}

void KIO::CopyJob::setDefaultPermissions( bool b )
{
    d->m_defaultPermissions = b;
}

CopyJob *KIO::copy(const KUrl& src, const KUrl& dest, bool showProgressInfo )
{
    //kDebug(7007) << "KIO::copy src=" << src << " dest=" << dest << endl;
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Copy, false, showProgressInfo );
}

CopyJob *KIO::copyAs(const KUrl& src, const KUrl& dest, bool showProgressInfo )
{
    //kDebug(7007) << "KIO::copyAs src=" << src << " dest=" << dest << endl;
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Copy, true, showProgressInfo );
}

CopyJob *KIO::copy( const KUrl::List& src, const KUrl& dest, bool showProgressInfo )
{
    //kdDebug(7007) << src << " " << dest << endl;
    return new CopyJob( src, dest, CopyJob::Copy, false, showProgressInfo );
}

CopyJob *KIO::move(const KUrl& src, const KUrl& dest, bool showProgressInfo )
{
    //kDebug(7007) << src << " " << dest << endl;
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Move, false, showProgressInfo );
}

CopyJob *KIO::moveAs(const KUrl& src, const KUrl& dest, bool showProgressInfo )
{
    //kDebug(7007) << src << " " << dest << endl;
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Move, true, showProgressInfo );
}

CopyJob *KIO::move( const KUrl::List& src, const KUrl& dest, bool showProgressInfo )
{
    //kdDebug(7007) << src << " " << dest << endl;
    return new CopyJob( src, dest, CopyJob::Move, false, showProgressInfo );
}

CopyJob *KIO::link(const KUrl& src, const KUrl& destDir, bool showProgressInfo )
{
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

CopyJob *KIO::link(const KUrl::List& srcList, const KUrl& destDir, bool showProgressInfo )
{
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

CopyJob *KIO::linkAs(const KUrl& src, const KUrl& destDir, bool showProgressInfo )
{
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

CopyJob *KIO::trash(const KUrl& src, bool showProgressInfo )
{
    KUrl::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, KUrl( "trash:/" ), CopyJob::Move, false, showProgressInfo );
}

CopyJob *KIO::trash(const KUrl::List& srcList, bool showProgressInfo )
{
    return new CopyJob( srcList, KUrl( "trash:/" ), CopyJob::Move, false, showProgressInfo );
}

//////////

DeleteJob::DeleteJob( const KUrl::List& src, bool /*shred*/, bool showProgressInfo )
: Job(showProgressInfo), m_totalSize( 0 ), m_processedSize( 0 ), m_fileProcessedSize( 0 ),
  m_processedFiles( 0 ), m_processedDirs( 0 ), m_totalFilesDirs( 0 ),
  m_srcList(src), m_currentStat(m_srcList.begin()), m_reportTimer(0)
{
  if ( showProgressInfo ) {

     connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
              Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );

     connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
              Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

     // See slotReport
     /*connect( this, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
      m_observer, SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );

      connect( this, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
      m_observer, SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

      connect( this, SIGNAL( deleting( KIO::Job*, const KUrl& ) ),
      m_observer, SLOT( slotDeleting( KIO::Job*, const KUrl& ) ) );*/

     m_reportTimer=new QTimer(this);
     connect(m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
     //this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
     m_reportTimer->start(REPORT_TIMEOUT);
  }

  QTimer::singleShot(0, this, SLOT(slotStart()));
}

void DeleteJob::slotStart()
{
  statNextSrc();
}

//this is called often, so calling the functions
//from Observer here directly might improve the performance a little bit
//aleXXX
void DeleteJob::slotReport()
{
   if (progressId()==0)
      return;

   Observer * observer = Observer::self();

   emit deleting( this, m_currentURL );
   observer->slotDeleting(this,m_currentURL);

   switch( state ) {
        case STATE_STATING:
        case STATE_LISTING:
            emit totalSize( this, m_totalSize );
            emit totalFiles( this, files.count() );
            emit totalDirs( this, dirs.count() );
            break;
        case STATE_DELETING_DIRS:
            emit processedDirs( this, m_processedDirs );
            observer->slotProcessedDirs(this,m_processedDirs);
            emitPercent( m_processedFiles + m_processedDirs, m_totalFilesDirs );
            break;
        case STATE_DELETING_FILES:
            observer->slotProcessedFiles(this,m_processedFiles);
            emit processedFiles( this, m_processedFiles );
            emitPercent( m_processedFiles, m_totalFilesDirs );
            break;
   }
}


void DeleteJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    UDSEntryList::ConstIterator it = list.begin();
    const UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it)
    {
        const UDSEntry& entry = *it;
        const QString displayName = entry.stringValue( KIO::UDS_NAME );

        assert(!displayName.isEmpty());
        if (displayName != ".." && displayName != ".")
        {
            KUrl url;
            const QString urlStr = entry.stringValue( KIO::UDS_URL );
            if ( !urlStr.isEmpty() )
                url = urlStr;
            else {
                url = ((SimpleJob *)job)->url(); // assumed to be a dir
                url.addPath( displayName );
            }

            m_totalSize += (KIO::filesize_t)entry.numberValue( KIO::UDS_SIZE, 0 );

            //kDebug(7007) << "DeleteJob::slotEntries " << displayName << " (" << url << ")" << endl;
            if ( entry.isLink() )
                symlinks.append( url );
            else if ( entry.isDir() )
                dirs.append( url );
            else
                files.append( url );
        }
    }
}


void DeleteJob::statNextSrc()
{
    //kDebug(7007) << "statNextSrc" << endl;
    if ( m_currentStat != m_srcList.end() )
    {
        m_currentURL = (*m_currentStat);

        // if the file system doesn't support deleting, we do not even stat
        if (!KProtocolManager::supportsDeleting(m_currentURL)) {
            QPointer<DeleteJob> that = this;
            ++m_currentStat;
            emit warning( this, buildErrorString(ERR_CANNOT_DELETE, m_currentURL.prettyUrl()) );
            if (that)
                statNextSrc();
            return;
        }
        // Stat it
        state = STATE_STATING;
        KIO::SimpleJob * job = KIO::stat( m_currentURL, true, 1, false );
        Scheduler::scheduleJob(job);
        //kDebug(7007) << "KIO::stat (DeleteJob) " << m_currentURL << endl;
        addSubjob(job);
        //if ( progressId() ) // Did we get an ID from the observer ?
        //  Observer::self()->slotDeleting( this, *it ); // show asap
    } else
    {
        m_totalFilesDirs = files.count()+symlinks.count() + dirs.count();
        slotReport();
        // Now we know which dirs hold the files we're going to delete.
        // To speed things up and prevent double-notification, we disable KDirWatch
        // on those dirs temporarily (using KDirWatch::self, that's the instanced
        // used by e.g. kdirlister).
        for ( QStringList::Iterator it = m_parentDirs.begin() ; it != m_parentDirs.end() ; ++it )
            KDirWatch::self()->stopDirScan( *it );
        state = STATE_DELETING_FILES;
        deleteNextFile();
    }
}

void DeleteJob::deleteNextFile()
{
    //kDebug(7007) << "deleteNextFile" << endl;
    if ( !files.isEmpty() || !symlinks.isEmpty() )
    {
        SimpleJob *job;
        do {
            // Take first file to delete out of list
            KUrl::List::Iterator it = files.begin();
            bool isLink = false;
            if ( it == files.end() ) // No more files
            {
                it = symlinks.begin(); // Pick up a symlink to delete
                isLink = true;
            }
            // Normal deletion
            // If local file, try do it directly
            if ( (*it).isLocalFile() && unlink( QFile::encodeName((*it).path()) ) == 0 ) {
                //kdDebug(7007) << "DeleteJob deleted " << (*it).path() << endl;
                job = 0;
                m_processedFiles++;
                if ( m_processedFiles % 300 == 0 || m_totalFilesDirs < 300) { // update progress info every 300 files
                    m_currentURL = *it;
                    slotReport();
                }
            } else
            { // if remote - or if unlink() failed (we'll use the job's error handling in that case)
                job = KIO::file_delete( *it, false /*no GUI*/);
                Scheduler::scheduleJob(job);
                m_currentURL=(*it);
            }
            if ( isLink )
                symlinks.erase(it);
            else
                files.erase(it);
            if ( job ) {
                addSubjob(job);
                return;
            }
            // loop only if direct deletion worked (job=0) and there is something else to delete
        } while (!job && (!files.isEmpty() || !symlinks.isEmpty()));
    }
    state = STATE_DELETING_DIRS;
    deleteNextDir();
}

void DeleteJob::deleteNextDir()
{
    if ( !dirs.isEmpty() ) // some dirs to delete ?
    {
        do {
            // Take first dir to delete out of list - last ones first !
            KUrl::List::Iterator it = --dirs.end();
            // If local dir, try to rmdir it directly
            if ( (*it).isLocalFile() && ::rmdir( QFile::encodeName((*it).path()) ) == 0 ) {

                m_processedDirs++;
                if ( m_processedDirs % 100 == 0 ) { // update progress info every 100 dirs
                    m_currentURL = *it;
                    slotReport();
                }
            } else {
                SimpleJob* job;
                if ( KProtocolManager::canDeleteRecursive( *it ) ) {
                    // If the ioslave supports recursive deletion of a directory, then
                    // we only need to send a single CMD_DEL command, so we use file_delete :)
                    job = KIO::file_delete( *it, false /*no gui*/ );
                } else {
                    job = KIO::rmdir( *it );
                }
                Scheduler::scheduleJob(job);
                dirs.erase(it);
                addSubjob( job );
                return;
            }
            dirs.erase(it);
        } while ( !dirs.isEmpty() );
    }

    // Re-enable watching on the dirs that held the deleted files
    for ( QStringList::Iterator it = m_parentDirs.begin() ; it != m_parentDirs.end() ; ++it )
        KDirWatch::self()->restartDirScan( *it );

    // Finished - tell the world
    if ( !m_srcList.isEmpty() )
    {
        //kDebug(7007) << "KDirNotify'ing FilesRemoved " << m_srcList.toStringList() << endl;
        org::kde::KDirNotify::emitFilesRemoved( m_srcList.toStringList() );
    }
    if (m_reportTimer!=0)
       m_reportTimer->stop();
    emitResult();
}

void DeleteJob::slotProcessedSize( KJob*, qulonglong data_size )
{
   // Note: this is the same implementation as CopyJob::slotProcessedSize but
   // it's different from FileCopyJob::slotProcessedSize - which is why this
   // is not in Job.

   m_fileProcessedSize = data_size;
   setProcessedSize(m_processedSize + m_fileProcessedSize);

   //kDebug(7007) << "DeleteJob::slotProcessedSize " << (unsigned int) (m_processedSize + m_fileProcessedSize) << endl;

   emit processedSize( this, m_processedSize + m_fileProcessedSize );

   // calculate percents
   unsigned long ipercent = percent();

   if ( m_totalSize == 0 )
      setPercent( 100 );
   else
      setPercent( (unsigned long)(( (float)(m_processedSize + m_fileProcessedSize) / (float)m_totalSize ) * 100.0) );

   if ( percent() > ipercent )
   {
      emit percent( this, percent() );
      //kDebug(7007) << "DeleteJob::slotProcessedSize - percent =  " << (unsigned int) m_percent << endl;
   }

}

void DeleteJob::slotResult( KJob *job )
{
    switch ( state )
    {
    case STATE_STATING:
    {
        // Was there an error while stating ?
        if (job->error() )
        {
            // Probably : doesn't exist
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }

        const UDSEntry entry = static_cast<StatJob*>(job)->statResult();
        const KUrl url = static_cast<SimpleJob*>(job)->url();
        const bool isLink = entry.isLink();

        removeSubjob( job );
        assert( !hasSubjobs() );

        // Is it a file or a dir ?
        if (entry.isDir() && !isLink)
        {
            // Add toplevel dir in list of dirs
            dirs.append( url );
            if ( url.isLocalFile() && !m_parentDirs.contains( url.path(KUrl::RemoveTrailingSlash) ) )
              m_parentDirs.append( url.path(KUrl::RemoveTrailingSlash) );

            if ( !KProtocolManager::canDeleteRecursive( url ) ) {
                //kDebug(7007) << " Target is a directory " << endl;
                // List it
                state = STATE_LISTING;
                ListJob *newjob = listRecursive( url, false );
                newjob->setUnrestricted(true); // No KIOSK restrictions
                Scheduler::scheduleJob(newjob);
                connect(newjob, SIGNAL(entries( KIO::Job *,
                                                const KIO::UDSEntryList& )),
                        SLOT( slotEntries( KIO::Job*,
                                           const KIO::UDSEntryList& )));
                addSubjob(newjob);
            } else {
                ++m_currentStat;
                statNextSrc();
            }
        }
        else
        {
            if ( isLink ) {
                //kDebug(7007) << " Target is a symlink" << endl;
                symlinks.append( url );
            } else {
                //kDebug(7007) << " Target is a file" << endl;
                files.append( url );
            }
            if ( url.isLocalFile() && !m_parentDirs.contains( url.directory(KUrl::ObeyTrailingSlash) ) )
                m_parentDirs.append( url.directory(KUrl::ObeyTrailingSlash) );
            ++m_currentStat;
            statNextSrc();
        }
    }
        break;
    case STATE_LISTING:
        if ( job->error() )
        {
            // Try deleting nonetheless, it may be empty (and non-listable)
        }
        removeSubjob( job );
        assert( !hasSubjobs() );
        ++m_currentStat;
        statNextSrc();
        break;
    case STATE_DELETING_FILES:
        if ( job->error() )
        {
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
        removeSubjob( job );
        assert( !hasSubjobs() );
        m_processedFiles++;

        deleteNextFile();
        break;
    case STATE_DELETING_DIRS:
        if ( job->error() )
        {
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
        removeSubjob( job );
        assert( !hasSubjobs() );
        m_processedDirs++;
        //emit processedDirs( this, m_processedDirs );
        //if (!m_shred)
        //emitPercent( m_processedFiles + m_processedDirs, m_totalFilesDirs );

        deleteNextDir();
        break;
    default:
        assert(0);
    }
}

DeleteJob *KIO::del( const KUrl& src, bool shred, bool showProgressInfo )
{
  KUrl::List srcList;
  srcList.append( src );
  DeleteJob *job = new DeleteJob( srcList, shred, showProgressInfo );
  return job;
}

DeleteJob *KIO::del( const KUrl::List& src, bool shred, bool showProgressInfo )
{
  DeleteJob *job = new DeleteJob( src, shred, showProgressInfo );
  return job;
}

MultiGetJob::MultiGetJob(const KUrl& url,
                         bool showProgressInfo)
 : TransferJob(url, 0, QByteArray(), QByteArray(), showProgressInfo),
   m_currentEntry( 0, KUrl(), MetaData() )
{
}

MultiGetJob::~MultiGetJob()
{
}

void MultiGetJob::get(long id, const KUrl &url, const MetaData &metaData)
{
   GetRequest entry(id, url, metaData);
   entry.metaData["request-id"] = QString::number(id);
   m_waitQueue.append(entry);
}

void MultiGetJob::flushQueue(RequestQueue &queue)
{
   // Use multi-get
   // Scan all jobs in m_waitQueue
   RequestQueue::iterator wqit = m_waitQueue.begin();
   const RequestQueue::iterator wqend = m_waitQueue.end();
   while ( wqit != wqend )
   {
       const GetRequest& entry = *wqit;
      if ((m_url.protocol() == entry.url.protocol()) &&
          (m_url.host() == entry.url.host()) &&
          (m_url.port() == entry.url.port()) &&
          (m_url.user() == entry.url.user()))
      {
         queue.append( entry );
         wqit = m_waitQueue.erase( wqit );
      }
      else
      {
         ++wqit;
      }
   }
   // Send number of URLs, (URL, metadata)*
   KIO_ARGS << (qint32) queue.count();
   RequestQueue::const_iterator qit = queue.begin();
   const RequestQueue::const_iterator qend = queue.end();
   for( ; qit != qend; ++qit )
   {
      stream << (*qit).url << (*qit).metaData;
   }
   m_packedArgs = packedArgs;
   m_command = CMD_MULTI_GET;
   m_outgoingMetaData.clear();
}

void MultiGetJob::start(Slave *slave)
{
   // Add first job from m_waitQueue and add it to m_activeQueue
   GetRequest entry = m_waitQueue.takeFirst();
   m_activeQueue.append(entry);

   m_url = entry.url;

   if (!entry.url.protocol().startsWith("http"))
   {
      // Use normal get
      KIO_ARGS << entry.url;
      m_packedArgs = packedArgs;
      m_outgoingMetaData = entry.metaData;
      m_command = CMD_GET;
      b_multiGetActive = false;
   }
   else
   {
      flushQueue(m_activeQueue);
      b_multiGetActive = true;
   }

   TransferJob::start(slave); // Anything else to do??
}

bool MultiGetJob::findCurrentEntry()
{
   if (b_multiGetActive)
   {
      long id = m_incomingMetaData["request-id"].toLong();
      RequestQueue::const_iterator qit = m_activeQueue.begin();
      const RequestQueue::const_iterator qend = m_activeQueue.end();
      for( ; qit != qend; ++qit )
      {
         if ((*qit).id == id)
         {
            m_currentEntry = *qit;
            return true;
         }
      }
      m_currentEntry.id = 0;
      return false;
   }
   else
   {
      if ( m_activeQueue.isEmpty() )
        return false;
      m_currentEntry = m_activeQueue.first();
      return true;
   }
}

void MultiGetJob::slotRedirection( const KUrl &url)
{
  if (!findCurrentEntry()) return; // Error
  if (!KAuthorized::authorizeURLAction("redirect", m_url, url))
  {
     kWarning(7007) << "MultiGetJob: Redirection from " << m_currentEntry.url << " to " << url << " REJECTED!" << endl;
     return;
  }
  m_redirectionURL = url;
  if (m_currentEntry.url.hasUser() && !url.hasUser() && (m_currentEntry.url.host().toLower() == url.host().toLower()))
      m_redirectionURL.setUser(m_currentEntry.url.user()); // Preserve user
  get(m_currentEntry.id, m_redirectionURL, m_currentEntry.metaData); // Try again
}


void MultiGetJob::slotFinished()
{
  if (!findCurrentEntry()) return;
  if (m_redirectionURL.isEmpty())
  {
     // No redirection, tell the world that we are finished.
     emit result(m_currentEntry.id);
  }
  m_redirectionURL = KUrl();
  setError( 0 );
  m_incomingMetaData.clear();
  m_activeQueue.removeAll(m_currentEntry);
  if (m_activeQueue.count() == 0)
  {
     if (m_waitQueue.count() == 0)
     {
        // All done
        TransferJob::slotFinished();
     }
     else
     {
        // return slave to pool
        // fetch new slave for first entry in m_waitQueue and call start
        // again.
        m_url = m_waitQueue.first().url;
        slaveDone();
        Scheduler::doJob(this);
     }
  }
}

void MultiGetJob::slotData( const QByteArray &_data)
{
  if(m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error())
     emit data(m_currentEntry.id, _data);
}

void MultiGetJob::slotMimetype( const QString &_mimetype )
{
  if (b_multiGetActive)
  {
     RequestQueue newQueue;
     flushQueue(newQueue);
     if (!newQueue.isEmpty())
     {
        m_activeQueue += newQueue;
        m_slave->send( m_command, m_packedArgs );
     }
  }
  if (!findCurrentEntry()) return; // Error, unknown request!
  emit mimetype(m_currentEntry.id, _mimetype);
}

MultiGetJob *KIO::multi_get(long id, const KUrl &url, const MetaData &metaData)
{
    MultiGetJob * job = new MultiGetJob( url, false );
    job->get(id, url, metaData);
    return job;
}


#ifdef CACHE_INFO
CacheInfo::CacheInfo(const KUrl &url)
{
    m_url = url;
}

QString CacheInfo::cachedFileName()
{
   const QChar separator = '_';

   QString CEF = m_url.path();

   int p = CEF.find('/');

   while(p != -1)
   {
      CEF[p] = separator;
      p = CEF.find('/', p);
   }

   QString host = m_url.host().toLower();
   CEF = host + CEF + '_';

   QString dir = KProtocolManager::cacheDir();
   if (dir[dir.length()-1] != '/')
      dir += '/';

   int l = m_url.host().length();
   for(int i = 0; i < l; i++)
   {
      if (host[i].isLetter() && (host[i] != 'w'))
      {
         dir += host[i];
         break;
      }
   }
   if (dir[dir.length()-1] == '/')
      dir += '0';

   unsigned long hash = 0x00000000;
   QString u = m_url.url().toLatin1();
   for(int i = u.length(); i--;)
   {
      hash = (hash * 12211 + u[i]) % 2147483563;
   }

   QString hashString;
   hashString.sprintf("%08lx", hash);

   CEF = CEF + hashString;

   CEF = dir + '/' + CEF;

   return CEF;
}

QFile *CacheInfo::cachedFile()
{
#ifdef Q_WS_WIN
   const char *mode = (readWrite ? "rb+" : "rb");
#else
   const char *mode = (readWrite ? "r+" : "r");
#endif

   FILE *fs = fopen(QFile::encodeName(CEF), mode); // Open for reading and writing
   if (!fs)
      return 0;

   char buffer[401];
   bool ok = true;

  // CacheRevision
  if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   time_t date;
   time_t currentDate = time(0);

   // URL
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      int l = strlen(buffer);
      if (l>0)
         buffer[l-1] = 0; // Strip newline
      if (m_.url.url() != buffer)
      {
         ok = false; // Hash collision
      }
   }

   // Creation Date
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      date = (time_t) strtoul(buffer, 0, 10);
      if (m_maxCacheAge && (difftime(currentDate, date) > m_maxCacheAge))
      {
         m_bMustRevalidate = true;
         m_expireDate = currentDate;
      }
   }

   // Expiration Date
   m_cacheExpireDateOffset = ftell(fs);
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      if (m_request.cache == CC_Verify)
      {
         date = (time_t) strtoul(buffer, 0, 10);
         // After the expire date we need to revalidate.
         if (!date || difftime(currentDate, date) >= 0)
            m_bMustRevalidate = true;
         m_expireDate = date;
      }
   }

   // ETag
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_etag = QString(buffer).trimmed();
   }

   // Last-Modified
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_lastModified = QString(buffer).trimmed();
   }

   fclose(fs);

   if (ok)
      return fs;

   unlink( QFile::encodeName(CEF) );
   return 0;

}

void CacheInfo::flush()
{
    cachedFile().remove();
}

void CacheInfo::touch()
{

}
void CacheInfo::setExpireDate(int);
void CacheInfo::setExpireTimeout(int);


int CacheInfo::creationDate();
int CacheInfo::expireDate();
int CacheInfo::expireTimeout();
#endif

void Job::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void SimpleJob::virtual_hook( int id, void* data )
{ KIO::Job::virtual_hook( id, data ); }

void MkdirJob::virtual_hook( int id, void* data )
{ SimpleJob::virtual_hook( id, data ); }

void StatJob::virtual_hook( int id, void* data )
{ SimpleJob::virtual_hook( id, data ); }

void TransferJob::virtual_hook( int id, void* data )
{ SimpleJob::virtual_hook( id, data ); }

void MultiGetJob::virtual_hook( int id, void* data )
{ TransferJob::virtual_hook( id, data ); }

void MimetypeJob::virtual_hook( int id, void* data )
{ TransferJob::virtual_hook( id, data ); }

void FileCopyJob::virtual_hook( int id, void* data )
{ Job::virtual_hook( id, data ); }

void ListJob::virtual_hook( int id, void* data )
{ SimpleJob::virtual_hook( id, data ); }

void CopyJob::virtual_hook( int id, void* data )
{ Job::virtual_hook( id, data ); }

void DeleteJob::virtual_hook( int id, void* data )
{ Job::virtual_hook( id, data ); }


#include "jobclasses.moc"
