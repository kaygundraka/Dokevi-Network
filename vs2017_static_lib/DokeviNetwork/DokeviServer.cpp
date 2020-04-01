#include "pch.h"
#include "DokeviServer.h"
#include "ConfigManager.h"
#include "ThreadManager.h"
#include "NetworkUtils.h"
#include "DokeviException.h"

using namespace DokeviNet;

DokeviServer& DokeviServer::Init()
{
	RECORD_STACK;

	_shutDown = false;
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (_iocpHandle == nullptr) {
		LOG_CRITICAL("FourServer::Run() : CreateIoCompletionPort Associate Failed With Error");
	}

	return *this;
}

DokeviServer& DokeviServer::SetListenPort(unsigned short inPort, unsigned int inMaxConnections, void* (*inAllocFunc)(std::shared_ptr<TCPSocket>))
{
	RECORD_STACK;

	auto iter = _portAllocMap.find(inPort);

	if (iter == _portAllocMap.end())
	{
		_portAllocMap.insert(std::make_pair(inPort, inAllocFunc));
	}
	else
	{
		LOG_ERROR("already existed port : {}", inPort);
		iter->second = inAllocFunc;
	}

	std::shared_ptr<TCPSocket> _listenSocket = std::make_shared<TCPSocket>(this, inPort);
	SocketAddress address(inPort);

	_listenSocket->Bind(address);
	_listenSocket->Listen(true);

	if (CreateIoCompletionPort((HANDLE)*_listenSocket->GetSocket(), _iocpHandle, (u_long)0, 0) == 0) {
		LOG_CRITICAL("FourServer::Run() : CreateIoCompletionPort Associate Failed With Error");
		return *this;
	}

	_listenSockets.insert(std::make_pair(inPort, _listenSocket));

	for (unsigned int i = 0; i < inMaxConnections; i++)
	{
		ReadyAcceptEx(inPort);
	}

	return *this;
}

void DokeviServer::Run(unsigned short inIOThreadCount)
{
	RECORD_STACK;

	for (int i = 0; i < inIOThreadCount; i++)
	{
		SINGLETON(ThreadManager)->TaskRun(this);
	}

	LOG_INFO("DokeviNet Server Running...");
}

void DokeviServer::ThreadTask()
{
	RECORD_STACK;

	SINGLETON(ThreadManager)->RegisterThread(ThreadInfo::THREAD_TYPE::IO_TASK);

	FOUR_OVERLAPPED* overlapped = nullptr;
	DWORD transferredSize = 0;
	ULONG completionKey = 0;

	LOG_INFO("Running DokeviNet IOThread[{}]", SINGLETON(ThreadManager)->GetCurrentThreadIndex());

	try {

		while (true)
		{
			if (_shutDown == true)
				return;

			bool success = GetQueuedCompletionStatus(_iocpHandle, &transferredSize, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&overlapped, INFINITE);

			FOUR_OVERLAPPED* ioMessage = (FOUR_OVERLAPPED*)overlapped;

			if (success == false)
			{
				int Err = WSAGetLastError();

				LOG_WARN("DokeviServer::ThreadTask() : GetQueuedCompletionStatus is Failed, GetLastError() : {}", Err);

				if (ioMessage != nullptr && Err == 64)
				{
					LOG_WARN("DokeviServer::ThreadTask() : Inavlid Socket Closed");
					DisconnectHandler((TCPSocket*)ioMessage->object, false);
				}
								
				continue;
			}

			switch (ioMessage->ioType)
			{
			case FOUR_OVERLAPPED::IO_TYPE::ACCEPT:
				LOG_INFO("DokeviServer::ThreadTask() : New Accept!!");
				ConnectHanadler((TCPSocket*)ioMessage->object);
				break;
			case FOUR_OVERLAPPED::IO_TYPE::RECV:
				if (transferredSize == 0)
				{
					LOG_WARN("DokeviServer::ThreadTask() : Graceful Closed!!");
					DisconnectHandler((TCPSocket*)ioMessage->object, true);
				}
				else
					RecvHandler((TCPSocket*)ioMessage->object, transferredSize);
				break;

			case FOUR_OVERLAPPED::IO_TYPE::SEND:
				SendHandler((TCPSocket*)ioMessage->object);
				break;

			case FOUR_OVERLAPPED::IO_TYPE::IOCP_OBJECT:
				((IOCPObject*)ioMessage->object)->OnIOHandler();
				break;

			default:
				LOG_CRITICAL("DokeviServer::ThreadTask() : IO_TYPE -> Unknown Error Type");
				break;
			}
		}
	}
	catch (...) {
		LOG_CRITICAL("DokeviServer::ThreadTask() : IO Thread Exception!!");
	}
}

void DokeviServer::ReadyAcceptEx(unsigned short inPort)
{
	RECORD_STACK;

	auto iter = _listenSockets.find(inPort);

	if (iter == _listenSockets.end())
	{
		LOG_CRITICAL("DokeviServer::CreateConnectionPool() : cannot find port!!");
		return;
	}

	auto iter2 = _portAllocMap.find(inPort);

	if (iter2 == _portAllocMap.end())
	{
		LOG_CRITICAL("DokeviServer::CreateConnectionPool() : cannot find port handler!!");
		return;
	}

	BYTE buffer[(sizeof(sockaddr_in6) + 16) * 2];
	memset(buffer, '\0', sizeof buffer);

	std::shared_ptr<TCPSocket> socket = std::make_shared<TCPSocket>(this, inPort);
	NetworkClient* newClient = (NetworkClient*)(iter2->second(socket));
	socket->SetClient(newClient);

	int zero = 0;

	setsockopt(*socket->GetSocket(), SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof zero);
	setsockopt(*socket->GetSocket(), SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof zero);

	if (!AcceptEx(*iter->second->GetSocket(), *socket->GetSocket(), &buffer, 0, sizeof(sockaddr_in) + 16, 
		sizeof(sockaddr_in) + 16, nullptr, (LPOVERLAPPED)socket->GetAcceptOverlapped()))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LOG_CRITICAL("DokeviServer::CreateConnectionPool() : AcceptEx failed with error");
			return;
		}
	}
}

void DokeviServer::ConnectHanadler(TCPSocket* inSocket)
{
	RECORD_STACK;

	if (CreateIoCompletionPort((HANDLE)*inSocket->GetSocket(), _iocpHandle, (u_long)0, 0) == 0) {
		LOG_CRITICAL("DokeviServer::ConnectHanadler() : CreateIoCompletionPort Associate Failed With Error");
		return;
	}

	inSocket->RecvPacket(0);
	inSocket->GetNetworkClient()->ConnectedHandler();

	_clientSize.fetch_add(1);
}

void DokeviServer::Send(TCPSocket* inSocket, const PacketInterface* inPacket)
{
	RECORD_STACK;

	WSABUF buf;
	buf.buf = (const_cast<PacketInterface*>(inPacket)->_buffer);
	buf.len = (const_cast<PacketInterface*>(inPacket)->_size);
	int sendBytes = 0;

	auto overlapped = inSocket->GetSendOverlapped();

	if (WSASend(*inSocket->GetSocket(),
		&buf, 1, (LPDWORD)&sendBytes, 0, (LPOVERLAPPED)overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			LOG_WARN("DokeviServer::Send() : Send failed with error - {}", WSAGetLastError());
	}
}

void DokeviServer::DisconnectHandler(TCPSocket* inSocket, bool inGracefulClose)
{
	RECORD_STACK;

	LOG_INFO("DokeviServer::DisconnectHandler()");

	inSocket->ResetSocket(inGracefulClose, true);

	auto pClient = inSocket->GetNetworkClient();

	if (pClient != nullptr)
	{
		pClient->DisconnectedHandler();
		_clientSize.fetch_sub(1);
	}
}

void DokeviServer::SendHandler(TCPSocket* inSocket)
{
	RECORD_STACK;

	inSocket->OnWriteCallback();
}

void DokeviServer::RecvHandler(TCPSocket* inSocket, unsigned int inTransferred)
{
	RECORD_STACK;

	inSocket->RecvPacket(inTransferred);
}