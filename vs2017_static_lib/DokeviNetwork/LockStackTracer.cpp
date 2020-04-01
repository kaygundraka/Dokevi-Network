#include "pch.h"
#include "LockStackTracer.h"
using namespace DokeviNet;

thread_local LockStack gLockStackFrames[MAX_CALL_STACK_DEPTH];
thread_local std::atomic<int> gLockStackDepth;
thread_local bool gRegisteredLockStackFrame = false;

LockStack* gRecordLockStackFrames[MAX_STACK_CHECK_THREAD_NUM];
std::atomic<int>* gLockStackDepths[MAX_STACK_CHECK_THREAD_NUM];

void LockStackTracer::Initialize() { LOG_INFO("Init - LockStackTracer"); }

void LockStackTracer::Finalize() {}

void LockStackTracer::RegisterThread()
{
	gRegisteredLockStackFrame = true;

	_sync.Enter(SYNC_TYPE::WRITE, false);

	if (_threadNumbers.find(std::this_thread::get_id()) != _threadNumbers.end())
	{
		return;
	}

	_lastThreadId.fetch_add(1);

	if (_lastThreadId >= MAX_STACK_CHECK_THREAD_NUM)
		LOG_CRITICAL("LockStackTracer GetCurrentThreadId OverFlow!!");

	_threadNumbers.insert(std::make_pair(std::this_thread::get_id(), (int)_lastThreadId));

	gRecordLockStackFrames[_lastThreadId] = gLockStackFrames;
	gLockStackDepths[_lastThreadId] = &gLockStackDepth;

	_sync.Leave(SYNC_TYPE::WRITE, false);
}

void LockStackTracer::TryLockStackTrace(SyncSection* inLockSync)
{
	if (!gRegisteredLockStackFrame)
	{
		RegisterThread();
	}

	const int depth = gLockStackDepth.fetch_add(1);

	gLockStackFrames[depth]._isEnter = false;
	gLockStackFrames[depth]._lockAddress = inLockSync;
}

void LockStackTracer::InLockStackTrace()
{
	gLockStackFrames[gLockStackDepth]._isEnter = true;
}

void LockStackTracer::OutLockStackTrace()
{
	const int depth = gLockStackDepth.fetch_sub(1);

	gLockStackFrames[depth]._lockAddress = nullptr;
	gLockStackFrames[depth]._isEnter = false;
}

std::vector<std::list<LockStack>> LockStackTracer::GetLockStackFrameArray()
{
	_sync.Enter(SYNC_TYPE::READ, false);

	std::vector<std::list<LockStack>> lockStackFrameArray;

	for (auto ThreadId : _threadNumbers)
	{
		std::list<LockStack> lockStackFrames;

		int depth = *(gLockStackDepths[ThreadId.second]);
		LockStack* stacks = gRecordLockStackFrames[ThreadId.second];

		for (int i = 0; i <= depth; i++)
			lockStackFrames.push_back(stacks[i]);

		if (gLockStackDepth != 0)
			lockStackFrameArray.push_back(lockStackFrames);
	}

	return lockStackFrameArray;

	_sync.Leave(SYNC_TYPE::READ, false);
}