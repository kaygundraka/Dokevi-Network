#include "pch.h"
#include "SyncSection.h"
#include "LockStackTracer.h"

using namespace DokeviNet;

void SyncSection::Enter(const SYNC_TYPE& inType, bool inRecordLock)
{	
	if (_ownerThread == GetCurrentThreadId())
	{
		_lockDepth.fetch_add(1);
		return;
	}

	if (inRecordLock)
		RECORD_LOCK_TRY(this);

	if (inType == SYNC_TYPE::READ)
		_mutex.lock_shared();
	else
		_mutex.lock();

	_ownerThread = GetCurrentThreadId();

	if (inRecordLock)
		RECORD_LOCK_ENTER();
}

void SyncSection::Leave(SYNC_TYPE inType, bool inRecordLock)
{
	if (_lockDepth > 0)
	{
		_lockDepth.fetch_sub(1);
	}
	else
	{
		if (inRecordLock)
			RECORD_LOCK_LEAVE();

		if (inType == SYNC_TYPE::READ)
			_mutex.unlock_shared();
		else
			_mutex.unlock();

		_ownerThread = -1;

		return;
	}
}