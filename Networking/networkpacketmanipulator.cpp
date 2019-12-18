#include "networkpacketmanipulator.h"
#include <QtMath>

using namespace Networking;
NetworkPacketManipulator::NetworkPacketManipulator(void)
{
}

NetworkPacketManipulator::~NetworkPacketManipulator(void)
{
}

bool NetworkPacketManipulator::split(Networking::NetworkPacket & _np, QList<NetworkPacket> * _nps)
{
	_np.toBuffer(); // force override to fill buffer

	QByteArray & data = _np.data();
	if (data.size() > NetworkingGlobals::FixedBufferSize)
	{
		int i = 0;
		uint partOrder = 0;
		uint partCount = (uint)(qCeil((double)data.size() / (double)NetworkingGlobals::MaximumDataSize));
		quint32 id = _np.id();
		quint32 type_id = _np.typeId();

		QByteArray ba;
		while (i < data.size())
		{
			ba.append(data.at(i));
			if (++i == ((partOrder + 1) * NetworkingGlobals::MaximumDataSize) || i == data.size())
			{
				NetworkPacket np(i == data.size() ? NPT_PARTIAL_DATA_END : NPT_PARTIAL_DATA);
				np.setId(id);
				np.setTypeId(type_id);
				np.setPartOrder(partOrder++);
				np.setPartCount(partCount);
				np.setData(ba);
				_nps->append(np);
				ba.clear();
			}
		}
	}
	else
		_nps->append(_np);

	return true;
}

bool NetworkPacketManipulator::join(QList<NetworkPacket> & _nps, Networking::NetworkPacket * _np)
{
	if (_nps.size() > 0)
	{
		QByteArray data;
		int ord = 0;

		for (quint32 i = 0; i < _nps.count(); i++)
		{
			NetworkPacket * p = const_cast<NetworkPacket *>(&_nps.at(i));

			if (i != p->partOrder())
				return false;

			data.append(p->data());

			if (p->packetType() == NPT_PARTIAL_DATA_END)
			{
				_np->setId(p->id());
				_np->setTypeId(p->typeId());
				_np->setData(data);
			}
		}
		return true;
	}

	return false;
}
