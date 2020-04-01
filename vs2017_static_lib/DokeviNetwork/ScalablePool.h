#pragma once
#include "pch.h"

namespace DokeviNet
{
	class ScalablePool
	{
	private:
		void* _data;
		void** _pointerArray;
		unsigned int _size;
		unsigned int _allocSize;
		unsigned int _allocIndex;
		unsigned int _useSize;

		std::vector<unsigned int> _freeIndex;
		std::map<void*, unsigned int> _usedIndex;

		std::map<unsigned int, void*> _memCheckArray;
		std::map<unsigned int, unsigned int> _generationFree;

		unsigned int _curGeneration;

		void IncreaseSize();

		std::mutex mLock;

	public:
		ScalablePool();
		~ScalablePool();

		void Init(unsigned int inSize, unsigned int inAllocSize);

		void* Pop(unsigned int inSize);
		void Push(void* inFreePtr, int inGeneration);

		unsigned int GetUseSize() const;
		unsigned int GetAllocSize() const;
		unsigned int GetItemSize() const;
	};
}