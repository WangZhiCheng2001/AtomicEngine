#include <source_location>

#include <atomCore/log/log.h>
#include <atomCore/log/manager.hpp>

ATOM_EXTERN_C ATOM_API void atom_log_set_level(eLogLevel level)
{
    spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
}

ATOM_EXTERN_C ATOM_API void atom_log_flush()
{
    spdlog::apply_all([](std::shared_ptr<Logger> logger) -> void { logger->flush(); });
}

ATOM_EXTERN_C ATOM_API void atom_log_log(int level, ...)
{
    if (level < static_cast<int>(spdlog::get_level()) || level >= static_cast<int>(SPDLOG_LEVEL_OFF)) return;

    va_list va_args;
    va_start(va_args, level);
    spdlog::log(spdlog::source_loc{std::source_location::current().file_name(),
                                   static_cast<int>(std::source_location::current().line()),
                                   std::source_location::current().function_name()},
                static_cast<spdlog::level::level_enum>(level),
                va_args);
    va_end(va_args);
}
