/*
** Declaration of Logger
** Should be treaded as singleton
*/

#pragma once

#include <memory>

// Adjust settings for spdlog, ref. spdlog/tweakme.h
#define SPDLOG_FINAL final
#define SPDLOG_PREVENT_CHILD_FD
#define SPDLOG_ENABLE_SYSLOG
#include <spdlog/spdlog.h>

namespace Logger {

class Logger {
    std::shared_ptr<spdlog::logger> _console = nullptr;
    std::shared_ptr<spdlog::logger> _syslog = nullptr;

  public:
    Logger(const std::string& name) {
#ifdef SPDLOG_TRACE_ON

        // Create console logger
        _console = spdlog::stdout_color_mt(name + ":stdout");
        _console->set_level(spdlog::level::trace);
        _console->set_pattern("[%b %d %H:%M:%S][%n] %v");
        _console->trace(">>>Tracing on stdout is enabled<<<");

#endif

#ifdef __linux__

        // Create Linux syslog logger
        _syslog = spdlog::syslog_logger("syslog", name);
        info(">>>Syslog is enabled<<<");
#ifdef SPDLOG_DEBUG_ON
        _syslog->set_level(spdlog::level::debug);
#endif

#else // Windows

#endif
    }

    // Necessary if someone want to inherit from that
    virtual ~Logger() = default;

    // Non-copyable
    Logger(const Logger&) = delete;
    Logger &operator=(const Logger&) = delete;

    // Non-movable
    Logger(Logger&&) = delete;
    Logger &operator=(Logger&&) = delete;

    template <typename... Args>
    void trace(const std::string& fmt, const Args&... args) {
#ifdef SPDLOG_TRACE_ON
        _console->trace(fmt.data(), args...);
#endif
    }

    template <typename... Args>
    void debug(const std::string& fmt, const Args&... args) {
#ifdef SPDLOG_DEBUG_ON
        _syslog->debug(fmt.data(), args...);
#endif
    }

    template <typename... Args>
    void info(const std::string& fmt, const Args&... args) {
        _syslog->info(fmt.data(), args...);

        // If SPDLOG_TRACE_ON output info at console, too
        trace(fmt.data(), args...);
    }

    template <typename... Args>
    void warn(const std::string& fmt, const Args&... args) {
        _syslog->warn(fmt.data(), args...);
    }

    template <typename... Args>
    void error(const std::string& fmt, const Args&... args) {
        _syslog->error(fmt.data(), args...);
    }

    template <typename... Args>
    void critical(const std::string& fmt, const Args&... args) {
        _syslog->critical(fmt.data(), args...);
    }
};

extern std::unique_ptr<Logger> pLOG;

}
