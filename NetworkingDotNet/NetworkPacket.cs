/*
TCP Networking Library (.Net Version)

Author: Recep ALTIN 
*/
using System.Collections.Generic;
using System.Text;

namespace NetworkingDotNet
{
    public class NetworkPacket
    {
        #region Properties
        public uint Id { get; set; }
        public NetworkPacketType PacketType { get; set; }
        public uint TypeId { get; set; }
        public uint PartOrder { get; set; }
        public uint PartCount { get; set; }
        public List<byte> Data { get; set; }

        public string StringData
        {
            get { return Encoding.UTF8.GetString(Data.ToArray()); }
            set { Data = new List<byte>(Encoding.UTF8.GetBytes(value)); }
        }

        public bool IsDataPacket => PacketType == NetworkPacketType.NPT_DATA;
        #endregion

        #region Constructors
        public NetworkPacket()
        {
            Clear();

            PacketType = NetworkPacketType.NPT_NULL;
        }

        public NetworkPacket(NetworkPacket _other)
        {
            Clear();

            this.FromBuffer(_other.ToBuffer());
        }

        public NetworkPacket(List<byte> _buffer)
        {
            Clear();

            this.FromBuffer(_buffer);
        }

        public NetworkPacket(NetworkPacketType _packetType)
        {
            Clear();

            PacketType = _packetType;
        }
        #endregion

        public static bool operator ==(NetworkPacket _p1, NetworkPacket _p2)
        {
            return
                (_p1.Id == _p2.Id) &&
                (_p1.PacketType == _p2.PacketType) &&
                (_p1.TypeId == _p2.TypeId) &&
                (_p1.Data == _p2.Data);
        }

        public static bool operator !=(NetworkPacket _p1, NetworkPacket _p2)
        {
            return !(_p1 == _p2);
        }

        public override bool Equals(object obj)
        {
            return this == (NetworkPacket)obj;
        }

        public override int GetHashCode()
        {
            return Id.GetHashCode() ^ PacketType.GetHashCode() ^ TypeId.GetHashCode() ^ PartOrder.GetHashCode() ^ PartCount.GetHashCode() ^ Data.GetHashCode();
        }

        protected virtual void Clear()
        {
            Id = 0;
            PacketType = NetworkPacketType.NPT_NULL;
            TypeId = 0;
            PartOrder = 0;
            PartCount = 0;
            Data = new List<byte>();
        }

        public virtual List<byte> ToBuffer()
        {
            List<byte> buffer = new List<byte>();

            // Buffer : Id(4-Bytes) + PacketType(4-Bytes) + TypeId(4-Byte) + PartOrder(4-Bytes) + PartCount(4-Bytes) + Data.Count(4-Bytes) + Data.Elements(Data.Count x Bytes) + ZERO_PADDING(MaximumDataSize - Data.Count x Bytes)
            buffer.AddRange(NetworkingDotNetGlobals.GetBytes32(Id));
            buffer.AddRange(NetworkingDotNetGlobals.GetBytes32((uint)PacketType));
            buffer.AddRange(NetworkingDotNetGlobals.GetBytes32(TypeId));
            buffer.AddRange(NetworkingDotNetGlobals.GetBytes32(PartOrder));
            buffer.AddRange(NetworkingDotNetGlobals.GetBytes32(PartCount));
            buffer.AddRange(NetworkingDotNetGlobals.GetBytes32((uint)Data.Count));
            buffer.AddRange(Data);

            if (buffer.Count < NetworkingDotNetGlobals.FixedBufferSize)
                buffer.AddRange(new byte[NetworkingDotNetGlobals.FixedBufferSize - buffer.Count]);

            return buffer;
        }

        public virtual void FromBuffer(List<byte> _buffer)
        {
            byte[] arr = _buffer.ToArray();

            // Buffer : Id(4-Bytes) + PacketType(4-Bytes) + TypeId(4-Byte) + PartOrder(4-Bytes) + PartCount(4-Bytes) + Data.Count(4-Bytes) + Data.Elements(Data.Count x Bytes) + ZERO_PADDING(MaximumDataSize - Data.Count x Bytes)
            Id = NetworkingDotNetGlobals.ToUInt32(arr, 0);
            PacketType = (NetworkPacketType)NetworkingDotNetGlobals.ToUInt32(arr, 4);
            TypeId = NetworkingDotNetGlobals.ToUInt32(arr, 8);
            PartOrder = NetworkingDotNetGlobals.ToUInt32(arr, 12);
            PartCount = NetworkingDotNetGlobals.ToUInt32(arr, 16);
            int dataCount = (int)NetworkingDotNetGlobals.ToUInt32(arr, 20);

            if (dataCount > 0)
            {
                Data = new List<byte>(_buffer.GetRange(NetworkingDotNetGlobals.PacketHeaderSize, dataCount));
            }
        }
    }
}
