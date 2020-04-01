#pragma once
#include "pch.h"
#include "ThreadManager.h"
#include "LogManager.h"

#define READ_LOCK { Sync _readLock(_sync, DokeviNet::SYNC_TYPE::READ, __FILE__, __LINE__);
#define WRITE_LOCK { Sync _wrtieLock(_sync, DokeviNet::SYNC_TYPE::WRITE, __FILE__, __LINE__);
#define END_LOCK }

#define WRITE_GUARD(inst) Sync _wrtieLock(inst->_sync, DokeviNet::SYNC_TYPE::WRITE, __FILE__, __LINE__);
#define READ_GUARD(inst) Sync _readLock(inst->_sync, DokeviNet::SYNC_TYPE::READ, __FILE__, __LINE__);

#define WRITE_GUARD_2(lock) Sync _wrtieLock(lock, DokeviNet::SYNC_TYPE::WRITE, __FILE__, __LINE__);
#define READ_GUARD_2(lock) Sync _readLock(lock, DokeviNet::SYNC_TYPE::READ, __FILE__, __LINE__);

#define REQUIRED_LOCK_CHECKER(inst) if (!inst->_sync.IsEntered()) { _ASSERT(0); }

namespace DokeviNet
{
    enum class SYNC_TYPE { READ, WRITE };
	
	class SyncSection
	{
	private:
		mutable std::shared_mutex _mutex;
		unsigned int _ownerThread = -1;
		std::atomic<int> _lockDepth;

	public:
		SyncSection() { _lockDepth = 0; }
		~SyncSection() = default;

		void Enter(const SYNC_TYPE& inType, bool inRecordLock = false);
		void Leave(SYNC_TYPE inType, bool inRecordLock = false);
		bool IsEntered() { return _ownerThread == GetCurrentThreadId(); }
	};

    class MultiThreadSync {
    protected:
        SYNC_TYPE _type;
        
    public:
		mutable SyncSection _sync;

		MultiThreadSync() {}

		class Sync {
        private:
            SYNC_TYPE _syncType;
			SyncSection& _ownerSync;

			std::string _file;
			int _line;

        public:
			Sync(SyncSection& sync, SYNC_TYPE type, std::string file, int line)
				: _ownerSync(sync), _syncType(type), _file(file), _line(line)
			{	_ownerSync.Enter(_syncType); }
		
			~Sync() { _ownerSync.Leave(_syncType); }
		};

		void ReadLock()
		{
			_type = SYNC_TYPE::READ;
			_sync.Enter(_type);
		}

		void WriteLock()
		{
			_type = SYNC_TYPE::WRITE;
			_sync.Enter(_type);
		}

		bool IsEntered()
		{
			return _sync.IsEntered();
		}

		void EndLock()
		{
			_sync.Leave(_type);
		}

	protected:
		friend class Sync;
    };
}