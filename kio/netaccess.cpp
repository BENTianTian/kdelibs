/*  $Id$

    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999 David Faure (faure@kde.org)

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

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <qstring.h>
#include <qapplication.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>

#include "kio/netaccess.h"

using namespace KIO;

bool NetAccess::download(const KURL& u, QString & target, Job *job)
{
  if (u.isLocalFile()) {
    // file protocol. We do not need the network
    target = u.path();
    return true;
  }

  if (!job) kDebugFatal("Setting the job in KIO::NetAccess is not allowed anymore!");

  if (target.isEmpty())
  {
      target = tmpnam(0);  // Generate a temp file name
      if (!tmpfiles)
	  tmpfiles = new QStringList;
      tmpfiles->append(target);
  }

  NetAccess kioNet;
  QString dest( target );
  KURL::encode(dest);
  return kioNet.copyInternal( u, KURL( dest ) );
}

bool NetAccess::upload(const QString& src, const KURL& target, Job *job)
{
  if (target.isEmpty())
    return false;

  if (!job) kDebugFatal("Setting the job in KIO::NetAccess is not allowed anymore!");

  // TODO : What do we do if target.isLocalFile() ? Copy ? Nothing ?

  NetAccess kioNet;
  QString s(src);
  KURL::encode(s);
  return kioNet.copyInternal( KURL(s), target );
}

bool NetAccess::copy( const KURL & src, const KURL & target )
{
  NetAccess kioNet;
  return kioNet.copyInternal( src, target );
}

bool NetAccess::exists( const KURL & url )
{
  NetAccess kioNet;
  return kioNet.existsInternal( url );
}

bool NetAccess::del( const KURL & url )
{
  NetAccess kioNet;
  return kioNet.delInternal( url );
}

QStringList* NetAccess::tmpfiles = 0L;

void NetAccess::removeTempFile(const QString& name)
{
  if (!tmpfiles)
    return;
  if (tmpfiles->contains(name))
  {
    unlink(name);
    tmpfiles->remove(name);
  }
}

bool NetAccess::copyInternal(const KURL& src, const KURL& target)
{
  bDownloadOk = true; // success unless further error occurs

  KIO::Job * job = KIO::file_copy( src, target );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  qApp->enter_loop();
  return bDownloadOk;
}

bool NetAccess::existsInternal( const KURL & url )
{
  bDownloadOk = true; // success unless further error occurs
  KIO::Job * job = KIO::stat( url );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  qApp->enter_loop();
  return bDownloadOk;
}

bool NetAccess::delInternal( const KURL & url )
{
  bDownloadOk = true; // success unless further error occurs
  bDisplayErrors = true;
  KIO::Job * job = KIO::del( url );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  qApp->enter_loop();
  return bDownloadOk;
}

void NetAccess::slotResult( KIO::Job * job )
{
  bDownloadOk = !job->error();
  if ( job->error() && bDisplayErrors )
    job->showErrorDialog();

  qApp->exit_loop();
}

#include "netaccess.moc"
