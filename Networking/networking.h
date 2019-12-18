/*
TCP Networking Library

Author: Recep ALTIN
*/

#ifndef NETWORKING_H
#define NETWORKING_H

namespace Networking
{
	enum NetworkPacketType
	{
		NPT_NULL,
		NPT_ACK,
		NPT_DATA,
		NPT_PARTIAL_DATA,
		NPT_PARTIAL_DATA_END,
		NPT_LASTITEM
	};

	class NetworkingGlobals
	{
	public:
		typedef unsigned int uint;
		typedef unsigned long int uint32;

		// IMPORTANT! DON'T CHANGE VALUES BELOW!
		//
		// MTU (Maximum Transmission Unit) = 1500 Bytes
		// MSS (Maximum Segment Size) = 40 Bytes (TCP = 20 Bytes + IP = 20 Bytes)
		//
		// By informtion above payload(FixedBufferSize) can be 1500 - 40 = 1460 Bytes to send without fragment in theory 
		// but in practise may user have a VPN traverse or something which will shrink down the payload few more bytes.
		// So I leaved extra 136 Bytes for it then FixedBufferSize becomes 1324 Bytes:
		static constexpr uint MaximumDataSize = 1300;
		static constexpr uint PacketHeaderSize = 24;
		static constexpr uint FixedBufferSize = MaximumDataSize + PacketHeaderSize;
		// ~

		static constexpr auto DefaultServerIP = "127.0.0.1";
		static constexpr uint DefaultServerPort = 11372;
		static constexpr uint DefaultTimeOut = 5000;

		static void registerMetaTypes();

		static void getBytes(int _val, char * _p);
		static void getBytes(uint32 _val, char * _p);

		static int toInt32(char * _p);
		static uint32 toUInt32(char * _p);

	private:
		NetworkingGlobals(void);
	};
}
#endif // NETWORKING_H