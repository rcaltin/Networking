/*
TCP Networking Library

Author: Recep ALTIN
*/

#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QObject>
#include <QTcpServer>
#include "networking.h"
#include "networkpacket.h"
#include "networkclientinfo.h"
#include "networkclient.h"

namespace Networking
{
	class NetworkServer : public QObject
	{
		Q_OBJECT

	public:
		NetworkServer(const quint16 _listenPort = NetworkingGlobals::DefaultServerPort, const QHostAddress & _hostAddress = QHostAddress::Any, const int _maxConnections = 65535);
		~NetworkServer(void);

		bool startServer();
		void stopServer();
		bool isListening();

		void setMaxClients(int _maxConnections);
		int maxClients();
		bool hasAnyClient();

		void pauseClientAcception();
		void resumeClientAcception();

		bool sendPacket(NetworkPacket & _packet, NetworkClientInfo & _clientInfo);

		QList<NetworkClientInfo> clients();
		quint32 packetCounter();
		QString lastError();
		QString lastClientError(NetworkClientInfo & _clientInfo);

	private:
		QHostAddress mHostAddress;
		quint16 mListenPort;
		QTcpServer * mServer;
		QList<QTcpSocket *> mClientSockets;
		quint32 mPacketCounter;
		QMap<QTcpSocket *, QList<NetworkPacket>> mPartialDataBuffer;

#pragma region Signals-Slots
	signals:
		void clientConnected(NetworkClientInfo _clientInfo);
		void clientDisconnected(NetworkClientInfo _clientInfo);
		void clientError(NetworkClientInfo _clientInfo, QAbstractSocket::SocketError _socketError);
		void packetReceived(NetworkClientInfo _clientInfo, NetworkPacket _packet);
		void error(QAbstractSocket::SocketError _socketError);

	private slots:
		void tcpNewConnection();
		void tcpAcceptError(QAbstractSocket::SocketError _socketError);
		void socketDisconnected();
		void socketError(QAbstractSocket::SocketError _socketError);
		void socketReadyRead();
#pragma endregion

	};
}
#endif // NETWORKSERVER_H
