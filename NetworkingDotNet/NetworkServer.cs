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
    public class NetworkServer
    {
        #region Properties
        public uint ListenPort { get; }

        public bool IsListening
        {
            get { return mServer.Active; }
        }

        public bool AcceptClient { get; set; } = true;

        public bool HasAnyClient
        {
          get { return mClientSockets.Count > 0; }
        }

        public int ReceiveTimeOut { get; set; }
        public int SendTimeOut { get; set; }
        public string LastError { get; private set; }
        #endregion

        #region Events
        public event ServerHandler ClientConnected;
        public event ServerHandler ClientDisconnected;
        public event ServerPacketReceiveHandler PacketReceived;
        public event ServerErrorHandler Error;
        public event ServerClientErrorHandler ClientError;
        #endregion

        #region Fields
        TcpServer mServer;
        Thread mThreadListener;
        uint mPacketCounter;
        List<TcpClient> mClientSockets;
        Dictionary<TcpClient, List<NetworkPacket>> mPartialDataBuffer;
        #endregion

        public NetworkServer()
        {
            ListenPort = NetworkingDotNetGlobals.DefaultServerPort;

            mServer = new TcpServer(IPAddress.Any, (int)ListenPort);
            mThreadListener = new Thread(new ThreadStart(ClientHandler));
            LastError = string.Empty;
            mClientSockets = new List<TcpClient>();
            mPartialDataBuffer = new Dictionary<TcpClient, List<NetworkPacket>>();
        }

        public NetworkServer(uint _listenPort)
        {
            ListenPort = _listenPort;

            mServer = new TcpServer(IPAddress.Any, (int)ListenPort);
            mThreadListener = new Thread(new ThreadStart(ClientHandler));
            LastError = string.Empty;
            mClientSockets = new List<TcpClient>();
            mPartialDataBuffer = new Dictionary<TcpClient, List<NetworkPacket>>();
        }

        ~NetworkServer()
        {
            StopServer();
        }

        public bool StartServer()
        {
            bool res = false;
            try
            {
                mServer.Start();
                if (res = IsListening)
                    mThreadListener.Start();
            }
            catch (Exception ex)
            {
                LastError = ex.Message;
                Error?.Invoke(LastError);
            }

            return res;
        }

        public bool StopServer()
        {
            bool res = false;
            try
            {
                mServer.Stop();
                if (res = !IsListening)
                {
                    mThreadListener.Abort();
                    mThreadListener.Join();
                }
            }
            catch (Exception ex)
            {
                LastError = ex.Message;
                Error?.Invoke(LastError);
            }

            return res;
        }

        protected void ClientHandler()
        {
            while (mServer.Active)
            {
                try
                {
                    if (AcceptClient)
                    {
                        TcpClient client = mServer.AcceptTcpClient();
                        if (client != null)
                            ThreadPool.QueueUserWorkItem(new WaitCallback(ListenTCP), client);
                    }
                }
                catch (SocketException e)
                {
                    if ((e.SocketErrorCode == SocketError.Interrupted))
                        break;
                }
            }
        }

        protected void ListenTCP(object _clientObj)
        {
            TcpClient client = (TcpClient)_clientObj;
            client.ReceiveTimeout = ReceiveTimeOut;
            client.SendTimeout = SendTimeOut;

            Timer timerCallBack = new Timer(new TimerCallback(ClientKeepAliveCallBack), client, 0, 1000);

            mClientSockets.Add(client);
            mPartialDataBuffer.Add(client, new List<NetworkPacket>());

            NetworkClientInfo nci = new NetworkClientInfo(
                ((IPEndPoint)client.Client.RemoteEndPoint).Address.ToString(),
                (uint)((IPEndPoint)client.Client.RemoteEndPoint).Port
                );

            ClientConnected?.Invoke(nci);

            NetworkStream ns = client.GetStream();
            while (client.Connected)
            {
                try
                {
                    if (ns.DataAvailable)
                    {
                        byte[] buffer = new byte[NetworkingDotNetGlobals.FixedBufferSize];
                        ns.Read(buffer, 0, buffer.Length);

                        NetworkPacket np = new NetworkPacket(NetworkPacketType.NPT_NULL);
                        np.FromBuffer(new List<byte>(buffer));
                        if (np.PacketType == NetworkPacketType.NPT_DATA)
                        {
                            // partial data end check
                            if (mPartialDataBuffer[client].Count > 0)
                            {
                                uint id = mPartialDataBuffer[client][0].Id;
                                mPartialDataBuffer[client].Clear();

                                LastError = String.Format("Packet ID:{0} lost.", id);
                                Error?.Invoke(LastError);
                            }

                            PacketReceived?.Invoke(nci, np);
                        }
                        else if (np.PacketType == NetworkPacketType.NPT_PARTIAL_DATA)
                        {
                            mPartialDataBuffer[client].Add(np);

                            PacketReceived?.Invoke(nci, np);
                        }
                        else if (np.PacketType == NetworkPacketType.NPT_PARTIAL_DATA_END)
                        {
                            mPartialDataBuffer[client].Add(np);

                            NetworkPacket npj = new NetworkPacket(NetworkPacketType.NPT_DATA);
                            if (NetworkPacketManipulator.Join(mPartialDataBuffer[client], ref npj))
                            {
                                PacketReceived?.Invoke(nci, npj);
                            }
                            else
                            {
                                LastError = String.Format("Packet ID:{0} corrupt data.", npj.Id);
                                Error?.Invoke(LastError);
                            }

                            mPartialDataBuffer[client].Clear();
                        }
                    }
                }
                catch (SocketException ex)
                {
                    LastError = ex.Message;
                    ClientError?.Invoke(nci, LastError);
                    break;
                }
                catch (Exception ex)
                {
                    LastError = ex.Message;
                    ClientError?.Invoke(nci, LastError);
                    break;
                }
            }

            timerCallBack.Dispose();
            mClientSockets.Remove(client);
            ClientDisconnected?.Invoke(nci);
        }

        private void ClientKeepAliveCallBack(object _clientObj)
        {
            TcpClient client = (TcpClient)_clientObj;
            NetworkClientInfo nci = new NetworkClientInfo(
                ((IPEndPoint)client.Client.RemoteEndPoint).Address.ToString(),
                (uint)((IPEndPoint)client.Client.RemoteEndPoint).Port
                );

            SendPacket(new NetworkPacket(NetworkPacketType.NPT_ACK), nci);
        }

        public bool SendPacket(NetworkPacket _packet, NetworkClientInfo _clientInfo)
        {
            if (!mServer.Active)
                return false;

            foreach (TcpClient client in mClientSockets)
            {
                NetworkClientInfo nci = new NetworkClientInfo(
                ((IPEndPoint)client.Client.RemoteEndPoint).Address.ToString(),
                (uint)((IPEndPoint)client.Client.RemoteEndPoint).Port
                );
                if (nci == _clientInfo)
                {
                    _packet.Id = ++mPacketCounter;
                    List<NetworkPacket> nps = new List<NetworkPacket>();
                    if (NetworkPacketManipulator.Split(_packet, ref nps))
                    {
                        try
                        {
                            NetworkStream ns = client.GetStream();
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
                    break;
                }
            }

            return false;
        }

        public List<NetworkClientInfo> GetClients()
        {
            List<NetworkClientInfo> res = new List<NetworkClientInfo>();
	 
            foreach (TcpClient client in mClientSockets)
            {
                IPEndPoint endPoint = (IPEndPoint) client.Client.RemoteEndPoint;
                res.Add(new NetworkClientInfo(((IPAddress)endPoint.Address).ToString(), (uint)endPoint.Port));
	        }

            return res;
        }

        public uint GetPacketCounter()
        {
            return mPacketCounter;
        }
    }
}
