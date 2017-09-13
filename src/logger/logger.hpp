/*
** Declaration of Logger
** Should be treaded as singleton
*/

#pragma once

#ifdef __linux__
#include <syslog.h>
#else // Windows
#include <stdio.h>
#endif

#include <memory>
#include <exception>

// Adjust settings for spdlog, refer to, spdlog/tweakme.h
#define SPDLOG_FINAL final
#define SPDLOG_PREVENT_CHILD_FD
#define SPDLOG_ENABLE_SYSLOG
#include <spdlog/spdlog.h>

namespace Logger {

class Logger {
    std::shared_ptr<spdlog::logger> _console;
    std::shared_ptr<spdlog::logger> _syslog;

  public:
    explicit Logger(const std::string& name) {
        try
        {
#ifdef SPDLOG_TRACE_ON
            // Create console logger
            _console = spdlog::stdout_color_mt(name);
            _console->set_pattern("[%Y-%m-%d %T][%n] %v");
            _console->set_level(spdlog::level::trace);
            _console->trace("+++Tracing on stdout is enabled+++");
#endif

#ifdef __linux__
            // Create Linux syslog logger
            _syslog = spdlog::syslog_logger("syslog", name);
#else // Windows
            // Create Windows syslog logger
            // _syslog = spdlog::rotating_logger_mt(name, "syslog", 1024 * 1024 * 5, 10);
            // _syslog->set_pattern("[%Y-%m-%d %T][%n] %v");
#endif

            info("+++Syslog is enabled+++");

#ifdef SPDLOG_DEBUG_ON
            _syslog->set_level(spdlog::level::debug);
#endif
        }
        catch (const std::exception& ex)
        {
            // Due to the early stage initialization, refer __attribute__((init_priority(1000))), std::cout is not valid yet
#ifdef __linux__
            ::openlog(name.data(), LOG_PERROR | LOG_NDELAY, LOG_USER);
            ::syslog(LOG_ALERT, "---Initialization of logger failed: %s---", ex.what());
            ::closelog();
#else // Windows
            //::puts("---Initialization of logger failed---");
#endif

            std::this_thread::sleep_for(std::chrono::seconds(3));
            throw; // Rethrows the exception object of type std::exception, without logger nothing makes sense
        }
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
