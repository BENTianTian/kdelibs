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

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "common.h"
#include "pool.h"
#include "iomanager.h"
#include "tcpserver.h"
#include "unixserver.h"
#include "connection.h"
#include "notification.h"

#include <deque>
#include <stack>
#include <vector>
#include <list>

class ObjectReference;
class FlowSystem_impl;
class GlobalComm;
class InterfaceRepo;
class ObjectManager;
class Object_skel;
class ReferenceClean;

extern "C" {
  typedef void (*mcop_sighandler)(int); 
}

class Dispatcher {
private:
	class DispatcherPrivate *d;

protected:
	static Dispatcher *_instance;

	Pool<Buffer> requestResultPool;
	Pool<Object_skel> objectPool;
	std::list<Connection *> connections;

	std::string serverID;
	void generateServerID();	// helper function

	bool deleteIOManagerOnExit;

	TCPServer *tcpServer;
	UnixServer *unixServer;
	IOManager *_ioManager;
	FlowSystem_impl *_flowSystem;
	ObjectManager *objectManager;
	ReferenceClean *referenceClean;
	NotificationManager *notificationManager;

	mcop_sighandler orig_sigpipe;		// original signal handler for SIG_PIPE
	Connection *_activeConnection;		// internal use only (for refcounting)

public:
	enum StartServer {
		noServer = 0,
		startUnixServer = 1,
		startTCPServer = 2
	};

	Dispatcher(IOManager *ioManager = 0, StartServer startServer = noServer);
	~Dispatcher();

	static Dispatcher *the();
	inline IOManager *ioManager() { return _ioManager; };
	InterfaceRepo interfaceRepo();
	FlowSystem_impl *flowSystem();
	GlobalComm globalComm();
	void setFlowSystem(FlowSystem_impl *fs);

	void refillRequestIDs();

	// blocking wait for result
	Buffer *waitForResult(long requestID,Connection *connection);

	// request creation for oneway and twoway requests
	Buffer *createRequest(long& requestID, long objectID, long methodID);
	Buffer *createOnewayRequest(long objectID, long methodID);

	// processes messages
	void handle(Connection *conn, Buffer *buffer, long messageType);

	/*
	 * special hook to handle corrupt messages
	 */
	void handleCorrupt(Connection *conn);

	/**
	 * object registration
	 * 
	 * do not call manually, this is used by the Object_skel constructor
	 * to register itself
	 */
	long addObject(Object_skel *object);

	/**
	 * object deregistration
	 *
	 * do not call manually, this is called by the Object_skel destructor
	 * as soon as the object is deleted
	 */
	void removeObject(long objectID);

	/**
	 * connects to a local object, returning a readily casted "interface" *
	 * if success, or NULL if this object is not local
	 */
	void *connectObjectLocal(ObjectReference& reference, std::string interface);

	/**
	 * connects to a remote object, establishing a connection if necessary
	 *
	 * returns NULL if connecting fails or object isn't present or whatever
	 * else could go wrong
	 */
	Connection *connectObjectRemote(ObjectReference& reference);

	/**
	 * stringification of objects (only used by the object implementations,
	 * which in turn provide a _toString method)
	 */
	std::string objectToString(long objectID);

	/**
	 * destringification of objects (mainly used by the object implementations,
	 * which in turn provide a _fromString method)
	 */
	bool stringToObjectReference(ObjectReference& r, std::string s);

	/**
	 * main loop
	 */
	void run();

	/**
	 * this function quits the main loop (it must be running, of course)
	 * and causes the run() function to return.
	 */
	void terminate();

	/**
	 * Is called by the transport services as soon as a new connection is
	 * created - this will send authentication request, etc.
	 */
	void initiateConnection(Connection *connection);

	/**
	 * Is called by the Connection as soon as the connection goes down,
	 * which may happen due to a normal cause (client exits), and may
	 * happen if an error occurs as well (network down)
	 */
	void handleConnectionClose(Connection *connection);

	/**
	 * - internal usage only -
	 *
	 * this will return the Connection the last request came from
	 */
	Connection *activeConnection();
};

#endif
