#include <QCoreApplication>
#include <QString>

#include <iostream>
using namespace std;

#include "ksocketaddress.h"
#include "netif.h"

Test::Test(const QString& interface)
  : QObject(0,0),
    m_interface(0)
{
	m_interface = new KNetworkInterface(interface);
}

void Test::display()
{
	if (m_interface)
	{
		cout << "Interface: " << m_interface->name().latin1() << endl
		     << "Index: " << m_interface->index() << " Flags: " << m_interface->flags() << endl;

		QList<KSocketAddress> addresses = m_interface->address(),
				      netmasks = m_interface->netmask(),
				      broadcasts = m_interface->broadcastAddress(),
				      destinations = m_interface->destinationAddress();
		cout << "Addresses: ";
		foreach (KSocketAddress a, addresses)
		{
			cout << a.toString().latin1() << " ";
		}
		cout << endl;

		cout << "Netmasks: ";
		foreach (KSocketAddress n, netmasks)
		{
			cout << n.toString().latin1() << " ";
		}
		cout << endl;

		cout << "Broadcasts: ";
		foreach (KSocketAddress b, broadcasts)
		{
			cout << b.toString().latin1() << " ";
		}
		cout << endl;

		cout << "Destinations: ";
		foreach (KSocketAddress d, destinations)
		{
			cout << d.toString().latin1() << " ";
		}
		cout << endl;
		
		/*
		     << "Address: " << m_interface->address().toString().latin1() << " Netmask: " << m_interface->netmask().toString().latin1() << endl
		     << "Broadcast: " << m_interface->broadcastAddress().toString().latin1() << " Destination: " << m_interface->destinationAddress().toString().latin1() << endl
		   */
		cout << "Stats:" << endl 
		     << " Received: " << endl
		     << "       Bytes: " << m_interface->receiveBytes()      << "     Packets: " << m_interface->receivePackets() << endl
		     << "      Errors: " << m_interface->receiveErrors()     << "\t     Dropped: " << m_interface->receiveDropped() << endl
		     << "  FifoErrors: " << m_interface->receiveFifoErrors() << " FrameErrors: " << m_interface->receiveFrameErrors() << endl
		     << "  Compressed: " << m_interface->receiveCompressed() << "   Multicast: " << m_interface->receiveMulticast() << endl << endl
                     << "     Sent: " << endl
		     << "       Bytes: " << m_interface->sendBytes()      << "     Packets: " << m_interface->sendPackets() << endl
		     << "      Errors: " << m_interface->sendErrors()     << "     Dropped: " << m_interface->sendDropped() << endl
		     << "  FifoErrors: " << m_interface->sendFifoErrors() << "  Collisions: " << m_interface->sendColls() << endl
		     << "CarrierError: " << m_interface->sendCarrierErrors() << "  Compressed: " << m_interface->sendCompressed() << endl;
	}
}

int main(int argc, char **argv)
{
  QCoreApplication a(argc, argv);

  char i[5]; 
  cout << "Choose interface: ";
  cin >> i;
  cout << endl;
  
  Test test(QString::fromUtf8(i));
  test.display();
       
  return a.exec();
}

#include "netif.moc"

