//g++ -Wall -fexceptions -g -std=c++14 -I/opt/local/include -c /media/rp/EXT4_40GB/codeblocks/BorisSchaeling/main2.cpp -o obj/Debug/main2.o
//g++ -L/opt/local/lib -o bin/Debug/BorisSchaeling obj/Debug/main2.o   /opt/local/lib/libboost_system.so /opt/local/lib/libboost_thread.so

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>

// Concept of data object
//
// Data objects (aka DO) are container to hold data (content) and a callback function as a notifier
// DOs can be linked together
// DOs can be locked and unlocked to handle there content consistently
// The associated functions must be called if the content of the linked DO has been changed
//
// DO1 --------------> DO2    DO1 is linked to DO2
//        |
//        +----------> DO3    Also DO1 is linked to DO3

// Concept of reactor
//
// All registered callback functions should be called within the reactor
// This decouples the changing and the notify process based on that
// This allows the introduction of priorities how important a change has been notified

// Everything that all DOs have in common
class DataObjectBase
{
    // Make it easier to handle the callback function
    typedef void (CallbackType)(DataObjectBase*);

    protected:
        // A data object should have a name to identify it by humans
        std::string _name;
        // This should be a at least a list to hold all data objects linked to that
        DataObjectBase* _ptr=nullptr;
        // This is a pointer to a function which is called if the content of data object linked to has been changed
        CallbackType* _cb=nullptr;
        // Some constructors
        DataObjectBase() {}
        DataObjectBase(std::string name) : _name(name) {}

    public:
        // Necessary for a base class
        virtual ~DataObjectBase() {}
        // Set the DO name explicitly
        void setName(std::string name) { _name = name; }
        // Get out the DO name for humans
        const std::string& getName() const { return _name; }
        // Link a DO to that DO
        void registerLink(DataObjectBase* ptr) { _ptr = ptr; std::cout << "Link " << ptr->getName() << " to " << getName() << std::endl; }
        // Store a callback function it is called if the content of the DO linked to has been changed
        void registerCallback(CallbackType* cb=nullptr) { _cb = cb; }
        // Access to the DO which is linked to, needed by the reactor
        DataObjectBase* getLinkedDO() { return _ptr; }
        // Access to the registered callback function, needed by the reactor
        void call(DataObjectBase* ptr) { (*_cb)(ptr); }
};

// Template class for arbitrary  content
template <typename T> class DataObject : public DataObjectBase
{
    private:
        // Helper templates to difference locked or not locked accesses
        template <bool B, class M = void>
        using enable_if_t = typename std::enable_if<B, M>::type;

        template <class M, class Enable = void>
        struct mutex
        {
            using type = boost::shared_mutex;
        };

        template <class M>
        struct mutex<M, enable_if_t<std::is_integral<M>::value>>
        {
            using type = boost::null_mutex;
        };

        // Content to handle
        T _content;
        // Make it easier to name it
        using mutex_t = typename mutex<T>::type;
        // Mtable mutex_ member as it needs to be modified in the const member function get()
        mutable mutex_t mutex_;

    public:
        // Some constructors
        DataObject() {}
        DataObject(std::string name) : DataObjectBase(name) {}
        // Necessary if someone want to inherit from that
        virtual ~DataObject() {}

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
        void get(Visitor visitor) const
        {
            // Shared lock to support concurrent access from multiple visitors in different threads
            boost::shared_lock_guard<mutex_t> lock(mutex_);
            visitor(_content);
        }

        // General access to the content by reference
        //T& get() { return _content; }
};

// A simple reactor
class AsynchronousMachine
{
    protected:
        // This should be a at least a list to hold all data objects which content has been changed
        DataObjectBase* _ptr;

    public:
        // Announce the change of content to the reactor
        void trigger(DataObjectBase* ptr) { _ptr = ptr; std::cout << "Trigger DO " << ptr->getName() << std::endl; }
        // Call the registered callback function to notify the DO like DO2.CALL(&DO1)
        void execute() { _ptr->getLinkedDO()->call(_ptr); }
};

// A callback function
void my_cb(DataObjectBase* ptr)
{
    int tmp = 0;

    dynamic_cast<DataObject<int>*>(ptr)->get([&tmp](int i){ tmp = i; });

    std::cout << "Got DO.name: " << ptr->getName() << std::endl;
    std::cout << "Got DO.value: " << tmp << std::endl;
}

// Helper for data access
void fi(int i) {std::cout << i << '\n';}

// Alternate helper for data access
struct cb
{
    void operator() (int i) {std::cout << i << '\n';}
} cbi;

int main(void)
{
    AsynchronousMachine asm1;

    std::string h("Hello");

    DataObject<int> do1;
    DataObject<int> do2("World");

    do1.setName(h);

    std::cout << do1.getName() << std::endl;
    std::cout << do2.getName() << std::endl;

    // Link together: do1<int> -------> do2<int>
    do2.registerCallback(my_cb);
    // Register a callback to call if content of DO1 has been changed
    do1.registerLink(&do2);

    // Access content consistently, wrapper with dummy mutex
    do1.set([](int &i){ i = 1; });
	do1.get(fi);
	do1.set([](int &i){ i = i + 1; });
	do1.get(cbi);

    asm1.trigger(&do1); // Because of changed content of do1

    // Complex DO
    DataObject<std::vector<int>> do3("Vector");
	do3.set([](std::vector<int> &v) {v = std::vector<int>{1, 2, 3};});
	do3.get([](const std::vector<int> &v){ std::cout << v[0] << ',' << v[1] << '\n'; });

    // More complex DO
    int tmp = 0;
	DataObject<std::map<std::string, int>> do4;
	do4.set([](std::map<std::string, int> &v) { v = std::map<std::string, int>{{"1", 42}, {"2", 21}}; });
	do4.get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
	do4.set([](std::map<std::string, int> &v){ v["1"] = v.at("1") + 1; });
	do4.get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
    do4.get([&tmp](const std::map<std::string, int> &v){ tmp = v.at("1") + 2; });
    std::cout << tmp << '\n';

    // Simulate the job of ASM with dDO2.CALL(&DO1)
    // Should notify all callbacks if all DOs linked to
    asm1.execute();

    exit(0);
}

