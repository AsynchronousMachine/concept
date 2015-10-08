//g++ -Wall -fexceptions -g -std=c++14 -I/opt/local/include -c /media/rp/EXT4_40GB/codeblocks/BorisSchaeling/main2.cpp -o obj/Debug/main2.o
//g++ -L/opt/local/lib -o bin/Debug/BorisSchaeling obj/Debug/main2.o   /opt/local/lib/libboost_system.so /opt/local/lib/libboost_thread.so

#include <pthread.h>

#include <iostream>
#include <string>
#include <vector>
#include <forward_list>
#include <queue>
#include <map>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <chrono>
#include <cstdio>

#include <boost/thread/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/algorithm.hpp>

// Concept of data object
//
// Data objects (aka DO) are container to hold data (content) and a callback function (link) as a notifier
// DOs can be linked together
// DOs can be locked and unlocked to handle there content consistently
// The associated functions must be called if the content of the linked DO has been changed
//
// DO1 --------------> DO2    DO1 is linked to DO2
//        |
//        +----------> DO3    Also DO1 is linked to DO3

// Concept of reactor
//
// All registered callback functions (links) should be called within the reactor
// This decouples the changing and the notify process based on that
// This allows the introduction of priorities how important a change has been notified

// Concept of module
//
// A module groups DOs and callbacks/links to gether and give them a common environment.
// A callback/link can access all members or methods of its module. The module can have a more
// complex state than a DO.
//
// Module
//    private
//        +-----DO1
//        +-----DO2
//        +-----...
//    public
//        +-----DO3
//        +-----DO4
//        +-----...
//        +-----Cb/Link1
//        +-----Cb/Link2
//        +-----...

// Still open topics
// What happens if triggered DO is no longer valid when called inside reactor?
// How to access DOs and Callbacks/Links from outside of module if only a text based description of that module is available?
// ...

enum PRIORITIES {HIGH=0, MIDDLE=1, LOW=2, MAX_PRIO=3};

// Template class for arbitrary  content
template <typename T> class DataObject
{
    friend class AsynchronousMachine;

    private:
        template <class M, class Enable = void>
        struct mutex
        {
            using type = boost::shared_mutex;
        };

        template <class M>
        struct mutex<M, std::enable_if_t<std::is_integral<M>::value>>
        {
            using type = boost::null_mutex;
        };

        // Make it easier to name it
        using mutex_t = typename mutex<T>::type;

        // A data object should have a constant name to identify it
        const std::string _name;

        // Content to handle
        T _content;

        // Mutable mutex_ member as it needs to be modified in the const member function get()
        mutable mutex_t mutex_;

        // Protect the list of linked DOs
        boost::mutex linkedDOs_mutex[MAX_PRIO];

        // This should be a at least a simple list to hold all data objects linked to that
        std::forward_list<std::function<void(DataObject<T>&)>> linkedDOs[MAX_PRIO];

        // Only called by reactor
        std::vector<std::function<void()>> get_callbacks(int prio)
        {
            std::vector<std::function<void()>> callbacks;
            boost::lock_guard<boost::mutex> lock(linkedDOs_mutex[prio]);
            for (auto it = linkedDOs[prio].begin(); it != linkedDOs[prio].end(); ++it)
            {
                auto callback = *it;
                callbacks.push_back([this, callback](){ callback(*this); });
            }
            return callbacks;
        }

    public:
        DataObject(const std::string name) : _name(name) {}
        DataObject(const std::string name, T content) : _name(name), _content(content) {}

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
            boost::lock_guard<mutex_t> lock(mutex_);
            visitor(_content);
        }

        // Const member function to avoid that a non-const reference is passed to the visitor
        // as that would allow the visitor to change the data_ member
        template <class Visitor>
        std::result_of_t<Visitor(T)> get(Visitor visitor) const
        {
            // Shared lock to support concurrent access from multiple visitors in different threads
            boost::shared_lock_guard<mutex_t> lock(mutex_);
            return visitor(_content);
        }

        // Get out the DO name for humansfor(int n : {0,1,2,3,4,5})
        const std::string& getName() const { return _name; }

        // Link a DO to that DO
        template <typename U, typename Callback>
        void registerLink(DataObject<U> &d2, Callback cb, PRIORITIES prio = LOW)
        {
            boost::lock_guard<boost::mutex> lock(linkedDOs_mutex[prio]);
            linkedDOs[prio].push_front([cb, &d2](DataObject<T>& d1){ cb(d1, d2); });
            std::cout << "Link " << d2.getName() << " to " << getName() << std::endl;
        }

        // Remove a link to a DO by pointer to DO
        template <typename U>
        void unregisterLink(DataObject<U>* ptr) { /*todo*/ }

        // Remove a link to a DO by name
        // Get out
        void unregisterLink(std::string name) { /*todo*/ }
};

// A simple reactor
class AsynchronousMachine
{
    protected:
        template <PRIORITIES Prio>
        struct threadpool
        {
            boost::thread_group threadgroup;
            boost::condition_variable_any cond;
            boost::mutex triggeredDOs_mutex;
            std::queue<std::function<void()>> triggeredDOs;

            threadpool() = default;
            threadpool(const threadpool&) = delete;

            void init(unsigned threads)
            {
                unsigned cores = boost::thread::hardware_concurrency();

                std::cout << "Found " << cores << " cores" << std::endl;

                if (threads == 0 || threads > cores)
                    threads = cores;boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
                for (unsigned i = 0; i < threads; ++i)
                    threadgroup.create_thread(std::bind(&threadpool::thread, this));
            }

            template <class Visitor>
            void init(unsigned threads, Visitor visitor)
            {
                unsigned cores = boost::thread::hardware_concurrency();
                if (threads == 0 || threads > cores)
                    threads = cores;
                for (unsigned i = 0; i < threads; ++i)
                {
                    boost::thread *t = threadgroup.create_thread(std::bind(&threadpool::thread, this));
                    visitor(t);
                }
            }

            ~threadpool()
            {
                threadgroup.interrupt_all();
                threadgroup.join_all();
            }

            template <class T>
            void trigger(DataObject<T> *ptr)
            {
                std::cout << "Trigger " << ptr->getName() << std::endl;

                bool notify = false;
                {
                    boost::lock_guard<boost::mutex> lock(triggeredDOs_mutex);
                    // I am a friend of DO so everything take in place
                    if (!ptr->linkedDOs[Prio].empty())
                    {
                        boost::lock_guard<boost::mutex> lock(ptr->linkedDOs_mutex[Prio]);

                        for(auto &callback : ptr->linkedDOs[Prio])
                            triggeredDOs.push([callback, ptr](){ callback(*ptr); });

                        notify = true;
                    }
                }
                // Call notify_all() without locked mutex
                if(notify)
                    cond.notify_all();
            }

            void thread()
            {
                for (;;)
                {
                    std::function<void()> f;
                    {
                        boost::lock_guard<boost::mutex> lock(triggeredDOs_mutex);
                        while (triggeredDOs.empty())
                            cond.wait(triggeredDOs_mutex);
                        f = triggeredDOs.front();
                        triggeredDOs.pop();
                    }
                    f();
                }
            }
        };

        boost::fusion::tuple<threadpool<HIGH>, threadpool<MIDDLE>, threadpool<LOW>> threadpools;

    public:
        explicit AsynchronousMachine(unsigned threads = 1)
        {
            boost::fusion::for_each(threadpools, [threads](auto &threadpool){ threadpool.init(threads); });
        }

        template <class Visitor>
        AsynchronousMachine(unsigned threads, Visitor visitor)
        {
            boost::fusion::for_each(threadpools, [threads, visitor](auto &threadpool){ threadpool.init(threads, visitor); });
        }

        template <class T>
        void trigger(DataObject<T> *ptr)
        {
            boost::fusion::for_each(threadpools, [ptr](auto &threadpool){ threadpool.trigger(ptr); });
        }
};

void my_cb(DataObject<int> &do1, DataObject<double> &do2)
{
    std::cout << "My DO.name: " << do2.getName() << std::endl;
    std::cout << "Got DO.name: " << do1.getName() << std::endl;
    do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
}

void my_cb2(DataObject<int> &do1, DataObject<std::string> &do2)
{
    std::cout << "My DO.name: " << do2.getName() << std::endl;
    std::cout << "Got DO.name: " << do1.getName() << std::endl;
    do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
}

void my_cb3(DataObject<double> &do1, DataObject<std::string> &do2)
{
    std::cout << "My DO.name: " << do2.getName() << std::endl;
    std::cout << "Got DO.name: " << do1.getName() << std::endl;
    do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
}

// Helper for data access
void fi(int i) {std::cout << i << '\n';}

// Helper for data access with return value
int fir(int i) {return i+1;}

// Alternate helper for data access
struct cb
{
    void operator() (int i) {std::cout << i << '\n';}
} cbi;

// Missing reflection to access content
class Module1
{
    private:
        // A module should have a constant name to identify it
        const std::string _name;

    public:
        // Only one constructor
        Module1(const std::string name) : _name(name), do1("DO1"), do2("DO2") {} // You have to choose a name
        DataObject<int> do1;
        DataObject<std::string> do2;
};

// Missing reflection to access content
class Module2
{
    private:
        // A module should have a constant name to identify it
        const std::string _name;
        const std::string _cmd;
        static constexpr int _state = 1;

        DataObject<int> do3; // Access is fully thread safe

    public:
        DataObject<int> do1;
        DataObject<std::string> do2;
        DataObject<double> do4;

        // Only one constructor
        Module2(const std::string name) : _name(name), _cmd("Init"), do3("DO3", 11), do1("DO1"), do2("DO2"), do4("DO4")
        {
            do1.set([](int &i){ i = _state; });
            do2.set([this](std::string &s){ s = _cmd; });
            do4.set([](double &i){ i = 2.0; });
        }

        void Link1(DataObject<int> &do1, DataObject<std::string> &do2)
        {
            boost::this_thread::sleep_for(boost::chrono::seconds(3));

            std::cout << "/My DO.name: " << do2.getName() << std::endl;
            std::cout << "/Got DO.name: " << do1.getName() << std::endl;
            do1.get([](int i){ std::cout << "/Got DO.value: " << i << std::endl; });
            std::cout << "/State: " << _state << std::endl;
            do2.get([](std::string s){ std::cout << "/Internal DO2.value: " << s << std::endl; });
            do3.set([](int &i){ ++i; });
            do3.get([](int i){ std::cout << "/Internal DO3.value: " << i << std::endl; });
        }

        void Link2(DataObject<int> &do1, DataObject<double> &do2)
        {
            boost::this_thread::sleep_for(boost::chrono::seconds(3));

            std::cout << "//My DO.name: " << do2.getName() << std::endl;
            std::cout << "//Got DO.name: " << do1.getName() << std::endl;
            do1.get([](int i){ std::cout << "//Got DO.value: " << i << std::endl; });
            std::cout << "/State: " << _state << std::endl;
            do2.get([](double d){ std::cout << "//Internal DO2.value: " << d << std::endl; });
            do3.set([](int &i){ ++i; });
            do3.get([](int i){ std::cout << "//Internal DO3.value: " << i << std::endl; });
        }
};

#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

void display_pthread_attr(pthread_attr_t *attr, char *prefix)
{
   int s, i;
   size_t v;
   void *stkaddr;
   struct sched_param sp;

   s = pthread_attr_getdetachstate(attr, &i);
   if (s != 0)
       handle_error_en(s, "pthread_attr_getdetachstate");
   printf("%sDetach state        = %s\n", prefix,
           (i == PTHREAD_CREATE_DETACHED) ? "PTHREAD_CREATE_DETACHED" :
           (i == PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" :
           "???");

   s = pthread_attr_getscope(attr, &i);
   if (s != 0)
       handle_error_en(s, "pthread_attr_getscope");
   printf("%sScope               = %s\n", prefix,
           (i == PTHREAD_SCOPE_SYSTEM)  ? "PTHREAD_SCOPE_SYSTEM" :
           (i == PTHREAD_SCOPE_PROCESS) ? "PTHREAD_SCOPE_PROCESS" :
           "???");

   s = pthread_attr_getinheritsched(attr, &i);
   if (s != 0)
       handle_error_en(s, "pthread_attr_getinheritsched");
   printf("%sInherit scheduler   = %s\n", prefix,
           (i == PTHREAD_INHERIT_SCHED)  ? "PTHREAD_INHERIT_SCHED" :
           (i == PTHREAD_EXPLICIT_SCHED) ? "PTHREAD_EXPLICIT_SCHED" :
           "???");

   s = pthread_attr_getschedpolicy(attr, &i);
   if (s != 0)
       handle_error_en(s, "pthread_attr_getschedpolicy");
   printf("%sScheduling policy   = %s\n", prefix,
           (i == SCHED_OTHER) ? "SCHED_OTHER" :
           (i == SCHED_FIFO)  ? "SCHED_FIFO" :
           (i == SCHED_RR)    ? "SCHED_RR" :
           "???");

   s = pthread_attr_getschedparam(attr, &sp);
   if (s != 0)
       handle_error_en(s, "pthread_attr_getschedparam");
   printf("%sScheduling priority = %d\n", prefix, sp.sched_priority);

   s = pthread_attr_getguardsize(attr, &v);
   if (s != 0)
       handle_error_en(s, "pthread_attr_getguardsize");
   printf("%sGuard size          = %d bytes\n", prefix, v);

   s = pthread_attr_getstack(attr, &stkaddr, &v);
   if (s != 0)
       handle_error_en(s, "pthread_attr_getstack");
   printf("%sStack address       = %p\n", prefix, stkaddr);
   printf("%sStack size          = 0x%zx bytes\n", prefix, v);
}

void get_threadinfo(boost::thread *t)
{
    char *prefix = "\t";
    pthread_attr_t attr;

    boost::thread::native_handle_type hnd = t->native_handle();

    if(pthread_getattr_np(hnd, &attr))
        return;

    display_pthread_attr(&attr, prefix);

    pthread_attr_destroy(&attr);
}

int main(void)
{
    DataObject<int> do1("Hello", 9);
    DataObject<double> do2("World");
    DataObject<std::string> do3("World2");

    // A module should have a constant name to identify it
    Module1 Hello("Hello");
    Module2 World("World");

    AsynchronousMachine asm1(2, get_threadinfo);

    std::cout << do1.getName() << std::endl;
    std::cout << do2.getName() << std::endl;

    // Link together: do1<int> -------> do2<double>
    do1.registerLink(do2, my_cb);

    // Link together: do1<int> -------> do3<string>
    do1.registerLink(do3, my_cb2);

    // Link together: do1<double> -------> do3<string>
    // Will not compile due to wrong data type of callback parameter
    //do1.registerLink(do3, my_cb3);

    // Access content consistently
    do1.get(fi);
    do1.set([](int &i){ i = 1; });
    do1.get(fi);
    do1.set([](int &i){ i = i + 1; });
    do1.get(cbi);
    std::cout << do1.get(fir) << '\n';

    asm1.trigger(&do1); // Because of changed content of do1

    // Complex DO
    DataObject<std::vector<int>> do4("Vector");
    do4.set([](std::vector<int> &v) {v = std::vector<int>{1, 2, 3};});
    do4.get([](const std::vector<int> &v){ std::cout << v[0] << ',' << v[1] << '\n'; });

    // More complex DO
    int tmp = 0;
    DataObject<std::map<std::string, int>> do5("Map", std::map<std::string, int>{{"3", 23}, {"4", 24}});
    do5.get([](const std::map<std::string, int> &v){ for (auto& m : v) {std::cout << m.first << " has value " << m.second << '\n';} } );
    do5.set([](std::map<std::string, int> &v) { v = std::map<std::string, int>{{"1", 42}, {"2", 43}}; });
    do5.get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
    do5.set([](std::map<std::string, int> &v){ v["1"] = v.at("1") + 1; });
    do5.get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
    do5.get([&tmp](const std::map<std::string, int> &v){ tmp = v.at("1") + 2; });
    std::cout << tmp << '\n';
    do5.get([](const std::map<std::string, int> &v){ for (auto& m : v) {std::cout << m.first << " has value " << m.second << '\n';} } );

    // Simulates changes of DO content faster than executable inside the reactor
    //do1.set([](int &i){ i = 10; });
    //asm1.trigger(&do1); // Because of changed content of do1

    // Simulate the job of ASM, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs linked to
    //asm1.execute();

    std::cout << "++++++++++++++++" << std::endl;

    // Link together
    Hello.do1.registerLink(World.do2, [&World](DataObject<int> &do1, DataObject<std::string> &do2){ World.Link1(do1, do2); });
    Hello.do1.registerLink(World.do4, [&World](DataObject<int> &do1, DataObject<double> &do2){ World.Link2(do1, do2); }, HIGH);

    Hello.do1.set([](int &i){ i = 10; });
    asm1.trigger(&Hello.do1); // Because of changed content of do1

    // Simulate the job of ASM, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs linked to
    //asm1.execute();

    boost::this_thread::sleep_for(boost::chrono::seconds(10));

    exit(0);
}

