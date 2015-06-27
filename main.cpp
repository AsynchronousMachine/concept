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

        // Content to handleor rather
        T _content;

        // Mutable mutex_ member as it needs to be modified in the const member function get()
        mutable mutex_t mutex_;

        // A data object should have a constant name to identify it
        const std::string _name;

        // This should be a at least a simple list to hold all data objects linked to that
        // A mutex for exlusive access will properly also necessary
        std::forward_list<std::function<void(DataObject<T>&)>> linkedDOs;

        // Only called by reactor
        void notify_all()
        {
            std::for_each(linkedDOs.begin(), linkedDOs.end(), [this](std::function<void(DataObject<T>&)> f){ f(*this); });
        }

    public:
        // Only one constructor
        DataObject(const std::string name) : _name(name) {} // You have to choose a name

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

        // Get out the DO name for humans
        const std::string& getName() const { return _name; }

        // Link a DO to that DO
        template <typename U, typename Callback>
        void registerLink(DataObject<U> &d2, Callback cb)
        {
            linkedDOs.push_front([cb, &d2](DataObject<T>& d1){ cb(d1, d2); });
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
        // Protect the list of triggered DOs
        boost::mutex triggeredDOs_mutex;
        // This should be a at least a queue to hold all data objects which content has been changed or rather triggered
        std::queue<std::function<void()>> triggeredDOs;

    public:
        // Announce the change of content to the reactor
        template <class T>
        void trigger(DataObject<T>* ptr)
        {
            std::cout << "Trigger DO " << ptr->getName() << std::endl;
            boost::lock_guard<boost::mutex> lock(triggeredDOs_mutex);
            triggeredDOs.push([ptr](){ ptr->notify_all(); });
        }

        // Call all DOs which are linked to that DOs which have been triggered like DO2.CALL(&DO1) / DO1 ---> DO2
        void execute()
        {
            while(!triggeredDOs.empty())
            {
                std::function<void()> f;
                {
                    boost::lock_guard<boost::mutex> lock(triggeredDOs_mutex);
                    // What happens if triggered DO is no longer valid?
                    f = triggeredDOs.front();
                    triggeredDOs.pop();
                }
                f();
            }
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

class Module2
{
    private:
        // A module should have a constant name to identify it
        const std::string _name;

        int dummy = 1; // This is evt. not thread save depends on type

    public:
        // Only one constructor
        Module2(const std::string name) : _name(name), do3("DO3"), do4("DO4") {} // You have to choose a name
        DataObject<int> do3;
        DataObject<std::string> do4;


        void Link1(DataObject<int> &do1, DataObject<std::string> &do2)
        {
            std::cout << "My DO.name: " << do2.getName() << std::endl;
            std::cout << "Got DO.name: " << do1.getName() << std::endl;
            do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
            std::cout << "Dummy: " << dummy << std::endl;
        }
};

int main(void)
{
    AsynchronousMachine asm1;

    DataObject<int> do1("Hello");
    DataObject<double> do2("World");
    DataObject<std::string> do3("World2");

    std::cout << do1.getName() << std::endl;
    std::cout << do2.getName() << std::endl;

    // Link together: do1<int> -------> do2<double>
    do1.registerLink(do2, my_cb);

    // Link together: do1<int> -------> do3<string>
    do1.registerLink(do3, my_cb2);

    // Link together: do1<double> -------> do3<string>
    // Will not compile due to wrong data type of callback parameter
    //do1.registerLink(do3, my_cb3);

    // Access content consistently, wra
    // A module should have a constant name to identify itpper with dummy mutex
    do1.set([](int &i){ i = 1; });
    do1.get(fi);
    do1.set([](int &i){ i = i + 1; });
    do1.get(cbi);
    std::cout << do1.get(fir) << '\n';

    asm1.trigger(&do1); // Because of changed content of do1

    // Get out
    // Complex DO
    DataObject<std::vector<int>> do4("Vector");
    do4.set([](std::vector<int> &v) {v = std::vector<int>{1, 2, 3};});
    do4.get([](const std::vector<int> &v){ std::cout << v[0] << ',' << v[1] << '\n'; });

    // More complex DO
    int tmp = 0;
    DataObject<std::map<std::string, int>> do5("Map");
    do5.set([](std::map<std::string, int> &v) { v = std::map<std::string, int>{{"1", 42}, {"2", 21}}; });
    do5.get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
    do5.set([](std::map<std::string, int> &v){ v["1"] = v.at("1") + 1; });
    do5.get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
    do5.get([&tmp](const std::map<std::string, int> &v){ tmp = v.at("1") + 2; });
    std::cout << tmp << '\n';

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
    Hello.do1.registerLink(World.do4, [&World](DataObject<int> &do1, DataObject<std::string> &do2){ World.Link1(do1, do2); });

    Hello.do1.set([](int &i){ i = 10; });
    asm1.trigger(&Hello.do1); // Because of changed content of do1

    // Simulate the job of ASM
    // Should notify all callbacks of all DOs linked to
    asm1.execute();

    exit(0);
}

