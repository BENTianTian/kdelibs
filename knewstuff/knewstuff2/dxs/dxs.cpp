#include "dxs.h"

#include "soap.h"

#include <kdebug.h>

#include <qdom.h>
#include <qstringlist.h>

using namespace KNS;

Dxs::Dxs()
: QObject()
{
	m_soap = new Soap();
	connect(m_soap, SIGNAL(signalResult(QDomNode)), SLOT(slotResult(QDomNode)));
}

Dxs::~Dxs()
{
}

void Dxs::setEndpoint(QString endpoint)
{
	m_endpoint = endpoint;
}

void Dxs::call_info()
{
	QDomDocument doc;
	QDomElement info = doc.createElement("ns:GHNSInfo");
	//QDomText t = doc.createTextNode("text");
	//check.appendChild(t);
	m_soap->call(info, m_endpoint);
}

void Dxs::call_categories()
{
	QDomDocument doc;
	QDomElement info = doc.createElement("ns:GHNSCategories");
	m_soap->call(info, m_endpoint);
}

void Dxs::call_comments(int id)
{
	QDomDocument doc;
	QDomElement comments = doc.createElement("ns:GHNSComments");
	QDomElement eid = doc.createElement("id");
	QDomText t = doc.createTextNode(QString::number(id));
	eid.appendChild(t);
	comments.appendChild(eid);
	m_soap->call(comments, m_endpoint);
}

void Dxs::call_changes(int id)
{
	QDomDocument doc;
	QDomElement changes = doc.createElement("ns:GHNSChanges");
	QDomElement eid = doc.createElement("id");
	QDomText t = doc.createTextNode(QString::number(id));
	eid.appendChild(t);
	changes.appendChild(eid);
	m_soap->call(changes, m_endpoint);
}

void Dxs::call_history(int id)
{
	QDomDocument doc;
	QDomElement history = doc.createElement("ns:GHNSHistory");
	QDomElement eid = doc.createElement("id");
	QDomText t = doc.createTextNode(QString::number(id));
	eid.appendChild(t);
	history.appendChild(eid);
	m_soap->call(history, m_endpoint);
}

void Dxs::call_removal(int id)
{
	QDomDocument doc;
	QDomElement removal = doc.createElement("ns:GHNSRemoval");
	QDomElement eid = doc.createElement("id");
	QDomText t = doc.createTextNode(QString::number(id));
	eid.appendChild(t);
	removal.appendChild(eid);
	m_soap->call(removal, m_endpoint);
}

void Dxs::call_subscription(int id, bool subscribe)
{
	QDomDocument doc;
	QDomElement subscription = doc.createElement("ns:GHNSSubscription");
	QDomElement eid = doc.createElement("id");
	QDomText t = doc.createTextNode(QString::number(id));
	eid.appendChild(t);
	subscription.appendChild(eid);
	QDomElement esubscribe = doc.createElement("subscribe");
	QDomText t2 = doc.createTextNode((subscribe ? "true" : "false"));
	esubscribe.appendChild(t2);
	subscription.appendChild(esubscribe);
	m_soap->call(subscription, m_endpoint);
}

void Dxs::call_comment(int id, QString comment)
{
	QDomDocument doc;
	QDomElement ecomment = doc.createElement("ns:GHNSComment");
	QDomElement eid = doc.createElement("id");
	QDomText tid = doc.createTextNode(QString::number(id));
	eid.appendChild(tid);
	ecomment.appendChild(eid);
	QDomElement ecommenttext = doc.createElement("comment");
	QDomText tcomment = doc.createTextNode(comment);
	ecommenttext.appendChild(tcomment);
	ecomment.appendChild(ecommenttext);
	m_soap->call(ecomment, m_endpoint);
}

void Dxs::call_rating(int id, int rating)
{
	QDomDocument doc;
	QDomElement erating = doc.createElement("ns:GHNSRating");
	QDomElement eid = doc.createElement("id");
	QDomText tid = doc.createTextNode(QString::number(id));
	eid.appendChild(tid);
	erating.appendChild(eid);
	QDomElement eratingtext = doc.createElement("rating");
	QDomText trating = doc.createTextNode(QString::number(rating));
	eratingtext.appendChild(trating);
	erating.appendChild(eratingtext);
	m_soap->call(erating, m_endpoint);
}

void Dxs::slotResult(QDomNode node)
{
	kdDebug() << "LOCALNAME: " << m_soap->localname(node) << endl;

	bool success = true;
	if(m_soap->localname(node) == "Fault")
	{
		success = false;
		emit signalFault();
		return;
	}

	if(m_soap->localname(node) == "GHNSInfoResponse")
	{
		QString provider = m_soap->xpath(node, "/SOAP-ENC:Array/provider");
		QString server = m_soap->xpath(node, "/SOAP-ENC:Array/server");
		QString version = m_soap->xpath(node, "/SOAP-ENC:Array/version");

		emit signalInfo(provider, server, version);
	}
	else if(m_soap->localname(node) == "GHNSCategoriesResponse")
	{
		QStringList categories;

		QDomNode array = node.firstChild();
		QDomNodeList catlist = array.toElement().elementsByTagName("categories");
		for(unsigned int i = 0; i < catlist.count(); i++)
		{
			categories << catlist.item(i).toElement().text();
		}

		emit signalCategories(categories);
	}
	else if(m_soap->localname(node) == "GHNSCommentsResponse")
	{
		QStringList comments;

		QDomNode array = node.firstChild();
		QDomNodeList comlist = array.toElement().elementsByTagName("comments");
		for(unsigned int i = 0; i < comlist.count(); i++)
		{
			comments << comlist.item(i).toElement().text();
		}

		emit signalComments(comments);
	}
	else if(m_soap->localname(node) == "GHNSChangesResponse")
	{
		QStringList changes;

		QDomNode array = node.firstChild();
		QDomNodeList changelist = array.toElement().elementsByTagName("entry");
		for(unsigned int i = 0; i < changelist.count(); i++)
		{
			QDomNode node = changelist.item(i);

			QString version = m_soap->xpath(node, "/version");
			QString changelog = m_soap->xpath(node, "/changelog");
			kdDebug() << "CHANGELOG: " << version << " " << changelog << endl;

			changes << changelog;
		}

		// FIXME: pass (version, changelog) pairs - Python I miss you :-)
		emit signalChanges(changes);
	}
	else if(m_soap->localname(node) == "GHNSHistoryResponse")
	{
		QStringList entries;

		QDomNode array = node.firstChild();
		QDomNodeList entrylist = array.toElement().elementsByTagName("entry");
		for(unsigned int i = 0; i < entrylist.count(); i++)
		{
			entries << entrylist.item(i).toElement().text();
		}

		emit signalHistory(entries);
	}
	else if(m_soap->localname(node) == "GHNSRemovalResponse")
	{
		//QString status = m_soap->xpath(node, "/SOAP-ENC:Array/???");

		emit signalRemoval(success);
	}
	else if(m_soap->localname(node) == "GHNSSubscriptionResponse")
	{
		//QString status = m_soap->xpath(node, "/SOAP-ENC:Array/???");

		emit signalSubscription(success);
	}
	else if(m_soap->localname(node) == "GHNSCommentResponse")
	{
		//QString status = m_soap->xpath(node, "/SOAP-ENC:Array/???");

		emit signalComment(success);
	}
	else if(m_soap->localname(node) == "GHNSRatingResponse")
	{
		//QString status = m_soap->xpath(node, "/SOAP-ENC:Array/???");

		emit signalRating(success);
	}
}

#include "dxs.moc"
