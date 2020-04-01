#include "pch.h"
#include "LogManager.h"
#include "ConfigManager.h"
using namespace DokeviNet;

void LogManager::Initialize()
{
	try
	{
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_level(spdlog::level::trace);

		if (SINGLETON(ConfigManager)->GetInt("Log", "EnableDailyLog"))
		{
			auto dailySink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
				SINGLETON(ConfigManager)->GetStr("Log", "DailyLogFile"),
				SINGLETON(ConfigManager)->GetInt("Log", "DailyLogHour"),
				SINGLETON(ConfigManager)->GetInt("Log", "DailyLogMin")
				);

			dailySink->set_level(spdlog::level::info);
			spdlog::sinks_init_list SinkList = { consoleSink, dailySink };

			_logger = std::make_shared<spdlog::logger>("DokeviNetLogger", SinkList);
		}
		else
		{
			_logger = std::make_shared<spdlog::logger>("DokeviNetLogger", consoleSink);
		}

		if (!SINGLETON(ConfigManager)->GetInt("Log", "UseLog"))
		{
			_useSpdlog = true;
		}

		_logger->set_level(spdlog::level::trace);

		_logger->flush_on(spdlog::level::trace);
	}
	catch (...)
	{
		cout << "Failed Init LogManager : Please Check Log Folder" << endl;
	}

	LOG_INFO("Init - LogManager");

	_isInit = true;
}

void LogManager::Finalize()
{
	LOG_INFO("LogManager Init Release!");
}

Logger LogManager::GetLogger()
{
	return _logger;
}