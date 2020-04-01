#include "pch.h"
#include "InterfaceFunction.h"
#include "ConfigManager.h"
#include "ThreadManager.h"
#include "PacketSelector.h"
#include "LogManager.h"
#include "DokeviException.h"
#include "RandGenerator.h"
#include "LockStackTracer.h"

using namespace DokeviNet;

void DokeviNet::InitializeManagers(std::string inInfoIni)
{	
	SINGLETON(RandGenerator)->Initialize();
	SINGLETON(ConfigManager)->Initialize(inInfoIni);
	SINGLETON(LogManager)->Initialize();
	SINGLETON(ThreadManager)->Initialize();
	SINGLETON(LockStackTracer)->Initialize();
	SINGLETON(StackTracer)->Initialize();

	PacketSelector::InitPacketSelector();
	
	if (SINGLETON(ConfigManager)->GetInt("System", "UseMiniDump"))
	{
		SetMinidumpEvent();
	}
}

void DokeviNet::ReleaseManagers()
{
	SINGLETON(StackTracer)->Finalize();
	SINGLETON(LockStackTracer)->Finalize();
	SINGLETON(ThreadManager)->WaitForAllThread();
	SINGLETON(ThreadManager)->Finalize();
	SINGLETON(LogManager)->Finalize();
	SINGLETON(ConfigManager)->Finalize();
}