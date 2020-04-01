#pragma once
#include "pch.h"

namespace DokeviNet
{
	class ProgressLock
	{
	private:
		volatile unsigned int _curWork;
		int _timeout;
		int _startTick;

	public:
		ProgressLock(int inTimeOut) : _curWork(0), _timeout(inTimeOut), _startTick(0) {}
		ProgressLock() : _curWork(0), _timeout(-1), _startTick(0) {}

		bool Start(unsigned int inWorkType, unsigned int& inCurWorkType)
		{
			const bool checkTimeOut = _timeout != -1 && _curWork != 0;
			bool isTimeOut = false;

			if (checkTimeOut)
			{
				isTimeOut = (GetTickCount() - _startTick) >= _timeout;
			}

			if (!isTimeOut && InterlockedCompareExchange(&_curWork, inWorkType, 0) != 0)
			{
				inCurWorkType = _curWork;
				return false;
			}

			inCurWorkType = inWorkType;
			_startTick = GetTickCount();
			return true;
		}

		void End()
		{
			InterlockedExchange(&_curWork, 0);
		}
	};
}