#include "networkserver.h"
#include "networkpacketmanipulator.h"
#include <QThread>

using namespace Networking;

NetworkServer::NetworkServer(const quint16 _listenPort, const QHostAddress & _hostAddress, const int _maxConnections)
	: mHostAddress(_hostAddress),
	mListenPort(_listenPort),
	mServer(nullptr),
	mPacketCounter(0)
{
	mServer = new QTcpServer(this);
	mServer->setObjectName("NetworkServer");

	QObject::connect(mServer, SIGNAL(newConnection()), this, SLOT(tcpNewConnection()));
	QObject::connect(mServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(tcpAcceptError(QAbstractSocket::SocketError)));

	mServer->setMaxPendingConnections(_maxConnections);
}

NetworkServer::~NetworkServer(void)
{
	if (mServer)
		delete mServer;

	mServer = nullptr;
}

bool NetworkServer::startServer()
{
	return mServer->listen(mHostAddress, mListenPort);
}

void NetworkServer::stopServer()
{
	if (mServer->isListening())
		mServer->close();

	mRWClients.tryLockForWrite();
	mClientSockets.clear();
	mRWClients.unlock();
}

bool NetworkServer::isListening()
{
	return mServer->isListening();
}

void NetworkServer::setMaxClients(int _maxConnections)
{
	mServer->setMaxPendingConnections(_maxConnections);
}

int NetworkServer::maxClients()
{
	return mServer->maxPendingConnections();
}

bool NetworkServer::hasAnyClient()
{
	bool res = false;

	mRWClients.lockForRead();
	res = mClientSockets.size() > 0;
	mRWClients.unlock();

	return res;
}

void NetworkServer::pauseClientAcception()
{
	mServer->pauseAccepting();
}
void NetworkServer::resumeClientAcception()
{
	mServer->resumeAccepting();
}

bool NetworkServer::sendPacket(NetworkPacket & _packet, NetworkClientInfo & _clientInfo)
{
	bool res = false;

	mRWClients.lockForRead();
	for (int i = 0; i < mClientSockets.count(); i++)
	{
		QTcpSocket *socket = mClientSockets.at(i);

		if (socket->peerAddress() == _clientInfo.getAddress() && socket->peerPort() == _clientInfo.getPort())
		{
			if (socket->state() == QTcpSocket::ConnectedState)
			{
				res = true;
				_packet.setId(++mPacketCounter);

				QList<NetworkPacket> nps;
				if (NetworkPacketManipulator::split(_packet, &nps))
				{
					int len = 0;
					for each(NetworkPacket np in nps)
					{
						QByteArray & ba = np.toBuffer();
						len = ba.length();
						if (len != socket->write(ba, len))
						{
							res = false;
							break;
						}
					}
				}
			}

			break;
		}
	}
	mRWClients.unlock();

	return res;
}

QList<NetworkClientInfo> NetworkServer::clients()
{
	QList<NetworkClientInfo> list;

	mRWClients.lockForRead();
	for (QList<QTcpSocket*>::Iterator it = mClientSockets.begin(); it != mClientSockets.end(); it++)
	{
		NetworkClientInfo info;
		info.setAddress((*it)->peerAddress());
		info.setPort((*it)->peerPort());
		list.append(info);
	}
	mRWClients.unlock();

	return list;
}

quint32 NetworkServer::packetCounter()
{
	return mPacketCounter;
}

QString NetworkServer::lastError()
{
	return mServer->errorString();
}

QString NetworkServer::lastClientError(NetworkClientInfo & _clientInfo)
{
	QString err;

	mRWClients.lockForRead();
	for (int i = 0; i < mClientSockets.count(); i++)
	{
		QTcpSocket *socket = mClientSockets.at(i);
		if (socket->peerAddress() == _clientInfo.getAddress() && socket->peerPort() == _clientInfo.getPort())
		{
			err = socket->errorString();
			break;
		}
	}
	mRWClients.unlock();

	return err;
}

#pragma region Slots
void NetworkServer::tcpNewConnection()
{
	QTcpSocket* socket = mServer->nextPendingConnection();

	QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
	QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()), Qt::QueuedConnection); //Qt::QueuedConnection

	mPartialDataBuffer.insert(socket, QList<NetworkPacket>());

	mRWClients.lockForWrite();
	mClientSockets.append(socket);
	mRWClients.unlock();

	emit clientConnected(NetworkClientInfo(socket->peerAddress(), socket->peerPort()));
}

void NetworkServer::tcpAcceptError(QAbstractSocket::SocketError _socketError)
{
	emit error(_socketError);
}

void NetworkServer::socketDisconnected()
{
	QTcpSocket* socket = qobject_cast<QTcpSocket *>(QObject::sender());
	if (socket)
	{
		emit clientDisconnected(NetworkClientInfo(socket->peerAddress(), socket->peerPort()));

		mRWClients.lockForWrite();
		int i = mClientSockets.indexOf(socket);
		if (i >= 0)
			mClientSockets.removeAt(i);
		mRWClients.unlock();

		mPartialDataBuffer.remove(socket);

		socket->deleteLater();
	}
}

void NetworkServer::socketError(QAbstractSocket::SocketError _socketError)
{
	QTcpSocket* socket = qobject_cast<QTcpSocket *>(QObject::sender());
	emit clientError(NetworkClientInfo(socket->peerAddress(), socket->peerPort()), _socketError);
}

void NetworkServer::socketReadyRead()
{
	QTcpSocket* socket = qobject_cast<QTcpSocket *>(QObject::sender());

	int totalBytes = socket->bytesAvailable();

	while (totalBytes >= NetworkingGlobals::FixedBufferSize)
	{
		QByteArray & data = socket->read(NetworkingGlobals::FixedBufferSize);
		NetworkPacket np(data);

		if (np.packetType() == NPT_DATA)
		{
			// partial data end check
			if (mPartialDataBuffer[socket].size() > 0)
			{
				mPartialDataBuffer[socket].clear();
				emit error(QAbstractSocket::NetworkError);
			}

			emit packetReceived(NetworkClientInfo(socket->peerAddress(), socket->peerPort()), np);
		}
		else if (np.packetType() == NPT_PARTIAL_DATA)
		{
			mPartialDataBuffer[socket].append(np);
		}
		else if (np.packetType() == NPT_PARTIAL_DATA_END)
		{
			mPartialDataBuffer[socket].append(np);

			NetworkPacket npj(NPT_DATA);
			if (NetworkPacketManipulator::join(mPartialDataBuffer[socket], &npj))
				emit packetReceived(NetworkClientInfo(socket->peerAddress(), socket->peerPort()), npj);
			else
				emit error(QAbstractSocket::NetworkError);

			mPartialDataBuffer[socket].clear();
		}

		totalBytes -= data.size();
	}
}
#pragma endregion ~slots