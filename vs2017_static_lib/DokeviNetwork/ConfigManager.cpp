#include "pch.h"
#include "ConfigManager.h"

using namespace DokeviNet;

void ConfigManager::Initialize(std::string inIniFileName)
{
	_fileAddr = inIniFileName;
}

void ConfigManager::Finalize()
{
	_released = true;
}

std::string ConfigManager::GetStr(const char* inType, const char* inAttribute)
{
	WRITE_LOCK

	auto iter = cache_.find(inType);

	if (iter == cache_.end())
	{
		char value[MAX_PATH];
		GetPrivateProfileStringA(inType, inAttribute, "", value, MAX_PATH, _fileAddr.c_str());

		std::map<std::string, std::string> newMap;
		newMap.insert(std::pair<std::string, std::string>(inAttribute, value));
		cache_.insert(std::pair<std::string, std::map<std::string, std::string>>(inType, newMap));

		return value;
	}

	auto iter2 = iter->second.find(inAttribute);

	if (iter2 == iter->second.end())
	{
		char value[MAX_PATH];
		GetPrivateProfileStringA(inType, inAttribute, "", value, MAX_PATH, _fileAddr.c_str());

		cache_[inType].insert(std::pair<std::string, std::string>(inAttribute, value));
		
		return value;
	}

	return cache_[inType][inAttribute];

	END_LOCK
}


const int ConfigManager::GetInt(const char* inType, const char* inAttribute)
{
	WRITE_LOCK

	auto iter = cache_.find(inType);

	if (iter == cache_.end())
	{
		char value[MAX_PATH];
		GetPrivateProfileStringA(inType, inAttribute, "", value, MAX_PATH, _fileAddr.c_str());

		std::map<std::string, std::string> NewMap;
		NewMap.insert(std::pair<std::string, std::string>(inAttribute, value));
		cache_.insert(std::pair<std::string, std::map<std::string, std::string>>(inType, NewMap));

		return std::atoi(value);
	}

	auto iter2 = iter->second.find(inAttribute);

	if (iter2 == iter->second.end())
	{
		char value[MAX_PATH];
		GetPrivateProfileStringA(inType, inAttribute, "", value, MAX_PATH, _fileAddr.c_str());

		cache_[inType].insert(std::pair<std::string, std::string>(inAttribute, value));

		return  std::atoi(value);
	}

	return  std::atoi(cache_[inType][inAttribute].c_str());

	END_LOCK
}