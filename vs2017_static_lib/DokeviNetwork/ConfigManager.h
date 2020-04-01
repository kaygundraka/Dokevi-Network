#pragma once
#include "pch.h"
#include "ISingleton.h"
#include "SyncSection.h"

namespace DokeviNet
{
	class ConfigManager : public ISingleton<ConfigManager>, MultiThreadSync
	{
	private:
		std::string _fileAddr;

		virtual void Initialize() override 
		{
			LOG_INFO("Init - ConfigManager");
		}

		std::map<std::string, std::map<std::string, std::string>> cache_;

	public:
		void Initialize(std::string inIniFileName);
		virtual void Finalize() override;
		
		std::string GetStr(const char* inType, const char* inAttribute);
		const int GetInt(const char* inType, const char* inAttribute);
	};
}