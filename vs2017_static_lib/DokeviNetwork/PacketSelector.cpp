#include "pch.h"
#include "PacketInterface.h"
#include "PacketSelector.h"

using namespace DokeviNet;

AtomicStaticPool<StaticSizePacket<64>> PacketSelector::_packet64;
AtomicStaticPool<StaticSizePacket<128>> PacketSelector::_packet128;
AtomicStaticPool<StaticSizePacket<256>> PacketSelector::_packet256;
AtomicStaticPool<StaticSizePacket<512>> PacketSelector::_packet512;
AtomicStaticPool<StaticSizePacket<1024>> PacketSelector::_packet1024;
AtomicStaticPool<StaticSizePacket<2048>> PacketSelector::_packet2048;
AtomicStaticPool<StaticSizePacket<4096>> PacketSelector::_packet4096;