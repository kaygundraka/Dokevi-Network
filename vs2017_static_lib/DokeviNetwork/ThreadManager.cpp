#include "pch.h"
#include "ThreadManager.h"

using namespace DokeviNet;

void ManagedThread::RealTask()
{
	ThreadTask();
}

void ThreadManager::Initialize()
{
	_curThread = 0;
	_count = 1;
	_size = 1;

	ThreadInfo info;
	info.type = ThreadInfo::THREAD_TYPE::MAIN_THREAD;
	info.index = 0;
	_mainThreadId = std::this_thread::get_id();
	_infoMap.insert(std::pair<thread::id, ThreadInfo>(_mainThreadId, info));

	LOG_INFO("Init - ThreadManager");
}

void ThreadManager::WaitForAllThread()
{
	for (thread* var : _threadPool)
		if (var != nullptr)
			var->join();
}

void ThreadManager::Finalize()
{
	for(thread* var : _threadPool)
		if (var != nullptr)
			var->join();
	
	for (thread* var : _threadPool)
		if (var != nullptr)
			delete var;
	
	_released = true;
}

void ThreadManager::TaskRun(ManagedThread* inObject)
{
	RECORD_STACK;

	auto taskThread = new thread(&ManagedThread::RealTask, inObject);
	_threadPool.push_back(taskThread);
}

ThreadIndex ThreadManager::RegisterThread(std::thread* inThread, ThreadInfo::THREAD_TYPE inType)
{
	RECORD_STACK;

	std::lock_guard<std::mutex> lock(_lock);

	ThreadInfo info;
	info.type = inType;
	info.index = _count++;
	_infoMap.insert(std::pair<thread::id, ThreadInfo>(inThread->get_id(), info));

	++_size;

	return info.index;
}

ThreadIndex ThreadManager::RegisterThread(ThreadInfo::THREAD_TYPE inType)
{
	RECORD_STACK;

	std::lock_guard<std::mutex> lock(_lock);

	ThreadInfo info;
	info.type = inType;
	info.index = _count++;
	_infoMap.insert(std::pair<thread::id, ThreadInfo>(std::this_thread::get_id(), info));

	++_size;

	return info.index;
}

void ThreadManager::RemoveThread()
{
	RECORD_STACK;

	std::lock_guard<std::mutex> lock(_lock);

	auto iter = _threadPool.front();
	int loop = _infoMap[std::this_thread::get_id()].index;

	for (int i = 0; i < loop; i++)
		iter++;

	_threadPool.remove(iter);

	--_size;
}

ThreadInfo ThreadManager::GetCurrentThreadInfo()
{
	return _infoMap[std::this_thread::get_id()];
}

ThreadIndex ThreadManager::GetCurrentThreadIndex()
{
	return _infoMap[std::this_thread::get_id()].index;
}

unsigned short ThreadManager::GetThreadCount()
{
	return _size;
}