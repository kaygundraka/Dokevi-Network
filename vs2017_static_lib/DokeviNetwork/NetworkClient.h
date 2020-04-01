#pragma once
#include "pch.h"
#include "TCPSocket.h"
#include "DokeviServer.h"
#include "PacketSelector.h"

namespace DokeviNet
{
	class NetworkClient abstract
	{
	protected:
		std::shared_ptr<TCPSocket> _socket;

	public:
		NetworkClient(std::shared_ptr<TCPSocket> inSocket) : _socket(inSocket) {}
		
		const std::shared_ptr<TCPSocket> GetSocket() { return _socket; }
		~NetworkClient();

		virtual void ConnectedHandler() = 0;
		virtual void DisconnectedHandler() = 0;
		virtual void PacketHandler(unsigned int inSize, void* inData) = 0;
	};

	class IndependentNetworkClient : public MultiThreadSync
	{
	private:
		SOCKET _listenSocket;
		std::thread* _recvThread;
		std::atomic<bool> _disconnect;

	public:
		IndependentNetworkClient() : _curOffset(0) {}
		~IndependentNetworkClient();

		bool Connect(std::string inServerIp, short inServerPort);
		void Disconnect();

		void Wait();

		void SendPacket(int inProtocol, void* inPacket, unsigned int inLength);
		virtual void RecvHandler(int inProtocol, void* inData, int inSize) {}

	private:
		void RecvPacket();

		unsigned int GetProtocol(char* inBuffer, char** inCoreBuffer);
		void DeserializePacket(char* ReceivedData, int SizeRead);
		
		char _packetBuffer[MAX_PACKET_BUF_SIZE] = "";
		unsigned int _curOffset;
		unsigned int _bufferSize;
		unsigned int _receiveBufferSize;
	};
}