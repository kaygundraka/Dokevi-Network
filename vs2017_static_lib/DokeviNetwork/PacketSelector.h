#pragma once
#include "pch.h"
#include "AtomicStaticPool.h"
#include "PacketInterface.h"

namespace DokeviNet
{
	class PacketSelector
	{
	public:
		static AtomicStaticPool<StaticSizePacket<64>> _packet64;
		static AtomicStaticPool<StaticSizePacket<128>> _packet128;
		static AtomicStaticPool<StaticSizePacket<256>> _packet256;
		static AtomicStaticPool<StaticSizePacket<512>> _packet512;
		static AtomicStaticPool<StaticSizePacket<1024>> _packet1024;
		static AtomicStaticPool<StaticSizePacket<2048>> _packet2048;
		static AtomicStaticPool<StaticSizePacket<4096>> _packet4096;
		
	public:
		static void InitPacketSelector()
		{
			_packet64.Init(1000);
			_packet128.Init(1000);
			_packet256.Init(1000);
			_packet512.Init(1000);
			_packet1024.Init(1000);
			_packet2048.Init(1000);
			_packet4096.Init(1000);

			LOG_INFO("Init - PacketSelector");
		}

		static PacketInterface* AllocPacket(unsigned int inSize)
		{
			if (inSize > MAX_PACKET_BUF_SIZE)
			{
				LOG_CRITICAL("PacketSelector : AllocPacket() Overflow Size[{}]", inSize);
				_ASSERT(0);
				return nullptr;
			}

			PacketInterface* sendPacket = nullptr;
			
			if (inSize <= 64)                         { sendPacket = _packet64.Alloc(); }
			else if (inSize > 64 && inSize <= 128)    { sendPacket = _packet128.Alloc(); }
			else if (inSize > 128 && inSize <= 256)   { sendPacket = _packet256.Alloc(); }
			else if (inSize > 256 && inSize <= 512)   { sendPacket = _packet512.Alloc(); }
			else if (inSize > 512 && inSize <= 1024)  { sendPacket = _packet1024.Alloc(); }
			else if (inSize > 1024 && inSize <= 2048) { sendPacket = _packet2048.Alloc(); }
			else if (inSize > 2048 && inSize <= 4096) {	sendPacket = _packet4096.Alloc(); }
			else
			{
				LOG_CRITICAL("PacketSelector : AllocPacket() Overflow Size[{}]", inSize);
				_ASSERT(0);
				return nullptr;
			}

			return sendPacket;
		}

		static void FreePacket(PacketInterface* inPacket)
		{
			unsigned __int64 size = inPacket->_maxSize;

			if (size <= 64)						  { _packet64.Free(static_cast<StaticSizePacket<64>*>(inPacket)); }
			else if (size > 64 && size <= 128)	  { _packet128.Free(static_cast<StaticSizePacket<128>*>(inPacket)); }
			else if (size > 128 && size <= 256)   { _packet256.Free(static_cast<StaticSizePacket<256>*>(inPacket)); }
			else if (size > 256 && size <= 512)   { _packet512.Free(static_cast<StaticSizePacket<512>*>(inPacket)); }
			else if (size > 512 && size <= 1024)  { _packet1024.Free(static_cast<StaticSizePacket<1024>*>(inPacket)); }
			else if (size > 1024 && size <= 2048) { _packet2048.Free(static_cast<StaticSizePacket<2048>*>(inPacket)); }
			else if (size > 2048 && size <= 4096) { _packet4096.Free(static_cast<StaticSizePacket<4096>*>(inPacket)); }
			else
			{
				LOG_CRITICAL("PacketSelector : FreePacket() Overflow Size[{}]", size);
				_ASSERT(0);
				return;
			}
		}
	};
}