#include <type_traits>
#include <unordered_map>
#include <functional>

#include <boost/thread/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

#include <pthread.h>
#include <time.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

// AsynchronousMachine
namespace Asm {

// Concept of dataobject
//
// Dataobjects (aka DO) are container to hold data (content) and callback functions (aka LINKS) as notifier
// DOs can be linked together via LINKS
// DOs can be locked and unlocked to handle there content consistently
// The associated callback functions (LINKS) are called if the content of the original DO has been changed
//
// DO1 ------LINK1-------> DO2    DO1 is linked to DO2
//        |
//        +--LINK3-------> DO3    Also DO1 is linked to DO3

// Template class for a dataobject
template <typename D>
class DataObject
{
    friend class Reactor; // This enables the reactor to traverse the links from outside

private:
    template <class M, class Enable = void>
    struct mutex
    {
        using type = void; //Invalid type for mutex
    };

    template <class M>
    struct mutex<M, std::enable_if_t<std::is_integral<M>::value>>
    {
        using type = boost::null_mutex;
    };

    template <class M>
    struct mutex<M, std::enable_if_t<!std::is_integral<M>::value>>
    {
        using type = boost::shared_mutex;
    };

    // Make it easier to name it
    using mutex_t = typename mutex<D>::type;

    // A dataobject should have a constant name to identify it
    const std::string _name;

    // Content for this DO
    D _content;

    // Mutable member as it needs to be modified in the const member function get()
    mutable mutex_t _mutex;

    // This holds all callbacks (LINKS) linked to that DO
    std::unordered_map<std::string, std::function<void()>> _links;

    // Protect the map of linked DOs
    boost::mutex _links_mutex;

public:
    DataObject(const std::string name) : _name(name) {}
    DataObject(const std::string name, D content) : _name(name), _content(content) {}

    // Non-copyable
    DataObject(const DataObject&) = delete;
    DataObject &operator=(const DataObject&) = delete;

    // Non-movable
    DataObject(DataObject&&) = delete;
    DataObject &operator=(DataObject&&) = delete;

    // Necessary if someone want to inherit from that
    virtual ~DataObject() = default;

    template <class Visitor>
    void set(Visitor visitor)
    {
        // Exclusive lock for write access
        boost::lock_guard<mutex_t> lock(_mutex);
        visitor(_content);
    }

    void set(const boost::any &value)
    {
        // Exclusive lock for write access
        boost::lock_guard<mutex_t> lock(_mutex);
        _content = boost::any_cast<D>(value);
    }

    // Const member function to avoid that a non-const reference is passed to the visitor
    // as that would allow the visitor to change the data_ member
    template <class Visitor>
    std::result_of_t<Visitor(D)> get(Visitor visitor) const
    {
        // Shared lock to support concurrent access from multiple visitors in different threads
        boost::shared_lock_guard<mutex_t> lock(_mutex);
        return visitor(_content);
    }

    // Get out the DO name for humans
    const std::string& getName() const { return _name; }

    // Link a DO to that DO
    template <typename D2, typename CB>
    void registerLink(std::string name, DataObject<D2> &d2, CB cb)
    {
        boost::lock_guard<boost::mutex> lock(_links_mutex);
        _links.insert({name, [cb, this, &d2](){ cb(*this, d2); }});
    }

    // Remove a link to that DO by name
    void unregisterLink(std::string name)
    {
        boost::lock_guard<boost::mutex> lock(_links_mutex);
        _links.erase(name);
    }
};

// Template class for a link
template <typename D1, typename D2>
class Link
{
public:
    using cb_type = std::function<void(D1&, D2&)>;

private:
    cb_type _cb;

public:
    template <typename MemFun, typename ThisPtr>
    Link(MemFun memfun, ThisPtr thisptr) : _cb([thisptr, memfun](D1& d1, D2& d2){std::mem_fn(memfun)(thisptr, d1, d2);}) {}
    Link(cb_type cb) : _cb(cb) {}

    // Non-copyable
    Link(const Link&) = delete;
    Link &operator=(const Link&) = delete;

    // Non-movable
    Link(Link&&) = delete;
    Link &operator=(Link&&) = delete;

    // Necessary if someone want to inherit from that
    virtual ~Link() = default;

    void set(const std::string name, boost::any a1, boost::any a2)
    {
        D1 *d1 = boost::any_cast<D1*>(a1);
        D2 *d2 = boost::any_cast<D2*>(a2);
        d1->registerLink(name, *d2, _cb);
    }

    void clear(const std::string name, boost::any a)
    {
        D1 *d1 = boost::any_cast<D1*>(a);
        d1->unregisterLink(name);
    }
};

// Concept of reactor
//
// All registered callback functions (aka LINKS) should be called within the reactor
// This decouples the changing of data (content) and the notify process based on that
// This allows the introduction of priorities how important a change has been notified

// A simple reactor
// To simulate the not implemented asynchronous behavior call the public function execute()
class Reactor
{
private:
    // Max. queue capacity
    static constexpr size_t MAX_CAPACITY = 1024;

    // Realtime priority
    static constexpr int RT_PRIO = 30;

    // This is a circular buffer to hold all links from dataobjects which content has been changed
    boost::circular_buffer<std::function<void()>> _triggeredLinks{MAX_CAPACITY};

    // Protect the circular buffer of triggered DOs
    boost::mutex _triggeredLinks_mutex;

    struct Threadpool
    {
        boost::thread_group _tg;
        boost::condition_variable _cv;
        std::function<void()> _f;

        Threadpool(unsigned threads, std::function<void()> f) : _f(f)
        {
            unsigned cores = boost::thread::hardware_concurrency();

            std::cout << "Found " << cores << " cores" << std::endl;

            if(threads == 0 || threads > cores)
                threads = cores;

            for(unsigned i = 0; i < threads; ++i)
            {
                boost::thread *t = _tg.create_thread([this](){ Threadpool::thread(); });

                //The thread name is a meaningful C language string, whose length is
                //restricted to 16 characters, including the terminating null byte ('\0')
                std::string s = "ASM-TP" + boost::lexical_cast<std::string>(i);
                std::cout << s << std::endl;

#ifdef __linux__
                if(pthread_setname_np(t->native_handle(), s.data()))
                    std::cout << "Could not set threadpool name" << std::endl;

                struct sched_param param = {.sched_priority = RT_PRIO};

                if(pthread_setschedparam(t->native_handle(), SCHED_FIFO, &param))
                    std::cout << "Could not set realtime parameter" << std::endl;
#endif
            }

            std::cout << "Have " << _tg.size() << " thread/s running" << std::endl << std::endl;
        }

        ~Threadpool()
        {
            std::cout << "Delete treadpool" << std::endl;
            _tg.interrupt_all();
            _tg.join_all();
        }

        void wait(boost::unique_lock<boost::mutex>& lock) { _cv.wait(lock); }

        void notify() { _cv.notify_all(); }

        void thread() { _f(); }
    };

    struct Threadpool _tp;

    // Call all DOs which are linked to that DOs which have been triggered like DO2.CALL(&DO1) / DO1 ---> DO2
    // These method is typically private and called with in a thread related to a priority
    // This thread is typically waiting on a synchronization element
    void execute()
    {
        std::function<void()> f;

        for(;;)
        {
            {
                boost::unique_lock<boost::mutex> lock(_triggeredLinks_mutex);

                while(_triggeredLinks.empty())
                {
                    _tp.wait(lock);
                    std::cout << ">>>" << std::endl;
                }

                f = _triggeredLinks.front();
                _triggeredLinks.pop_front();
            }

            // Execute the link without holding the lock
            f();
        }
    }

public:
    Reactor(unsigned threads = 1) : _tp(threads, [this](){Reactor::execute();}) {}

    // Non-copyable
    Reactor(const Reactor&) = delete;
    Reactor &operator=(const Reactor&) = delete;

    // Non-movable
    Reactor(Reactor&&) = delete;
    Reactor &operator=(Reactor&&) = delete;

    ~Reactor() { std::cout << std::endl << "Delete reactor" << std::endl; }

    // Announce the change of the content of a dataobject to the reactor
    template <class D>
    void trigger(DataObject<D> &d)
    {
        {
            boost::lock_guard<boost::mutex> lock(d._links_mutex);

            if(d._links.empty())
                return;

            for(auto &p : d._links)
                _triggeredLinks.push_back(p.second);
        }

        // Now trigger a synchronization element to release at least a waiting thread
        _tp.notify();
    }
};

class Timer
{
    friend class TimerReactor; // This enables the timer reactor to traverse the file descriptors from outside

private:
    int _fd;
    long _interval;
    long _next;

public:
    Timer() : _fd(-1), _interval(0), _next(0)
    {
        if((_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC)) < 0)
        {
            std::cout << "Timer could not be created: " << std::strerror(errno) << std::endl;
        }
    }

    // Non-copyable
    Timer(const Timer&) = delete;
    Timer &operator=(const Timer&) = delete;

    // Non-movable
    Timer(Timer&&) = delete;
    Timer &operator=(Timer&&) = delete;

    ~Timer()
    {
        Stop();
        ::close(_fd);
    }

    bool SetRelativeInterval(long interval, long next = 0)
    {
        itimerspec val;

        if(interval == 0 && next == 0)
        {
            std::cout << "Interval and next start time must be different from zero" << std::endl;
            return false;
        }

        _interval = interval;
        _next = next ? next : interval;

        val.it_interval.tv_sec = _interval / 1000;
        val.it_interval.tv_nsec = (_interval % 1000) * 1000000;
        val.it_value.tv_sec = _next / 1000;
        val.it_value.tv_nsec = (_next % 1000) * 1000000;


        if(::timerfd_settime(_fd, 0, &val, 0) < 0)
        {
            std::cout << "Timer could not set: " << std::strerror(errno) << std::endl;
            return false;
        }

        return true;
    }

    bool Stop()
    {
        itimerspec val {};

        if(::timerfd_settime(_fd, 0, &val, 0) < 0)
        {
            std::cout << "Timer could not stopped: " << std::strerror(errno) << std::endl;
            return false;
        }

        return true;
    }

    long GetInterval() { return _interval; }

    bool Restart() { return SetRelativeInterval(_interval, _next); }

    bool Wait(uint64_t& elapsed)
    {
        if(::read(_fd, &elapsed, sizeof(uint64_t)) != sizeof(uint64_t))
        {
            std::cout << "Timer could not read: " << std::strerror(errno) << std::endl;
            return false;
        }

        return true;
    }
};

class TimerReactor
{
private:
    // Max. epoll capacity, can be found at /proc/sys/fs/epoll/max_user_watches
    static constexpr size_t MAX_CAPACITY = 256;

    // Epoll file descriptor associated data
    struct TimerEntry
    {
        bool ref;
        bool one_shot;
        Timer* ptimer;
    };

    // File descriptor for epoll machanism
    int _fd;

    // Reference to the event reactor
    Reactor& _reactor;

    // Protects internal data
    boost::mutex _mtx;

    // Holds the timer thread reference
    boost::thread _t;

    // Holds all epoll file descriptor associated data
    std::unordered_map<int, TimerEntry> _notify;

    bool _break;

    // Threaded timer function mechanism
    void execute()
    {
        for(;;)
        {
            epoll_event evt[MAX_CAPACITY];
            // Use eventfd for breaking epoll_wait ...
            int evt_cnt = ::epoll_wait(_fd, &evt[0], MAX_CAPACITY, 1000);

            if(evt_cnt == 0 && _break)
            {
                std::cout << "Epoll breaked: " << std::strerror(errno) << std::endl;
                return;
            }

            if(evt_cnt < 0)
            {
                std::cout << "Epoll wait error: " << std::strerror(errno) << std::endl;
                continue;
            }

            for(int i = 0; i < evt_cnt; i++)
            {
                if(evt[i].events & EPOLLIN)
                {
                    uint64_t elapsed;

                    if(::read(evt[i].data.fd, &elapsed, sizeof(elapsed)) != sizeof(elapsed))
                    {
                        std::cout << "Read timer returns wrong size: " << std::strerror(errno) << std::endl;
                        continue;
                    }

                    std::cout << "Timer has fired" << std::endl;

//                    const int& fd = events[i].data.fd;
//                    {
//                        if (m_NotifyMap[fd].NotifyPtr != 0)
//                            m_pDOReactor->trigger(m_NotifyMap[fd].level, m_NotifyMap[fd].NotifyPtr);
//                    }
                }
            }
        }
    }

public:
    TimerReactor(Reactor& reactor) : _fd(-1), _reactor(reactor), _break(false)
    {
        if((_fd = ::epoll_create1(EPOLL_CLOEXEC)) < 0)
        {
            std::cout << "Epoll file handle could not be created: " << std::strerror(errno) << std::endl;
            return;
        }

        _t = boost::thread([this](){TimerReactor::execute();});
    }

    // Non-copyable
    TimerReactor(const TimerReactor&) = delete;
    TimerReactor &operator=(const TimerReactor&) = delete;

    // Non-movable
    TimerReactor(TimerReactor&&) = delete;
    TimerReactor &operator=(TimerReactor&&) = delete;

    ~TimerReactor()
    {
        std::cout << "Delete timer reactor" << std::endl;

        _break = true;

        _t.join();

        if(_fd >= 0)
            ::close(_fd);
    }

    bool Register(Timer* ptimer, /*Reference to ???*/ bool ref = true, bool one_shot = false)
    {
        epoll_event evt;

        boost::lock_guard<boost::mutex> lock(_mtx);

        _notify.insert({ptimer->_fd, TimerEntry {ref, one_shot, ptimer}});

        evt.events = EPOLLIN;
        if(one_shot)
            evt.events |= EPOLLONESHOT;

        evt.data.fd = ptimer->_fd;

        if(::epoll_ctl(_fd, EPOLL_CTL_ADD, ptimer->_fd, &evt) < 0)
        {
            // Delete it again in the case of error
            _notify.erase(ptimer->_fd);

            std::cout << "Epoll control error at ADD: " << std::strerror(errno) << std::endl;
            return false;
        }

        return true;
    }

    bool Unregister(Timer* ptimer)
    {
        bool ret = true;

        boost::lock_guard<boost::mutex> lock(_mtx);

        if(::epoll_ctl(_fd, EPOLL_CTL_DEL, ptimer->_fd, 0) < 0)
        {
            std::cout << "Epoll control error at DEL: " << std::strerror(errno) << std::endl;
            ret = false;
        }

        // Erase it in any case
        _notify.erase(ptimer->_fd);

        return ret;
    }
};

} // End of namespace
