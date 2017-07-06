/*
** This is the TimerObjectReactor dealing with TimerObjects
** TimerObjects which have fired will be notified within the DataObjectReactor so the interface is
** only related to there. This approach avoids to build up its own thread pool
*/

#pragma once

#ifdef __linux__
#include <pthread.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

#include <unordered_map>
#include <thread>
#include <chrono>
#include <mutex>

namespace Asm {
// Forward declarations
class TimerObject;
class DataObjectReactor;

class TimerObjectReactor {
  private:
    static constexpr int RT_PRIO = 31; // Realtime priority, at least one higher than DOR
    static constexpr size_t MAX_CAPACITY = 256; // Max. epoll capacity, can be found at /proc/sys/fs/epoll/max_user_watches

    // File descriptor for epoll mechanism
    int _epfd;

    // File descriptor for epoll stop mechanism
    int _evtfd;

    // Reference to the event reactor
    DataObjectReactor &_dor;

    // Holds the timer thread reference
    std::thread _thrd;

    // Holds all epoll file descriptor associated data
    std::unordered_map<int, DataObject<TimerObject>&> _notify;

    // Protects the access to epoll file descriptor associated data
    std::mutex _mtx;

    // Threaded timer function mechanism
    void run() {
#ifdef __linux__
        std::cout << "TOR-THRD-0 has TID-" << syscall(SYS_gettid) << std::endl;

        for (;;) {
            epoll_event evt[MAX_CAPACITY] {};

            int evt_cnt = ::epoll_wait(_epfd, &evt[0], MAX_CAPACITY, -1);

            if (evt_cnt <= 0) {
                std::cout << "Epoll wait error: " << std::strerror(errno) << std::endl;
                continue;
            }

            for (int i = 0; i < evt_cnt; i++) {
                if (evt[i].events & EPOLLIN) {
                    uint64_t elapsed;

                    if (::read(evt[i].data.fd, &elapsed, sizeof(elapsed)) != sizeof(elapsed)) {
                        std::cout << "Read TOR event returns wrong size: " << std::strerror(errno) << std::endl;
                        continue;
                    }

                    if (evt[i].data.fd == _evtfd && elapsed > 0) {
                        std::cout << "Read TOR event returns stop command" << std::endl;
                        return;
                    }

                    std::cout << "TOR with TID " << syscall(SYS_gettid) << " has fired" << std::endl;

                    std::unique_lock<std::mutex> lock(_mtx);
                    auto itr = _notify.find(evt[i].data.fd);
                    if ( itr != _notify.end() ) {
                        lock.unlock();
                        _dor.trigger(itr->second);
                    }
                }
            }
        }

        std::cout << "TOR-THRD-0 has stopped" << std::endl;
#endif
    }

  public:
    TimerObjectReactor(DataObjectReactor& dor) : _epfd(-1), _evtfd(-1), _dor(dor) {
#ifdef __linux__
        if ((_evtfd = ::eventfd(0, EFD_CLOEXEC)) < 0) {
            std::cout << "Eventfd file handle could not be created in TOR: " << std::strerror(errno) << std::endl;
            return;
        }

        if ((_epfd = ::epoll_create1(EPOLL_CLOEXEC)) < 0) {
            std::cout << "Epoll file handle could not be created in TOR: " << std::strerror(errno) << std::endl;
            close(_evtfd);
            return;
        }

        // Add it first to stop epoll_wait in case of destruction
        epoll_event evt {};
        evt.events = EPOLLIN;
        evt.data.fd = _evtfd;

        if (::epoll_ctl(_epfd, EPOLL_CTL_ADD, _evtfd, &evt) < 0) {
            std::cout << "Epoll control error at ADD stop event in TOR: " << std::strerror(errno) << std::endl;
            close(_epfd);
            close(_evtfd);
            return;
        }

        _thrd = std::thread([this](){ TimerObjectReactor::run(); });

        //The thread name is a meaningful C language string, whose length is
        //restricted to 16 characters, including the terminating null byte ('\0')
        std::string s = "TOR-THRD-0";
        std::cout << "Created " << s << std::endl;

        if (pthread_setname_np(_thrd.native_handle(), s.data()))
            std::cout << "Could not set name for TOR-THRD-0" << std::endl;

        struct sched_param param {};
        param.sched_priority = RT_PRIO;

        if (pthread_setschedparam(_thrd.native_handle(), SCHED_FIFO, &param))
            std::cout << "Could not set realtime parameter for TOR-THRD-0" << std::endl;
#endif
    }

    // Non-copyable
    TimerObjectReactor(const TimerObjectReactor&) = delete;
    TimerObjectReactor &operator=(const TimerObjectReactor&) = delete;

    // Non-movable
    TimerObjectReactor(TimerObjectReactor&&) = delete;
    TimerObjectReactor &operator=(TimerObjectReactor&&) = delete;

    ~TimerObjectReactor() {
        std::cout << "Delete TOR" << std::endl;

        uint64_t stop = 1;

#ifdef __linux__
        if (::write(_evtfd, &stop, sizeof(stop)) != sizeof(stop)) {
            std::cout << "Timer stop failed: " << std::strerror(errno) << std::endl;
        }

        _thrd.join();

        if (_evtfd >= 0)
            ::close(_evtfd);

        if (_epfd >= 0)
            ::close(_epfd);
#endif
    }

    bool registerTimer(DataObject<TimerObject>& dot) {
        int fd = dot.get([](const TimerObject& t){ return t._fd; });
#ifdef __linux__
        epoll_event evt {};
        evt.events = EPOLLIN;
        evt.data.fd = fd;

        {
            std::lock_guard<std::mutex> lock(_mtx);
            _notify.insert({ fd, dot });
        }

        if (::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &evt) < 0) {
            // Delete it again in the case of error
            {
                std::lock_guard<std::mutex> lock(_mtx);
                _notify.erase(fd);
            }

            std::cout << "Epoll control error at TimerObjectReactor.registerTimer: " << std::strerror(errno) << std::endl;
            return false;
        }
#endif
        return true;
    }

    bool unregisterTimer(DataObject<TimerObject>& dot) {
        bool ret = true;

        int fd = dot.get([](const TimerObject& t){ return t._fd; });
#ifdef __linux__
        if (::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, 0) < 0) {
            std::cout << "Epoll control error at TimerObjectReactor.unregisterTimer: " << std::strerror(errno) << std::endl;
            ret = false;
        }

        // Erase it in any case
        {
            std::lock_guard<std::mutex> lock(_mtx);
            _notify.erase(fd);
        }
#endif
        return ret;
    }
};
}
