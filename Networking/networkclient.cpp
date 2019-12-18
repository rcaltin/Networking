#include "networkclient.h"
#include "networkpacketmanipulator.h"

using namespace Networking;

NetworkClient::NetworkClient(QString _serverIp, quint16 _serverPort)
	: mServerIp(_serverIp),
	mServerPort(_serverPort),
	mSocket(NULL),
	mPacketCounter(0)
{
	mSocket = new QTcpSocket(this);
	mSocket->setObjectName("NetworkClient");
	QObject::connect(mSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
	QObject::connect(mSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	QObject::connect(mSocket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()), Qt::QueuedConnection);
	QObject::connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
}

NetworkClient::~NetworkClient(void)
{
	if (mSocket)
		mSocket->deleteLater();

	mSocket = nullptr;
}

QString Networking::NetworkClient::serverIp()
{
	return mServerIp;
}

void NetworkClient::setServerIp(QString _serverIp)
{
	mServerIp = _serverIp;
}

quint16 Networking::NetworkClient::serverPort()
{
	return mServerPort;
}

void NetworkClient::setServerPort(quint16 _serverPort)
{
	mServerPort = _serverPort;
}

bool NetworkClient::connectToServer(int _timeOut)
{
	mSocket->connectToHost(QHostAddress(mServerIp), mServerPort);
	return mSocket->waitForConnected(_timeOut);
}

bool NetworkClient::disconnectFromServer(int _timeOut)
{
	if (mSocket->state() == QAbstractSocket::ConnectedState)
	{
		mSocket->disconnectFromHost();
		return mSocket->waitForDisconnected(_timeOut);
	}
	return true;
}

QTcpSocket::SocketState NetworkClient::state()
{
	return mSocket->state();
}

bool NetworkClient::sendPacket(NetworkPacket & _packet)
{
	if (mSocket->state() != QTcpSocket::ConnectedState)
		return false;

	_packet.setId(++mPacketCounter);

	QList<NetworkPacket> nps;
	if (NetworkPacketManipulator::split(_packet, &nps))
	{
		int len = 0;
		for each(NetworkPacket np in nps)
		{
			QByteArray & ba = np.toBuffer();
			len = ba.length();
			if (len != mSocket->write(ba, len))
				return false;
		}

		return true;
	}

	return false;
}

NetworkClientInfo NetworkClient::clientInfo()
{
	return NetworkClientInfo(mSocket->peerAddress(), mSocket->peerPort());
}

quint32 NetworkClient::packetCounter()
{
	return mPacketCounter;
}

QString NetworkClient::lastError()
{
	return mSocket->errorString();
}

#pragma region slots
void NetworkClient::socketConnected()
{
	emit connected();
}

void NetworkClient::socketDisconnected()
{
	emit disconnected();
}

void NetworkClient::socketReadyRead()
{
	int totalBytes = mSocket->bytesAvailable();

	while (totalBytes >= NetworkingGlobals::FixedBufferSize)
	{
		QByteArray & data = mSocket->read(NetworkingGlobals::FixedBufferSize);
		NetworkPacket np(data);

		if (np.packetType() == NPT_DATA)
		{
			// partial data end check
			if (mPartialDataBuffer.size() > 0)
			{
				mPartialDataBuffer.clear();
				emit error(QAbstractSocket::NetworkError);
			}

			emit packetReceived(np);
		}
		else if (np.packetType() == NPT_PARTIAL_DATA)
		{
			mPartialDataBuffer.append(np);
		}
		else if (np.packetType() == NPT_PARTIAL_DATA_END)
		{
			mPartialDataBuffer.append(np);

			NetworkPacket npj(NPT_DATA);
			if (NetworkPacketManipulator::join(mPartialDataBuffer, &npj))
				emit packetReceived(npj);
			else
				emit error(QAbstractSocket::NetworkError);

			mPartialDataBuffer.clear();
		}

		totalBytes -= data.size();
	}
}

void NetworkClient::socketError(QAbstractSocket::SocketError _socketError)
{
	emit error(_socketError);
}
#pragma endregion ~slots