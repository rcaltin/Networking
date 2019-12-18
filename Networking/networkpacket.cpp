#include "networkpacket.h"
#include <QDataStream>

using namespace Networking;

NetworkPacket::NetworkPacket(void)
{
	clear();

	setPacketType(NPT_NULL);
}

NetworkPacket::NetworkPacket(const NetworkPacket & _other)
{
	clear();

	NetworkPacket & p = const_cast<NetworkPacket &>(_other);
	this->fromBuffer(p.toBuffer());
}

NetworkPacket::NetworkPacket(QByteArray & _buffer)
{
	clear();

	this->fromBuffer(_buffer);
}

NetworkPacket::NetworkPacket(NetworkPacketType _packetType)
{
	clear();

	setPacketType(_packetType);
}

NetworkPacket::~NetworkPacket(void)
{
}

bool NetworkPacket::operator==(const NetworkPacket & _other) const
{
	NetworkPacket & np = const_cast<NetworkPacket &>(_other);
	return
		(np.id() == mId)
		&& (np.packetType() == mType)
		&& (np.typeId() == mTypeId)
		&& (np.partOrder() == mPartOrder)
		&& (np.partCount() == mPartCount)
		&& (np.data() == mData);
}

void NetworkPacket::clear()
{
	mId = 0;
	mType = NPT_NULL;
	mTypeId = 0;
	mPartOrder = 0;
	mPartCount = 0;
}

bool Networking::NetworkPacket::isDataPacket()
{
	return mType == NPT_DATA;
}

QByteArray NetworkPacket::toBuffer()
{
	QByteArray buff;
	QDataStream ds(&buff, QIODevice::WriteOnly);
	ds << mId;
	ds << mType;
	ds << mTypeId;
	ds << mPartOrder;
	ds << mPartCount;
	ds << mData;

	if (buff.size() < NetworkingGlobals::FixedBufferSize)
		buff.resize(NetworkingGlobals::FixedBufferSize);

	return buff;
}

void NetworkPacket::fromBuffer(QByteArray & _buffer)
{
	QDataStream ds(&_buffer, QIODevice::ReadOnly);
	ds >> mId;
	ds >> mType;
	ds >> mTypeId;
	ds >> mPartOrder;
	ds >> mPartCount;
	ds >> mData;
}

#pragma region getters-setters
void NetworkPacket::setId(const quint32 _packetId)
{
	mId = _packetId;
}

quint32 NetworkPacket::id()
{
	return mId;
}

void NetworkPacket::setPacketType(const NetworkPacketType _packetType)
{
	mType = (quint32)_packetType;
}

NetworkPacketType NetworkPacket::packetType()
{
	return (NetworkPacketType)mType;
}

void NetworkPacket::setTypeId(const quint32 _typeId)
{
	mTypeId = _typeId;
}

quint32 NetworkPacket::typeId()
{
	return mTypeId;
}

void NetworkPacket::setString(const QString & _strData)
{
	mData = _strData.toUtf8();
}

QString NetworkPacket::string()
{
	return QString(mData);
}

void NetworkPacket::setPartOrder(const quint32 _partOrder)
{
	mPartOrder = _partOrder;
}

quint32 NetworkPacket::partOrder()
{
	return mPartOrder;
}

void NetworkPacket::setPartCount(const quint32 _partCount)
{
	mPartCount = _partCount;
}

quint32 NetworkPacket::partCount()
{
	return mPartCount;
}

void NetworkPacket::setData(const QByteArray & _data)
{
	mData = _data;
}

QByteArray NetworkPacket::data()
{
	return mData;
}
QByteArray * Networking::NetworkPacket::dataPtr()
{
	return &mData;
}
#pragma endregion ~getters-setters