#include "pch.h"
#include "TCPSocket.h"
#include "NetworkClient.h"

using namespace DokeviNet;

NetworkClient::~NetworkClient() { _socket.reset(); }

IndependentNetworkClient::~IndependentNetworkClient()
{
	if (_disconnect)
	{
		return;
	}

	Disconnect();
}

bool IndependentNetworkClient::Connect(std::string inServerIp, short inServerPort)
{
	RECORD_STACK;

	_disconnect = false;

	_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (_listenSocket == INVALID_SOCKET)
	{
		return false;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(inServerPort);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(inServerIp.c_str());

	if (connect(_listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		closesocket(_listenSocket);
		return false;
	}

	_recvThread = new std::thread(&IndependentNetworkClient::RecvPacket, this);

	return true;
}

void IndependentNetworkClient::Wait()
{
	_recvThread->join();
}

void IndependentNetworkClient::Disconnect()
{
	RECORD_STACK;

	_disconnect = true;

	closesocket(_disconnect);

	_recvThread->join();
}

void IndependentNetworkClient::RecvPacket()
{
	RECORD_STACK;

	while (!_disconnect)
	{
		const int maxBuffer = MAX_PACKET_BUF_SIZE;

		char buffer[maxBuffer] = "";

		int receiveBytes = recv(_listenSocket, buffer, maxBuffer, 0);

		if (receiveBytes == -1)
		{
			break;
		}

		DeserializePacket(buffer, receiveBytes);
	}
}

unsigned int IndependentNetworkClient::GetProtocol(char* inBuffer, char** inCoreBuffer)
{
	*inCoreBuffer = inBuffer + 2 * sizeof(unsigned int);
	return (unsigned int)(*(inBuffer + sizeof(unsigned int)));
}

void IndependentNetworkClient::DeserializePacket(char* ReceivedData, int SizeRead)
{
	RECORD_STACK;

	memcpy(_packetBuffer + _curOffset, ReceivedData, SizeRead);
	_curOffset += SizeRead;

	if (_bufferSize == 0 && _curOffset >= sizeof(unsigned int))
		_bufferSize = *((unsigned int*)(_packetBuffer));

	if (_curOffset <= sizeof(unsigned int))
		return;

	while (_bufferSize <= _curOffset && _bufferSize != 0)
	{
		char* buffer = nullptr;

		auto protocol = GetProtocol(_packetBuffer, &buffer);

		RecvHandler(protocol, buffer, _bufferSize);

		_curOffset -= (_bufferSize);

		memmove(_packetBuffer, static_cast<char*>(_packetBuffer) + _bufferSize, _curOffset);
		_bufferSize = 0;

		if (_bufferSize == 0 && _curOffset >= sizeof(unsigned int))
			_bufferSize = *((unsigned int*)(_packetBuffer));

		if (_curOffset <= sizeof(unsigned int))
			return;
	}
}

void IndependentNetworkClient::SendPacket(int inProtocol, void* inPacket, unsigned int inLength)
{
	RECORD_STACK;

	char packet[MAX_PACKET_BUF_SIZE] = "";

	unsigned int protocol = inProtocol;
	unsigned int packetLength = inLength + sizeof(unsigned int) * 2;

	memcpy(packet, &(packetLength), sizeof(unsigned int));
	memcpy(packet + sizeof(unsigned int), &protocol, sizeof(unsigned int));
	memcpy(packet + sizeof(unsigned int) * 2, inPacket, inLength);

	unsigned int SendCount = 0;

	do
	{
		SendCount += send(_listenSocket, packet, packetLength, 0);
	} while (SendCount != packetLength);
}