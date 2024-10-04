#pragma once

#include <atomCore/config.h>

enum eLogLevel {
    ATOM_LOG_LEVEL_TRACE,
    ATOM_LOG_LEVEL_DEBUG,
    ATOM_LOG_LEVEL_INFO,
    ATOM_LOG_LEVEL_WARN,
    ATOM_LOG_LEVEL_ERROR,
    ATOM_LOG_LEVEL_FATAL
};

ATOM_EXTERN_C ATOM_API void atom_log_set_level(enum eLogLevel level);
ATOM_EXTERN_C ATOM_API void atom_log_flush();
ATOM_EXTERN_C ATOM_API void atom_log_log(int level, ...);