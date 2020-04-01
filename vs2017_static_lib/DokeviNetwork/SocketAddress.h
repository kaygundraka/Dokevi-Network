#pragma once

namespace DokeviNet
{
	class TCPSocket;

	class SocketAddress
	{
	public:
		SocketAddress(uint32_t inAddress, uint16_t inPort)
		{
			GetAsSockAddrIn()->sin_family = AF_INET;
			GetIP4Ref() = htonl(inAddress);
			GetAsSockAddrIn()->sin_port = htons(inPort);
		}

		SocketAddress(const sockaddr& inSockAddr)
		{
			memcpy(&_sockAddr, &inSockAddr, sizeof(sockaddr));
		}

		SocketAddress(uint16_t inPort)
		{
			GetAsSockAddrIn()->sin_family = AF_INET;
			GetIP4Ref() = htonl(INADDR_ANY);
			GetAsSockAddrIn()->sin_port = htons(inPort);
		}

		bool operator==(const SocketAddress& inOther) const
		{
			return (_sockAddr.sa_family == AF_INET &&
				GetAsSockAddrIn()->sin_port == inOther.GetAsSockAddrIn()->sin_port) &&
				(GetIP4Ref() == inOther.GetIP4Ref());
		}
		
		uint32_t GetSize() const { return sizeof(sockaddr); }
		
	private:
		friend class TCPSocket;

		sockaddr _sockAddr;

		uint32_t&			GetIP4Ref() { return *reinterpret_cast<uint32_t*>(&GetAsSockAddrIn()->sin_addr.S_un.S_addr); }
		const uint32_t&		GetIP4Ref()	const { return *reinterpret_cast<const uint32_t*>(&GetAsSockAddrIn()->sin_addr.S_un.S_addr); }

		sockaddr_in*        GetAsSockAddrIn() { return reinterpret_cast<sockaddr_in*>(&_sockAddr); }
		const sockaddr_in*  GetAsSockAddrIn() const { return reinterpret_cast<const sockaddr_in*>(&_sockAddr); }
	};
}