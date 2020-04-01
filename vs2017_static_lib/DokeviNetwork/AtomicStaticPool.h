#pragma once
#include "pch.h"

namespace DokeviNet
{
    template <typename T>
    class AtomicStaticPool
    {
    private:
		unsigned __int64 _used;
		unsigned __int64 _queueSize;

		std::mutex _lock;
        T* _memory;

		std::map<T*, unsigned __int64> _memoryIndicate;

    public:
		explicit AtomicStaticPool() = default;
		AtomicStaticPool(const AtomicStaticPool& inRight) = delete;

		void Init(unsigned __int64 inSize)
		{
			_queueSize = inSize;
			_memory = new T[_queueSize];
			
			_used = 0;

			for (unsigned __int64 i = 0; i < _queueSize; i++)
			{
				_memoryIndicate.insert(std::make_pair(&_memory[i], i));
			}
		}

		~AtomicStaticPool()
		{
			delete[] _memory;
		}

		T* Alloc()
        {
			std::lock_guard<mutex> Guard(_lock);

			if (_used == _queueSize)
			{
				LOG_CRITICAL("[{}][{}] AtomicStaticPool is full", __FILE__, __LINE__);
				return nullptr;
			}
			else
			{
				_memory[_used].Release();
				return &_memory[_used++];
			}
		}

		void Free(T* inElement)
		{
			std::lock_guard<mutex> Guard(_lock);

			auto iter = _memoryIndicate.find(inElement);

			if (iter == _memoryIndicate.end())
			{
				LOG_CRITICAL("[{}][{}] invalid element.", __FILE__, __LINE__);
			}
			else
			{
				_used--;
				_memory[iter->second].Release();
			}
		}

		__int64 GetCapacitiy() const
        {
			return _queueSize;
        }

		__int64 GetUsedSize() const
		{
			std::lock_guard<mutex> Guard(_lock);
			return _used;
		}
    };
}