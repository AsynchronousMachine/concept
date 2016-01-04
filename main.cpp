#include "dataobject.hpp"
#include "reactor.hpp"

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
/*
struct Cb4
{
    void operator() (DataObject<int> &do1, DataObject<std::vector<int>> &do2)
    {
        int i = do1.get([](int i){ return i; });
        int v = do2.get([&i](const std::vector<int> &v){ return v[i]; });
        std::cout << "Vector element content: " << v << '\n';
    }
}
my_cb4;
*/
void my_cb4(DataObject<int> &do1, DataObject<std::vector<int>> &do2)
    {
        int i = do1.get([](int i){ return i; });
        int v = do2.get([&i](const std::vector<int> &v){ return v[i]; });
        std::cout << "Vector element content: " << v << '\n';
    }
/*
struct Cb5
{
    void operator() (DataObject<int> &do1, DataObject<std::map<std::string, int>> &do2)
    {
        int i = do1.get([](int i){ return i; });
        int m = do2.get([&i](const std::map<std::string, int> &m){ std::string s = std::to_string(i); return m.at(s); });
        std::cout << "Map element content: " << m << '\n';
    }
}
my_cb5;
*/
    void my_cb5(DataObject<int> &do1, DataObject<std::map<std::string, int>> &do2)
    {
        int i = do1.get([](int i){ return i; });
        int m = do2.get([&i](const std::map<std::string, int> &m){ std::string s = std::to_string(i); return m.at(s); });
        std::cout << "Map element content: " << m << '\n';
    }

int main(void)
{
    Reactor reactor;

    DataObject<int> do1("Hello", 9);
    DataObject<double> do2("World");
    DataObject<std::string> do3("World2");

    // Link together: do1<int> -------> do2<double>
    do1.registerLink(do2, my_cb);

    // Link together: do1<int> -------> do3<string>
    do1.registerLink(do3, my_cb2);

    // Link together: do1<double> -------> do3<string>
    // Will not compile due to wrong data type of callback parameter
    // do1.registerLink(do3, my_cb3);

    // Access content consistently
    do1.set([](int &i){ i = 7; });

    // Usually now is time to announce the change of this DO to the reactor
    reactor.trigger(do1);

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    reactor.execute();

    // Complex DO
    DataObject<std::vector<int>> do4("Vector");
    do4.set([](std::vector<int> &v) {v = std::vector<int>{1, 2, 3};});
    do1.set([](int &i){ i = 0; });

    // Link together: do1<int> -------> do4<vector>
    do1.registerLink(do4, my_cb4);

    // Usually now is time to announce the change of this DO to the reactor
    reactor.trigger(do1);

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    reactor.execute();

    // More complex DO
    DataObject<std::map<std::string, int>> do5("Map", std::map<std::string, int>{{"3", 23}, {"4", 24}});
    do5.set([](std::map<std::string, int> &v) { v = std::map<std::string, int>{{"1", 42}, {"2", 43}}; });
    do1.set([](int &i){ i = 1; });

    // Link together: do1<int> -------> do4<map>
    do1.registerLink(do5, my_cb5);

    // Usually now is time to announce the change of this DO to the reactor
    reactor.trigger(do1);

    // Simulate the job of reactor, typically inside a thread related with a prioritiy
    // Should notify all callbacks of all DOs which have been triggered
    reactor.execute();

    exit(0);
}

