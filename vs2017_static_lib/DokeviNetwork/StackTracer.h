#pragma once
#include "pch.h"
#include "ISingleton.h"
#include "SyncSection.h"

namespace DokeviNet
{
	struct StackFrame
	{
		int _line;

		char* _file;
		char* _function;
	};

	class StackFrameRecorder
	{
	public:
		StackFrameRecorder(const char* inFile, int inLine, const char* inFunction);
		~StackFrameRecorder();
	};

	class StackTracer : public ISingleton<StackTracer>, MultiThreadSync
	{
	private:
		int _lastThreadId;

		std::map<std::thread::id, int> _threadNumbers;

	public:
		virtual void Initialize() override;
		virtual void Finalize() override;

		void RegisterThread();
		
		void InStackTrace(const char* inFile, int inLine, const char* inFunction);
		void OutStackTrace();

		static void MakeFileNameUsingAddress(char* inFileName, const char* inFileAddr);

		std::vector<std::list<StackFrame>> GetStackFrameArray();
	};
}

#define RECORD_STACK \
	static char _fileName[MAX_PATH] = ""; \
	static bool _isSetFileName = false; \
	static const char* _functionName = __FUNCTION__; \
	if (_isSetFileName == false) \
	{ \
		StackTracer::MakeFileNameUsingAddress(_fileName, __FILE__); \
		_isSetFileName = true; \
	} \
	DokeviNet::StackFrameRecorder stackFrameRecorderInstance(_fileName, __LINE__, _functionName);
