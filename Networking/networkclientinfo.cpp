#include "networkclientinfo.h"

using namespace Networking;

NetworkClientInfo::NetworkClientInfo(void)
{
}

NetworkClientInfo::NetworkClientInfo(const NetworkClientInfo & _other)
{
	mAddress = const_cast<NetworkClientInfo &>(_other).getAddress();
	mPort = const_cast<NetworkClientInfo &>(_other).getPort();
}

NetworkClientInfo::NetworkClientInfo(QHostAddress & _address, quint16 _port)
{
	mAddress = _address;
	mPort = _port;
}

NetworkClientInfo::~NetworkClientInfo(void)
{
}

bool NetworkClientInfo::operator==(const NetworkClientInfo & _other) const
{
	return (const_cast<NetworkClientInfo &>(_other).getAddress() == mAddress) && (const_cast<NetworkClientInfo&>(_other).getPort() == mPort);
}

QHostAddress NetworkClientInfo::getAddress()
{
	return mAddress;
}

void NetworkClientInfo::setAddress(QHostAddress & _address)
{
	mAddress = _address;
}

quint16 NetworkClientInfo::getPort()
{
	return mPort;
}

void NetworkClientInfo::setPort(quint16 _port)
{
	mPort = _port;
}

QString NetworkClientInfo::toString()
{
	bool res = false;
	QHostAddress ipv4Address(mAddress.toIPv4Address(&res));
	return (res ? ipv4Address.toString() : mAddress.toString()) + ":" + QString::number(getPort());
}