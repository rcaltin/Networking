/*
TCP Networking Library (.Net Version)

Author: Recep ALTIN 
*/
using System;
using System.Collections.Generic;

namespace NetworkingDotNet
{
    public class NetworkPacketManipulator
    {
        public static bool Split(NetworkPacket _np, ref List<NetworkPacket> _nps)
        {
            _np.ToBuffer(); // force override to fill buffer

            if (_np.Data.Count > NetworkingDotNetGlobals.MaximumDataSize)
            {
                int i = 0;
                uint partOrder = 0;
                uint partCount = (uint)(Math.Ceiling((double)_np.Data.Count / (double)NetworkingDotNetGlobals.MaximumDataSize));
                List<byte> ba = new List<byte>();

                while (i < _np.Data.Count)
                {
                    ba.Add(_np.Data[i]);
                    if (++i == ((partOrder + 1) * NetworkingDotNetGlobals.MaximumDataSize) || i == _np.Data.Count)
                    {
                        NetworkPacket np = new NetworkPacket(i == _np.Data.Count ? NetworkPacketType.NPT_PARTIAL_DATA_END : NetworkPacketType.NPT_PARTIAL_DATA)
                        {
                            Id = _np.Id,
                            TypeId = _np.TypeId,
                            PartOrder = partOrder++,
                            PartCount = partCount,
                            Data = new List<byte>(ba)
                        };
                        _nps.Add(np);
                        ba.Clear();
                    }
                }
            }
            else
            {
                _nps.Add(new NetworkPacket(_np));
            }

            return true;
        }

        public static bool Join(List<NetworkPacket> _nps, ref NetworkPacket _np)
        {
            if (_nps.Count > 0)
            {
                List<byte> data = new List<byte>();

                for (int i = 0; i < _nps.Count; i++)
                {
                    NetworkPacket p = _nps[i];

                    if (i != p.PartOrder)
                    {
                        return false;
                    }

                    data.AddRange(p.Data);

                    if (p.PacketType == NetworkPacketType.NPT_PARTIAL_DATA_END)
                    {
                        _np.Id = p.Id;
                        _np.TypeId = p.TypeId;
                        _np.Data = new List<byte>(data);
                    }
                }
                return true;
            }

            return false;
        }
    }
}
