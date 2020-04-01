#pragma once
#include "pch.h"
#include "ISingleton.h"
#include "SyncSection.h"

namespace DokeviNet
{
	struct LockStack
	{
		bool _isEnter = false;
		SyncSection* _lockAddress = nullptr;
	};

	class LockStackTracer : public ISingleton<LockStackTracer>, MultiThreadSync
	{
	private:
		std::atomic<int> _lastThreadId;
		std::map<std::thread::id, int> _threadNumbers;

	public:
		virtual void Initialize() override;
		virtual void Finalize() override;

		void RegisterThread();

		void TryLockStackTrace(SyncSection* inLockSync);
		void InLockStackTrace();
		void OutLockStackTrace();

		std::vector<std::list<LockStack>> GetLockStackFrameArray();
	};
}

#define RECORD_LOCK_TRY(sync) SINGLETON(LockStackTracer)->TryLockStackTrace(sync);
#define RECORD_LOCK_ENTER() SINGLETON(LockStackTracer)->InLockStackTrace();
#define RECORD_LOCK_LEAVE() SINGLETON(LockStackTracer)->OutLockStackTrace();