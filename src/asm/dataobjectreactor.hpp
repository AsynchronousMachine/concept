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

#include "../logger/logger.hpp"

namespace Asm {

template <typename D>
class DataObject;

class DataObjectReactor {
  private:
    static constexpr int RT_PRIO = 30; // Realtime priority

    struct Threadpool {
        boost::thread_group _tg;
        std::function<void(unsigned inst)> _f;

        Threadpool(unsigned thrd_cnt, const std::function<void(unsigned inst)>& f) : _f(f) {
            unsigned cores = boost::thread::hardware_concurrency();

            Logger::pLOG->info("Found {} cores for DOR", cores);

            if (thrd_cnt == 0 || thrd_cnt > cores)
                thrd_cnt = cores;

            for (unsigned i = 0; i < thrd_cnt; ++i) {
                boost::thread* t = _tg.create_thread([this, i](){ Threadpool::_f(i); });

                //The thread name is a meaningful C language string, whose length is
                //restricted to 16 characters, including the terminating null byte ('\0')
                std::string s = "DOR-THRD-" + std::to_string(i);
                Logger::pLOG->info("Created {}", s);

#ifdef __linux__
                if (pthread_setname_np(t->native_handle(), s.data()))
                    Logger::pLOG->warn("Could not set name for {}", s);

                struct sched_param param {};
                param.sched_priority = RT_PRIO;

                if (pthread_setschedparam(t->native_handle(), SCHED_FIFO, &param))
                    Logger::pLOG->warn("Could not set realtime parameter for {}", s);
#endif
            }

            Logger::pLOG->info("DOR has {} thread/s running", _tg.size());
        }

        ~Threadpool() {
            Logger::pLOG->info("Delete threadpool of DOR");
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
            Logger::pLOG->info("DOR-THRD-{} has TID-{}", inst, syscall(SYS_gettid));
#endif

            while (_run_state) {
                _tbbExecutionQueue.pop(f); // Pop of concurrent_bounded_queue waits if queue empty
                f();
            }
        } catch(const tbb::user_abort& abortException) {
            Logger::pLOG->info("Abort DOR-THRD-{}", inst);
            _run_state = false;
        }
        Logger::pLOG->info("DOR-THRD-{} has stopped", inst);
    }

  public:
    explicit DataObjectReactor(unsigned thrd_cnt = 0) : _run_state(true), _tp(thrd_cnt, [this](unsigned inst){ DataObjectReactor::run(inst); }) {}

    // Non-copyable
    DataObjectReactor(const DataObjectReactor&) = delete;
    DataObjectReactor &operator=(const DataObjectReactor&) = delete;

    // Non-movable
    DataObjectReactor(DataObjectReactor&&) = delete;
    DataObjectReactor &operator=(DataObjectReactor&&) = delete;

    ~DataObjectReactor() {
        Logger::pLOG->info("Delete DOR");
        _run_state = false;
        _tbbExecutionQueue.abort(); // Stops waiting of pop() in run()
    }

    // Announce the change of the content of a DataObject to the reactor
    template <class D>
    void trigger(DataObject<D>& d) {
        boost::shared_lock_guard<boost::shared_mutex> lock(d._mtx_links);

        if (d._links.empty())
            return;

        for (auto &p : d._links)
            _tbbExecutionQueue.push(p.second);
    }
};

}
