#pragma once

#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>

#include <atomCore/config.h>

using Logger      = spdlog::logger;
using AsyncLogger = spdlog::async_logger;

// basically a wrapper of spdlog::registry
class ATOM_API LogManager
{
public:
    LogManager() ATOM_NOEXCEPT;
    ~LogManager() ATOM_NOEXCEPT;

    static LogManager&      instance() ATOM_NOEXCEPT;
    std::shared_ptr<Logger> get_default_logger() ATOM_NOEXCEPT;

    void initialize() ATOM_NOEXCEPT;

    void                    register_logger(std::shared_ptr<Logger> logger);
    std::shared_ptr<Logger> find_logger(std::string&& name);
};