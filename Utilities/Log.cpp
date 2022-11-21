#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Log::m_Logger;

std::shared_ptr<spdlog::logger>& Log::GetLogger()
{
    return m_Logger;
}

void Log::CreateLogger(const std::string& name)
{
    spdlog::set_pattern("%^[%T] %n: %v%$");
    m_Logger = spdlog::stdout_color_mt(name);
    m_Logger->set_level(spdlog::level::trace);
}

void Log::EndLogging()
{
    spdlog::shutdown();
}