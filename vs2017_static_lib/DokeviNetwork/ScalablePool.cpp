#include "pch.h"
#include "ScalablePool.h"
#include "ThreadManager.h"

using namespace DokeviNet;

ScalablePool::ScalablePool() {}

void ScalablePool::Init(unsigned int inSize, unsigned int inAllocSize)
{
	_size = inSize;
	_allocSize = inAllocSize;
	_allocIndex = 0;
	_useSize = 0;
	_curGeneration = 0;

	_data = std::malloc((inSize + sizeof(int) * 3) * _allocSize);
	_freeIndex.reserve(_allocSize);

	_memCheckArray[_curGeneration] = _data;
	_generationFree[_curGeneration] = _allocSize;
}

ScalablePool::~ScalablePool()
{
	std::free(_data);
}

void ScalablePool::IncreaseSize()
{
	lock_guard<mutex> Lock(mLock);

	void* data = _data;
	_allocSize *= 2;

	size_t newSize = (_size + sizeof(int) * 3) * _allocSize;

	void* newData = std::malloc(newSize);

	//std::memcpy(newData, mData, newSize / 2);
	_data = newData;
	//std::free(data);

	_freeIndex.reserve(_allocSize);

	_memCheckArray[++_curGeneration] = _data;
	_generationFree[_curGeneration] = _allocSize;
}

void* ScalablePool::Pop(unsigned int inSize)
{
	unsigned int allocIndex = _allocIndex;

	if (_freeIndex.size() == 0)
	{
		_allocIndex++;

		if (_allocIndex > _allocSize)
			IncreaseSize();
	}
	else
	{
		allocIndex = _freeIndex.back();
		_freeIndex.pop_back();
	}

	{
		lock_guard<mutex> Lock(mLock);

		void* allocMem = static_cast<void*>((char*)_data + (allocIndex * _size));
		*((unsigned int*)allocMem) = inSize;
		*((int*)allocMem + 1) = ThreadManager::GetInstance()->GetCurrentThreadIndex();

		*((int*)allocMem + 2) = _curGeneration;
		_usedIndex[allocMem] = allocIndex;

		++_useSize;

		return static_cast<void*>((int*)allocMem + 3);
	}
}

void ScalablePool::Push(void* inFreePtr, int inGeneration)
{
	lock_guard<mutex> Lock(mLock);

	_freeIndex.push_back(_usedIndex[inFreePtr]);
	_usedIndex.erase(inFreePtr);

	if (inGeneration != _curGeneration)
	{
		if (--_generationFree[inGeneration] == 0)
			std::free(_memCheckArray[inGeneration]);
	}

	--_useSize;
}

unsigned int ScalablePool::GetUseSize() const
{
	return _useSize;
}

unsigned int ScalablePool::GetAllocSize() const
{
	return _allocSize;
}

unsigned int ScalablePool::GetItemSize() const
{
	return _size;
}

//void ScalablePool::Init(size_t inSize, unsigned int inAllocSize)
//{
//	mSize = inSize;
//	mAllocSize = inAllocSize;
//	mAllocIndex = 0;
//	mUseSize = 0;
//
//	mData = std::malloc(inSize * mAllocSize * sizeof(int) * 2);
//	mFreeIndex.reserve(mAllocSize);
//}
//
//ScalablePool::~ScalablePool()
//{
//	std::free(mData);
//}
//
//void ScalablePool::IncreaseSize()
//{
//	void* data = mData;
//	mAllocSize *= 2;
//
//	size_t newSize = mAllocSize * mSize * sizeof(int) * 2;
//
//	void* newData = std::malloc(newSize);
//
//	std::memcpy(newData, mData, newSize / 2);
//	mData = newData;
//	std::free(data);
//
//	mFreeIndex.reserve(mAllocSize);
//}
//
//void* ScalablePool::Pop(size_t inSize)
//{
//	unsigned int allocIndex = mAllocIndex;
//
//	if (mFreeIndex.size() == 0)
//	{
//		mAllocIndex++;
//
//		if (mAllocIndex > mAllocSize)
//			IncreaseSize();
//	}
//	else
//	{
//		allocIndex = mFreeIndex.back();
//		mFreeIndex.pop_back();
//	}
//
//	void* allocMem = static_cast<void*>((char*)mData + (allocIndex * mSize));
//	*((int*)allocMem) = inSize;
//	*((int*)allocMem + 1) = ThreadManager::GetInstance()->GetCurrentThreadIndex();
//	
//	{
//		lock_guard<mutex> Lock(mLock);
//		mUsedIndex[allocMem] = allocIndex;
//	}
//
//	++mUseSize;
//
//	return static_cast<void*>((int*)allocMem + 2);
//}
//
//void ScalablePool::Push(void* inFreePtr)
//{
//	lock_guard<mutex> Lock(mLock);
//
//	mFreeIndex.push_back(mUsedIndex[inFreePtr]);
//	mUsedIndex.erase(inFreePtr);
//
//	--mUseSize;
//}
//
//unsigned int ScalablePool::GetUseSize() const
//{
//	return mUseSize;
//}
//
//unsigned int ScalablePool::GetAllocSize() const
//{
//	return mAllocSize;
//}
//
//unsigned int ScalablePool::GetItemSize() const
//{
//	return mSize;
//}