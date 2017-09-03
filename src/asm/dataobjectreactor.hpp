/*
** All stored/registered callback functions (aka LINKs) should be called within the reactor
** This decouples the changing of the content of a DataObject) and the notify process based on that
** This allows the introduction of priorities how important a change has to be notified
*/

#pragma once

#ifdef __linux__
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

#include <functional>
#include <iostream>
#include <string>

#include <boost/thread.hpp>

#include <tbb/tbb.h>

namespace Asm {

template <typename D>
class DataObject;

class DataObjectReactor {
  private:
    static constexpr int RT_PRIO = 30; // Realtime priority

    struct Threadpool {
        boost::thread_group _tg;
        std::function<void(unsigned inst)> _f;

        Threadpool(unsigned thrd_cnt, std::function<void(unsigned inst)> f) : _f(f) {
            unsigned cores = boost::thread::hardware_concurrency();

            std::cout << "Found " << cores << " cores for DOR" << std::endl;

            if (thrd_cnt == 0 || thrd_cnt > cores)
                thrd_cnt = cores;

            for (unsigned i = 0; i < thrd_cnt; ++i) {
                boost::thread *t = _tg.create_thread([this, i](){ Threadpool::_f(i); });

                //The thread name is a meaningful C language string, whose length is
                //restricted to 16 characters, including the terminating null byte ('\0')
                std::string s = "DOR-THRD-" + std::to_string(i);
                std::cout << "Created " << s << std::endl;

#ifdef __linux__
                if (pthread_setname_np(t->native_handle(), s.data()))
                    std::cout << "Could not set name for DOR-THRD-" << std::to_string(i) << std::endl;

                struct sched_param param {};
                param.sched_priority = RT_PRIO;

                if (pthread_setschedparam(t->native_handle(), SCHED_FIFO, &param))
                    std::cout << "Could not set realtime parameter for DOR-THRD-" << std::to_string(i) << std::endl;
#endif
            }

            std::cout << "DOR has " << _tg.size() << " thread/s running" << std::endl;
        }

        ~Threadpool() {
            std::cout << "Delete threadpool of DOR" << std::endl;
            _tg.interrupt_all();
            _tg.join_all();
        }
    };

    tbb::concurrent_bounded_queue<std::function<void()>> _tbbExecutionQueue;
    bool _run_state;
    struct Threadpool _tp;

    // Call all DOs which are linked to that DOs which have been triggered
    // These method is typically private and called with in a thread related to a priority
    // This thread is typically waiting on a synchronization element
    void run(unsigned inst) {
        std::function<void()> f;

        try {

#ifdef __linux__
            std::cout << "DOR-THRD-" << inst << " has TID-" << syscall(SYS_gettid) << std::endl;
#endif

            while (_run_state) {
                _tbbExecutionQueue.pop(f); // Pop of concurrent_bounded_queue waits if queue empty
                f();
            }
        } catch(tbb::user_abort abortException) {
            std::cout << "Abort DOR-THRD-" << inst << std::endl;
            _run_state = false;
        }
        std::cout << "DOR-THRD-" << inst << " has stopped" << std::endl;
    }

  public:
    DataObjectReactor(unsigned thrd_cnt = 0) : _run_state(true), _tp(thrd_cnt, [this](unsigned inst){ DataObjectReactor::run(inst); }) {}

    // Non-copyable
    DataObjectReactor(const DataObjectReactor&) = delete;
    DataObjectReactor &operator=(const DataObjectReactor&) = delete;

    // Non-movable
    DataObjectReactor(DataObjectReactor&&) = delete;
    DataObjectReactor &operator=(DataObjectReactor&&) = delete;

    ~DataObjectReactor() {
        std::cout << std::endl << "Delete DOR" << std::endl;
        _run_state = false;
        _tbbExecutionQueue.abort(); // Stops waiting of pop() in run()
    }

    // Announce the change of the content of a DataObject to the reactor
    template <class D>
    void trigger(DataObject<D>& d) {
        boost::lock_guard<boost::mutex> lock(d._mtx_links);

        if (d._links.empty())
            return;

        for (auto &p : d._links)
            _tbbExecutionQueue.push(p.second);
    }
};

}
