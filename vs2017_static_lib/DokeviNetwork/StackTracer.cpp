#include "pch.h"
#include "StackTracer.h"
using namespace DokeviNet;

thread_local StackFrame gStackFrames[MAX_CALL_STACK_DEPTH];
thread_local unsigned int gStackDepth = 0;
thread_local bool gRegisteredStackFrame = false;

StackFrame* gRecordStackFrames[MAX_STACK_CHECK_THREAD_NUM];
unsigned int* gStackDepths[MAX_STACK_CHECK_THREAD_NUM];

StackFrameRecorder::StackFrameRecorder(const char* inFile, int inLine, const char* inFunction)
{
	SINGLETON(StackTracer)->InStackTrace(inFile, inLine, inFunction);
}

StackFrameRecorder::~StackFrameRecorder()
{
	SINGLETON(StackTracer)->OutStackTrace();
}

void StackTracer::Initialize() { LOG_INFO("Init - StackTracer"); }

void StackTracer::Finalize() {}

void StackTracer::RegisterThread()
{
	gRegisteredStackFrame = true;

	WRITE_LOCK;

	_lastThreadId++;
		
	if (_lastThreadId >= MAX_STACK_CHECK_THREAD_NUM)
	{
		LOG_CRITICAL("StackTracer GetCurrentThreadId OverFlow!!");
	}

	_threadNumbers.insert(std::make_pair(std::this_thread::get_id(), (int)_lastThreadId));
	
	gRecordStackFrames[_lastThreadId] = gStackFrames;
	gStackDepths[_lastThreadId] = &gStackDepth;

	END_LOCK;
}

void StackTracer::InStackTrace(const char* inFile, int inLine, const char* inFunction)
{
	if (gRegisteredStackFrame == false)
	{
		RegisterThread();
	}

	if (gStackDepth == MAX_CALL_STACK_DEPTH - 1)
	{
		LOG_CRITICAL("over max size call stack depth.");
		return;
	}

	const int depth = gStackDepth++;
	
	gStackFrames[depth]._file = const_cast<char*>(inFile);
	gStackFrames[depth]._line = inLine;
	gStackFrames[depth]._function = const_cast<char*>(inFunction);
}

void StackTracer::OutStackTrace()
{
	gStackDepth--;
}

void StackTracer::MakeFileNameUsingAddress(char* inFileName, const char* inFileAddr)
{
	char sfileAddr[MAX_PATH];
	strcpy(sfileAddr, inFileAddr);
	strcpy(inFileName, strrchr(sfileAddr, '\\') + 1);
}

std::vector<std::list<StackFrame>> StackTracer::GetStackFrameArray()
{
	READ_LOCK;

	std::vector<std::list<StackFrame>> stackFrameArray;

	for (auto ThreadId : _threadNumbers)
	{
		std::list<StackFrame> stackFrames;

		int depth = *(gStackDepths[ThreadId.second]);
		StackFrame* stacks = gRecordStackFrames[ThreadId.second];

		for (int i = 0; i <= depth; i++)
			stackFrames.push_back(stacks[i]);
		
		if (gStackDepth != 0)
			stackFrameArray.push_back(stackFrames);
	}

	return stackFrameArray;

	END_LOCK;
}