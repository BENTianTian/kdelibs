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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "buffer.h"

namespace Arts {

class Connection {
public:
	enum ConnectionState {
		unknown = 0,
		expectServerHello = 1,
		expectClientHello = 2,
		expectAuthAccept = 3,
		established = 4
	};
protected:
	Buffer *rcbuf;
	bool receiveHeader;
	long remaining;
	long messageType;
	ConnectionState _connState;
	std::string serverID;
	std::string _cookie;

	long _refCnt;

	/**
	 * If you don't want to handle message fragmentation yourself:
	 * 
	 * Call initReceive in your derived Connection constructor, ald
	 * receive as soon as you receive some data - the connection object
	 * will handle the rest (put the messages into buffers and send them
	 * to the dispatcher)
	 */
	void initReceive();
	void receive(unsigned char *data, long len);

	virtual ~Connection();
public:
	Connection();

	inline void setServerID(std::string serverID) { this->serverID = serverID; }
	inline bool isConnected(std::string s) { return (serverID == s); } 
	inline void setConnState(ConnectionState cs) { _connState = cs; };

	inline std::string cookie() { return _cookie; }
	void setCookie(std::string c) { _cookie = c; }

	inline ConnectionState connState() { return _connState; };
	virtual void drop() = 0;
	virtual bool broken() = 0;
	virtual void qSendBuffer(Buffer *buffer) = 0;

	void _release();
	void _copy();
};

};

#endif
