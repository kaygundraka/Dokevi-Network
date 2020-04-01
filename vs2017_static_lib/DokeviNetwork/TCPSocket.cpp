#include "pch.h"
#include "DokeviServer.h"
#include "TCPSocket.h"
#include "NetworkUtils.h"
#include "PacketSelector.h"

using namespace DokeviNet;

TCPSocket::TCPSocket(DokeviServer* inServer, unsigned short inPort)
	: _server(inServer), _networkClient(nullptr), _recvBytes(0), _packetSize(0), _port(inPort)
{
	RECORD_STACK;

	_socket = socket(SocketAddressFamily::INET, SOCK_STREAM, IPPROTO_TCP); 
	
	if (_socket == INVALID_SOCKET)
	{
		LOG_CRITICAL("TCPSocket() : Don't Create Socket");
	}

	_packetBuf = new char[MAX_PACKET_BUF_SIZE];
	_recvSize = 0;
	
	_recvBuf.buf = new char[MAX_PACKET_BUF_SIZE];
	_recvBuf.len = MAX_PACKET_BUF_SIZE;

	ZeroMemory(&_sendOverlapped, sizeof(FOUR_OVERLAPPED));
	ZeroMemory(&_recvOverlapped, sizeof(FOUR_OVERLAPPED));
	ZeroMemory(&_acceptOverlapped, sizeof(FOUR_OVERLAPPED));

	_sendOverlapped.ioType = FOUR_OVERLAPPED::IO_TYPE::SEND;
	_sendOverlapped.object = this;

	_recvOverlapped.ioType = FOUR_OVERLAPPED::IO_TYPE::RECV;
	_recvOverlapped.object = this;

	_acceptOverlapped.ioType = FOUR_OVERLAPPED::IO_TYPE::ACCEPT;
	_acceptOverlapped.object = this;

	_lastPacket = nullptr;
	_sendingPacket = nullptr;
}

TCPSocket::~TCPSocket()
{
	RECORD_STACK;

	delete[] _recvBuf.buf;
	delete[] _packetBuf;
}

SOCKET* TCPSocket::GetSocket()
{
	return &_socket;
}

void TCPSocket::ResetSocket(bool inGracefulClose, bool callConnectionHandler)
{
	RECORD_STACK;

	_acceptOverlapped.flag = false;

	LINGER  ling = { 0, };

	if (inGracefulClose)
	{
		ling.l_onoff = 0;  
		ling.l_linger = 0;  
	}
	else
	{
		ling.l_onoff = 1;
		ling.l_linger = 0;  
	}

	setsockopt(_socket, SOL_SOCKET, SO_LINGER, (CHAR*)&ling, sizeof(ling));
	closesocket(_socket);

	if (callConnectionHandler)
	{
		_server->ReadyAcceptEx(_port);
	}
}

int TCPSocket::Connect(const SocketAddress& inAddress)
{
	RECORD_STACK;

	int err = connect(_socket, &inAddress._sockAddr, inAddress.GetSize());
	if (err == SOCKET_ERROR)
	{
		LOG_ERROR("TCPSocket::Connect() : WSAGetLastError({})", WSAGetLastError());
		return WSAGetLastError();
	}
	return NO_ERROR;
}

int TCPSocket::Listen(bool inUseIOCP, int inBackLog)
{
	RECORD_STACK;

	if (inUseIOCP)
	{
		const char opt = 1;
		
		if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR)
		{
			LOG_ERROR("TCPSocket::Listen() : WSAGetLastError({})", WSAGetLastError());
			return WSAGetLastError();
		}
	}
	
	if (listen(_socket, inBackLog) == SOCKET_ERROR)
	{
		LOG_ERROR("TCPSocket::Listen() : WSAGetLastError({})", WSAGetLastError());
		return WSAGetLastError();
	}

	return NO_ERROR;
}

int TCPSocket::Bind(const SocketAddress& inBindAddress)
{	
	RECORD_STACK;

	int err = ::bind(_socket, (sockaddr*)inBindAddress.GetAsSockAddrIn(), inBindAddress.GetSize());

	if (err == SOCKET_ERROR)
	{
		LOG_ERROR("TCPSocket::Bind() : WSAGetLastError({})", WSAGetLastError());
		return WSAGetLastError();
	}

	return NO_ERROR;
}

void TCPSocket::OnWriteCallback()
{
	RECORD_STACK;

	WRITE_GUARD(this);

	PacketInterface* temp = _sendingPacket;

	if (_sendingPacket == nullptr)
		LOG_CRITICAL("TCPSocket::AtomicSendJob() : _sendingPacket is nullptr");

	if (_sendingPacket->_next != nullptr)
	{
		_sendingPacket = _sendingPacket->_next;
		_sendingPacket->_isSending = true;
		_server->Send(this, _sendingPacket);
	}
	else
	{
		_sendingPacket = nullptr;
		_lastPacket = nullptr;
	}

	PacketSelector::FreePacket(temp);
}

void TCPSocket::Send(const void* inData, unsigned int inLen, bool inImmediateSend)
{
	RECORD_STACK;

	WRITE_GUARD(this);

	unsigned int totalPacketSize = inLen + sizeof(unsigned int);

	if (_lastPacket != nullptr &&
		_lastPacket->_isSending == false &&
		_lastPacket->_immediateSend == false &&
		_lastPacket->_remainSize >= totalPacketSize &&
		inImmediateSend == false)
	{
		void* curBufferPos = NetworkUtils::SetSizeOnByteArrayFront(_lastPacket->_buffer + _lastPacket->_size, inLen);

		std::memcpy(curBufferPos, inData, inLen);
		_lastPacket->_size += totalPacketSize;
		_lastPacket->_remainSize -= totalPacketSize;
	}
	else
	{
		PacketInterface* sendPacket = PacketSelector::AllocPacket(inLen);

		void* curBufferPos = NetworkUtils::SetSizeOnByteArrayFront(sendPacket->_buffer, inLen);
		std::memcpy(curBufferPos, inData, inLen);
		sendPacket->_immediateSend = inImmediateSend;
		sendPacket->_size = totalPacketSize;
		sendPacket->_remainSize -= totalPacketSize;

		if (_sendingPacket == nullptr)
		{
			_sendingPacket = sendPacket;
			_sendingPacket->_isSending = true;

			_lastPacket = _sendingPacket;

			_server->Send(this, _sendingPacket);
		}
		else
		{
			_lastPacket->_next = sendPacket;
			_lastPacket = sendPacket;
		}
	}
}

void TCPSocket::RecvPacket(unsigned int inSize)
{
	RECORD_STACK;

	int flags = 0;

	if (inSize == 0)
	{
		if (WSARecv(_socket, &_recvBuf, 1, (LPDWORD)&_recvBytes,
			(LPDWORD)&flags, (LPOVERLAPPED)&_recvOverlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				LOG_WARN("TCPSocket::RecvPacket() : WSARecv failed with error");
		}

		return;
	}
	else
	{
		std::memcpy((char*)_packetBuf + _recvSize, _recvBuf.buf, inSize);
		_recvSize += inSize;
	}
	
	while (true)
	{
		if (_recvStep == RECV_STEP::RECV_LEN)
		{
			if (_recvSize >= sizeof(unsigned int))
			{
				_packetSize = NetworkUtils::GetSizeInByteArray(_packetBuf);
				_recvStep = RECV_STEP::RECV_BODY;
			}
			else
			{
				break;
			}
		}
		else if (_recvStep == RECV_STEP::RECV_BODY)
		{
			if (_recvSize >= _packetSize)
			{
				_networkClient->PacketHandler(_packetSize, static_cast<void*>((char*)_packetBuf));
				
				unsigned int overValue = _recvSize - _packetSize;

				_recvStep = RECV_STEP::RECV_LEN;

				if (overValue > 0)
				{
					std::memmove(_packetBuf, (char*)_packetBuf + _packetSize, overValue);
					_recvSize = overValue;
				}
				else
				{
					_recvSize = 0;
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	if (WSARecv(_socket, &_recvBuf, 1, (LPDWORD)&_recvBytes,
		(LPDWORD)&flags, (LPOVERLAPPED)&_recvOverlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			LOG_WARN("TCPSocket::RecvPacket() : WSARecv failed with error");
	}
}