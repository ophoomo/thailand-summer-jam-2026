
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "utils/logger.h"

// Define static members
std::shared_ptr<spdlog::logger> Logger::s_coreLogger;
std::shared_ptr<spdlog::logger> Logger::s_clientLogger;

// ============================================================
// Public Methods
// ============================================================

void Logger::init()
{
    constexpr size_t queueSize = 8192;
    constexpr size_t threadCount = 1;
    spdlog::init_thread_pool(queueSize, threadCount);

    spdlog::set_pattern("%^[%Y-%m-%d %T] [%n] [%l] %v%$");

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);

    auto fileSink =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/game.log",
                                                               1024 * 1024 * 10, // 10 MB
                                                               5                 // keep 5 files
        );
    fileSink->set_level(spdlog::level::trace);
    std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};

    Logger::s_coreLogger = std::make_shared<spdlog::async_logger>(
        "ENGINE", sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    Logger::s_clientLogger = std::make_shared<spdlog::async_logger>(
        "APP", sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

#ifdef NDEBUG
    Logger::s_coreLogger->set_level(spdlog::level::info);
    Logger::s_clientLogger->set_level(spdlog::level::info);
#else
    Logger::s_coreLogger->set_level(spdlog::level::trace);
    Logger::s_clientLogger->set_level(spdlog::level::trace);
#endif

    Logger::s_coreLogger->flush_on(spdlog::level::err);
    Logger::s_clientLogger->flush_on(spdlog::level::err);

    spdlog::register_logger(Logger::s_coreLogger);
    spdlog::register_logger(Logger::s_clientLogger);
}

void Logger::shutdown()
{
    spdlog::shutdown();
}

std::shared_ptr<spdlog::logger> &Logger::core()
{
    assert(Logger::s_coreLogger && "Logger not initialized!");
    return Logger::s_coreLogger;
}

std::shared_ptr<spdlog::logger> &Logger::client()
{
    assert(Logger::s_clientLogger && "Logger not initialized!");
    return Logger::s_clientLogger;
}
