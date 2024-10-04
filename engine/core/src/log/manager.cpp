#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

#include <atomCore/log/manager.hpp>

enum class eConsoleColor : uint8_t { black, red, green, yellow, blue, magenta, cyan, white };

enum class eConsoleStyle : uint8_t { normal, highlight };

struct ConsoleColorWindows {
    template <eConsoleColor>
    struct FrontColor {
        [[maybe_unused]] static constexpr uint16_t value = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    };

    template <>
    struct FrontColor<eConsoleColor::black> {
        [[maybe_unused]] static constexpr uint16_t value = 0x0000;
    };

    template <>
    struct FrontColor<eConsoleColor::red> {
        [[maybe_unused]] static constexpr uint16_t value = FOREGROUND_RED;
    };

    template <>
    struct FrontColor<eConsoleColor::green> {
        [[maybe_unused]] static constexpr uint16_t value = FOREGROUND_GREEN;
    };

    template <>
    struct FrontColor<eConsoleColor::yellow> {
        [[maybe_unused]] static constexpr uint16_t value = FOREGROUND_RED | FOREGROUND_GREEN;
    };

    template <>
    struct FrontColor<eConsoleColor::blue> {
        [[maybe_unused]] static constexpr uint16_t value = FOREGROUND_BLUE;
    };

    template <>
    struct FrontColor<eConsoleColor::magenta> {
        [[maybe_unused]] static constexpr uint16_t value = FOREGROUND_RED | FOREGROUND_BLUE;
    };

    template <>
    struct FrontColor<eConsoleColor::cyan> {
        [[maybe_unused]] static constexpr uint16_t value = FOREGROUND_GREEN | FOREGROUND_BLUE;
    };

    template <eConsoleColor>
    struct BackColor {
        [[maybe_unused]] static constexpr uint16_t value = 0x0000;
    };

    template <>
    struct BackColor<eConsoleColor::black> {
        [[maybe_unused]] static constexpr uint16_t value = 0x0000;
    };

    template <>
    struct BackColor<eConsoleColor::red> {
        [[maybe_unused]] static constexpr uint16_t value = BACKGROUND_RED;
    };

    template <>
    struct BackColor<eConsoleColor::green> {
        [[maybe_unused]] static constexpr uint16_t value = BACKGROUND_GREEN;
    };

    template <>
    struct BackColor<eConsoleColor::yellow> {
        [[maybe_unused]] static constexpr uint16_t value = BACKGROUND_RED | BACKGROUND_GREEN;
    };

    template <>
    struct BackColor<eConsoleColor::blue> {
        [[maybe_unused]] static constexpr uint16_t value = BACKGROUND_BLUE;
    };

    template <>
    struct BackColor<eConsoleColor::magenta> {
        [[maybe_unused]] static constexpr uint16_t value = BACKGROUND_RED | BACKGROUND_BLUE;
    };

    template <>
    struct BackColor<eConsoleColor::cyan> {
        [[maybe_unused]] static constexpr uint16_t value = BACKGROUND_GREEN | BACKGROUND_BLUE;
    };

    template <>
    struct BackColor<eConsoleColor::white> {
        [[maybe_unused]] static constexpr uint16_t value = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
    };

    template <eConsoleStyle>
    struct Style {
        [[maybe_unused]] static constexpr uint16_t value = 0x0000;
    };

    template <>
    struct Style<eConsoleStyle::highlight> {
        [[maybe_unused]] static constexpr uint16_t value = FOREGROUND_INTENSITY;
    };
};

struct ConsoleColorANSI {
    template <eConsoleColor>
    struct FrontColor {
        [[maybe_unused]] static constexpr std::string value = "\033[37m";
    };

    template <>
    struct FrontColor<eConsoleColor::black> {
        [[maybe_unused]] static constexpr std::string value = "\033[30m";
    };

    template <>
    struct FrontColor<eConsoleColor::red> {
        [[maybe_unused]] static constexpr std::string value = "\033[31m";
    };

    template <>
    struct FrontColor<eConsoleColor::green> {
        [[maybe_unused]] static constexpr std::string value = "\033[32m";
    };

    template <>
    struct FrontColor<eConsoleColor::yellow> {
        [[maybe_unused]] static constexpr std::string value = "\033[33m";
    };

    template <>
    struct FrontColor<eConsoleColor::blue> {
        [[maybe_unused]] static constexpr std::string value = "\033[34m";
    };

    template <>
    struct FrontColor<eConsoleColor::magenta> {
        [[maybe_unused]] static constexpr std::string value = "\033[35m";
    };

    template <>
    struct FrontColor<eConsoleColor::cyan> {
        [[maybe_unused]] static constexpr std::string value = "\033[36m";
    };

    template <eConsoleColor>
    struct BackColor {
        [[maybe_unused]] static constexpr std::string value = "";
    };

    template <>
    struct BackColor<eConsoleColor::black> {
        [[maybe_unused]] static constexpr std::string value = "";
    };

    template <>
    struct BackColor<eConsoleColor::red> {
        [[maybe_unused]] static constexpr std::string value = "\033[41m";
    };

    template <>
    struct BackColor<eConsoleColor::green> {
        [[maybe_unused]] static constexpr std::string value = "\033[42m";
    };

    template <>
    struct BackColor<eConsoleColor::yellow> {
        [[maybe_unused]] static constexpr std::string value = "\033[43m";
    };

    template <>
    struct BackColor<eConsoleColor::blue> {
        [[maybe_unused]] static constexpr std::string value = "\033[44m";
    };

    template <>
    struct BackColor<eConsoleColor::magenta> {
        [[maybe_unused]] static constexpr std::string value = "\033[45m";
    };

    template <>
    struct BackColor<eConsoleColor::cyan> {
        [[maybe_unused]] static constexpr std::string value = "\033[46m";
    };

    template <>
    struct BackColor<eConsoleColor::white> {
        [[maybe_unused]] static constexpr std::string value = "\033[47m";
    };

    template <eConsoleStyle>
    struct Style {
        [[maybe_unused]] static constexpr std::string value = "";
    };

    template <>
    struct Style<eConsoleStyle::highlight> {
        [[maybe_unused]] static constexpr std::string value = "\033[1m";
    };
};

LogManager::LogManager() ATOM_NOEXCEPT {}

LogManager::~LogManager() ATOM_NOEXCEPT
{
    spdlog::drop_all();
    spdlog::shutdown();
}

void LogManager::initialize() ATOM_NOEXCEPT
{
    spdlog::init_thread_pool(8192, 4);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("%^[%Y-%m-%d %T][pid:%P tid:%t] %n.%l: %v%$\n [In %! At %@]");
#ifdef ATOM_PLAT_WINDOWS
    console_sink->set_color(spdlog::level::trace, ConsoleColorWindows::FrontColor<eConsoleColor::white>::value);
    console_sink->set_color(spdlog::level::debug, ConsoleColorWindows::FrontColor<eConsoleColor::cyan>::value);
    console_sink->set_color(spdlog::level::info, ConsoleColorWindows::FrontColor<eConsoleColor::green>::value);
    console_sink->set_color(spdlog::level::warn,
                            ConsoleColorWindows::FrontColor<eConsoleColor::yellow>::value
                                | ConsoleColorWindows::Style<eConsoleStyle::highlight>::value);
    console_sink->set_color(spdlog::level::err,
                            ConsoleColorWindows::FrontColor<eConsoleColor::red>::value
                                | ConsoleColorWindows::Style<eConsoleStyle::highlight>::value);
    console_sink->set_color(spdlog::level::critical,
                            ConsoleColorWindows::FrontColor<eConsoleColor::white>::value
                                | ConsoleColorWindows::BackColor<eConsoleColor::red>::value
                                | ConsoleColorWindows::Style<eConsoleStyle::highlight>::value);
#else
    console_sink->set_color(spdlog::level::trace, ConsoleColorANSI::FrontColor<eConsoleColor::white>::value);
    console_sink->set_color(spdlog::level::debug, ConsoleColorANSI::FrontColor<eConsoleColor::cyan>::value);
    console_sink->set_color(spdlog::level::info, ConsoleColorANSI::FrontColor<eConsoleColor::green>::value);
    console_sink->set_color(
        spdlog::level::warn,
        ConsoleColorANSI::FrontColor<eConsoleColor::yellow>::value + ConsoleColorANSI::Style<eConsoleStyle::highlight>::value);
    console_sink->set_color(
        spdlog::level::err,
        ConsoleColorANSI::FrontColor<eConsoleColor::red>::value + ConsoleColorANSI::Style<eConsoleStyle::highlight>::value);
    console_sink->set_color(spdlog::level::critical,
                            ConsoleColorANSI::FrontColor<eConsoleColor::white>::value
                                + ConsoleColorANSI::BackColor<eConsoleColor::red>::value
                                + ConsoleColorANSI::Style<eConsoleStyle::highlight>::value);
#endif

    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("log.txt", 0, 0);
    file_sink->set_pattern("[%Y-%m-%d %T][tid:%t] %n.%l: %v\n [In %! At %@]");

    spdlog::set_default_logger(std::make_shared<AsyncLogger>("engine_logger",
                                                             spdlog::sinks_init_list{console_sink, file_sink},
                                                             spdlog::thread_pool()));
}

LogManager& LogManager::instance() ATOM_NOEXCEPT
{
    static std::unique_ptr<LogManager> obj = std::make_unique<LogManager>();
    static bool                        initialized_instance{false};
    if (!initialized_instance) {
        initialized_instance = true;
        obj->initialize();
    }
    return *obj.get();
}

std::shared_ptr<Logger> LogManager::get_default_logger() ATOM_NOEXCEPT { return spdlog::default_logger(); }

void LogManager::register_logger(std::shared_ptr<Logger> logger) { spdlog::register_logger(logger); }

std::shared_ptr<Logger> LogManager::find_logger(std::string&& name) { return spdlog::get(name); }