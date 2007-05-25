// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include "davjob.h"

#include <kurl.h>

#include <QtCore/QObject>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QPointer>
#include <Qt/qdom.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kdebug.h>
#include <kuiserverjobtracker.h>
#include <kio/http.h>

#include "jobclasses.h"
#include "global.h"
#include "job.h"
#include "slaveinterface.h"

#include "jobuidelegate.h"

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( &packedArgs, QIODevice::WriteOnly ); stream

using namespace KIO;

/** @internal */
class DavJob::DavJobPrivate
{
public:
  QByteArray savedStaticData;
	QByteArray str_response; // replaces the QString previously used in DavJob itself
};

DavJob::DavJob(const KUrl& url, int method, const QString& request)
  : TransferJob(url, KIO::CMD_SPECIAL, QByteArray(), QByteArray()),d(new DavJobPrivate)
{
  // We couldn't set the args when calling the parent constructor,
  // so do it now.
  QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
  stream << (int) 7 << url << method;
  // Same for static data
  if ( ! request.isEmpty() && ! request.isNull() ) {
    staticData = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" + request.toUtf8();
    staticData.truncate( staticData.size() - 1 );
    d->savedStaticData = staticData;
  }
}

QDomDocument& DavJob::response()
{
    return m_response;
}

void DavJob::slotData( const QByteArray& data )
{
  if(m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error()) {
    unsigned int oldSize = d->str_response.size();
    d->str_response.resize( oldSize + data.size() );
    memcpy( d->str_response.data() + oldSize, data.data(), data.size() );
  }
}

void DavJob::slotFinished()
{
  // kDebug(7113) << "DavJob::slotFinished()" << endl;
  // kDebug(7113) << d->str_response << endl;
	if (!m_redirectionURL.isEmpty() && m_redirectionURL.isValid() && (m_command == CMD_SPECIAL)) {
		QDataStream istream( m_packedArgs );
		int s_cmd, s_method;
		KUrl s_url;
		istream >> s_cmd;
		istream >> s_url;
		istream >> s_method;
		// PROPFIND
		if ( (s_cmd == 7) && (s_method == (int)KIO::DAV_PROPFIND) ) {
			m_packedArgs.truncate(0);
			QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
			stream << (int)7 << m_redirectionURL << (int)KIO::DAV_PROPFIND;
		}
  } else if ( ! m_response.setContent( d->str_response, true ) ) {
		// An error occurred parsing the XML response
		QDomElement root = m_response.createElementNS( "DAV:", "error-report" );
		m_response.appendChild( root );

		QDomElement el = m_response.createElementNS( "DAV:", "offending-response" );
    QDomText textnode = m_response.createTextNode( d->str_response );
		el.appendChild( textnode );
		root.appendChild( el );
		delete d; // Should be in virtual destructor
	} else {
		delete d; // Should be in virtual destructor
	}
  // kDebug(7113) << m_response.toString() << endl;
	TransferJob::slotFinished();
	if( d ) staticData = d->savedStaticData; // Need to send DAV request to this host too
}

/* Convenience methods */

DavJob* KIO::davPropFind( const KUrl& url, const QDomDocument& properties, const QString &depth, bool showProgressInfo )
{
  DavJob *job = new DavJob(url, (int) KIO::DAV_PROPFIND, properties.toString());
  job->addMetaData( "davDepth", depth );
  job->setUiDelegate(new JobUiDelegate());
  if (showProgressInfo)
      KIO::getJobTracker()->registerJob(job);
  return job;
}


DavJob* KIO::davPropPatch( const KUrl& url, const QDomDocument& properties, bool showProgressInfo )
{
    DavJob *job = new DavJob(url, (int) KIO::DAV_PROPPATCH, properties.toString());
    job->setUiDelegate(new JobUiDelegate());
    if (showProgressInfo)
        KIO::getJobTracker()->registerJob(job);
    return job;
}

DavJob* KIO::davSearch( const KUrl& url, const QString& nsURI, const QString& qName, const QString& query, bool showProgressInfo )
{
  QDomDocument doc;
  QDomElement searchrequest = doc.createElementNS( "DAV:", "searchrequest" );
  QDomElement searchelement = doc.createElementNS( nsURI, qName );
  QDomText text = doc.createTextNode( query );
  searchelement.appendChild( text );
  searchrequest.appendChild( searchelement );
  doc.appendChild( searchrequest );
  DavJob *job = new DavJob(url, KIO::DAV_SEARCH, doc.toString());
  job->setUiDelegate(new JobUiDelegate());
  if (showProgressInfo)
      KIO::getJobTracker()->registerJob(job);
  return job;
}

#include "davjob.moc"
