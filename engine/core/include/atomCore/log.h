#pragma once

#include "log/log.h"

#ifdef __cplusplus
#include "log/manager.hpp"
#endif

// #define ATOM_backtrace(...) atom_log_backtrace(__VA_ARGS__)
#define ATOM_trace(...) atom_log_log(ATOM_LOG_LEVEL_TRACE, __VA_ARGS__)
#define ATOM_debug(...) atom_log_log(ATOM_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define ATOM_info(...)  atom_log_log(ATOM_LOG_LEVEL_INFO, __VA_ARGS__)
#define ATOM_warn(...)  atom_log_log(ATOM_LOG_LEVEL_WARN, __VA_ARGS__)
#define ATOM_error(...) atom_log_log(ATOM_LOG_LEVEL_ERROR, __VA_ARGS__)
#define ATOM_fatal(...) atom_log_log(ATOM_LOG_LEVEL_FATAL, __VA_ARGS__)