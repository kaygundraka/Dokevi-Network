#pragma once
#include "pch.h"
#include "AtomicStaticPool.h"
#include "ThreadManager.h"
#include "SyncSection.h"
#include "TCPSocket.h"
#include "NetworkClient.h"

namespace DokeviNet
{
	class IOCPObject abstract
	{
	public:
		virtual void OnIOHandler() = 0;
	};

	class DokeviServer : public ManagedThread, public MultiThreadSync
    {
    private:
		std::map<unsigned short, std::function<void* (std::shared_ptr<TCPSocket>)>> _portAllocMap;
		std::map<unsigned short, std::shared_ptr<TCPSocket>> _listenSockets;

		HANDLE _iocpHandle;

		bool _shutDown;

		std::atomic<long long> _clientSize;

		virtual void ThreadTask();
		
    public:
		DokeviServer& Init();
		DokeviServer& SetListenPort(unsigned short inPort, unsigned int inMaxConnections, void* (*inAllocFunc)(std::shared_ptr<TCPSocket>));
		void Run(unsigned short inIOThreadCount);

		void Send(TCPSocket* inSocket, const PacketInterface* inPacket);

		void ReadyAcceptEx(unsigned short inPort);
		void ConnectHanadler(TCPSocket* inSocket);
		void DisconnectHandler(TCPSocket* inSocket, bool inGracefulClose);
		void SendHandler(TCPSocket* inSocket);
		void RecvHandler(TCPSocket* inSocket, unsigned int inTransferred);

		long long GetClientSize() const { return _clientSize; }
    };
}