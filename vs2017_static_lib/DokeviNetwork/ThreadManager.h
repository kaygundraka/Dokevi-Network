#pragma once
#include "pch.h"
#include "ISingleton.h"

namespace DokeviNet
{
	using ThreadIndex = short;

	struct ThreadInfo
	{
		enum THREAD_TYPE { 
			MAIN_THREAD = 0,
			IO_TASK
		};

		ThreadIndex index;
		THREAD_TYPE type;
	};

	class ManagedThread
	{
	private:
		virtual void ThreadTask() = 0;

	public:
		friend class ThreadManager;

		void RealTask();
	};

    class ThreadManager : public ISingleton<ThreadManager>
    {
    private:
        unsigned short _curThread;
        unsigned short _count;
		unsigned short _size;

		std::thread::id _mainThreadId;

		std::mutex _lock;

		std::map<std::thread::id, ThreadInfo> _infoMap;
		std::list<std::thread*> _threadPool;

    public:
		virtual void Initialize() override;
		virtual void Finalize() override;

		void TaskRun(ManagedThread* inObject);
		void WaitForAllThread();
		
		ThreadIndex RegisterThread(std::thread* inThread, ThreadInfo::THREAD_TYPE inType);
		ThreadIndex RegisterThread(ThreadInfo::THREAD_TYPE inType);

		void RemoveThread();
		
		ThreadInfo GetCurrentThreadInfo();
        ThreadIndex GetCurrentThreadIndex();

		unsigned short GetThreadCount();
    };
}
