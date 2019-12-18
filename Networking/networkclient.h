/*
TCP Networking Library

Author: Recep ALTIN
*/

#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QList>
#include "networking.h"
#include "networkpacket.h"
#include "networkclientinfo.h"

namespace Networking
{
	class NetworkClient : public QObject
	{
		Q_OBJECT

	public:
		NetworkClient(QString _serverIp = NetworkingGlobals::DefaultServerIP, quint16 _serverPort = NetworkingGlobals::DefaultServerPort);
		~NetworkClient(void);

		QString serverIp();
		void setServerIp(QString _serverIp);

		quint16 serverPort();
		void setServerPort(quint16 _serverPort);

		bool connectToServer(int _timeOut = NetworkingGlobals::DefaultTimeOut);
		bool disconnectFromServer(int _timeOut = NetworkingGlobals::DefaultTimeOut);
		QTcpSocket::SocketState state();

		bool sendPacket(NetworkPacket & _packet);

		NetworkClientInfo clientInfo();
		quint32 packetCounter();
		QString lastError();

	private:
		QString mServerIp;
		quint16 mServerPort;
		QTcpSocket * mSocket;
		quint32 mPacketCounter;
		QList<NetworkPacket> mPartialDataBuffer;

#pragma region Signals-Slots
	signals:
		void connected();
		void disconnected();
		void packetReceived(NetworkPacket _packet);
		void error(QAbstractSocket::SocketError _socketError);

	private slots:
		void socketConnected();
		void socketDisconnected();
		void socketReadyRead();
		void socketError(QAbstractSocket::SocketError _socketError);
#pragma endregion
	};
}
#endif // NETWORKCLIENT_H

