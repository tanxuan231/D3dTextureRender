#include "log.h"

LogSeivice::LogSeivice() = default;
LogSeivice::~LogSeivice()
{
}

auto LogSeivice::GetLogger() const
{
    return m_logger;
}

void LogSeivice::InitLogService(const std::string & fileName, BLogType type, int logLevel)
{
    switch ((int)type)
    {
    case BLOG_FILE:
        m_logger = spdlog::rotating_logger_mt<spdlog::async_factory>("rotating_logger", fileName, 1024 * 1024 * 5, 3);
        break;
    case BLOG_ALL:
        //m_logger = spdlog::daily_logger_mt<spdlog::async_factory>("logger", fileName, 2, 30);
        m_logger = spdlog::rotating_logger_mt<spdlog::async_factory>("rotating_logger", fileName, 1024 * 1024 * 5, 3);
        m_logger_ = spdlog::stdout_color_mt<spdlog::async_factory>("console");
        m_logger_->set_level(static_cast<spdlog::level::level_enum>(logLevel));
        m_logger_->flush_on(spdlog::level::err);
        break;
    case BLOG_CONSOLE:
        m_logger = spdlog::stdout_color_mt<spdlog::async_factory>("console");
    default:
        break;
    }

    m_type_ = type;
    m_logger->set_level(static_cast<spdlog::level::level_enum>(logLevel));
    m_logger->flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(3));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%# %!] %v");
}

void LogSeivice::Close()
{
    spdlog::drop_all();
}

static bool IsSpdlogInited = false;

void InitSpdLog(const char* fileName, enum BLogType type, int logLevel)
{
    LogServiceInstance.InitLogService(fileName, type, logLevel);
    IsSpdlogInited = true;
}

void DeinitSpdLog()
{
    if (IsSpdlogInited) {
        LogServiceInstance.Close();
    }
}