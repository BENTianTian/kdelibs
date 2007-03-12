#ifndef KNEWSTUFF_SOAP_H
#define KNEWSTUFF_SOAP_H

#include <qobject.h>

namespace KIO
{
	class Job;
}
class KJob;

class QDomDocument;
class QDomNode;
class QDomElement;
class QTcpSocket;
class QCString;

namespace KNS
{

class Soap : public QObject
{
Q_OBJECT
public:
	Soap();
	~Soap();

	enum Model
	{
		soap,
		canonicaltree
	};

	void setModel(Model m);

	void call(QDomElement element, QString endpoint);
	QString localname(QDomNode node);
	QString xpath(QDomNode node, QString expr);

signals:
	void signalResult(QDomNode node);
	void signalError();

private slots:
	void slotData(KIO::Job *job, const QByteArray& data);
	void slotResult(KJob *job);
	void slotSocket();
	void slotSocketError(int error);

private:
	void call_soap(QDomElement element, QString endpoint);
	void call_tree(QDomElement element, QString endpoint);
	QDomDocument buildtree(QDomDocument doc, QDomElement cur, QString data);

	QString m_data;
	Model m_model;
	QTcpSocket *m_socket;
	QByteArray m_buffer;
	bool m_inprogress;
};

}

#endif
