#pragma once

#include <memory>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>

class LogSystem
{
public:
    LogSystem(const LogSystem &) = delete;
    LogSystem(LogSystem &&) = delete;
    LogSystem &operator=(const LogSystem &) = delete;
    LogSystem &operator=(LogSystem &&) = delete;

    LogSystem();
    ~LogSystem();

    static std::shared_ptr<LogSystem> getInstance()
    {
        static std::shared_ptr<LogSystem> instanceHandle{std::make_shared<LogSystem>()};
        return instanceHandle;
    }

    static inline auto getEngineLogger() { return s_engineLogger; }
    static inline auto getAppLogger() { return s_appLogger; }

    static inline auto getLatestLogs() { return s_ringbufferSink->last_formatted(); }

    static inline void setLogLevel(spdlog::level::level_enum level) { s_consoleSink->set_level(level); }

protected:
    static std::shared_ptr<spdlog::logger> s_engineLogger;
    static std::shared_ptr<spdlog::logger> s_appLogger;

    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> s_consoleSink;
    static std::shared_ptr<spdlog::sinks::daily_file_sink_mt> s_fileSink;
    static std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> s_ringbufferSink;
};