/*
TCP Networking Library (.Net Version)

Author: Recep ALTIN 
*/

namespace NetworkingDotNet
{
    public class NetworkingDotNetGlobals
    {
        // IMPORTANT! DON'T CHANGE VALUES BELOW!
        //
        // MTU (Maximum Transmission Unit) = 1500 Bytes
        // MSS (Maximum Segment Size) = 40 Bytes (TCP = 20 Bytes + IP = 20 Bytes)
        //
        // By informtion above payload(FixedBufferSize) can be 1500 - 40 = 1460 Bytes to send without fragment in theory 
        // but in practise may user have a VPN traverse or something which will shrink down the payload few more bytes.
        // So I leaved extra 136 Bytes for it then FixedBufferSize becomes 1324 Bytes:
        public static readonly int PacketHeaderSize = 24;
        public static readonly int MaximumDataSize = 1300;
        public static readonly int FixedBufferSize = MaximumDataSize + PacketHeaderSize;
        //~

        public static readonly string DefaultServerIP = "127.0.0.1";
        public static readonly uint DefaultServerPort = 11372;
        public static readonly uint DefaultTimeOut = 5000;

        private NetworkingDotNetGlobals()
        { }

        #region BitConversion
        public static byte[] GetBytes32(uint _p)
        {
            byte[] b = new byte[4];
            for (int i = 0; i < 4; i++)
                b[3 - i] = (byte)(_p >> (i * 8));
            return b;
        }

        public static uint ToUInt32(byte[] _p, int _offset)
        {
            return (uint)((_p[_offset] << 24) | (_p[_offset + 1] << 16) | (_p[_offset + 2] << 8) | _p[_offset + 3]);
        }
        #endregion
    }

    public enum NetworkPacketType
	{
		NPT_NULL,
		NPT_ACK,
		NPT_DATA,
		NPT_PARTIAL_DATA,
		NPT_PARTIAL_DATA_END,
		NPT_LASTITEM
	}

    public delegate void ServerHandler(NetworkClientInfo _clientInfo);
	public delegate void ServerErrorHandler(string _error);
	public delegate void ServerClientErrorHandler(NetworkClientInfo _clientInfo, string _error); 
	public delegate void ServerPacketReceiveHandler(NetworkClientInfo _clientInfo, NetworkPacket _packet);
	public delegate void ClientHandler();
	public delegate void ClientErrorHandler(string _error); 
	public delegate void ClientPacketReceiveHandler(NetworkPacket _packet);
}
