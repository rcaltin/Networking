/*
TCP Networking Library

Author: Recep ALTIN
*/

#ifndef NETWORKPACKETMANIPULATOR_H
#define NETWORKPACKETMANIPULATOR_H

#include <QString>
#include <QList>
#include "networking.h"
#include "networkpacket.h"

namespace Networking
{
	class NetworkPacketManipulator
	{
	public:
		~NetworkPacketManipulator(void);
		static bool split(Networking::NetworkPacket & _np, QList<Networking::NetworkPacket> * _nps);
		static bool join(QList<Networking::NetworkPacket> & _nps, Networking::NetworkPacket * _np);

	private:
		NetworkPacketManipulator(void);
	};
}
#endif // NETWORKPACKETMANIPULATOR_H
