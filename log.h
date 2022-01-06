#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/fmt/bundled/printf.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "singleton.h"

enum {
    LOG_CRITICAL = 100,
    LOG_ERROR = 200,
    LOG_WARNING = 300,
    LOG_INFO = 400,
    LOG_DEBUG = 500,
    LOG_TRACE = 600
};

enum BLogLevel
{
    BLOG_LEVEL_TRACE = 0,
    BLOG_LEVEL_DEBUG,
    BLOG_LEVEL_INFO,
    BLOG_LEVEL_WARN,
    BLOG_LEVEL_ERROR,
    BLOG_LEVEL_CRITICAL,
    BLOG_LEVEL_OFF
};

enum BLogType {
    BLOG_CONSOLE,   // output console only
    BLOG_FILE,      // output file only
    BLOG_ALL        // output file and console
};

class LogSeivice
{
    SINGLETON(LogSeivice)
public:
    void InitLogService(const std::string& fileName, BLogType type = BLOG_CONSOLE,
        int logLevel = spdlog::level::trace);

    void Close();

    template <typename... Args>
    void Log(const char* filename_in, int line_in, const char* funcname_in, spdlog::level::level_enum lvl,
        const char* fmt, const Args &... args) {
        spdlog::source_loc loc = spdlog::source_loc(filename_in, line_in, funcname_in);

        if (m_type_ == BLOG_ALL) {
            m_logger->log(loc, lvl, fmt::sprintf(fmt, args...).c_str());
            m_logger_->log(loc, lvl, fmt::sprintf(fmt, args...).c_str());
        }
        else {
            m_logger->log(loc, lvl, fmt::sprintf(fmt, args...).c_str());
        }
    }

    template <typename... Args>
    void Printf(const spdlog::source_loc& loc, spdlog::level::level_enum lvl,
        const char* fmt, const Args &... args);

    auto GetLogger() const;

private:
    BLogType m_type_;
    std::shared_ptr<spdlog::logger> m_logger;
    std::shared_ptr<spdlog::logger> m_logger_;
};

#define LogServiceInstance Singleton<LogSeivice>::Instance()

#define SPDLOG_BASE(logger, level, filename, line, func, ...) (logger).Log(filename, line, func, level, __VA_ARGS__)

#define LOG_TRACE(filename, line, func, ...)         SPDLOG_BASE(LogServiceInstance, spdlog::level::trace, filename, line, func, __VA_ARGS__)
#define LOG_DEBUG(filename, line, func, ...)         SPDLOG_BASE(LogServiceInstance, spdlog::level::debug, filename, line, func, __VA_ARGS__)
#define LOG_INFO(filename, line, func, ...)          SPDLOG_BASE(LogServiceInstance, spdlog::level::info, filename, line, func, __VA_ARGS__)
#define LOG_WARN(filename, line, func, ...)          SPDLOG_BASE(LogServiceInstance, spdlog::level::warn,  filename, line, func, __VA_ARGS__)
#define LOG_ERROR(filename, line, func, ...)         SPDLOG_BASE(LogServiceInstance, spdlog::level::err,  filename, line, func, __VA_ARGS__)
#define LOG_CRITICAL(filename, line, func, ...)      SPDLOG_BASE(LogServiceInstance, spdlog::level::critical,  filename, line, func, __VA_ARGS__)

#define Log(logLevel, ...)  \
    do {    \
        switch (logLevel) { \
        case LOG_INFO:  \
            LOG_INFO(__FILE__, __LINE__, __func__, __VA_ARGS__); break; \
        case LOG_DEBUG:  \
            SPDLOG_BASE(LogServiceInstance, spdlog::level::debug, __FILE__, __LINE__, __func__, __VA_ARGS__); break; \
        case LOG_TRACE:  \
            SPDLOG_BASE(LogServiceInstance, spdlog::level::trace, __FILE__, __LINE__, __func__, __VA_ARGS__); break; \
        case LOG_ERROR:  \
            SPDLOG_BASE(LogServiceInstance, spdlog::level::err, __FILE__, __LINE__, __func__, __VA_ARGS__); break; \
        case LOG_WARNING:  \
            SPDLOG_BASE(LogServiceInstance, spdlog::level::warn, __FILE__, __LINE__, __func__, __VA_ARGS__); break; \
        case LOG_CRITICAL:  \
            SPDLOG_BASE(LogServiceInstance, spdlog::level::critical, __FILE__, __LINE__, __func__, __VA_ARGS__); break; \
        }   \
    } while (false)


void InitSpdLog(const char* fileName, enum BLogType type, int logLevel);
void DeinitSpdLog();