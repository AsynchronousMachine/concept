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

// Template class for arbitrary  content
template <typename T> class DataObject
{
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

        // Make it easier to handle the callback function
        using CallbackType = void(DataObject<T>*);

        // A data object should have a name to identify it by humans
        std::string _name;
        // This should be a at least a simple list to hold all data objects linked to that
        // A mutex for exlusive access will properly also necessary
        std::forward_list<std::function<void(DataObject<T>*)>*> linkedDOs;
        // This is a pointer to a function which is called if the content of data object linked to has been changed
        std::function<CallbackType> _cb;

    public:
        // Some constructors
        DataObject() = default;
        DataObject(std::string name) : _name(name) {}
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
        std::result_of_t<Visitor(T)> get(Visitor visitor) const
        {
            // Shared lock to support concurrent access from multiple visitors in different threads
            boost::shared_lock_guard<mutex_t> lock(mutex_);
            return visitor(_content);
        }

        // Set the DO name explicitly
        void setName(std::string name) { _name = name; }
        // Get out the DO name for humans
        const std::string& getName() const { return _name; }
        // Link a DO to that DO
        template <typename U>
        void registerLink(DataObject<U>* ptr)
        {
            linkedDOs.push_front(&ptr->_cb);
            std::cout << "Link " << ptr->getName() << " to " << getName() << std::endl;
        }
        // Remove a link to a DO by pointer to DO
        template <typename U>
        void unregisterLink(DataObject<U>* ptr) { /*todo*/ }
        // Remove a link to a DO by name
        void unregisterLink(std::string name) { /*todo*/ }
        // Store a callback function it is called if the content of the DO linked to has been changed
        void registerCallback(CallbackType* cb) { _cb = cb; }
        // Called by reactor
        void notify_all()
        {
            std::for_each(linkedDOs.begin(), linkedDOs.end(), [this](std::function<void(DataObject<T>*)> *f){ (*f)(this); });
        }
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

// A callback function
void my_cb(DataObject<int>* ptr)
{
    int tmp = 0;

    ptr->get([&tmp](int i){ tmp = i; });

    std::cout << "Got DO.name: " << ptr->getName() << std::endl;
    std::cout << "Got DO.value: " << tmp << std::endl;
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

int main(void)
{
    AsynchronousMachine asm1;

    std::string h("Hello");

    DataObject<int> do1;
    DataObject<int> do2("World");
    DataObject<int> do3("World2");

    do1.setName(h);

    std::cout << do1.getName() << std::endl;
    std::cout << do2.getName() << std::endl;

    // Link together: do1<int> -------> do2<int>
    do1.registerLink(&do2);
    // Register a callback to call if content of DO1 has been changed
    do2.registerCallback(my_cb);

    // Link together: do1<int> -------> do3<int>
    do1.registerLink(&do3);
    // Register a callback to call if content of DO1 has been changed
    do3.registerCallback(my_cb);

    // Access content consistently, wrapper with dummy mutex
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
	DataObject<std::map<std::string, int>> do5;
	do5.set([](std::map<std::string, int> &v) { v = std::map<std::string, int>{{"1", 42}, {"2", 21}}; });
	do5.get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
	do5.set([](std::map<std::string, int> &v){ v["1"] = v.at("1") + 1; });
	do5.get([](const std::map<std::string, int> &v){ std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
    do5.get([&tmp](const std::map<std::string, int> &v){ tmp = v.at("1") + 2; });
    std::cout << tmp << '\n';

    // Simulates changes of DO content faster than executable inside the reactor
	//do1.set([](int &i){ i = 10; });
	//asm1.trigger(&do1); // Because of changed content of do1

    // Simulate the job of ASM with DO2.CALL(&DO1) and DO3.CALL(&DO1)
    // Should notify all callbacks of all DOs linked to
    asm1.execute();

    exit(0);
}

