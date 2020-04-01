#pragma once
#include "pch.h"

namespace DokeviNet 
{
	class PacketSelector;

	class PacketInterface 
	{
	public:
		char* _buffer;

		bool _immediateSend;
		bool _isSending;

		unsigned int _size;
		unsigned int _remainSize;
		unsigned int _maxSize;

		PacketInterface* _next;
		
	public:
		virtual void Release() = 0;
	};

	template<unsigned int SIZE = MAX_PACKET_BUF_SIZE>
	class StaticSizePacket : public PacketInterface
	{
	public:
		StaticSizePacket() = default;

		virtual void Release() override 
		{
			_buffer = buf;
			_immediateSend = { false };
			_next = { nullptr };
			_isSending = { false };
			_remainSize = { SIZE };
			_maxSize = SIZE;

			strcpy(buf, "");
		}
		
	private:
		char buf[SIZE];
		StaticSizePacket(const StaticSizePacket& copy) = delete;
	};
}