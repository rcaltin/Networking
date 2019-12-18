#include "networking.h"
#include "networkclientinfo.h"
#include "networkpacket.h"

using namespace Networking;

NetworkingGlobals::NetworkingGlobals(void)
{
}

void Networking::NetworkingGlobals::registerMetaTypes()
{
	qRegisterMetaType<Networking::NetworkClientInfo>("Networking::NetworkClientInfo");
	qRegisterMetaType<Networking::NetworkPacket>("Networking::NetworkPacket");
	qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
}

void NetworkingGlobals::getBytes(int _val, char * _p)
{
	for (int i = 0; i < 4; i++)
		*(_p + i) = (unsigned char)(_val >> (i * 8));
}

void NetworkingGlobals::getBytes(uint32 _val, char * _p)
{
	getBytes((int)_val, _p);
}

int NetworkingGlobals::toInt32(char * _p)
{
	return ((unsigned char)*(_p + 3) << 24) | ((unsigned char)*(_p + 2) << 16) | ((unsigned char)*(_p + 1) << 8) | (unsigned char)*_p;
}

NetworkingGlobals::uint32 NetworkingGlobals::toUInt32(char * _p)
{
	return (uint32)toInt32(_p);
}
