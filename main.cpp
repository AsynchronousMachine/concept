#include <atomic>

#include "dataobject.hpp"

// Test concept of data object

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
}
cbi;

struct MySpecial
{
    int i;
    std::string s;
};

int main(void)
{
    DataObject<int> do1("Hello", 9);
    DataObject<double> do2("World");
    DataObject<std::string> do3("World2");

    std::cout << do1.getName() << std::endl;
    std::cout << do2.getName() << std::endl;
    std::cout << do3.getName() << std::endl;

    // Link together: do1<int> -------> do2<double>
    do1.registerLink(do2, my_cb);

    // Link together: do1<int> -------> do3<string>
    do1.registerLink(do3, my_cb2);

    // Link together: do1<double> -------> do3<string>
    // Will not compile due to wrong data type of callback parameter
    // do1.registerLink(do3, my_cb3);

    // Access content consistently within held dataobject mutex
    do1.get(fi);
    do1.set([](int &i){ i = 1; });
    do1.get(fi);
    do1.set([](int &i){ i = i + 1; });
    do1.get(cbi);
    std::cout << do1.get(fir) << '\n';

    // Usually now is time to announce the change of this DO
    do1.notify_all();

    // Complex DO
    DataObject<std::vector<int>> do4("Vector");
    do4.set([](std::vector<int> &v) {v = std::vector<int>{1, 2, 3};});
    do4.get([](const std::vector<int> &v){ std::cout << v[0] << ',' << v[1] << '\n'; });

    // Usually now is time to announce the change of this DO
    do4.notify_all();

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
    do5.set([](std::map<std::string, int> &v){ v["1"] = v.at("1") + 3; });
    tmp = do5.get([](const std::map<std::string, int> &v){ return (v.at("1") + 2); });
    std::cout << tmp << '\n';

    // Usually now is time to announce the change of this DO
    do5.notify_all();

    // DO as none standard type
    DataObject<MySpecial> do6("MySpecial", {1, "step1"});
    do6.get([](const MySpecial &m){std::cout << m.i << " has value " << m.s << '\n';});
    do6.set([](MySpecial &m){m.i = 2; m.s = "step2";});
    MySpecial ms;
    do6.get([&ms](const MySpecial &m){ms = m;});
    std::cout << ms.i << " has value " << ms.s << '\n';
    do6.set([](MySpecial &m){m.i = 3; m.s = "step3";});
    ms = do6.get([](const MySpecial &m){return m;});
    std::cout << ms.i << " has value " << ms.s << '\n';

    // Usually now is time to announce the change of this DO
    do6.notify_all();

    // Possible misuse by return of address of reference
    DataObject<int> do7("MisuseTest1", 3);
    const int *tmp5 = do7.get([](const int &i) {return &i;});
    std::cout << "*tmp5 " << *tmp5 << std::endl;
    *const_cast<int*>(tmp5) = 4; // Misuse
    do7.get([](const int i) {std::cout << "Misused by return of address of reference " << i << std::endl;});

    // Possible misuse by return of reference
    DataObject<int> do8("MisuseTest2", 5);
    do8.get([](const int &i) {int& tmp = const_cast<int&>(i); tmp = 6;}); // Misuse
    do8.get([](const int i) {std::cout << "Misused by return of reference " << i << std::endl;});

    // Possible misuse by return of reference ????
    DataObject<int> do9("MisuseTest3", 7);
    const int &tmp6 = do9.get([](const int &i) {return i;});
    const_cast<int&>(tmp6) = 8; // Misuse ????
    do9.get([](const int i) {std::cout << "Misused by return of reference " << i << std::endl;});

    // Atomic Tests, atomic operations are still done within held dataobject mutex
    int tmp_atomic = 0;
    DataObject<std::atomic_int> do13("AtomicTest");
    do13.set([](std::atomic_int& i){i = 4;});
    tmp_atomic = do13.get([](const std::atomic_int& i) {return std::atomic_load(&i);});
    //do13.get([&tmp_atomic](const std::atomic_int& i) {tmp_atomic = std::atomic_load(&i);});
    std::cout << "tmp_atomic load init " << tmp_atomic << std::endl;
    do13.set([](std::atomic_int& i){++i;});
    //do13.get([&tmp_atomic](const std::atomic_int& i) {tmp_atomic = std::atomic_load(&i);});
    tmp_atomic = do13.get([](const std::atomic_int& i) {return std::atomic_load(&i);});
    std::cout << "tmp_atomic load after inc " << tmp_atomic << std::endl;

    exit(0);
}

