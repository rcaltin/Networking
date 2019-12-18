/*
TCP Networking Library (.Net Version)

Author: Recep ALTIN 
*/
using System;

namespace NetworkingDotNet
{
    public class NetworkClientInfo
    {
        #region Properties
        public uint Port { get; set; }
        public string Address { get; set; }
        #endregion

        #region Constructors
        public NetworkClientInfo()
        {
            Port = new uint();
            Address = string.Empty;
        }

        public NetworkClientInfo(NetworkClientInfo _other)
        {
            Address = _other.Address;
            Port = _other.Port;
        }

        public NetworkClientInfo(string _address, uint _port)
        {
            Address = _address;
            Port = _port;
        }
        #endregion

        public static bool operator ==(NetworkClientInfo _p1, NetworkClientInfo _p2)
        {
            return (_p1.Address == _p2.Address) && (_p1.Port == _p2.Port);
        }

        public static bool operator !=(NetworkClientInfo _p1, NetworkClientInfo _p2)
        {
            return !(_p1 == _p2);
        }

        public override bool Equals(object obj)
        {
            return this == (NetworkClientInfo)obj;
        }

        public override int GetHashCode()
        {
            return Port.GetHashCode() ^ Address.GetHashCode();
        }

        public override string ToString()
        {
            return String.Format("{0}:{1}", Address, Port);
        }
    };
}
