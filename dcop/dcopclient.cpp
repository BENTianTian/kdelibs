/*****************************************************************

Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <sys/types.h>
#include <sys/file.h>

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#define QT_CLEAN_NAMESPACE
#include <qtextstream.h>
#include <qfile.h>
#include <qsocketnotifier.h>

#include <dcopserver.h>
#include <dcopglobal.h>
#include <dcopclient.h>
#include <dcopobject.h>

#include <X11/Xmd.h>
extern "C" {
#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
}

template class QList<DCOPObjectProxy>;
template class QList<DCOPClientTransaction>;

class DCOPClientPrivate
{
public:
    DCOPClient *parent;
    QCString appId;
    IceConn iceConn;
    int majorOpcode; // major opcode negotiated w/server and used to tag all comms.

    int majorVersion, minorVersion; // protocol versions negotiated w/server
    char *vendor, *release; // information from server

    static const char* serverAddr; // location of server in ICE-friendly format.
    QSocketNotifier *notifier;
    bool registered;

    QCString senderId;

    QList<DCOPObjectProxy> proxies;

    QCString defaultObject;
    QList<DCOPClientTransaction> *transactionList;
    bool transaction;
    Q_INT32 transactionId;
};

class DCOPClientTransaction
{
public:
    Q_INT32 id;
    QCString senderId;
};

struct ReplyStruct
{
    bool result;
    QCString* replyType;
    QByteArray* replyData;
    Q_INT32 replyId;
};

const char* DCOPClientPrivate::serverAddr = 0;

// SM DUMMY
#include <X11/SM/SMlib.h>
static Bool HostBasedAuthProc ( char* /*hostname*/)
{
    return false; // no host based authentication
}
static Status NewClientProc ( SmsConn, SmPointer, unsigned long*, SmsCallbacks*, char** )
{
    return 0;
};

static void registerXSM()
{
    char 	errormsg[256];
    if (!SmsInitialize ((char *)"SAMPLE-SM", (char *)"1.0",
			NewClientProc, NULL,
			HostBasedAuthProc, 256, errormsg))
	{
	    qFatal("register xsm failed");
	}
}



/**
 * Callback for ICE.
 */
void DCOPProcessMessage(IceConn iceConn, IcePointer clientObject,
			int opcode, unsigned long length, Bool /*swap*/,
			IceReplyWaitInfo *replyWait,
			Bool *replyWaitRet)
{
    DCOPMsg *pMsg = 0;
    DCOPClientPrivate *d = (DCOPClientPrivate *) clientObject;
    DCOPClient *c = d->parent;

    switch (opcode )
	{
	case DCOPReplyFailed:
	    if ( replyWait ) {
		IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
		QByteArray tmp( length );
		IceReadData(iceConn, length, tmp.data() );

		((ReplyStruct*) replyWait->reply)->result = false;

		*replyWaitRet = True;
		return;
	    } else {
		qWarning("Very strange! got a DCOPReplyFailed opcode, but we were not waiting for a reply!");
		return;
	    }
	case DCOPReply:
	    if ( replyWait ) {
		IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
		QByteArray tmp( length );
		IceReadData(iceConn, length, tmp.data() );

		QByteArray* b = ((ReplyStruct*) replyWait->reply)->replyData;
		QCString* t =  ((ReplyStruct*) replyWait->reply)->replyType;
		((ReplyStruct*) replyWait->reply)->result = true;

		// TODO: avoid this data copying
		QDataStream tmpStream( tmp, IO_ReadOnly );
		tmpStream >> *t >> *b;

		*replyWaitRet = True;
		return;
	    } else {
		qWarning("Very strange! got a DCOPReply opcode, but we were not waiting for a reply!");
		return;
	    }
	case DCOPReplyWait:
	    if ( replyWait ) {
		IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
		QByteArray tmp( length );
		IceReadData(iceConn, length, tmp.data() );

		Q_INT32 id;
		QDataStream tmpStream( tmp, IO_ReadOnly );
		tmpStream >> id;
		((ReplyStruct*) replyWait->reply)->replyId = id;
		return;
	    } else {
		qWarning("Very strange! got a DCOPReplyWait opcode, but we were not waiting for a reply!");
		return;
	    }
	case DCOPReplyDelayed:
	    if ( replyWait ) {
		IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
		QByteArray tmp( length );
		IceReadData(iceConn, length, tmp.data() );

		QByteArray* b = ((ReplyStruct*) replyWait->reply)->replyData;
		QCString* t =  ((ReplyStruct*) replyWait->reply)->replyType;
		((ReplyStruct*) replyWait->reply)->result = true;

		QDataStream ds( tmp, IO_ReadOnly );
		QCString calledApp, app;
		Q_INT32 id;

		ds >> calledApp >> app >> id >> *t >> *b;
		if (id != ((ReplyStruct*) replyWait->reply)->replyId) {
		    ((ReplyStruct*) replyWait->reply)->result = false;
		    qWarning("Very strange! DCOPReplyDelayed got wrong sequence id!");
		}

		*replyWaitRet = True;
		return;
	    } else {
		qWarning("Very strange! got a DCOPReplyDelayed opcode, but we were not waiting for a reply!");
		return;
	    }
	case DCOPCall:
	case DCOPSend:
	    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
	    QByteArray tmp( length );
	    IceReadData(iceConn, length, tmp.data() );
	    QDataStream ds( tmp, IO_ReadOnly );
	    QCString app, objId, fun;
	    QByteArray data;
	    ds >> d->senderId >> app >> objId >> fun >> data;

	    QCString replyType;
	    QByteArray replyData;
	    bool b = c->receive( app, objId, fun,
				 data, replyType, replyData );

	    if (opcode != DCOPCall)
		return;

	    QByteArray reply;
	    QDataStream replyStream( reply, IO_WriteOnly );

	    Q_INT32 id = c->transactionId();
	    if (id)
		{
		    // Call delayed. Send back the transaction ID.
		    replyStream << id;

		    IceGetHeader( iceConn, d->majorOpcode, DCOPReplyWait,
				  sizeof(DCOPMsg), DCOPMsg, pMsg );
		    pMsg->length += reply.size();
		    IceSendData( iceConn, reply.size(), (char *) reply.data());
		    return;
		}

	    if ( !b )
		{
		    qWarning("DCOP failure in app %s:\n   object '%s' has no function '%s'", app.data(), objId.data(), fun.data() );
		    // Call failed. No data send back.

		    IceGetHeader( iceConn, d->majorOpcode, DCOPReplyFailed,
				  sizeof(DCOPMsg), DCOPMsg, pMsg );
		    pMsg->length += reply.size();
		    IceSendData( iceConn, reply.size(), (char *) reply.data());
		    return;
		}

	    // Call successfull. Send back replyType and replyData.
	    replyStream << replyType << replyData.size();

	    // we are calling, so we need to set up reply data
	    IceGetHeader( iceConn, d->majorOpcode, DCOPReply,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    int datalen = reply.size() + replyData.size();
	    pMsg->length += datalen;
	    // use IceSendData not IceWriteData to avoid a copy.  Output buffer
	    // shouldn't need to be flushed.
	    IceSendData( iceConn, reply.size(), (char *) reply.data());
	    IceSendData( iceConn, replyData.size(), (char *) replyData.data());
	    return;
	}
}

static IcePoVersionRec DCOPVersions[] = {
    { DCOPVersionMajor, DCOPVersionMinor,  DCOPProcessMessage }
};

DCOPClient::DCOPClient()
{
    d = new DCOPClientPrivate;
    d->parent = this;
    d->iceConn = 0L;
    d->majorOpcode = 0;
    d->appId = 0;
    d->notifier = 0L;
    d->registered = false;
    d->transactionList = 0L;
    d->transactionId = 0;
}

DCOPClient::~DCOPClient()
{
    if (d->iceConn)
	if (IceConnectionStatus(d->iceConn) == IceConnectAccepted)
	    detach();

    delete d->notifier;
    delete d->transactionList;
    delete d;
}

void DCOPClient::setServerAddress(const QCString &addr)
{
    delete DCOPClientPrivate::serverAddr;
    DCOPClientPrivate::serverAddr = qstrdup( addr.data() );
}

bool DCOPClient::attach()
{
    return attachInternal( true );
}

void DCOPClient::bindToApp()
{
    // check if we have a qApp instantiated.  If we do,
    // we can create a QSocketNotifier and use it for receiving data.
    if (qApp) {
	if ( d->notifier )
	    delete d->notifier;
	d->notifier = new QSocketNotifier(socket(),
					  QSocketNotifier::Read, 0, 0);
	connect(d->notifier, SIGNAL(activated(int)),
		SLOT(processSocketData(int)));
    }
}

bool DCOPClient::attachInternal( bool registerAsAnonymous )
{
    char errBuf[1024];

    if ( isAttached() )
	detach();

    extern int _IceLastMajorOpcode; // from libICE
    if (_IceLastMajorOpcode < 1 )
	registerXSM();

    if ((d->majorOpcode = IceRegisterForProtocolSetup((char *) "DCOP", (char *) DCOPVendorString,
						      (char *) DCOPReleaseString, 1, DCOPVersions,
						      DCOPAuthCount, (char **) DCOPAuthNames,
						      DCOPClientAuthProcs, 0L)) < 0) {
	emit attachFailed("Communications could not be established.");
	return false;
    }

    // first, check if serverAddr was ever set.
    if (!d->serverAddr) {
	// here, we obtain the list of possible DCOP connections,
	// and attach to them.
	QString dcopSrv;
	dcopSrv = ::getenv("DCOPSERVER");
	if (dcopSrv.isNull()) {
	    QString fName = ::getenv("HOME");
	    fName += "/.DCOPserver";
	    QFile f(fName);
	    if (!f.open(IO_ReadOnly)) {
		emit attachFailed("Could not read network connection list.");
		return false;
	    }
	    QTextStream t(&f);
	    dcopSrv = t.readLine();
	}
	d->serverAddr = qstrdup( dcopSrv.latin1() );
    }

    if ((d->iceConn = IceOpenConnection((char*)d->serverAddr,
					(IcePointer) this, False, d->majorOpcode,
					sizeof(errBuf), errBuf)) == 0L) {
	emit attachFailed(errBuf);
	d->iceConn = 0;
	return false;
    }

    IceSetShutdownNegotiation(d->iceConn, False);

    int setupstat;
    setupstat = IceProtocolSetup(d->iceConn, d->majorOpcode,
				 (IcePointer) d,
				 True, /* must authenticate */
				 &(d->majorVersion), &(d->minorVersion),
				 &(d->vendor), &(d->release), 1024, errBuf);


    if (setupstat == IceProtocolSetupFailure ||
	setupstat == IceProtocolSetupIOError) {
	IceCloseConnection(d->iceConn);
	emit attachFailed(errBuf);
	return false;
    } else if (setupstat == IceProtocolAlreadyActive) {
	/* should not happen because 3rd arg to IceOpenConnection was 0. */
	emit attachFailed("internal error in IceOpenConnection");
	return false;
    }


    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted) {
	emit attachFailed("DCOP server did not accept the connection.");
	return false;
    }

    bindToApp();

    if ( registerAsAnonymous )
	registerAs( "anonymous", true );

    return true;
}


bool DCOPClient::detach()
{
    int status;

    if (d->iceConn) {
	IceProtocolShutdown(d->iceConn, d->majorOpcode);
	status = IceCloseConnection(d->iceConn);
	if (status != IceClosedNow)
	    return false;
	else
	    d->iceConn = 0L;
    }
    delete d->notifier;
    d->notifier = 0L;
    d->registered = false;
    return true;
}

bool DCOPClient::isAttached() const
{
    if (!d->iceConn)
	return false;

    return (IceConnectionStatus(d->iceConn) == IceConnectAccepted);
}


QCString DCOPClient::registerAs( QCString appId, bool addPID )
{
    QCString result;

    // Detach before reregistering.
    if ( isRegistered() ) {
	detach();
    }

    if ( !isAttached() ) {
	if ( !attachInternal( FALSE ) ) {
	    return result;
	}
    }

    if (addPID) {
	QCString pid;
	pid.sprintf("-%d", getpid());
	appId = appId + pid;
    }

    // register the application identifier with the server
    QCString replyType;
    QByteArray data, replyData;
    QDataStream arg( data, IO_WriteOnly );
    arg <<appId;
    if ( call( "DCOPServer", "", "registerAs(QCString)", data, replyType, replyData ) ) {
	QDataStream reply( replyData, IO_ReadOnly );
	reply >> result;
    }
    d->appId = result;
    d->registered = !result.isNull();
    return result;
}

bool DCOPClient::isRegistered() const
{
    return d->registered;
}


QCString DCOPClient::appId() const
{
    return d->appId;
}


int DCOPClient::socket() const
{
    if (d->iceConn)
	return IceConnectionNumber(d->iceConn);
    else
	return 0;
}

static inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

QCString DCOPClient::normalizeFunctionSignature( const QCString& fun ) {
    if ( fun.isEmpty() )				// nothing to do
	return fun.copy();
    QCString result( fun.size() );
    char *from	= fun.data();
    char *to	= result.data();
    char *first = to;
    char last = 0;
    while ( TRUE ) {
	while ( *from && isspace(*from) )
	    from++;
	if ( last && isIdentChar( last ) && isIdentChar( *from ) )
	    *to++ = 0x20;
	while ( *from && !isspace(*from) ) {
	    last = *from++;
	    *to++ = last;
	}
	if ( !*from )
	    break;
    }
    if ( to > first && *(to-1) == 0x20 )
	to--;
    *to = '\0';
    result.resize( (int)((long)to - (long)result.data()) + 1 );
    return result;
}


QCString DCOPClient::senderId() const
{
    return d->senderId;
}


bool DCOPClient::send(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QByteArray &data,
		      bool)
{
    if ( !isAttached() )
	return false;


    DCOPMsg *pMsg;

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << d->appId << remApp << remObjId << normalizeFunctionSignature(remFun) << data.size();

    IceGetHeader(d->iceConn, d->majorOpcode, DCOPSend,
		 sizeof(DCOPMsg), DCOPMsg, pMsg);

    int datalen = ba.size() + data.size();
    pMsg->length += datalen;

    IceSendData( d->iceConn, ba.size(), (char *) ba.data() );
    IceSendData( d->iceConn, data.size(), (char *) data.data() );

    //  IceFlush(d->iceConn);

    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
	return false;
    else
	return true;
}

bool DCOPClient::send(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QString &data,
		      bool)
{
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << data;
    return send(remApp, remObjId, remFun, ba);
}

bool DCOPClient::process(const QCString &, const QByteArray &,
			 QCString&, QByteArray &)
{
    return false;
}

bool DCOPClient::isApplicationRegistered( const QCString& remApp)
{
    QCString replyType;
    QByteArray data, replyData;
    QDataStream arg( data, IO_WriteOnly );
    arg << remApp;
    int result = false;
    if ( call( "DCOPServer", "", "isApplicationRegistered(QCString)", data, replyType, replyData ) ) {
	QDataStream reply( replyData, IO_ReadOnly );
	reply >> result;
    }
    return result;
}

QCStringList DCOPClient::registeredApplications()
{
    QCString replyType;
    QByteArray data, replyData;
    QCStringList result;
    if ( call( "DCOPServer", "", "registeredApplications()", data, replyType, replyData ) ) {
	QDataStream reply( replyData, IO_ReadOnly );
	reply >> result;
    }
    return result;
}

void DCOPClient::setNotifications(bool enabled)
{
    QByteArray data;
    QDataStream ds(data, IO_WriteOnly);
    ds << (Q_INT8) enabled;

    QCString replyType;
    QByteArray reply;
    if (!call("DCOPServer", "", "setNotifications( bool )", data, replyType, reply))
	qDebug("I couldn't enable notifications at the dcopserver!");
}

bool DCOPClient::receive(const QCString &app, const QCString &objId,
			 const QCString &fun, const QByteArray &data,
			 QCString& replyType, QByteArray &replyData)
{
    if ( !app.isEmpty() && app != d->appId && app[app.length()-1] != '*') {
	qWarning("WEIRD! we somehow received a DCOP message w/a different appId");
	return false;
    }

    if ( objId.isEmpty() ) {
	if ( fun == "applicationRegistered(QCString)" ) {
	    QDataStream ds( data, IO_ReadOnly );
	    QCString r;
	    ds >> r;
	    emit applicationRegistered( r );
	    return true;
	} else if ( fun == "applicationRemoved(QCString)" ) {
	    QDataStream ds( data, IO_ReadOnly );
	    QCString r;
	    ds >> r;
	    emit applicationRemoved( r );
	    return true;
	}
	if ( process( fun, data, replyType, replyData ) )
	    return true;

	// fall through and send to defaultObject if available
	if ( !d->defaultObject.isEmpty() && DCOPObject::hasObject( d->defaultObject ) ) {
	    if (DCOPObject::find( d->defaultObject )->process(fun, data, replyType, replyData))
		return true;
	}

	// fall through and send to object proxies
    }
    d->transaction = false; // Assume no transaction.
    if (!DCOPObject::hasObject(objId)) {

	for ( DCOPObjectProxy* proxy = d->proxies.first(); proxy; proxy = d->proxies.next() ) {
	    if ( proxy->process( objId, fun, data, replyType, replyData ) )
		return TRUE;
	}
	return false;

    } else {
	DCOPObject *objPtr = DCOPObject::find(objId);
	if (!objPtr->process(fun, data, replyType, replyData)) {
	    // obj doesn't understand function or some other error.
	    return false;
	}
    }

    return true;
}

bool DCOPClient::call(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QByteArray &data,
		      QCString& replyType, QByteArray &replyData, bool)
{
    if ( !isAttached() )
	return false;

    DCOPMsg *pMsg;

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << d->appId << remApp << remObjId << normalizeFunctionSignature(remFun) << data.size();

    IceGetHeader(d->iceConn, d->majorOpcode, DCOPCall,
		 sizeof(DCOPMsg), DCOPMsg, pMsg);

    int datalen = ba.size() + data.size();
    pMsg->length += datalen;

    IceSendData(d->iceConn, ba.size(), (char *) ba.data());
    IceSendData(d->iceConn, data.size(), (char *) data.data());


    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
	return false;

    IceFlush (d->iceConn);

    IceReplyWaitInfo waitInfo;
    waitInfo.sequence_of_request = IceLastSentSequenceNumber(d->iceConn);
    waitInfo.major_opcode_of_request = d->majorOpcode;
    waitInfo.minor_opcode_of_request = DCOPCall;
    ReplyStruct tmp;
    tmp.replyType = &replyType;
    tmp.replyData = &replyData;
    tmp.replyId = 0;
    waitInfo.reply = (IcePointer) &tmp;

    Bool readyRet = False;
    IceProcessMessagesStatus s;

    do {
	s = IceProcessMessages(d->iceConn, &waitInfo,
			       &readyRet);
	if (s == IceProcessMessagesIOError) {
	    IceCloseConnection(d->iceConn);
	    qWarning("received an error processing data from DCOP server!");
	    return false;
	}
    } while (!readyRet);


    return tmp.result;
}

void DCOPClient::processSocketData(int)
{

    IceProcessMessagesStatus s =  IceProcessMessages(d->iceConn, 0, 0);

    if (s == IceProcessMessagesIOError) {
	IceCloseConnection(d->iceConn);
	qWarning("received an error processing data from the DCOP server!");
	return;
    }
}

void DCOPClient::installObjectProxy( DCOPObjectProxy* obj)
{
    d->proxies.append( obj );
}

void DCOPClient::removeObjectProxy( DCOPObjectProxy* obj)
{
    d->proxies.removeRef( obj );
}

void DCOPClient::setDefaultObject( const QCString& objId )
{
    d->defaultObject = objId;
}


QCString DCOPClient::defaultObject() const
{
    return d->defaultObject;
}

DCOPClientTransaction *
DCOPClient::beginTransaction()
{
    if (!d->transactionList)
	d->transactionList = new QList<DCOPClientTransaction>;

    d->transaction = true;
    DCOPClientTransaction *trans = new DCOPClientTransaction();
    trans->senderId = d->senderId;
    if (!d->transactionId)  // transactionId should not be 0!
	d->transactionId++;
    trans->id = ++(d->transactionId);

    d->transactionList->append( trans );
    return trans;
}

Q_INT32
DCOPClient::transactionId()
{
    if (d->transaction)
	return d->transactionId;
    else
	return 0;
}

void
DCOPClient::endTransaction( DCOPClientTransaction *trans, QCString& replyType,
			    QByteArray &replyData)
{
    if ( !isAttached() )
	return;

    if ( !d->transactionList) {
	qWarning("Transaction unknown: No pending transactions!");
	return; // No pending transactions!
    }

    if ( !d->transactionList->removeRef( trans ) ) {
	qWarning("Transaction unknown: Not on list of pending transactions!");
	return; // Transaction
    }

    DCOPMsg *pMsg;

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << d->appId << trans->senderId << trans->id << replyType << replyData;

    IceGetHeader(d->iceConn, d->majorOpcode, DCOPReplyDelayed,
		 sizeof(DCOPMsg), DCOPMsg, pMsg);

    pMsg->length += ba.size();

    IceSendData( d->iceConn, ba.size(), (char *) ba.data() );

    delete trans;
}

#include <dcopclient.moc>

