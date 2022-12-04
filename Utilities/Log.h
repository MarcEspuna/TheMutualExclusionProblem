#pragma once
#include <iostream>
#include <memory>
#include "spdlog.h"
class Log {
public:
    /* Static impl */
    static void CreateLogger(const std::string& name);
    static std::shared_ptr<spdlog::logger>& GetLogger();
    static void EndLogging();
private:
    static std::shared_ptr<spdlog::logger> m_Logger;
    Log() = default;

};

//#define ACTIVE_LOGGING  
#ifdef ACTIVE_LOGGING
    #define LOG_TRACE(...)              Log::GetLogger()->trace(__VA_ARGS__)
    #define LOG_INFO(...)               Log::GetLogger()->info(__VA_ARGS__)
    #define LOG_WARN(...)               Log::GetLogger()->warn(__VA_ARGS__)
    #define LOG_ERROR(...)              Log::GetLogger()->error(__VA_ARGS__)
    #define LOG_CRITICAL(...)           Log::GetLogger()->critical(__VA_ARGS__)
    #define LOG_ASSERT(check, ...)      { if (!(check)) { Log::GetLogger()->error(__VA_ARGS__); assert(false); }}
#else
    #define LOG_TRACE(...)      
    #define LOG_INFO(...)       
    #define LOG_WARN(...)       
    #define LOG_ERROR(...)      
    #define LOG_CRITICAL(...)   
    #define LOG_ASSERT(check, ...)      assert(check);
#endif