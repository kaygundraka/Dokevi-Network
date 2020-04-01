#include "pch.h"
#include "NetworkUtils.h"

using namespace DokeviNet;

void* NetworkUtils::SetSizeOnByteArrayFront(void* inByteArray, unsigned int inSize)
{
	*(unsigned int*)inByteArray = inSize;

	return static_cast<void*>((unsigned int*)(inByteArray) + 1);
}

unsigned int NetworkUtils::GetSizeInByteArray(void* inByteArray)
{
	return *((unsigned int*)inByteArray);
}

bool NetworkUtils::StaticInit()
{
	RECORD_STACK;

#if _WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		LOG_CRITICAL("NetworkUtils::StaticInit() Failed");
		return false;
	}
#endif
	
	return true;
}

void NetworkUtils::CleanUp()
{
	RECORD_STACK;

#if _WIN32
	WSACleanup();
#endif
}