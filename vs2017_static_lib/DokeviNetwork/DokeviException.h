#pragma once
#include "pch.h"

namespace DokeviNet
{
	void MakeMinidump(EXCEPTION_POINTERS* e);
	void ReportCallStack();
	void ReportLockStack();
	void MakeReportFile();
	void SetMinidumpEvent();

	class Exception
	{
	public:
		static std::vector<void(*)(void)> excpetionHandler;
	};
}