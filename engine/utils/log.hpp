#pragma once

#include <logSystem.hpp>

#define ENGINE_LOG(level, ...) SPDLOG_LOGGER_##level##(::LogSystem::getInstance()->getEngineLogger(), __VA_ARGS__)
#define ENGINE_LOG_TRACE(...) ENGINE_LOG(TRACE, __VA_ARGS__)
#define ENGINE_LOG_DEBUG(...) ENGINE_LOG(DEBUG, __VA_ARGS__)
#define ENGINE_LOG_INFO(...) ENGINE_LOG(INFO, __VA_ARGS__)
#define ENGINE_LOG_WARN(...) ENGINE_LOG(WARN, __VA_ARGS__)
#define ENGINE_LOG_ERROR(...) ENGINE_LOG(ERROR, __VA_ARGS__)
#define ENGINE_LOG_CRITICAL(...)           \
    {                                      \
        ENGINE_LOG(CRITICAL, __VA_ARGS__); \
        throw std::runtime_error("");      \
    }

#define APP_LOG(level, ...) SPDLOG_LOGGER_##level##(::LogSystem::getInstance()->getAppLogger(), __VA_ARGS__)
#define APP_LOG_TRACE(...) APP_LOG(TRACE, __VA_ARGS__)
#define APP_LOG_DEBUG(...) APP_LOG(DEBUG, __VA_ARGS__)
#define APP_LOG_INFO(...) APP_LOG(INFO, __VA_ARGS__)
#define APP_LOG_WARN(...) APP_LOG(WARN, __VA_ARGS__)
#define APP_LOG_ERROR(...) APP_LOG(ERROR, __VA_ARGS__)
#define APP_LOG_CRITICAL(...) APP_LOG(CRITICAL, __VA_ARGS__)