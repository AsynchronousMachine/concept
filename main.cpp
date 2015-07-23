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
#include <stdexcept>
#include <typeinfo>
#include <utility>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/any_cast.hpp>
#include <boost/type_erasure/member.hpp>
#include <boost/type_erasure/placeholder.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/typeid_of.hpp>
#include <boost/any.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

BOOST_TYPE_ERASURE_MEMBER((has_getName), getName, 0)
BOOST_TYPE_ERASURE_MEMBER((has_set), set, 1)

// DOs support RTTI, have a member function getName() and are passed by reference
using data_object_type = boost::type_erasure::any<
    boost::mpl::vector<
        boost::type_erasure::typeid_<>,
        has_getName<const std::string&()>,
        has_set<void(const boost::any&)>
    >,
    boost::type_erasure::_self&
>;

BOOST_TYPE_ERASURE_MEMBER((has_registerDOs), registerDOs, 2)

// Links are copy-constructible and have the member functions getName() and registerDOs()
using link_type = boost::type_erasure::any<
    boost::mpl::vector<
        boost::type_erasure::copy_constructible<>,
        has_getName<const std::string&()>,
        has_registerDOs<void(data_object_type, data_object_type)>
    >
>;

BOOST_TYPE_ERASURE_MEMBER((has_getDataObjects), getDataObjects, 0)
BOOST_TYPE_ERASURE_MEMBER((has_getLinks), getLinks, 0)

// Modules have the member functions getName(), getDataObjects() and getLinks() and are passed by reference
struct module_type : public boost::type_erasure::any<
    boost::mpl::vector<
        has_getName<const std::string&()>,
        has_getDataObjects<std::vector<data_object_type>()>,
        has_getLinks<std::vector<link_type>()>
    >,
    boost::type_erasure::_self&
>
{
    using base = boost::type_erasure::any<boost::mpl::vector<has_getName<const std::string&()>, has_getDataObjects<std::vector<data_object_type>()>, has_getLinks<std::vector<link_type>()>>, boost::type_erasure::_self&>;
    using base::base;
    const std::string &getName() const { return base::getName(); }
};

using modules_type = boost::multi_index::multi_index_container<
    module_type,
    boost::multi_index::indexed_by<
        boost::multi_index::hashed_unique<
            boost::multi_index::const_mem_fun<
                module_type, const std::string&, &module_type::getName
            >
        >
    >
>;

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

        void set(const boost::any &value)
        {
            // Exclusive lock for write access
            boost::lock_guard<mutex_t> lock(mutex_);
            _content = boost::any_cast<T>(value);
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

template <class DO1Type, class DO2Type>
class Link
{
    public:
        using callback_type = std::function<void(DO1Type&, DO2Type&)>;

    private:
        std::string _name;
        callback_type _callback;

    public:
        Link(std::string name, callback_type callback) : _name(std::move(name)), _callback(std::move(callback)) {}

        const std::string& getName() const { return _name; }

        void registerDOs(data_object_type do1, data_object_type do2)
        {
            DO1Type &d1 = boost::type_erasure::any_cast<DO1Type&>(do1); // throws boost::bad_any_cast if casting fails
            DO2Type &d2 = boost::type_erasure::any_cast<DO2Type&>(do2); // throws boost::bad_any_cast if casting fails
            d1.registerLink(d2, _callback);
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

        const std::string& getName() const { return _name; }

        std::vector<data_object_type> getDataObjects()
        {
            return std::vector<data_object_type>{do1, do2};
        }

        std::vector<link_type> getLinks()
        {
            return std::vector<link_type>{};
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

        const std::string& getName() const { return _name; }

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

        std::vector<data_object_type> getDataObjects()
        {
            return std::vector<data_object_type>{do1, do2};
        }

        std::vector<link_type> getLinks()
        {
            return std::vector<link_type>{
                Link<DataObject<int>, DataObject<std::string>>("Link1", [this](DataObject<int> &do1, DataObject<std::string> &do2) { Link1(do1, do2); }),
                Link<DataObject<int>, DataObject<int>>("Link2", [this](DataObject<int> &do1, DataObject<int> &do2) { Link2(do1, do2); })
            };
        }
};

modules_type modules;

void link(std::string do1, std::string do2, std::string l)
{
    auto idx = do1.find('.');
    std::string module1 = do1.substr(0, idx);
    std::string module1do = do1.substr(idx + 1);

    idx = do2.find('.');
    std::string module2 = do2.substr(0, idx);
    std::string module2do = do2.substr(idx + 1);

    idx = l.find('.');
    std::string module3 = l.substr(0, idx);
    std::string module3link = l.substr(idx + 1);

    auto modit = modules.find(module1);
    if (modit == modules.end())
        throw std::runtime_error("unknown module " + module1);
    module_type mod1 = *modit;

    modit = modules.find(module2);
    if (modit == modules.end())
        throw std::runtime_error("unknown module " + module2);
    module_type mod2 = *modit;

    modit = modules.find(module3);
    if (modit == modules.end())
        throw std::runtime_error("unknown module " + module3);
    module_type mod3 = *modit;

    auto dataObjects = mod1.getDataObjects();
    auto doit = std::find_if(dataObjects.begin(), dataObjects.end(), [module1do](auto &d){ return d.getName() == module1do; });
    if (doit == dataObjects.end())
        throw std::runtime_error("unknown data object " + module1do);
    data_object_type d1 = *doit;

    dataObjects = mod2.getDataObjects();
    doit = std::find_if(dataObjects.begin(), dataObjects.end(), [module2do](auto &d){ return d.getName() == module2do; });
    if (doit == dataObjects.end())
        throw std::runtime_error("unknown data object " + module2do);
    data_object_type d2 = *doit;

    auto links = mod3.getLinks();
    auto linkit = std::find_if(links.begin(), links.end(), [module3link](auto &l){ return l.getName() == module3link; });
    if (linkit == links.end())
        throw std::runtime_error("unknown link " + module3link);
    link_type ll = *linkit;

    ll.registerDOs(d1, d2);
}

void set(std::string do1, boost::any value)
{
    auto idx = do1.find('.');
    std::string module1 = do1.substr(0, idx);
    std::string module1do = do1.substr(idx + 1);

    auto modit = modules.find(module1);
    if (modit == modules.end())
        throw std::runtime_error("unknown module " + module1);
    module_type mod1 = *modit;

    auto dataObjects = mod1.getDataObjects();
    auto doit = std::find_if(dataObjects.begin(), dataObjects.end(), [module1do](auto &d){ return d.getName() == module1do; });
    if (doit == dataObjects.end())
        throw std::runtime_error("unknown data object " + module1do);
    data_object_type d1 = *doit;

    d1.set(value);
}

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

    modules.insert(Hello);
    modules.insert(World);

    try
    {
        link("Hello.DO1", "World.DO2", "World.Link1");
        set("Hello.DO1", 10);
    }
    catch (std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
        std::exit(1);
    }

    asm1.trigger(&Hello.do1); // Because of changed content of do1

    // Simulate the job of ASM
    // Should notify all callbacks of all DOs linked to
    asm1.execute();

    exit(0);
}

