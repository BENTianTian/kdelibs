/* This file is part of the KDE libraries
   Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                      Waldo Bastian <bastian@kde.org>

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

#include "scheduler.h"
#include "slave.h"
#include <qlist.h>
#include <qdict.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kprotocolmanager.h>
#include <assert.h>
#include <kstaticdeleter.h>
#include <kdesu/client.h>

//
// Slaves may be idle for MAX_SLAVE_IDLE time before they are being returned
// to the system wide slave pool. (3 minutes)
#define MAX_SLAVE_IDLE (3*60)

using namespace KIO;

template class QDict<KIO::Scheduler::ProtocolInfo>;

Scheduler *Scheduler::instance = 0;

/*
* Structure to cache authorization info from io-slaves.
*/
struct KIO::AuthKey
{
  AuthKey() {}

  AuthKey(const QCString& k, const QCString& g, bool p) {
    key = k;
    group = g;
    persist = p;
  }

  bool isKeyMatch( const QCString& val ) {
    return (val==key);
  }

  bool isGroupMatch( const QCString& val ) {
    return (val==group);
  }

  QCString key;
  QCString group;
  bool persist;
};

class KIO::SlaveList: public QList<Slave>
{
   public:
      SlaveList() { }
};

class KIO::Scheduler::ProtocolInfo
{
  public:
     ProtocolInfo() : maxSlaves(3)
	{ joblist.setAutoDelete(false); }

     QList<SimpleJob> joblist;
     SlaveList activeSlaves;
     int maxSlaves;
};

class KIO::Scheduler::ProtocolInfoDict : public QDict<KIO::Scheduler::ProtocolInfo>
{
  public:
    ProtocolInfoDict() { }

    KIO::Scheduler::ProtocolInfo *get( const QString &key);
};

KIO::Scheduler::ProtocolInfo *
KIO::Scheduler::ProtocolInfoDict::get(const QString &_key)
{
  // The key is the protocol of the job, except when using an ftp proxy
  QString key = KProtocolManager::slaveProtocol( _key );
  //kdDebug() << "ProtocolInfoDict::get(" << _key << ")  getting info for " << key << endl;

  ProtocolInfo *info = find(key);
  if (!info)
  {
     info = new ProtocolInfo;
     insert(key, info);
  }
  return info;
}


Scheduler::Scheduler()
    : QObject(0, "scheduler"),
      DCOPObject( "KIO::Scheduler" ),
    slaveTimer(this, "Scheduler::slaveTimer"),
    coSlaveTimer(this, "Scheduler::coSlaveTimer"),
    cleanupTimer(this, "Scheduler::cleanupTimer")
{
    slaveOnHold = 0;
    protInfoDict = new ProtocolInfoDict;
    slaveList = new SlaveList;
    idleSlaves = new SlaveList;
    coIdleSlaves = new SlaveList;
    connect(&slaveTimer, SIGNAL(timeout()), SLOT(startStep()));
    connect(&coSlaveTimer, SIGNAL(timeout()), SLOT(slotScheduleCoSlave()));
    connect(&cleanupTimer, SIGNAL(timeout()), SLOT(slotCleanIdleSlaves()));
    busy = false;
    cachedAuthKeys.setAutoDelete( true );
}

Scheduler::~Scheduler()
{
    //fprintf(stdout, "Destructing KIO::Scheduler...\n");

    // Delete any stored authorization info now...
    if( !cachedAuthKeys.isEmpty() )
      delCachedAuthKeys( cachedAuthKeys );

    protInfoDict->setAutoDelete(true);
    delete protInfoDict; protInfoDict = 0;
    delete idleSlaves; idleSlaves = 0;
    delete coIdleSlaves; idleSlaves = 0;
    slaveList->setAutoDelete(true);
    delete slaveList; slaveList = 0;
    instance = 0;
}

void
Scheduler::debug_info()
{
#if 0
    kdDebug(7006) << "Scheduler Info" << endl;
    kdDebug(7006) << "==========" << endl;
    kdDebug(7006) << "Total Slaves: " << slaveList->count() << endl;
    Slave *slave = slaveList->first();
    for(; slave; slave = slaveList->next())
    {
        ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
        kdDebug(7006) << " Slave: " << slave->protocol() << " " << slave->host() << " " << slave->port() << " pid: " << slave->slave_pid() << endl;
        kdDebug(7006) << " -- active: " << protInfo->activeSlaves.count() << endl;
    }
    kdDebug(7006) << "Idle Slaves: " << idleSlaves->count() << endl;
    slave = idleSlaves->first();
    for(; slave; slave = idleSlaves->next())
    {
        kdDebug(7006) << " IdleSlave: " << slave->protocol() << " " << slave->host() << " " << slave->port() << endl;

    }
#endif
}

bool Scheduler::process(const QCString &fun, const QByteArray &data, QCString &replyType, QByteArray &replyData )
{
  if ( fun != "reparseSlaveConfiguration(QString)" )
    return DCOPObject::process( fun, data, replyType, replyData );

  replyType = "void";

  QDataStream stream( data, IO_ReadOnly );

  QString proto;

  stream >> proto;

  kdDebug( 7006 ) << "reparseConfiguration( " << proto << " )" << endl;

  KProtocolManager::reparseConfiguration();

  Slave *slave = slaveList->first();
  for (; slave; slave = slaveList->next() )
    if ( slave->protocol() == proto || proto.isEmpty() )
      slave->connection()->send( CMD_REPARSECONFIGURATION );

  return true;
}

QCStringList Scheduler::functions()
{
    QCStringList funcs = DCOPObject::functions();
    funcs << "void reparseSlaveConfiguration(QString)";
    return funcs;
}

void Scheduler::_doJob(SimpleJob *job) {
    newJobs.append(job);
    QString protocol = job->url().protocol();
    //kdDebug(7006) << "Scheduler::_doJob protocol=" << protocol << endl;
    slaveTimer.start(0, true);
}

void Scheduler::_scheduleJob(SimpleJob *job) {
    newJobs.removeRef(job);
    QString protocol = job->url().protocol();
    //kdDebug(7006) << "Scheduler::_doJob protocol=" << protocol << endl;
    ProtocolInfo *protInfo = protInfoDict->get(protocol);
    protInfo->joblist.append(job);
    slaveTimer.start(0, true);
}

void Scheduler::_cancelJob(SimpleJob *job) {
//    kdDebug(7006) << "Scheduler: canceling job " << job << endl;
    Slave *slave = job->slave();
    if ( !slave  )
    {
        // was not yet running (don't call this on a finished job!)
        newJobs.removeRef(job);
        QString protocol = job->url().protocol();
        ProtocolInfo *protInfo = protInfoDict->get(protocol);
        protInfo->joblist.removeRef(job);

        // Search all slaves to see if job is in the queue of a coSlave
        slave = slaveList->first();
        for(; slave; slave = slaveList->next())
        {
           JobList *list = coSlaves.find(slave);
           if (list && list->find(job))
              break;
        }
        if (!slave)
           return; // Job was not yet running and not in a coSlave queue.
    }
    kdDebug(7006) << "Scheduler: killing slave " << slave->slave_pid() << endl;
    slave->kill();
    _jobFinished( job, slave );
    slotSlaveDied( slave);
}

void Scheduler::startStep()
{
    while(newJobs.count())
    {
       (void) startJobDirect();
    }
    QDictIterator<KIO::Scheduler::ProtocolInfo> it(*protInfoDict);
    while(it.current())
    {
       if (startJobScheduled(it.current())) return;
       ++it;
    }
}

bool Scheduler::startJobScheduled(ProtocolInfo *protInfo)
{
    if (protInfo->joblist.isEmpty())
       return false;

//       kdDebug(7006) << "Scheduling job" << endl;
    debug_info();
    SimpleJob *job = protInfo->joblist.at(0);

    bool newSlave = false;

    // Look for matching slave
    Slave *slave = findIdleSlave(protInfo, job);

    if (!slave)
    {
       if ( protInfo->maxSlaves > static_cast<int>(protInfo->activeSlaves.count()) )
       {
          newSlave = true;
          slave = createSlave(protInfo, job, job->url());
       }
    }

    if (!slave)
    {
//          kdDebug(7006) << "No slaves available" << endl;
//          kdDebug(7006) << " -- active: " << protInfo->activeSlaves.count() << endl;
       return false;
    }

    protInfo->activeSlaves.append(slave);
    idleSlaves->removeRef(slave);
    protInfo->joblist.removeRef(job);
//       kdDebug(7006) << "scheduler: job started " << job << endl;

    KURL url =job->url();
    QString host = url.host();
    int port = url.port();
    QString user = url.user();
    QString passwd = url.pass();

    if ((newSlave) ||
        (slave->host() != host) ||
        (slave->port() != port) ||
        (slave->user() != user) ||
        (slave->passwd() != passwd))
    {
        slave->setHost(host, port, user, passwd);
    }
    job->start(slave);
    slaveTimer.start(0, true);
    return true;
}

bool Scheduler::startJobDirect()
{
    debug_info();
    SimpleJob *job = newJobs.take(0);
    QString protocol = job->url().protocol();
    ProtocolInfo *protInfo = protInfoDict->get(protocol);

    bool newSlave = false;

    // Look for matching slave
    Slave *slave = findIdleSlave(protInfo, job);

    if (!slave)
    {
       newSlave = true;
       slave = createSlave(protInfo, job, job->url());
    }

    if (!slave)
       return false;

    idleSlaves->removeRef(slave);
//       kdDebug(7006) << "scheduler: job started " << job << endl;

    KURL url =job->url();
    QString host = url.host();
    int port = url.port();
    QString user = url.user();
    QString passwd = url.pass();

    if ((newSlave) ||
        (slave->host() != host) ||
        (slave->port() != port) ||
        (slave->user() != user) ||
        (slave->passwd() != passwd))
    {
        slave->setHost(host, port, user, passwd);
    }
    job->start(slave);
    return true;
}

static Slave *searchIdleList(SlaveList *idleSlaves, const KURL &url)
{
    QString protocol = KProtocolManager::slaveProtocol( url.protocol() );
    QString host = url.host();
    int port = url.port();
    QString user = url.user();

    for( Slave *slave = idleSlaves->first();
         slave;
         slave = idleSlaves->next())
    {
       if ((protocol == slave->slaveProtocol()) &&
           (host == slave->host()) &&
           (port == slave->port()) &&
           (user == slave->user()))
           return slave;
    }

    // Look for slightly matching slave
    for( Slave *slave = idleSlaves->first();
         slave;
         slave = idleSlaves->next())
    {
       if (protocol == slave->slaveProtocol())
          return slave;
    }
    return 0;
}

Slave *Scheduler::findIdleSlave(ProtocolInfo *, SimpleJob *job)
{
    Slave *slave = 0;
    if (slaveOnHold)
    {
       // Make sure that the job wants to do a GET or a POST, and with no offset
       bool bCanReuse = (job->command() == CMD_GET);
       KIO::TransferJob * tJob = dynamic_cast<KIO::TransferJob *>(job);
       if ( tJob )
       {
          bCanReuse = (job->command() == CMD_GET || job->command() == CMD_SPECIAL);
          if ( bCanReuse )
          {
            KIO::MetaData outgoing = tJob->outgoingMetaData();
            QString resume = (!outgoing.contains("resume")) ? QString::null : outgoing["resume"];
            kdDebug(7006) << "Resume metadata is '" << resume << "'" << endl;
            bCanReuse = (resume.isEmpty() || resume == "0");
          }
       }
       kdDebug(7006) << "bCanReuse = " << bCanReuse << endl;
       if (bCanReuse)
       {
          if (job->url() == urlOnHold)
          {
             kdDebug(7006) << "HOLD: Reusing held slave for " << urlOnHold.prettyURL() << endl;
             slave = slaveOnHold;
          }
          else
          {
             kdDebug(7006) << "HOLD: Discarding held slave (" << urlOnHold.prettyURL() << ")" << endl;
             slaveOnHold->kill();
          }
          slaveOnHold = 0;
          urlOnHold = KURL();
       }
       if (slave)
          return slave;
    }

    return searchIdleList(idleSlaves, job->url());
}

Slave *Scheduler::createSlave(ProtocolInfo *protInfo, SimpleJob *job, const KURL &url)
{
   int error;
   QString errortext;
   Slave *slave = Slave::createSlave(url, error, errortext);
   if (slave)
   {
      slaveList->append(slave);
      idleSlaves->append(slave);
      connect(slave, SIGNAL(slaveDied(KIO::Slave *)),
                SLOT(slotSlaveDied(KIO::Slave *)));
      connect(slave, SIGNAL(slaveStatus(pid_t,const QCString &,const QString &, bool)),
                SLOT(slotSlaveStatus(pid_t,const QCString &, const QString &, bool)));
      connect(slave,SIGNAL(authorizationKey(const QCString&, const QCString&, bool)),
                SLOT(slotAuthorizationKey(const QCString&, const QCString&, bool)));
   }
   else
   {
      kdError() << "ERROR " << error << ": couldn't create slave : "
                << errortext << endl;
      if (job)
      {
         protInfo->joblist.removeRef(job);
         job->slotError( error, errortext );
      }
   }
   return slave;
}

#if 0
void Scheduler::slotSlaveStatus(pid_t pid, const QCString &protocol, const QString &host, bool connected) {
    Slave *slave = (Slave*)sender();
    kdDebug(7006) << "Slave = " << slave << " (PID = " << pid
                  << ") protocol = " << protocol.data() << " host = "
                  << ( !host.isNull() ? host : QString::fromLatin1("[None]") ) << " "
                  << ( connected ? "Connected" : "Not connected" ) << endl;
}
#else
void Scheduler::slotSlaveStatus(pid_t, const QCString &, const QString &, bool) {}
#endif

void Scheduler::_jobFinished(SimpleJob *job, Slave *slave)
{
    ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
    slave->disconnect(job);
    protInfo->activeSlaves.removeRef(slave);
    if (slave->isAlive())
    {
       JobList *list = coSlaves.find(slave);
       if (list)
       {
          coIdleSlaves->append(slave);
          if (!list->isEmpty())
             coSlaveTimer.start(0, true);
          return;
       }
       else
       {
          idleSlaves->append(slave);
          slave->setIdle();
          _scheduleCleanup();
          slave->connection()->send( CMD_SLAVE_STATUS );
       }
    }
    if (protInfo->joblist.count())
    {
       slaveTimer.start(0, true);
    }
}

void Scheduler::slotAuthorizationKey( const QCString& key,
                                      const QCString& group,
                                      bool keep )
{
    AuthKey* auth_key = cachedAuthKeys.first();
    for( ; auth_key !=0 ; auth_key=cachedAuthKeys.next() )
    {
        if( auth_key->isKeyMatch(key) )
            return ;
    }

    cachedAuthKeys.append( new AuthKey (key, group, keep) );
    regCachedAuthKey( key, group );
}

void Scheduler::slotDelAuthorization( const QCString& grpkey )
{
    AuthKey* key = cachedAuthKeys.first();
    for( ; key !=0 ; key=cachedAuthKeys.next() )
    {
        if( key->isGroupMatch(grpkey) )
        {
            AuthKeyList list;
            list.append(key);
            delCachedAuthKeys( list );
            cachedAuthKeys.remove();
            break;
        }
    }
}

bool Scheduler::pingCacheDaemon() const
{
    KDEsuClient client;
    int sucess = client.ping();
    if( sucess == -1 )
    {
        sucess = client.startServer();
        if( sucess == -1 )
            return false;
    }
    return true;
}

bool Scheduler::regCachedAuthKey( const QCString& key, const QCString& group )
{
    if( !pingCacheDaemon() )
        return false;

    bool ok;
    KDEsuClient client;
    QCString ref_key = key.copy() + "-refcount";
    int count = client.getVar(ref_key).toInt( &ok );
    if( ok )
    {
        QCString val;
        val.setNum( count+1 );
        client.setVar( ref_key, val, 0, group);
    }
    else
    {
        if( client.setVar( ref_key, "1", 0, group ) == -1 )
            return false;
    }
    return true;
}

void Scheduler::delCachedAuthKeys( const AuthKeyList& list )
{
    if ( !list.isEmpty() && pingCacheDaemon() )
    {
        bool ok;
        int count;
        KDEsuClient client;
        QCString val, ref_key;
        AuthKeyIterator it( list );
        for ( ; it.current(); ++it )
        {
            AuthKey* auth_key = it.current();
            // Do not delete passwords that are supposed
            // to be persistent
            if ( auth_key->persist )
                continue;

            ref_key = auth_key->key.copy() + "-refcount";
            count = client.getVar( ref_key ).toInt( &ok );
            if ( ok )
            {
                if ( count > 1 )
                {
                    val.setNum(count-1);
                    client.setVar( ref_key, val, 0, auth_key->group );
                }
                else
                    client.delVars(auth_key->key);
            }
        }
    }
}

void Scheduler::slotSlaveDied(KIO::Slave *slave)
{
    assert(!slave->isAlive());
    ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
    protInfo->activeSlaves.removeRef(slave);
    if (slave == slaveOnHold)
    {
       slaveOnHold = 0;
       urlOnHold = KURL();
    }
    idleSlaves->removeRef(slave);
    JobList *list = coSlaves.find(slave);
    if (list)
    {
       // coSlave dies, kill jobs waiting in queue
       disconnectSlave(slave);
    }

    if (!slaveList->removeRef(slave))
       kdDebug(7006) << "Scheduler: BUG!! Slave died, but is NOT in slaveList!!!\n" << endl;
    slave->deref(); // Delete slave
}

void Scheduler::slotCleanIdleSlaves()
{
   for(Slave *slave = idleSlaves->first();slave;)
   {
      if (slave->idleTime() >= MAX_SLAVE_IDLE)
      {
//         kdDebug(7006) << "Removing idle slave: " << slave->protocol() << " " << slave->host() << endl;
         Slave *removeSlave = slave;
         slave = idleSlaves->next();
         idleSlaves->removeRef(removeSlave);
         slaveList->removeRef(removeSlave);
         delete removeSlave;
      }
      else
      {
         slave = idleSlaves->next();
      }
   }
   _scheduleCleanup();
}

void Scheduler::_scheduleCleanup()
{
   if (idleSlaves->count())
   {
      if (!cleanupTimer.isActive())
         cleanupTimer.start( MAX_SLAVE_IDLE*1000, true );
   }
}

void Scheduler::_putSlaveOnHold(KIO::SimpleJob *job, const KURL &url)
{
   Slave *slave = job->slave();
   slave->disconnect(job);

   if (slaveOnHold)
   {
      slaveOnHold->kill();
   }
   slaveOnHold = slave;
   urlOnHold = url;
   slaveOnHold->suspend();
}

void Scheduler::_removeSlaveOnHold()
{
   if (slaveOnHold)
   {
      slaveOnHold->kill();
   }
   slaveOnHold = 0;
   urlOnHold = KURL();
}

Slave *
Scheduler::_getConnectedSlave(const KURL &url, const KIO::MetaData &metaData )
{
    Slave *slave = searchIdleList(idleSlaves, url);
    if (!slave)
    {
       QString protocol = url.protocol();
       ProtocolInfo *protInfo = protInfoDict->get(protocol);
       slave = createSlave(protInfo, 0, url);
    }
    if (!slave)
       return 0; // Error
    idleSlaves->removeRef(slave);

    slave->setHost(url.host(), url.port(), url.user(), url.pass());
    if (!metaData.isEmpty())
    {
       kdDebug(7006) << "Scheduler::getConnectedSlave : Sending metadata :" << endl;
       MetaData::ConstIterator mapit;
       for ( mapit = metaData.begin() ; mapit != metaData.end() ; ++mapit )
           kdDebug(7006) << "   " << mapit.key() << "=" << mapit.data() << endl;
       QByteArray packedArgs; 
       QDataStream stream( packedArgs, IO_WriteOnly ); 
       stream << metaData;
       slave->connection()->send( CMD_META_DATA, packedArgs );
    }
    slave->connection()->send( CMD_CONNECT );
    connect(slave, SIGNAL(connected()),
                SLOT(slotSlaveConnected()));
    connect(slave, SIGNAL(error(int, const QString &)),
                SLOT(slotSlaveError(int, const QString &)));

    coSlaves.insert(slave, new QList<SimpleJob>());
    coIdleSlaves->append(slave);    

    return slave;
}

void
Scheduler::slotScheduleCoSlave()
{
    Slave *nextSlave;
    for(Slave *slave = coIdleSlaves->first(); 
        slave;
        slave = nextSlave)
    {
        nextSlave = coIdleSlaves->next();
        JobList *list = coSlaves.find(slave);
        assert(list);
        if (!list->isEmpty())
        {
           SimpleJob *job = list->take(0);
           coIdleSlaves->removeRef(slave);
           kdDebug(7006) << "scheduler: job started " << job << endl;
 
           KURL url =job->url();
           QString host = url.host();
           int port = url.port();

#if 0
           QString user = url.user();
           QString passwd = url.pass();
 
           if ((slave->host() != host) ||
               (slave->port() != port) ||
               (slave->user() != user) ||
               (slave->passwd() != passwd))
           {
              slave->setHost(host, port, user, passwd);
           }
#endif
           assert(slave->host() == host);
           assert(slave->port() == port);
           job->start(slave);
        } 
    }
}

void
Scheduler::slotSlaveConnected()
{
    Slave *slave = (Slave *)sender();
    emit slaveConnected(slave);
}

void
Scheduler::slotSlaveError(int errorNr, const QString &errorMsg)
{
    Slave *slave = (Slave *)sender();
    if (coIdleSlaves->find(slave) != -1)
    {
       // Only forward to application if slave is idle 
       emit slaveError(slave, errorNr, errorMsg);
    }
}

bool 
Scheduler::_assignJobToSlave(KIO::Slave *slave, SimpleJob *job)
{
    if (slave->protocol() != KProtocolManager::slaveProtocol( job->url().protocol() ))
        return false;

    if (!newJobs.removeRef(job))
        return false;

    JobList *list = coSlaves.find(slave);
    assert(list);
    if (!list)
        return false;

    list->append(job);
    coSlaveTimer.start(0, true); // Start job on timer event

    return false;
}

bool 
Scheduler::_disconnectSlave(KIO::Slave *slave)
{
    JobList *list = coSlaves.take(slave);
    assert(list);
    if (!list)
       return false;
    // Kill jobs still in queue.
    while(!list->isEmpty())
    {
       Job *job = list->take(0);
       job->kill();
    }
    delete list;
    coIdleSlaves->removeRef(slave);
    if (slave->isAlive())
    {
       idleSlaves->append(slave);
       slave->connection()->send( CMD_DISCONNECT );
       slave->setIdle();
       _scheduleCleanup();
    }
    return true;
}

static KStaticDeleter<Scheduler> ksds;

Scheduler* Scheduler::self() {
    if ( !instance ) {
	    instance = ksds.setObject(new Scheduler);
    }

    return instance;
}



#include "scheduler.moc"
