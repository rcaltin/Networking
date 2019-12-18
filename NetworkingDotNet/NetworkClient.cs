/*
TCP Networking Library (.Net Version)

Author: Recep ALTIN 
*/

using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace NetworkingDotNet
{
    public class NetworkClient
    {
        #region Properties
        public string ServerIp { get; set; }
        public uint ServerPort { get; set; }
        public string LastError { get; protected set; }
        public bool IsConnected { get => (mClient != null ? mClient.Connected : false); }
        #endregion

        #region Events
        public event ClientHandler Disconnected;
        public event ClientPacketReceiveHandler PacketReceived;
        public event ClientErrorHandler Error;
        #endregion

        #region Fields
        protected TcpClient mClient = null;
        protected Thread mThreadListener = null;
        protected uint mPacketCounter = 0;
        protected List<NetworkPacket> mPartialDataBuffer = new List<NetworkPacket>();
        #endregion

        public NetworkClient()
        {
            ServerIp = NetworkingDotNetGlobals.DefaultServerIP;
            ServerPort = NetworkingDotNetGlobals.DefaultServerPort;
        }

        public NetworkClient(string _serverIp, uint _serverPort)
        {
            ServerIp = _serverIp;
            ServerPort = _serverPort;
        }

        public bool ConnectToServer(int _timeOut)
        {
            bool res = false;
            try
            {
                mClient = new TcpClient();
                mClient.Connect(ServerIp, (int)ServerPort);

                if (res = mClient.Connected)
                {
                    mThreadListener = new Thread(new ThreadStart(ListenTCP))
                    {
                        IsBackground = true
                    };
                    mThreadListener.Start();
                }
            }
            catch (Exception ex)
            {
                LastError = ex.Message;
                Error?.Invoke(LastError);
            }

            if (!res)
            {
                DisconnectFromServer(_timeOut);
            }

            return res;
        }

        public bool DisconnectFromServer(int _timeOut)
        {
            bool res = false;

            try
            {
                if (mClient != null)
                {
                    if (mThreadListener != null)
                    {
                        mThreadListener.Interrupt();
                        mThreadListener.Join();
                        mThreadListener = null;
                    }

                    mClient.Close();
                    mClient = null;
                }

                res = true;
            }
            catch (Exception ex)
            {
                LastError = ex.Message;
                Error?.Invoke(LastError);
            }

            return res;
        }

        private void ListenTCP()
        {
            try
            {
                NetworkStream ns = mClient.GetStream();
                while (mClient.Connected)
                {
                    if (ns.DataAvailable)
                    {
                        try
                        {
                            byte[] buffer = new byte[NetworkingDotNetGlobals.FixedBufferSize];
                            ns.Read(buffer, 0, buffer.Length);

                            NetworkPacket np = new NetworkPacket(NetworkPacketType.NPT_NULL);
                            np.FromBuffer(new List<byte>(buffer));

                            if (np.PacketType == NetworkPacketType.NPT_DATA)
                            {
                                // partial data end check
                                if (mPartialDataBuffer.Count > 0)
                                {
                                    uint id = mPartialDataBuffer[0].Id;
                                    mPartialDataBuffer.Clear();
                                    LastError = String.Format("Packet ID:{0} lost.", id);
                                    Error?.Invoke(LastError);
                                }

                                PacketReceived?.Invoke(np);
                            }
                            else if (np.PacketType == NetworkPacketType.NPT_PARTIAL_DATA)
                            {
                                mPartialDataBuffer.Add(np);

                                PacketReceived?.Invoke(np);
                            }
                            else if (np.PacketType == NetworkPacketType.NPT_PARTIAL_DATA_END)
                            {
                                mPartialDataBuffer.Add(np);

                                NetworkPacket npj = new NetworkPacket(NetworkPacketType.NPT_DATA);
                                if (NetworkPacketManipulator.Join(mPartialDataBuffer, ref npj))
                                {
                                    PacketReceived?.Invoke(npj);
                                }
                                else
                                {
                                    LastError = String.Format("Packet ID:{0} corrupt data.", npj.Id);
                                    Error?.Invoke(LastError);
                                }

                                mPartialDataBuffer.Clear();
                            }
                        }
                        catch (SocketException ex)
                        {
                            LastError = ex.Message;
                            Error?.Invoke(LastError);
                            //break;
                        }
                        catch (Exception ex)
                        {
                            LastError = ex.Message;
                            Error?.Invoke(LastError);
                            //break;
                        }
                    }
                }
            }
            catch (ThreadInterruptedException)
            { }

            Disconnected?.Invoke();
        }

        public bool SendPacket(NetworkPacket _packet)
        {
            if(!mClient.Connected)
		        return false;
	
	        _packet.Id = ++mPacketCounter;
	
	        List<NetworkPacket> nps = new List<NetworkPacket>();
            if (NetworkPacketManipulator.Split(_packet, ref nps))
            {
                try
                {
                    NetworkStream ns = mClient.GetStream();
                    foreach (NetworkPacket np in nps)
                    {
                        List<byte> ba = np.ToBuffer();
                        ns.Write(ba.ToArray(), 0, ba.Count);
                        ns.Flush();
                    }
                    return true;
                }
                catch (Exception ex)
                {
                    LastError = ex.Message;
                    Error?.Invoke(LastError);
                }
            }

	        return false;
        }

        public NetworkClientInfo GetInfo()
        {
            IPEndPoint ep = (IPEndPoint)mClient.Client.RemoteEndPoint;
            return new NetworkClientInfo(ep.Address.ToString(), (uint)ep.Port);
        }

        public uint GetPacketCounter()
        {
            return mPacketCounter;
        }

        public override string ToString()
        {
            return string.Format("<NetworkClient ServerIp:{0} ServerPort:{1} IsConnected:{2}>", ServerIp, ServerPort, mClient != null ? mClient.Connected : false);
        }
    }
}
