#pragma once
#include "pch.h"

namespace DokeviNet
{
	template <typename T>
	class AtomicStaticQueue
	{
	private:
		volatile ULONG _front;
		volatile ULONG _back;
		volatile ULONG _used;
		unsigned int _queueSize;

		T* _memory;

	public:
		explicit AtomicStaticQueue() = default;
		AtomicStaticQueue(const AtomicStaticQueue& inRight) = delete;

		void Init(unsigned int inSize)
		{
			_queueSize = inSize;
			_front = 1;
			_back = 0;
			_used = 0;
			_memory = new T[_queueSize];
		}

		~AtomicStaticQueue()
		{
			delete[] _memory;
		}

		void Push(T element)
		{
			unsigned int Index = InterlockedIncrement(&_back);
			_memory[Index % _queueSize] = element;

			InterlockedIncrement(&_used);
			if (_used >= _queueSize)
			{
				LOG_CRITICAL("AtomicStaticQueue Push() is overflow!!! mUsed:{}, mQueueSize:{}, mBack:{}, mFront:{}"
					, _used, _queueSize, _back, _front);
			}
		}

		void Pop()
		{
			InterlockedDecrement(&_used);
			if (_used < 0)
			{
				LOG_CRITICAL("AtomicStaticQueue Pop() is overflow!!! mUsed:{}, mQueueSize:{}, mBack:{}, mFront:{}"
					, _used, _queueSize, _back, _front);
			}

			InterlockedIncrement(&_front);
		}

		T* Front()
		{
			return &_memory[_front % _queueSize];
		}

		bool IsEmpty()
		{
			return _used == 0;
		}

		int GetCapacitiy() const
		{
			return _queueSize;
		}

		int GetUsedSize() const
		{
			return _used;
		}

	};
}