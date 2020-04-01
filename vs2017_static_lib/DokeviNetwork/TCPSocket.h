#pragma once
#include "pch.h"
#include "SocketAddress.h"
#include "AtomicStaticPool.h"
#include "SyncSection.h"
#include "PacketInterface.h"
#include "NetworkClient.h"

namespace DokeviNet
{
	struct FOUR_OVERLAPPED : public OVERLAPPED_ENTRY
	{
		enum class IO_TYPE {
			ACCEPT = 0, RECV = 1, SEND = 2, IOCP_OBJECT
		};

		IO_TYPE ioType;
		void* object;
		bool flag = true;
	};

	class DokeviServer;

	enum class RECV_STEP { RECV_LEN, RECV_BODY };

	class TCPSocket : public MultiThreadSync
	{
	private:
		DokeviServer* _server;
		NetworkClient* _networkClient;

		SOCKET _socket;

		PacketInterface* _sendingPacket;
		PacketInterface* _lastPacket;

		FOUR_OVERLAPPED _sendOverlapped;
		FOUR_OVERLAPPED _recvOverlapped;
		FOUR_OVERLAPPED _acceptOverlapped;

		WSABUF _recvBuf;
		void* _packetBuf;
		unsigned int _recvSize;
		unsigned int _packetSize;
		unsigned int _recvBytes;

		unsigned short _port;
				
		RECV_STEP _recvStep = RECV_STEP::RECV_LEN;
	
	public:
		TCPSocket(DokeviServer* inServer, unsigned short inPort);
		~TCPSocket();

		int	Connect(const SocketAddress& inAddress);
		int	Bind(const SocketAddress& inToAddress);
		int	Listen(bool inUseIOCP, int inBackLog = SOMAXCONN);

		void OnWriteCallback();
		void Send(const void* inData, unsigned int inLen, bool inImmediateSend);
		void RecvPacket(unsigned int inSize);

		void ResetSocket(bool inGracefulClose, bool callConnectionHandler);

		SOCKET* GetSocket();

		void SetClient(NetworkClient* inClient) { _networkClient = inClient; }

		void SetSocket(SOCKET inSocket) { _socket = inSocket; }
		NetworkClient* GetNetworkClient() { return _networkClient; }

		FOUR_OVERLAPPED* GetSendOverlapped() { return &_sendOverlapped; }
		FOUR_OVERLAPPED* GetRecvOverlapped() { return &_recvOverlapped; }
		FOUR_OVERLAPPED* GetAcceptOverlapped() { return &_acceptOverlapped; }
	};
}