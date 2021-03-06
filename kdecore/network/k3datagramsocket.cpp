/*  -*- C++ -*-
 *  Copyright (C) 2003,2004 Thiago Macieira <thiago@kde.org>
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "k3datagramsocket.h"

#include <config.h>
#include <config-network.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "k3socketaddress.h"
#include "k3resolver.h"
#include "k3socketdevice.h"

using namespace KNetwork;

/*
 * TODO:
 *
 * don't use signals and slots to track state changes: use stateChanging
 *
 */

KDatagramSocket::KDatagramSocket(QObject* parent)
  : KClientSocketBase(parent), d(0L)
{
  peerResolver().setFamily(KResolver::KnownFamily);
  localResolver().setFamily(KResolver::KnownFamily);

  peerResolver().setSocketType(SOCK_DGRAM);
  localResolver().setSocketType(SOCK_DGRAM);

  localResolver().setFlags(KResolver::Passive);

  //  QObject::connect(localResolver(), SIGNAL(finished(const KNetwork::KResolverResults&))
  //		   this, SLOT(lookupFinishedLocal()));
  QObject::connect(&peerResolver(),
		   SIGNAL(finished(const KNetwork::KResolverResults&)),
  		   this, SLOT(lookupFinishedPeer()));
  QObject::connect(this, SIGNAL(hostFound()), this, SLOT(lookupFinishedLocal()));
}

KDatagramSocket::~KDatagramSocket()
{
  // KClientSocketBase's destructor closes the socket

  //delete d;
}

bool KDatagramSocket::bind(const QString& node, const QString& service)
{
  if (state() >= Bound)
    return false;

  if (localResolver().isRunning())
    localResolver().cancel(false);

  // no, we must do a host lookup
  localResolver().setAddress(node, service);

  if (!lookup())
    return false;

  // see if lookup has finished already
  // this also catches blocking mode, since lookup has to finish
  // its processing if we're in blocking mode
  if (state() > HostLookup)
    return doBind();

  return true;
}

bool KDatagramSocket::bind(const KResolverEntry& entry)
{
  return KClientSocketBase::bind(entry);
}

bool KDatagramSocket::connect(const QString& node, const QString& service,
			      OpenMode mode)
{
  if (state() >= Connected)
    return true;		// already connected

  if (peerResolver().nodeName() != node ||
      peerResolver().serviceName() != service)
    peerResolver().setAddress(node, service); // this resets the resolver's state

  // KClientSocketBase::lookup only works if the state is Idle or HostLookup
  // therefore, we store the old state, call the lookup routine and then set
  // it back.
  SocketState s = state();
  setState(s == Connecting ? HostLookup : Idle);
  bool ok = lookup();
  if (!ok)
    {
      setState(s);		// go back
      return false;
    }

  // check if lookup is finished
  // if we're in blocking mode, then the lookup has to be finished
  if (state() == HostLookup)
    {
      // it hasn't finished
      setState(Connecting);
      emit stateChanged(Connecting);
      return true;
    }

  // it has to be finished here
  if (state() != Connected)
    {
      setState(Connecting);
      emit stateChanged(Connecting);
      lookupFinishedPeer();
    }

  KActiveSocketBase::open(mode | Unbuffered);
  return state() == Connected;
}

bool KDatagramSocket::connect(const KResolverEntry& entry, OpenMode mode)
{
    return KClientSocketBase::connect(entry, mode);
}

KDatagramPacket KDatagramSocket::receive()
{
  qint64 size = bytesAvailable();
  if (size == 0)
    {
      // nothing available yet to read
      // wait for data if we're not blocking
      if (blocking())
	socketDevice()->waitForMore(-1); // wait forever
      else
	{
	  // mimic error
	  setError(WouldBlock);
	  emit gotError(WouldBlock);
	  return KDatagramPacket();
	}

      // try again
      size = bytesAvailable();
    }

  QByteArray data;
  data.resize(size);
  KSocketAddress address;

  // now do the reading
  size = read(data.data(), size, address);
  if (size < 0)
    // error has been set
    return KDatagramPacket();

  data.resize(size);		// just to be sure
  return KDatagramPacket(data, address);
}

qint64 KDatagramSocket::send(const KDatagramPacket& packet)
{
  return write(packet.data(), packet.size(), packet.address());
}

qint64 KDatagramSocket::writeData(const char *data, qint64 len,
				  const KSocketAddress* to)
{
  if (to->family() != AF_UNSPEC)
    {
      // make sure the socket is open at this point
      if (!socketDevice()->isOpen())
	// error handling will happen below
	socketDevice()->create(to->family(), SOCK_DGRAM, 0);
    }
  return KClientSocketBase::writeData(data, len, to);
}

void KDatagramSocket::lookupFinishedLocal()
{
  // bind lookup has finished and succeeded
  // state() == HostFound

  if (!doBind())
    return;			// failed binding

  if (peerResults().count() > 0)
    {
      setState(Connecting);
      emit stateChanged(Connecting);

      lookupFinishedPeer();
    }
}

void KDatagramSocket::lookupFinishedPeer()
{
  // this function is called by lookupFinishedLocal above
  // and is also connected to a signal
  // so it might be called twice.

  if (state() != Connecting)
    return;

  if (peerResults().count() == 0)
    {
      setState(Unconnected);
      emit stateChanged(Unconnected);
      return;
    }

  KResolverResults::ConstIterator it = peerResults().begin();
  for ( ; it != peerResults().end(); ++it)
    if (connect(*it))
      {
	// weee, we connected

	setState(Connected);	// this sets up signals
	//setupSignals();	// setState sets up the signals

	emit stateChanged(Connected);
	emit connected(*it);
	return;
      }

  // no connection
  copyError();
  setState(Unconnected);
  emit stateChanged(Unconnected);
  emit gotError(error());
}

bool KDatagramSocket::doBind()
{
  if (localResults().count() == 0)
    return true;
  if (state() >= Bound)
    return true;		// already bound

  KResolverResults::ConstIterator it = localResults().begin();
  for ( ; it != localResults().end(); ++it)
    if (bind(*it))
      {
	// bound
	setupSignals();
	KActiveSocketBase::open(ReadWrite | Unbuffered);
	return true;
      }

  // not bound
  // no need to set state since it can only be HostFound already
  copyError();
  emit gotError(error());
  return false;
}

void KDatagramSocket::setupSignals()
{
  QSocketNotifier *n = socketDevice()->readNotifier();
  if (n)
    {
      n->setEnabled(emitsReadyRead());
      QObject::connect(n, SIGNAL(activated(int)), this, SLOT(slotReadActivity()));
    }
  else
    return;

  n = socketDevice()->writeNotifier();
  if (n)
    {
      n->setEnabled(emitsReadyWrite());
      QObject::connect(n, SIGNAL(activated(int)), this, SLOT(slotWriteActivity()));
    }
  else
    return;
}

#include "k3datagramsocket.moc"
