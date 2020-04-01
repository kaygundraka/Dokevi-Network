#pragma once
#include "pch.h"
#include "ISingleton.h"
#include <spdlog\spdlog.h>
#include <spdlog\sinks\stdout_color_sinks.h>
#include <spdlog\sinks\daily_file_sink.h>

namespace DokeviNet
{
	using Logger = std::shared_ptr<spdlog::logger>;

	class LogManager : public ISingleton<LogManager>
	{
	private:
		Logger _logger;
		
	public:
		bool _isInit = false;
		bool _useSpdlog = false;

		virtual void Initialize() override;
		virtual void Finalize() override;

		Logger GetLogger();
	};

	template <typename ... Types> void LOG_TRACE(Types ... args) 
	{
		if (!LogManager::GetInstance()->_isInit)
			return;

		if (!LogManager::GetInstance()->_useSpdlog)
			return;

		LogManager::GetInstance()->GetLogger()->trace(args...);
	}

	template <typename ... Types> void LOG_DEBUG(Types ... args)
	{
		if (!LogManager::GetInstance()->_isInit)
			return;

		if (!LogManager::GetInstance()->_useSpdlog)
			return;

		LogManager::GetInstance()->GetLogger()->debug(args...);
	}

	template <typename ... Types> void LOG_INFO(Types ... args)
	{
		if (!LogManager::GetInstance()->_isInit)
			return;

		if (!LogManager::GetInstance()->_useSpdlog)
			return;

		LogManager::GetInstance()->GetLogger()->info(args...);
	}

	template <typename ... Types> void LOG_WARN(Types ... args)
	{
		if (!LogManager::GetInstance()->_isInit)
			return;

		if (!LogManager::GetInstance()->_useSpdlog)
			return;

		LogManager::GetInstance()->GetLogger()->warn(args...);
	}

	template <typename ... Types> void LOG_ERROR(Types ... args)
	{
		if (!LogManager::GetInstance()->_isInit)
			return;

		if (!LogManager::GetInstance()->_useSpdlog)
			return;

		LogManager::GetInstance()->GetLogger()->error(args...);
		_ASSERT(0);
	}

	template <typename ... Types> void LOG_CRITICAL(Types ... args)
	{
		if (!LogManager::GetInstance()->_isInit)
			return;

		if (!LogManager::GetInstance()->_useSpdlog)
			return;

		LogManager::GetInstance()->GetLogger()->critical(args...);
		_ASSERT(0);

		int crash = 0;
		int temp = 10 / crash;
	}
}