/*
** Specific object dealing with timer functionalities.
*/

#pragma once

#ifdef __linux__
#include <pthread.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

namespace Asm {
class TimerObject {
    friend class TimerObjectReactor; // This enables the timer reactor to access the internals

  private:
    int _fd;
    long _interval;
    long _next;

  public:
    TimerObject() : _fd(-1), _interval(0), _next(0) {
#ifdef __linux__
        if ((_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC)) < 0) {
            std::cout << "Timer could not be created: " << std::strerror(errno) << std::endl;
        }
#endif
    }

    // Non-copyable
    TimerObject(const TimerObject&) = delete;
    TimerObject &operator=(const TimerObject&) = delete;

    // Non-movable
    TimerObject(TimerObject&&) = delete;
    TimerObject &operator=(TimerObject&&) = delete;

    ~TimerObject() {
#ifdef __linux__
        stop();
        ::close(_fd);
#endif
    }

    bool setRelativeInterval(long interval, long next = 0) {
#ifdef __linux__
        itimerspec val;

        if (interval == 0 && next == 0) {
            std::cout << "Interval and next start time must be different from zero" << std::endl;
            return false;
        }

        _interval = interval;
        _next = next ? next : interval;

        val.it_interval.tv_sec = _interval / 1000;
        val.it_interval.tv_nsec = (_interval % 1000) * 1000000;
        val.it_value.tv_sec = _next / 1000;
        val.it_value.tv_nsec = (_next % 1000) * 1000000;


        if (::timerfd_settime(_fd, 0, &val, 0) < 0) {
            std::cout << "Timer could not set: " << std::strerror(errno) << std::endl;
            return false;
        }
#endif
        return true;
    }

    bool stop() {
#ifdef __linux__
        itimerspec val {};

        if (::timerfd_settime(_fd, 0, &val, 0) < 0) {
            std::cout << "Timer could not stopped: " << std::strerror(errno) << std::endl;
            return false;
        }
#endif
        return true;
    }

    long getInterval() {
        return _interval;
    }

    bool restart() {
        return setRelativeInterval(_interval, _next);
    }

    bool wait(uint64_t& elapsed) {
#ifdef __linux__
        if (::read(_fd, &elapsed, sizeof(uint64_t)) != sizeof(uint64_t)) {
            std::cout << "Timer could not read: " << std::strerror(errno) << std::endl;
            return false;
        }
#endif
        return true;
    }

};

}
