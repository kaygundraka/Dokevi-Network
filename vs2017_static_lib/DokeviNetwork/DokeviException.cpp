#include "pch.h"
#include "DokeviException.h"
#include "ConfigManager.h"
#include <iostream>
#include <fstream>

using namespace DokeviNet;

std::vector<void(*)(void)> DokeviNet::Exception::excpetionHandler;

std::string serverInfoText = "";

void DokeviNet::MakeMinidump(EXCEPTION_POINTERS* e)
{
	auto dbgHelp = LoadLibraryA("dbghelp");
	
	if (dbgHelp == nullptr)
		return;

	auto miniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(dbgHelp, "MiniDumpWriteDump");
	
	if (miniDumpWriteDump == nullptr)
		return;

	char fileName[MAX_PATH];

	SYSTEMTIME t;
	GetSystemTime(&t);
	sprintf(fileName, "./Minidmp_%4d%02d%02d_%02d%02d%02d.dmp", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	
	auto file = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (file == INVALID_HANDLE_VALUE)
		return;

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = e;
	exceptionInfo.ClientPointers = FALSE;

	_MINIDUMP_TYPE type = MiniDumpNormal;
	
	if (SINGLETON(ConfigManager)->GetInt("System", "FullDump") == 1)
	{
		type = MiniDumpWithFullMemory;
	}

	auto dumped = miniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		file,
		type,
		e ? &exceptionInfo : nullptr,
		nullptr,
		nullptr);

	CloseHandle(file);

	return;
}

void DokeviNet::ReportCallStack()
{
	auto stackTraceArray = StackTracer::GetInstance()->GetStackFrameArray();

	int i = 0;

	char temp[1024] = "";

	for (auto stackFrameList : stackTraceArray)
	{
		sprintf_s(temp, "[Stack Trace : %d]\n", i);
		serverInfoText += temp;

		for (auto frame : stackFrameList)
		{
			sprintf_s(temp, "	func:%s, file:%s, line:%d\n", frame._function, frame._file, frame._line);
			serverInfoText += temp;
		}
	}
}

void DokeviNet::ReportLockStack()
{
	auto lockStackTraceArray = LockStackTracer::GetInstance()->GetLockStackFrameArray();

	int i = 0;

	char temp[1024] = "";

	for (auto lockStackList : lockStackTraceArray)
	{
		sprintf_s(temp, "[Lock Stack Trace : %d]\n", i);
		serverInfoText += temp;

		for (auto lockStack : lockStackList)
		{
			sprintf_s(temp, "	lock:%s, enter:%d\n", (char*)lockStack._lockAddress, lockStack._isEnter);
			serverInfoText += temp;
		}
	}
}

void DokeviNet::MakeReportFile()
{
	char fileName[MAX_PATH];

	SYSTEMTIME t;
	GetSystemTime(&t);
	sprintf(fileName, "./ServerCrashReport_%4d%02d%02d_%02d%02d%02d.txt", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

	ofstream reportFile;

	reportFile.open(fileName);
	reportFile.write(serverInfoText.c_str(), serverInfoText.size());
	reportFile.close();
}

LONG CALLBACK UnhandledHandler(EXCEPTION_POINTERS* e)
{
	DokeviNet::MakeMinidump(e);
	DokeviNet::ReportCallStack();
	DokeviNet::ReportLockStack();
	DokeviNet::MakeReportFile();

	for (auto handler : DokeviNet::Exception::excpetionHandler)
	{
		handler();
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void DokeviNet::SetMinidumpEvent()
{
	SetUnhandledExceptionFilter(UnhandledHandler);
}