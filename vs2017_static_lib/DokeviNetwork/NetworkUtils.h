#pragma once
#include "pch.h"

namespace DokeviNet
{
	enum SocketAddressFamily
	{
		INET = AF_INET,
		INET6 = AF_INET6
	};

	class NetworkUtils
	{
	public:
		static bool	StaticInit();
		static void	CleanUp();

		static void* SetSizeOnByteArrayFront(void* inByteArray, unsigned int inSize);
		static unsigned int GetSizeInByteArray(void* inByteArray);
	};
}