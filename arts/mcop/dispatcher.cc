    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include <config.h>
#include "dispatcher.h"
#include "startupmanager.h"
#include "unixconnection.h"
#include "tcpconnection.h"
#include "referenceclean.h"
#include "core.h"
#include "md5auth.h"
#include "mcoputils.h"
#include "loopback.h"
#include "debug.h"
#include "ifacerepo_impl.h"

#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#elif HAVE_SYS_TIME_H
# include <sys/time.h>
#else
# include <time.h>
#endif                                                                          

/* Dispatcher private data class (to ensure binary compatibility) */

using namespace std;
using namespace Arts;

namespace Arts {

class DispatcherPrivate {
public:
	GlobalComm globalComm;
	InterfaceRepo interfaceRepo;
	AuthAccept *accept;
	LoopbackConnection *loopbackConnection;
	bool allowNoAuthentication;
};

};

Dispatcher *Dispatcher::_instance = 0;

Dispatcher::Dispatcher(IOManager *ioManager, StartServer startServer)
{
	assert(!_instance);
	_instance = this;

	/* private data pointer */
	d = new DispatcherPrivate();

	generateServerID();

	if(ioManager)
	{
		_ioManager = ioManager;
		deleteIOManagerOnExit = false;
	}
	else
	{
		_ioManager = new StdIOManager;
		deleteIOManagerOnExit = true;
	}

	objectManager = new ObjectManager;

	notificationManager = new NotificationManager;

	if(startServer & startUnixServer)
	{
		unixServer = new UnixServer(this,serverID);
		if(!unixServer->running())
		{
			delete unixServer;
			arts_warning("MCOP: couldn't start UnixServer");
			unixServer = 0;
		}
	}
	else unixServer = 0;

	if(startServer & startTCPServer)
	{
		tcpServer = new TCPServer(this);
		if(!tcpServer->running())
		{
			delete tcpServer;
			arts_warning("MCOP: couldn't start TCPServer");
			tcpServer = 0;
		}
	}
	else tcpServer = 0;

	d->allowNoAuthentication = startServer & noAuthentication;
	d->accept = 0;
	d->loopbackConnection = new LoopbackConnection(serverID);
	d->interfaceRepo = InterfaceRepo::_from_base(new InterfaceRepo_impl());

	_flowSystem = 0;
	referenceClean = new ReferenceClean(objectPool);

	/*
	 * setup signal handler for SIGPIPE
	 */
	orig_sigpipe = signal(SIGPIPE,SIG_IGN);
	if(orig_sigpipe != SIG_DFL)
	{
		cerr << "mcop warning: user defined signal handler found for"
		        " SIG_PIPE, overriding" << endl;
	}
	
	StartupManager::startup();

	/*
	 * this is required for publishing global references - might be a good
	 * reason for startup priorities as since this is required for cookie&co,
	 * no communication is possible without that
	 */
	string globalCommName
			= MCOPUtils::readConfigEntry("GlobalComm","Arts::TmpGlobalComm");

	d->globalComm = GlobalComm(SubClass(globalCommName));

	// --- initialize MD5auth ---
	/*
	 * Path for random seed: better to store it in home, because some
	 * installations wipe /tmp on reboot.
	 */
	string seedpath = MCOPUtils::createFilePath("random-seed");
	char *home = getenv("HOME");
	if(home != 0) seedpath = string(home) + "/.MCOP-random-seed";
	md5_auth_init_seed(seedpath.c_str());

	/*
	 * first generate a new random cookie and try to set secret-cookie to it
	 * as put will not overwrite, this has no effect if there is already a
	 * secret cookie
	 */
	char *cookie = md5_auth_mkcookie();
	globalComm().put("secret-cookie",cookie);
	memset(cookie,0,strlen(cookie));	// try to keep memory clean
	free(cookie);

	/*
	 * Then get the secret cookie from globalComm. As we've just set one,
	 * and as it is never removed, this always works.
	 */
	string secretCookie = globalComm().get("secret-cookie");
	md5_auth_set_cookie(secretCookie.c_str());
	string::iterator i;	// try to keep memory clean from secret cookie
	for(i=secretCookie.begin();i != secretCookie.end();i++) *i = 'y';
}

Dispatcher::~Dispatcher()
{
	/* no interaction possible now anymore - remove our global references */
	if(objectManager)
		objectManager->removeGlobalReferences();

	/* remove everything that might have been tagged for remote copying */
	referenceClean->forceClean();
	delete referenceClean;

	d->globalComm = GlobalComm::null();

	/* shutdown all extensions we loaded */
	if(objectManager)
		objectManager->shutdownExtensions();

	StartupManager::shutdown();

	/* drop all open connections */
	list<Connection *>::iterator ci;
	for(ci=connections.begin(); ci != connections.end();ci++)
	{
		Connection *conn = *ci;
		conn->drop();
	}

	/*
	 * remove signal handler for SIGPIPE
	 */
	signal(SIGPIPE,orig_sigpipe);


	d->interfaceRepo = InterfaceRepo::null();

	if(d->accept)
	{
		delete d->accept;
		d->accept = 0;
	}

	if(d->loopbackConnection)
	{
		d->loopbackConnection->_release();
		d->loopbackConnection = 0;
	}
	if(unixServer)
	{
		delete unixServer;
		unixServer = 0;
	}

	if(tcpServer)
	{
		delete tcpServer;
		tcpServer = 0;
	}

	if(notificationManager)
	{
		delete notificationManager;
		notificationManager = 0;
	}

	if(objectManager && Object_base::_objectCount() == 0)
	{
		objectManager->removeExtensions();
		delete objectManager;
		objectManager = 0;
	}

	if(deleteIOManagerOnExit)
	{
		delete _ioManager;
		_ioManager = 0;
	}

	if(Object_base::_objectCount())
	{
		cerr << "warning: leaving MCOP Dispatcher and still "
			 << Object_base::_objectCount() << " object references alive." << endl;
		list<Object_skel *> which = objectPool.enumerate();
		list<Object_skel *>::iterator i;
		for(i = which.begin(); i != which.end();i++)
			cerr << "  - " << (*i)->_interfaceName() << endl;
	}

	if(Type::_typeCount())
	{
		cerr << "warning: leaving MCOP Dispatcher and still "
			 << Type::_typeCount() << " types alive." << endl;
	}

	if(GenericDataPacket::_dataPacketCount())
	{
		cerr << "warning: leaving MCOP Dispatcher and still "
			 << GenericDataPacket::_dataPacketCount()
			 << " data packets alive." << endl;
	}


	/* private data pointer */
	assert(d);
	delete d;
	d = 0;

	assert(_instance);
	_instance = 0;
}

InterfaceRepo Dispatcher::interfaceRepo()
{
	return d->interfaceRepo;
}

FlowSystem_impl *Dispatcher::flowSystem()
{
	assert(_flowSystem);
	return _flowSystem;
}

GlobalComm Dispatcher::globalComm()
{
	assert(!d->globalComm.isNull());
	return d->globalComm;
}

void Dispatcher::setFlowSystem(FlowSystem_impl *fs)
{
	assert(!_flowSystem);
	_flowSystem = fs;
}

Dispatcher *Dispatcher::the()
{
	assert(_instance);
	return _instance;
}

Buffer *Dispatcher::waitForResult(long requestID, Connection *connection)
{
	Buffer *b = requestResultPool[requestID];

	while(!b && !connection->broken()) {
		_ioManager->processOneEvent(true);
		b = requestResultPool[requestID];
	}

	requestResultPool.releaseSlot(requestID);

	if(connection->broken()) // connection went away before we got some result
		return 0;
	else
		return b;
}

Buffer *Dispatcher::createRequest(long& requestID, long objectID, long methodID)
{
	Buffer *buffer = new Buffer;

	// write mcop header record
	buffer->writeLong(MCOP_MAGIC);
	buffer->writeLong(0);			// message length - to be patched later
	buffer->writeLong(mcopInvocation);

	// generate a request ID
	requestID = requestResultPool.allocSlot();

	// write invocation record
	buffer->writeLong(objectID);
	buffer->writeLong(methodID);
	buffer->writeLong(requestID);

	return buffer;
}

Buffer *Dispatcher::createOnewayRequest(long objectID, long methodID)
{
	Buffer *buffer = new Buffer;

	// write mcop header record
	buffer->writeLong(MCOP_MAGIC);
	buffer->writeLong(0);			// message length - to be patched later
	buffer->writeLong(mcopOnewayInvocation);

	// write oneway invocation record
	buffer->writeLong(objectID);
	buffer->writeLong(methodID);

	return buffer;
}

void Dispatcher::handle(Connection *conn, Buffer *buffer, long messageType)
{
	_activeConnection = conn;

#ifdef DEBUG_IO
	printf("got a message %ld, %ld bytes in body\n",
			messageType,buffer->remaining());
	if(conn->connState() == Connection::unknown)
		cout << "connectionState = unknown" << endl;
	if(conn->connState() == Connection::expectClientHello)
		cout << "connectionState = expectClientHello" << endl;
	if(conn->connState() == Connection::expectServerHello)
		cout << "connectionState = expectServerHello" << endl;
	if(conn->connState() == Connection::expectAuthAccept)
		cout << "connectionState = expectAuthAccept" << endl;
	if(conn->connState() == Connection::established)
		cout << "connectionState = established" << endl;
#endif
	switch(conn->connState())
	{
		case Connection::established:
			/*
			 * we're connected to a trusted server, so we can accept
			 * invocations
			 */
			if(messageType == mcopInvocation) {
#ifdef DEBUG_MESSAGES
		printf("[got Invocation]\n");
#endif
				long objectID = buffer->readLong();
				long methodID = buffer->readLong();
				long requestID = buffer->readLong();

				Buffer *result = new Buffer;
				// write mcop header record
				result->writeLong(MCOP_MAGIC);
				result->writeLong(0);	// message length - to be patched later
				result->writeLong(mcopReturn);

				// write result record (returnCode is written by dispatch)
				result->writeLong(requestID);
	
				objectPool[objectID]->_dispatch(buffer,result,methodID);

				assert(!buffer->readError() && !buffer->remaining());
				delete buffer;

				result->patchLength();
				conn->qSendBuffer(result);

				return;		/* everything ok - leave here */
			}

			if(messageType == mcopReturn)
			{
#ifdef DEBUG_MESSAGES
				printf("[got Return]\n");
#endif
				long requestID = buffer->readLong();
				requestResultPool[requestID] = buffer;

				return;		/* everything ok - leave here */
			}

			if(messageType == mcopOnewayInvocation) {
#ifdef DEBUG_MESSAGES
		printf("[got OnewayInvocation]\n");
#endif
				long objectID = buffer->readLong();
				long methodID = buffer->readLong();

				objectPool[objectID]->_dispatch(buffer,methodID);

				assert(!buffer->readError() && !buffer->remaining());
				delete buffer;

				return;		/* everything ok - leave here */
			}
			break;

		case Connection::expectServerHello:
			if(messageType == mcopServerHello)
			{
#ifdef DEBUG_MESSAGES
				printf("[got ServerHello]\n");
#endif
				/*
		 		 * if we get a server hello, answer with a client hello
		 		 */
				ServerHello h;
				h.readType(*buffer);
				bool valid = (!buffer->readError() && !buffer->remaining());
				delete buffer;

				if(!valid) break;		// invalid hello received -> forget it

				conn->setServerID(h.serverID);

				/*
				 * check if md5auth or noauth is offered by the server
				 */
				bool md5authSupported = false;
				bool noauthSupported = false;
				vector<string>::iterator ai;
				for(ai = h.authProtocols.begin(); ai != h.authProtocols.end(); ai++)
				{
					if(*ai == "md5auth") md5authSupported = true;
					if(*ai == "noauth")  noauthSupported = true;
				}

				if(noauthSupported)		// noauth is usually easier to pass ;)
				{
					Buffer *helloBuffer = new Buffer;

					Header header(MCOP_MAGIC,0,mcopClientHello);
					header.writeType(*helloBuffer);
					ClientHello clientHello(serverID,"noauth","");
					clientHello.writeType(*helloBuffer);

					helloBuffer->patchLength();

					conn->qSendBuffer(helloBuffer);
					conn->setConnState(Connection::expectAuthAccept);
					return;		/* everything ok - leave here */
				}
				else if(md5authSupported)
				{
					Buffer *helloBuffer = new Buffer;

					Header header(MCOP_MAGIC,0,mcopClientHello);
					header.writeType(*helloBuffer);
					ClientHello clientHello(serverID,"md5auth","");

					const char *random_cookie = h.authSeed.c_str();
					if(strlen(random_cookie) == 32)
					{
						char *response = md5_auth_mangle(random_cookie);
						clientHello.authData = response;
#ifdef DEBUG_AUTH
						printf("  got random_cookie = %s\n",random_cookie);
						printf("reply with authData = %s\n",response);
#endif
						free(response);
					}
					clientHello.writeType(*helloBuffer);

					helloBuffer->patchLength();

					conn->qSendBuffer(helloBuffer);
					conn->setConnState(Connection::expectAuthAccept);
					return;		/* everything ok - leave here */
				}
				else
				{
					cerr << "MCOP error: don't know authentication protocol" << endl;
					cerr << "   server offered: ";
					for(ai = h.authProtocols.begin(); ai != h.authProtocols.end(); ai++)
						cerr << *ai << " ";
					cerr << endl;
				}
			}
			break;

		case Connection::expectClientHello:
			if(messageType == mcopClientHello)
			{
#ifdef DEBUG_MESSAGES
				printf("[got ClientHello]\n");
#endif
				ClientHello c;
				c.readType(*buffer);
				bool valid = (!buffer->readError() && !buffer->remaining());
				delete buffer;

				if(valid && (
				       (c.authProtocol == "md5auth" && c.authData == conn->cookie())
					|| (c.authProtocol == "noauth"  && d->allowNoAuthentication) ))
				{
					conn->setServerID(c.serverID);
	
					/* build hints only for the first connection */
					if(!d->accept)
					{
						d->accept = new AuthAccept();

						d->accept->hints.push_back(
							"GlobalComm="+d->globalComm.toString());
						d->accept->hints.push_back(
							"InterfaceRepo="+d->interfaceRepo.toString());
					}
	
					Buffer *helloBuffer = new Buffer;
					Header header(MCOP_MAGIC,0,mcopAuthAccept);
					header.writeType(*helloBuffer);
					d->accept->writeType(*helloBuffer);
	
					helloBuffer->patchLength();
					conn->qSendBuffer(helloBuffer);
					conn->setConnState(Connection::established);

					return;		/* everything ok - leave here */
				}
			}
			break;

		case Connection::expectAuthAccept:
			if(messageType == mcopAuthAccept)
			{
#ifdef DEBUG_MESSAGES
				printf("[got AuthAccept]\n");
#endif
				AuthAccept a;
				a.readType(*buffer);
				delete buffer;
#ifdef DEBUG_MESSAGES

				vector<string>::iterator hi;
				for(hi = a.hints.begin(); hi != a.hints.end(); hi++)
					cout << "[got ConnectionHint] " << *hi << endl;

#endif

				conn->setConnState(Connection::established);

				return;		/* everything ok - leave here */
			}
			break;

		case Connection::unknown:
			assert(false);
			break;
	}

	/*
	 * We shouldn't reach this point if everything went all right
	 */
	cerr << "Fatal communication error with a client" << endl;
	if(conn->connState() != Connection::established)
	{
		cerr << "  Authentication of this client was not successful" << endl;
		cerr << "  Connection dropped" << endl;
		conn->drop();
	}
}

long Dispatcher::addObject(Object_skel *object)
{
	long objectID = objectPool.allocSlot();

	objectPool[objectID] = object;
	return objectID;
}

void Dispatcher::removeObject(long objectID)
{
	assert(objectPool[objectID]);
	objectPool.releaseSlot(objectID);
}

void Dispatcher::generateServerID()
{
	char buffer[4096];
	sprintf(buffer,"%s-%04x-%08lx",MCOPUtils::getFullHostname().c_str(),
									getpid(),time(0));
	serverID = buffer;
}

string Dispatcher::objectToString(long objectID)
{
	Buffer b;
	ObjectReference oref;

	oref.serverID = serverID;
	oref.objectID = objectID;

	// prefer a unix domainsocket connection over a plain tcp connection
	if(unixServer) oref.urls.push_back(unixServer->url());
	if(tcpServer) oref.urls.push_back(tcpServer->url());

	oref.writeType(b);

	return b.toString("MCOP-Object");
}

bool Dispatcher::stringToObjectReference(ObjectReference& r, string s)
{
	if(strncmp(s.c_str(),"global:",7) == 0)
	{
		// if the object reference starts with "global:", it refers to
		// a global object which can be found with the objectManager

		string lookup = objectManager->getGlobalReference(&s.c_str()[7]);
		return stringToObjectReference(r,lookup);
	}


	Buffer b;
	if(!b.fromString(s,"MCOP-Object")) return false;

	r.readType(b);
	if(b.readError() || b.remaining()) return false;

	return true;
}

void *Dispatcher::connectObjectLocal(ObjectReference& reference,
													string interface)
{
	if(reference.serverID == serverID)
	{
		void *result = objectPool[reference.objectID]->_cast(interface);

		if(result)
		{
			objectPool[reference.objectID]->_copy();
			return result;
		}
	}

	return 0;
}

Connection *Dispatcher::connectObjectRemote(ObjectReference& reference)
{
	if(reference.serverID == "null")		// null reference?
		return 0;

	list<Connection *>::iterator i;

	for(i=connections.begin(); i != connections.end();i++)
	{
		Connection *conn = *i;

		if(conn->isConnected(reference.serverID))
		{
			// fixme: we should check for the existence of the object
			// and increment a reference count or something like that
			return conn;
		}
	}

	/* try to connect the server */

	vector<string>::iterator ui;
	for(ui = reference.urls.begin(); ui != reference.urls.end(); ui++)
	{
		Connection *conn = 0;
		if(strncmp(ui->c_str(),"tcp:",4) == 0)
		{
			conn = new TCPConnection(*ui);
		}
		else if(strncmp(ui->c_str(),"unix:",5) == 0)
		{
			conn = new UnixConnection(*ui);
		}

		if(conn)
		{
			conn->setConnState(Connection::expectServerHello);

			while((conn->connState() != Connection::established)
			       && !conn->broken())
			{
				_ioManager->processOneEvent(true);
			}

			if(conn->connState() == Connection::established)
			{
				connections.push_back(conn);

				assert(conn->isConnected(reference.serverID));
				return conn;
			}
			arts_debug("bad luck: connecting server didn't work");
			
			// well - bad luck (connecting that server failed)
			conn->_release();
		}
	}
	return 0;
}

void Dispatcher::run()
{
	_ioManager->run();
}

void Dispatcher::terminate()
{
	_ioManager->terminate();
}

void Dispatcher::initiateConnection(Connection *connection)
{
	vector<string> authProtocols;
	authProtocols.push_back("md5auth");

	if(d->allowNoAuthentication)
		authProtocols.push_back("noauth");

	char *authSeed = md5_auth_mkcookie();
	char *authResult = md5_auth_mangle(authSeed);

	Buffer *helloBuffer = new Buffer;

	Header header(MCOP_MAGIC,0,mcopServerHello);
	header.writeType(*helloBuffer);
	ServerHello serverHello("aRts/MCOP-1.0.0",serverID,authProtocols,authSeed);
	serverHello.writeType(*helloBuffer);

	helloBuffer->patchLength();

	connection->qSendBuffer(helloBuffer);
	connection->setConnState(Connection::expectClientHello);

	connection->setCookie(authResult);
	free(authSeed);
	free(authResult);

	connections.push_back(connection);
}

void Dispatcher::handleCorrupt(Connection *connection)
{
	if(connection->connState() != Connection::established)
	{
		cerr << "received corrupt message on unauthenticated connection" <<endl;
		cerr << "closing connection." << endl;
		connection->drop();
	}
	else
	{
		cerr << "WARNING: got corrupt MCOP message !??" << endl;
	}
}

void Dispatcher::handleConnectionClose(Connection *connection)
{
	/*
	 * we can't use enumerate here, because the "existing objects list" might
	 * be changing due to the other _disconnectRemote calls we make, so we
	 * enumerate() the objects manually
	 */
	unsigned long l, max;
	for(l=0, max = objectPool.max(); l<max; l++)
	{
		Object_skel *skel = objectPool[l]; 
		if(skel) skel->_disconnectRemote(connection);
	}

	/*
	 * FIXME:
	 *
	 * there may be error handling to do (e.g., check that the _stub's that
	 * still refer to that connection don't crash now).
	 */
	connection->_release();

	list<Connection *>::iterator i;
	for(i=connections.begin(); i != connections.end();i++)
	{
		if(*i == connection)
		{
			connections.erase(i);
			return;
		}
	}
}

Connection *Dispatcher::activeConnection()
{
	return _activeConnection;
}

Connection *Dispatcher::loopbackConnection()
{
	return d->loopbackConnection;
}
