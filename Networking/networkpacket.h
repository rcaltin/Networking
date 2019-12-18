/*
TCP Networking Library

Author: Recep ALTIN
*/

#ifndef NETWORKPACKET_H
#define NETWORKPACKET_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include "networking.h"

namespace Networking
{
	class NetworkPacket : public QObject
	{
		Q_OBJECT

	public:
		NetworkPacket(void);
		NetworkPacket(const NetworkPacket & _other);
		NetworkPacket(QByteArray & _buffer);
		NetworkPacket(NetworkPacketType _packetType);
		virtual ~NetworkPacket(void);

		bool operator==(const NetworkPacket & _other) const;

		virtual void clear();

		bool isDataPacket();
		QByteArray toBuffer();
		void fromBuffer(QByteArray & _buffer);

#pragma region getters-setters
		void setId(const quint32 _packetId);
		quint32 id();

		void setPacketType(const NetworkPacketType _packetType);
		NetworkPacketType packetType();

		void setTypeId(const quint32 _typeId);
		quint32 typeId();

		void setPartOrder(const quint32 _partOrder);
		quint32 partOrder();

		void setPartCount(const quint32 _partCount);
		quint32 partCount();

		void setString(const QString & _strData);
		QString string();

		void setData(const QByteArray & _data);
		QByteArray data();
		QByteArray* dataPtr();
#pragma endregion ~getters-setters

	private:
		quint32
			mId,
			mType,
			mTypeId,
			mPartOrder,
			mPartCount;
		QByteArray mData;
	};

	Q_DECLARE_METATYPE(NetworkPacket);

}
#endif // NETWORKPACKET_H
