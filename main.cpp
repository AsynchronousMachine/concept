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
#include <boost/fusion/container.hpp>
#include <boost/fusion/container/generation.hpp>
#include <boost/fusion/sequence.hpp>

// Concept of data object
//
// A Data object (aka DO) is a container to hold content (data)
// DOs can be linked together (source vrs. destination)
// A DO (source) holds references to callback functions (aka LINKs) to their linked DOs (destination) too
// The content of a DO can only be accessed in a locked state due to a visitor idiom
// The associated callback functions (LINKSs) are grouped and called via a notifier function if the content of the source DO has been changed
//
// DO1 ------(LINK1)-----> DO2          DO1 is linked via LINK1 to DO2
//        |
//        +--(LINK2)-----> DO3          DO1 is linked via LINK2 to DO3, too
//
// Concept of reactor
//
// All registered callback functions (LINKs) should be called within the reactor
// This decouples the changing and the notify process based on that
// This allows the introduction of priorities how important a change has been notified

// Concept of module
//
// A module groups DOs and callbacks functions (LINKs) together and give them a common environment.
// A LINK can access all members or methods of its module. The module can have a more complex state than a DO.
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
//        +-----LINK1
//        +-----LINK2
//        +-----...

// Still open topics
// How to access DOs and LINKs from outside of the module if only a text based description of that module is available?
// ...


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

        // This should be a at least a simple list to hold all data objects linked to that
        // A mutex for exlusive access will properly also necessary
        std::forward_list<std::function<void(DataObject<T>&)>> linkedDOs;

        // Only called by reactor
        void notify_all()
        {
            std::for_each(linkedDOs.begin(), linkedDOs.end(), [this](std::function<void(DataObject<T>&)> f){ f(*this); });
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
            std::cout << "Trigger " << ptr->getName() << std::endl;
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

        auto getDataObjects()
        {
            // Returns boost::fusion::vector<DataObject<int>&, DataObject<std::string>&>
            return boost::fusion::vector_tie(do1, do2);
        }

        auto getLinks()
        {
            // Returns boost::fusion::vector<>
            return boost::fusion::make_vector();
        }
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

        // Only one constructor
        Module2(const std::string name) : _name(name), _cmd("Init"), do3("DO3", 11), do1("DO1"), do2("DO2")
        {
            do1.set([](int &i){ i = _state; });
            do2.set([this](std::string &s){ s = _cmd; });
        }

        void Link1(DataObject<int> &do1, DataObject<std::string> &do2)
        {
            std::cout << "My DO.name: " << do2.getName() << std::endl;
            std::cout << "Got DO.name: " << do1.getName() << std::endl;
            do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
            std::cout << "State: " << _state << std::endl;
            do2.get([](std::string s){ std::cout << "Internal DO2.value: " << s << std::endl; });
            do3.set([](int &i){ ++i; });
            do3.get([](int i){ std::cout << "Internal DO3.value: " << i << std::endl; });
        }

        void Link2(DataObject<int> &do1, DataObject<int> &do2)
        {
        }

        auto getDataObjects()
        {
            // Returns boost::fusion::vector of references to DOs
            return boost::fusion::vector_tie(do1, do2);
        }

        auto getLinks()
        {
            //return boost::fusion::vector of Links
            return boost::fusion::make_vector([this](DataObject<int> &do1, DataObject<std::string> &do2) { Module2::Link1(do1, do2); },
                                              [this](DataObject<int> &do1, DataObject<int> &do2) { Module2::Link2(do1, do2); });
        }
};

int main(void)
{
    AsynchronousMachine asm1;

    DataObject<int> do1("Hello", 9);
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

    // Simulate the job of ASM
    // Should notify all callbacks of all DOs linked to
    asm1.execute();

    std::cout << "++++++++++++++++" << std::endl;

    // A module should have a constant name to identify it
    Module1 Hello("Hello");
    Module2 World("World");

    // Link together Hello.do1 and World.do2 via Word.Link1
    auto HelloDOs = Hello.getDataObjects();
    auto WorldDOs = World.getDataObjects();

    auto &HelloDO1 = boost::fusion::at_c<0>(HelloDOs);
    auto &WorldDO2 = boost::fusion::at_c<1>(WorldDOs);

    auto HelloLinks = Hello.getLinks();
    auto WorldLinks = World.getLinks();

    auto Link1 = boost::fusion::at_c<0>(WorldLinks);

    HelloDO1.registerLink(WorldDO2, [Link1](auto &do1, auto &do2) { Link1(do1, do2); });

    HelloDO1.set([](int &i){ i = 10; });
    asm1.trigger(&HelloDO1); // Because of changed content of do1

    // Simulate the job of ASM
    // Should notify all callbacks of all DOs linked to
    asm1.execute();

    exit(0);
}

