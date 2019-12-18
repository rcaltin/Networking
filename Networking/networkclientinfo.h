/*
TCP Networking Library

Author: Recep ALTIN
*/

#ifndef NETWORKCLIENTINFO_H
#define NETWORKCLIENTINFO_H

#include <QObject>
#include <QHostAddress>

namespace Networking
{
	class NetworkClientInfo : public QObject
	{
		Q_OBJECT

	public:
		NetworkClientInfo();
		NetworkClientInfo(const NetworkClientInfo & _other);
		NetworkClientInfo(QHostAddress & _address, quint16 _port);
		~NetworkClientInfo(void);

		bool operator==(const NetworkClientInfo & _other) const;

		QHostAddress getAddress();
		void setAddress(QHostAddress & _address);

		quint16 getPort();
		void setPort(quint16 _port);

		QString toString();

	private:
		QHostAddress mAddress;
		quint16 mPort;
	};

	Q_DECLARE_METATYPE(NetworkClientInfo);
}
#endif // NETWORKCLIENTINFO_H