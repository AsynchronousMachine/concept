#include <type_traits>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/thread.hpp>

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

        // Mutable mutex_ member as it needs to be modified in the const member function get()
        mutable mutex_t _mutex;

        // This holds all callbacks (LINKS) linked to that DO
        std::unordered_map<std::string, std::function<void()>> _links;

        // Protect the map of linked DOs
        std::mutex _links_mutex;

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
            std::lock_guard<std::mutex> lock(_links_mutex);
            _links.insert(std::make_pair(name, [cb, this, &d2](){ cb(*this, d2); }));
        }

        // Remove a link to that DO by name
        void unregisterLink(std::string name)
        {
            std::lock_guard<std::mutex> lock(_links_mutex);
            _links.erase(name);
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

        // This is a circular buffer to hold all links from dataobjects which content has been changed
        boost::circular_buffer<std::function<void()>> _triggeredDOs{MAX_CAPACITY};

        // Protect the list of triggered DOs
        std::mutex _triggeredDOs_mutex;

        struct Threadpool
        {
            boost::thread_group _threadgroup;
            std::condition_variable _cv_tg;

            Threadpool() = default;

            ~Threadpool()
            {
                _threadgroup.interrupt_all();
                _threadgroup.join_all();
            }

            void init(unsigned threads)
            {
                unsigned cores = boost::thread::hardware_concurrency();

                std::cout << "Found " << cores << " cores" << std::endl;

                if(threads == 0 || threads > cores)
                    threads = cores;

                for(unsigned i = 0; i < threads; ++i)
                    _threadgroup.create_thread([this](){Threadpool::thread();});

                 std::cout << "Have " << _threadgroup.size() << " thread/s running" << std::endl;
            }

            void thread()
            {
                for(;;)
                {
                    boost::this_thread::sleep_for(boost::chrono::milliseconds(3000));
                    std::cout << "thread" << std::endl;
                }
            }
        };

        struct Threadpool _tp;

    public:
        Reactor() = default;

        // Non-copyable
        Reactor(const Reactor&) = delete;
        Reactor &operator=(const Reactor&) = delete;

        // Non-movable
        Reactor(Reactor&&) = delete;
        Reactor &operator=(Reactor&&) = delete;

        // Initialize threadgroup and start the threads
        void init(unsigned threads = 1)
        {
            _tp.init(threads);
        }

        // Announce the change of the content of a dataobject to the reactor
        template <class D>
        void trigger(DataObject<D> &d)
        {
            std::lock_guard<std::mutex> lock(_triggeredDOs_mutex);

            {
                std::lock_guard<std::mutex> lock(d._links_mutex);

                if(d._links.empty())
                    return;

                for(auto &p : d._links)
                    _triggeredDOs.push_back(p.second);
            }

            // Now trigger a synchronization element to release at least a waiting thread
        }

        // Call all DOs which are linked to that DOs which have been triggered like DO2.CALL(&DO1) / DO1 ---> DO2
        // These method is typically private and called with in a thread related to a priority
        // This thread is typically waiting on a synchronization element
        void execute()
        {
            std::function<void()> cb;

            for(;;)
            {
                {
                    std::lock_guard<std::mutex> lock(_triggeredDOs_mutex);

                    if(_triggeredDOs.empty())
                        break;

                    cb = _triggeredDOs.front();
                    _triggeredDOs.pop_front();
                }

                // Execute the callback without holding the lock
                cb();
            }

            std::cout << "----" << std::endl;
        }
};

} // End of namespace