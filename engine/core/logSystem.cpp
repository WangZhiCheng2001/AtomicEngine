#include "logSystem.hpp"

LogSystem::LogSystem()
{
    s_consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    s_consoleSink->set_level(spdlog::level::debug);
    s_consoleSink->set_pattern("%^[%T] %n: %v (from %!, %@)%$");

    s_fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/log.txt", 0, 0);
    s_fileSink->set_level(spdlog::level::trace);
    s_fileSink->set_pattern("[%T][%l] %n: %v (from %!, %@)");

    s_ringbufferSink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(128);
    s_ringbufferSink->set_level(spdlog::level::debug);
    s_ringbufferSink->set_pattern("%^[%T] %n: %v (from %!, %@)%$");

#ifdef NDEBUG
    auto sinkList = spdlog::sinks_init_list{s_consoleSink, s_fileSink, s_ringbufferSink};
#else
    auto sinkList = spdlog::sinks_init_list{s_fileSink, s_ringbufferSink};
#endif

    s_engineLogger = std::make_shared<spdlog::logger>("Engine", sinkList);
    s_appLogger = std::make_shared<spdlog::logger>("Application", sinkList);
}

LogSystem::~LogSystem()
{
    spdlog::drop_all();
}