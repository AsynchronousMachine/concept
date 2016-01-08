#include "asm.hpp"

using namespace Asm;

// Test concept of dataobject and reactor

void my_cb(DataObject<int> &do1, DataObject<double> &do2)
{
    std::cout << "Got DO.name: " << do1.getName() << std::endl;
    std::cout << "My DO.name: " << do2.getName() << std::endl;
    do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
}

void my_cb2(DataObject<int> &do1, DataObject<std::string> &do2)
{
    std::cout << "Got DO.name: " << do1.getName() << std::endl;
    std::cout << "My DO.name: " << do2.getName() << std::endl;
    do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
}

void my_cb3(DataObject<double> &do1, DataObject<std::string> &do2)
{
    std::cout << "Got DO.name: " << do1.getName() << std::endl;
    std::cout << "My DO.name: " << do2.getName() << std::endl;
    do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
}

struct Cb4
{
    void operator() (DataObject<int> &do1, DataObject<std::vector<int>> &do2) const
    {
        std::cout << "Got DO.name: " << do1.getName() << std::endl;
        std::cout << "My DO.name: " << do2.getName() << std::endl;
        int i = do1.get([](int i){ return i; });
        int v = do2.get([&i](const std::vector<int> &v){ return v[i]; });
        std::cout << "Vector element content: " << v << std::endl;
    }
}
my_cb4;

struct Cb5
{
    void operator() (DataObject<int> &do1, DataObject<std::map<std::string, int>> &do2) const
    {
        std::cout << "Got DO.name: " << do1.getName() << std::endl;
        std::cout << "My DO.name: " << do2.getName() << std::endl;
        int i = do1.get([](int i){ return i; });
        int m = do2.get([&i](const std::map<std::string, int> &m){ return m.at(std::to_string(i)); });
        std::cout << "Map element content: " << m << std::endl;
    }
}
my_cb5;

/*
* It is not allowed to give back a reference or a pointer of the content of the dataobject
*
struct Cb6
{
    void operator() (DataObject<int> &do1, DataObject<std::map<std::string, int>> &do2) const
    {
        int i = do1.get([](int i){ return i; });
        const std::map<std::string, int> &m_ref = do2.get([](const std::map<std::string, int> &m){ return m; });
        std::cout << "Map element content: " << m_ref.at(std::to_string(i)) << '\n';
    }
}
my_cb6;
*/

int main(void)
{
    Reactor reactor;

    reactor.init(2);

    DataObject<int> do1("Hello", 9);
    DataObject<double> do2("World");
    DataObject<std::string> do3("World2");

    // Link together: do1<int> -------> do2<double>
    do1.registerLink("DO1->DO2", do2, my_cb);

    // Link together: do1<int> -------> do3<string>
    do1.registerLink("DO1->DO3",do3, my_cb2);

    // Usually now is time to announce the change of this DO to the reactor
    reactor.trigger(do1);
    // Let it run
    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    //reactor.execute();

    // Link together: do1<double> -------> do3<string>
    // Will not compile due to wrong data type of callback parameter
    // do1.registerLink(do3, my_cb3);

    // Access content consistently
    do1.set([](int &i){ i = 7; });

    // Usually now is time to announce the change of this DO to the reactor
    reactor.trigger(do1);
    // Let it run
    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    //reactor.execute();

    // Complex DO
    DataObject<std::vector<int>> do4("Vector");
    do4.set([](std::vector<int> &v) {v = std::vector<int>{1, 2, 3, 4};});
    do1.set([](int &i){ i = 0; });

    // Link together: do1<int> -------> do4<vector>
    do1.registerLink("DO1->DO4", do4, my_cb4);

    // Usually now is time to announce the change of this DO to the reactor
    reactor.trigger(do1);
    // Let it run
    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    //reactor.execute();

    // More complex DO
    DataObject<std::map<std::string, int>> do5("Map", std::map<std::string, int>{{"0", 22}, {"3", 23}, {"4", 24}});
    do5.set([](std::map<std::string, int> &v) { v.insert({{"1", 42}, {"2", 43}});});
    do1.set([](int &i){ i = 1; });

    // Link together: do1<int> -------> do4<map>
    do1.registerLink("DO1->DO5", do5, my_cb5);

    // Usually now is time to announce the change of this DO to the reactor
    reactor.trigger(do1);
    // Let it run
    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    //reactor.execute();

    // Unregister link
    do1.unregisterLink("DO1->DO2");

    // Set a new value
    do1.set([](int &i){ i = 2; });

    // Usually now is time to announce the change of this DO to the reactor
    reactor.trigger(do1);
    // Let it run
    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    //reactor.execute();

    /* Following should show the correct sequence of triggering */
    do1.set([](int &i){ i = 3; });
    DataObject<int> do10("Hallo", 11);
    DataObject<double> do11("Du");
    do10.registerLink("DO10->DO11",do11, my_cb);

    // Usually now is time to announce the change of these DOs to the reactor
    reactor.trigger(do1);
    reactor.trigger(do10);

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    //reactor.execute();

    boost::this_thread::sleep_for(boost::chrono::seconds(10));
    exit(0);
}

