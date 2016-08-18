//g++ -Wall -fexceptions -g -std=c++14 -I/opt/local/include -c /media/rp/EXT4_40GB/codeblocks/BorisSchaeling/main2.cpp -o obj/Debug/main2.o
//g++ -L/opt/local/lib -o bin/Debug/BorisSchaeling obj/Debug/main2.o   /opt/local/lib/libboost_system.so /opt/local/lib/libboost_thread.so

#include <iostream>
#include <string>
#include <vector>
#include <forward_list>
#include <queue>
#include <map>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <memory>
#include <thread>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>

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
// How to access DOs and Callbacks/Links from outside of module if only a text based description of that module is available?
// ...

// Forward declaration
template <typename T> class DataObject;

// For convenience
template <typename T>
using SharedDataObject = std::shared_ptr<DataObject<T>>;

// A simple reactor
class AsynchronousMachine
{
    protected:
        // Protect the list of triggered DOs
        boost::mutex triggeredDOs_mutex;
        // This should be a at least a queue to hold all data objects which content has been changed or rather triggered
        std::queue<std::function<void()>> triggeredDOs;

    public:
        //TODO: please, explain the difference between trigger() and execute()? which must I call from outside?
        // Announce the change of content to the reactor
        template <class T>
        void trigger(const SharedDataObject<T> &data_object)
        {
            std::cout << "Trigger " << data_object->getName() << std::endl;
            std::weak_ptr<DataObject<T>> weak_ptr = data_object;
            boost::lock_guard<boost::mutex> lock(triggeredDOs_mutex);
            triggeredDOs.push([weak_ptr](){
                auto shared_ptr = weak_ptr.lock();
                if (shared_ptr)
                    shared_ptr->notify_all();
                else
                    std::cout << "weak_ptr is empty\n";
            });
        }

        // Call all DOs which are linked to that DOs which have been triggered like DO2.CALL(&DO1) / DO1 ---> DO2
        void execute()
        {
            while(!triggeredDOs.empty())
            {
                std::function<void()> f;
                {
                    boost::lock_guard<boost::mutex> lock(triggeredDOs_mutex);
                    f = triggeredDOs.front();
                    triggeredDOs.pop();
                }
                f();
            }
        }
};

// Template class for arbitrary  content
template <typename T> class DataObject : public std::enable_shared_from_this<DataObject<T>>


{
    friend class AsynchronousMachine;

    private:
        // TODO: ??? extend documentation!
        template <class M, class Enable = void>
        struct mutex
        {
            using type = boost::shared_mutex;
        };

        // TODO: ??? extend documentation!
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

        // This should be a at least a simple list to hold all data objects linked to that
        // A mutex for exlusive access will properly also necessary
        std::forward_list<std::function<void(SharedDataObject<T>)>> linkedDOs;

        // Only called by reactor
        void notify_all()
        {
    // TODO:     should this be a reference? If not, please explain why, otherwise there will be errors --v
            std::for_each(linkedDOs.begin(), linkedDOs.end(), [this](std::function<void(SharedDataObject<T>)> f){ f(this->shared_from_this()); });
        }

    public:
        // TODO: could this be const ref?
        explicit DataObject(std::string name) : _name(std::move(name)) {}
        DataObject(std::string name, T content) : _name(std::move(name)), _content(std::move(content)) {}

        // Non-copyable
        // TODO: please explain, why? Otherwise, someone will "know better"
        DataObject(const DataObject&) = delete;
        DataObject &operator=(const DataObject&) = delete;

        // Non-movable
        // TODO: please explain, why? Otherwise, someone will "know better"
        DataObject(DataObject&&) = delete;
        DataObject &operator=(DataObject&&) = delete;

        // Necessary if someone want to inherit from that
        virtual ~DataObject() = default;

        // TODO: Do I need this? If so, why?
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

        // Get out the DO name for humans
        const std::string& getName() const { return _name; }

        // Link a DO to that DO
        template <typename U, typename Callback>
        void registerLink(SharedDataObject<U> &d2, Callback &&callback)
        {
            linkedDOs.push_front([cb = std::forward<Callback>(callback), &d2](SharedDataObject<T> d1){ cb(d1, d2); });
            std::cout << "Link " << d2->getName() << " to " << getName() << std::endl;
        }

        // Remove a link to a DO by pointer to DO
        template <typename U>
        void unregisterLink(SharedDataObject<U> data_object) { /*todo*/ }

        // Remove a link to a DO by name
        // Get out
        void unregisterLink(std::string name) { /*todo*/ }
};

void my_cb(SharedDataObject<int> &do1, SharedDataObject<double> &do2)
{
    std::cout << "My DO.name: " << do2->getName() << std::endl;
    std::cout << "Got DO.name: " << do1->getName() << std::endl;
    do1->get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
}

void my_cb2(SharedDataObject<int> &do1, SharedDataObject<std::string> &do2)
{
    std::cout << "My DO.name: " << do2->getName() << std::endl;
    std::cout << "Got DO.name: " << do1->getName() << std::endl;
    do1->get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
}

void my_cb3(SharedDataObject<double> &do1, SharedDataObject<std::string> &do2)
{
    std::cout << "My DO.name: " << do2->getName() << std::endl;
    std::cout << "Got DO.name: " << do1->getName() << std::endl;
    do1->get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
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
        Module1(std::string name) : _name(std::move(name)), do1(std::make_shared<DataObject<int>>("DO1")), do2(std::make_shared<DataObject<std::string>>("DO2")) {} // You have to choose a name
        SharedDataObject<int> do1;
        SharedDataObject<std::string> do2;
};

// Missing reflection to access content
class Module2
{
    private:
        // A module should have a constant name to identify it
        const std::string _name;
        const std::string _cmd;
        static constexpr int _state = 1;

        SharedDataObject<int> do3; // Access is fully thread safe

    public:
        SharedDataObject<int> do1;
        SharedDataObject<std::string> do2;

        // Only one constructor
        Module2(std::string name) : _name(std::move(name)), _cmd("Init"), do3(std::make_shared<DataObject<int>>("DO3", 11)), do1(std::make_shared<DataObject<int>>("DO1", _state)), do2(std::make_shared<DataObject<std::string>>("DO2", "Init"))
        {
        }

        void Link1(SharedDataObject<int> &do1, SharedDataObject<std::string> &do2)
        {
            std::cout << "My DO.name: " << do2->getName() << std::endl;
            std::cout << "Got DO.name: " << do1->getName() << std::endl;
            do1->get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
            std::cout << "State: " << _state << std::endl;
            do2->get([](std::string s){ std::cout << "Internal DO2.value: " << s << std::endl; });
            do3->set([](int &i){ ++i; });
            do3->get([](int i){ std::cout << "Internal DO3.value: " << i << std::endl; });
        }
};

constexpr int Module2::_state;

int main(void)
{
    AsynchronousMachine asm1;

    SharedDataObject<int> do1 = std::make_shared<DataObject<int>>("Hello", 9);
    SharedDataObject<double> do2 = std::make_shared<DataObject<double>>("World");
    SharedDataObject<std::string> do3 = std::make_shared<DataObject<std::string>>("World2");

    std::cout << do1->getName() << std::endl;
    std::cout << do2->getName() << std::endl;

    // Link together: do1<int> -------> do2<double>
    do1->registerLink(do2, my_cb);

    // Link together: do1<int> -------> do3<string>
    do1->registerLink(do3, my_cb2);

    // Link together: do1<double> -------> do3<string>
    // Will not compile due to wrong data type of callback parameter
    //do1.registerLink(do3, my_cb3);

    // Access content consistently
    do1->get(fi);
    do1->set([](int &i){ i = 1; });
    do1->get(fi);
    do1->set([](int &i){ i = i + 1; });
    do1->get(cbi);
    std::cout << do1->get(fir) << '\n';

    asm1.trigger(do1); // Because of changed content of do1

    // Complex DO
    SharedDataObject<std::vector<int>> do4 = std::make_shared<DataObject<std::vector<int>>>("Vector");
    do4->set([](std::vector<int> &v) {v = std::vector<int>{1, 2, 3};});
    do4->get([](const std::vector<int> &v){ std::cout << v[0] << ',' << v[1] << '\n'; });

    // More complex DO
    int tmp = 0;
    SharedDataObject<std::map<std::string, int>> do5 = std::make_shared<DataObject<std::map<std::string, int>>>("Map", std::map<std::string, int>{{"3", 23}, {"4", 24}});
    do5->get([](const std::map<std::string, int> &v){ for (auto& m : v) {std::cout << m.first << " has value " << m.second << '\n';} } );
    do5->set([](std::map<std::string, int> &v) { v = std::map<std::string, int>{{"1", 42}, {"2", 43}}; });
    do5->get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
    do5->set([](std::map<std::string, int> &v){ v["1"] = v.at("1") + 1; });
    do5->get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
    do5->get([&tmp](const std::map<std::string, int> &v){ tmp = v.at("1") + 2; });
    std::cout << tmp << '\n';
    do5->get([](const std::map<std::string, int> &v){ for (auto& m : v) {std::cout << m.first << " has value " << m.second << '\n';} } );

    // Simulates changes of DO content faster than executable inside the reactor
    //do1.set([](int &i){ i = 10; });
    //asm1.trigger(&do1); // Because of changed content of do1

    // Simulate the job of ASM
    // Should notify all callbacks of all DOs linked to
    asm1.execute();

    std::cout << "++++++++++++++++" << std::endl;

    // A module should have a constant name to identify it
    Module1 Hello("Hello");
    Module2 World("World");

    // Link together
    Hello.do1->registerLink(World.do2, [&World](SharedDataObject<int> &do1, SharedDataObject<std::string> &do2){ World.Link1(do1, do2); });

    Hello.do1->set([](int &i){ i = 10; });
    asm1.trigger(Hello.do1); // Because of changed content of do1

    // Simulate the job of ASM
    // Should notify all callbacks of all DOs linked to
    asm1.execute();

    std::cout << "++++++++++++++++" << std::endl;

    {
        SharedDataObject<int> do6 = std::make_shared<DataObject<int>>("Test", 6);
        SharedDataObject<int> do7 = std::make_shared<DataObject<int>>("Test2", 7);
        do6->registerLink(do7, [](SharedDataObject<int> &do1, SharedDataObject<int> &do2){ do1->get([](int i){ std::cout << i << '\n'; }); });
        asm1.trigger(do6);
    }
    asm1.execute();

    exit(0);
}
