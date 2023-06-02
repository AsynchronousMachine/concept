/*
** Declaration of Logger
** Should be treaded as singleton
*/

#pragma once

#include <syslog.h>

#include <string_view>

#define FMT_HEADER_ONLY

#include <fmt/format.h>

namespace Logger {

class Logger {

  private:
	  const std::string _name;
	  
	  void log(const int prio, const std::string& s) {
	      ::syslog(prio, "%s", s.data());
	  }

  public:
    explicit Logger(const std::string& name) : _name(name) {
        // Due to the early stage initialization, refer __attribute__((init_priority(1000))), std::cout is not valid yet
#ifdef LOG_TRACE_ON        
        ::openlog(_name.data(), LOG_PERROR | LOG_NDELAY, LOG_USER);
#else
        ::openlog(_name.data(),          0 | LOG_NDELAY, LOG_USER);
#endif
        log(LOG_NOTICE, "+++Syslog is enabled+++");
    }

    ~Logger() {
        log(LOG_NOTICE, "---Logger destructed---");
        // ::closelog(); Closing is defered to proccess's end
    }

    // Non-copyable
    Logger(const Logger&) = delete;
    Logger &operator=(const Logger&) = delete;

    // Non-movable
    Logger(Logger&&) = delete;
    Logger &operator=(Logger&&) = delete;

    template <typename... Args>
#ifdef LOG_TRACE_ON
    void trace(fmt::string_view fmt, const Args&... args) {
        log(LOG_DEBUG, fmt::vformat(fmt, fmt::make_format_args(args...)));
#else
    void trace(fmt::string_view, const Args&... args) {
#endif
    }

    template <typename... Args>
#ifdef LOG_DEBUG_ON
    void debug(fmt::string_view fmt, const Args&... args) {
        log(LOG_DEBUG, fmt::vformat(fmt, fmt::make_format_args(args...)));
#else
    void debug(fmt::string_view fmt, const Args&... args) {
#endif
    }
    
    template <typename... Args>
    void info(const std::string& fmt, const Args&... args) {
        log(LOG_INFO, fmt::vformat(fmt, fmt::make_format_args(args...)));
    }

    template <typename... Args>
    void warn(const std::string& fmt, const Args&... args) {
        log(LOG_WARNING, fmt::vformat(fmt, fmt::make_format_args(args...)));
    }

    template <typename... Args>
    void error(const std::string& fmt, const Args&... args) {
        log(LOG_ERR, fmt::vformat(fmt, fmt::make_format_args(args...)));
    }

    template <typename... Args>
    void critical(const std::string& fmt, const Args&... args) {
        log(LOG_CRIT, fmt::vformat(fmt, fmt::make_format_args(args...)));
    }
};

extern std::unique_ptr<Logger> pLOG;

}
