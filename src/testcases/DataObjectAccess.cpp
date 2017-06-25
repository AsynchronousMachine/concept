/*
** Test cases for showing almost all aspects of dealing with DataObjects.
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include <string>
#include <atomic>

#include "../asm/asm.hpp"

// Helper for simple DO access
static void fi(int i) {
    std::cout << i << std::endl;
}

// Helper for DO access with return value
static int fir(int i) {
    return i + 1;
}

// Alternate helper for DO access, based on a callable class
static struct cb {
    void operator() (int i) {
        std::cout << i << std::endl;
    }
}
cbi;

// Custom defined complex data type for a DataObject
struct ComplexStruct {
    int i;
    std::string s;
};

class ComplexClass {
    int inputCounter;
    int outputCounter;
    std::string message;

  public:
    ComplexClass() : inputCounter(-1), outputCounter(-2), message("---") {
        std::cout << "ComplexClass default CTOR" << std::endl;
    }
    ComplexClass(int i, int j, std::string s) : inputCounter(i), outputCounter(j), message(s) {
        std::cout << "ComplexClass initializer list CTOR" << std::endl;
    }
    ComplexClass(const ComplexClass& cc) : inputCounter(cc.inputCounter), outputCounter(cc.outputCounter), message(cc.message) {
        std::cout << "ComplexClass copy CTOR" << std::endl;
    }

    int get_ic() const {
        return inputCounter;
    }
    int get_oc() const {
        return outputCounter;
    }
    std::string get_m() const {
        return message;
    }

    // This is the boring standard feature
    // void set_ic(int i) { inputCounter = i; }
    void set_ic(int& i) {
        inputCounter = i;
    }
    void set_ic(int&& i) {
        inputCounter = i;
    }
    void set_oc(int& i) {
        outputCounter = i;
    }
    void set_m(std::string& m) {
        message = m;
    }
    void set_m(std::string&& m) {
        message = m;
    }
};

// Sample for creating instances of variant DataObject types, not stack based
static ComplexStruct csInst;
static Asm::DataObject<int> doInt(10);
static Asm::DataObject<std::string> doString("11");
static Asm::DataObject<ComplexStruct> doStruct(csInst);
static Asm::DataObject<ComplexStruct> doStruct2(ComplexStruct());
static Asm::DataObject<ComplexStruct> doStruct3(ComplexStruct{});
static Asm::DataObject<ComplexClass> doClass(ComplexClass{ 1, 2, "step1" });
static Asm::DataObject<ComplexClass> doClass2(ComplexClass{});
static Asm::DataObject<ComplexClass> doClass3(ComplexClass());

void runDOAccessExamples() {
    std::cout << "===================================================================" << std::endl;
    std::cout << "Run DataObject handling samples .." << std::endl;

    Asm::DataObject<ComplexClass> doClass1(ComplexClass{ 1, 2, "step1" }); // Will use DataObject(D content)
    doClass1.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    Asm::DataObject<ComplexClass> doClass2(ComplexClass{}); // Will use DataObject(D content)
    doClass2.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    Asm::DataObject<ComplexClass> doClass3(*new ComplexClass({ 3, 4, "step2" })); // Will use DataObject(D content)
    doClass3.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    Asm::DataObject<ComplexClass> doClass4(*new ComplexClass); // Will use DataObject(D content)
    doClass4.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    Asm::DataObject<ComplexClass> doClass5(ComplexClass({})); // will use DataObject(D content)
    doClass5.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    Asm::DataObject<ComplexClass> doClass6(*new ComplexClass({})); // will use DataObject(D content)
    doClass6.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    Asm::DataObject<ComplexClass> doClass7; // will use DataObject()
    doClass7.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    Asm::DataObject<ComplexClass> doClass8{}; // will use DataObject()
    doClass8.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    doClass1.set([](ComplexClass& c) {
        c.set_ic(1);
    });
    doClass2.set([](ComplexClass& c) {
        c.set_ic(2);
    });
    doClass3.set([](ComplexClass& c) {
        c.set_ic(3);
    });
    doClass4.set([](ComplexClass& c) {
        c.set_ic(4);
    });
    doClass5.set([](ComplexClass& c) {
        c.set_ic(5);
    });
    doClass6.set([](ComplexClass& c) {
        c.set_ic(6);
    });
    doClass7.set([](ComplexClass& c) {
        c.set_ic(7);
    });
    doClass8.set([](ComplexClass& c) {
        c.set_ic(8);
    });

    doClass1.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });
    doClass2.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });
    doClass3.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });
    doClass4.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });
    doClass5.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });
    doClass6.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });
    doClass7.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });
    doClass8.get([](const ComplexClass& c) {
        std::cout << "inputcounter: " << c.get_ic() << std::endl;
    });

    // Access content consistently
    std::cout << "DO simple examples..." << std::endl;

    doInt.get(fi);
    doInt.set([](std::atomic<int> &i) {
        i = 1;
    });
    doInt.get(fi);
    doInt.set([](std::atomic<int> &i) {
        i = i + 1;
    });
    doInt.get(cbi);
    std::cout << doInt.get(fir) << '\n';

    // Complex DO
    std::cout << "DO vector examples..." << std::endl;

    // Will not compile due to difficulties to select the correct ctor
    // Asm::DataObject<std::vector<int>> doVector3({});
    // Use this as solution for an empty Vector
    Asm::DataObject<std::vector<int>> doVector(std::vector<int> {});
    doVector.set([](std::vector<int> &v) {
        v = std::vector<int> { 1, 2, 3 };
    });
    doVector.get([](const std::vector<int> &v) {
        std::cout << v[0] << ',' << v[1] << '\n';
    });

    Asm::DataObject<std::vector<int>> doVector2({ 1, 2 });

    // More complex DO
    std::cout << "DO map examples..." << std::endl;

    int tmp = 0;
    Asm::DataObject<std::map<std::string, int>> doMap(std::map<std::string, int> { {"3", 23}, { "4", 24 }});
    doMap.get([](const std::map<std::string, int> &v) {
        for (auto& m : v) {
            std::cout << m.first << " has value " << m.second << '\n';
        }
    });
    doMap.set([](std::map<std::string, int> &v) {
        v = std::map<std::string, int> { { "1", 42 },{ "2", 43 } };
    });
    doMap.get([](const std::map<std::string, int> &v) {
        std::cout << v.at("1") << ',' << v.at("2") << '\n';
    });
    doMap.set([](std::map<std::string, int> &v) {
        v["1"] = v.at("1") + 1;
    });
    doMap.get([](const std::map<std::string, int> &v) {
        std::cout << v.at("1") << ',' << v.at("2") << '\n';
    });
    doMap.get([&tmp](const std::map<std::string, int> &v) {
        tmp = v.at("1") + 2;
    });
    std::cout << tmp << '\n';
    doMap.get([](const std::map<std::string, int> &v) {
        for (auto& m : v) {
            std::cout << m.first << " has value " << m.second << '\n';
        }
    });
    doMap.set([](std::map<std::string, int> &v) {
        v["1"] = v.at("1") + 3;
    });
    tmp = doMap.get([](const std::map<std::string, int> &v) {
        return (v.at("1") + 2);
    });
    std::cout << tmp << '\n';

    // DO as none standard type
    std::cout << "DO struct/class examples..." << std::endl;

    Asm::DataObject<ComplexStruct> doComplexStruct({ 1, "step1" });
    doComplexStruct.get([](const ComplexStruct &m) {
        std::cout << m.i << " has value " << m.s << '\n';
    });
    doComplexStruct.set([](ComplexStruct &m) {
        m.i = 2;
        m.s = "step2";
    });
    ComplexStruct cs;
    doComplexStruct.get([&cs](const ComplexStruct &m) {
        cs = m;
    });
    std::cout << cs.i << " has value " << cs.s << '\n';
    doComplexStruct.set([](ComplexStruct &m) {
        m.i = 3;
        m.s = "step3";
    });
    cs = doComplexStruct.get([](const ComplexStruct &m) {
        return m;
    });
    std::cout << cs.i << " has value " << cs.s << '\n';

    doClass.get([](const ComplexClass &m) {
        std::cout << m.get_ic() << " has value " << m.get_m() << '\n';
    });
    doClass.set([](ComplexClass &m) {
        m.set_ic(2);
        m.set_m("step2");
    });
    ComplexClass cc;
    doClass.get([&cc](const ComplexClass &m) {
        cc = m;
    });
    std::cout << cc.get_ic() << " has value " << cc.get_m() << '\n';
    doClass.set([](ComplexClass &m) {
        m.set_ic(3);
        m.set_m("step3");
    });
    cc = doClass.get([](const ComplexClass &m) {
        return m;
    });
    std::cout << cc.get_ic() << " has value " << cc.get_m() << '\n';


    // Possible misuse by return of address of reference
    std::cout << "Misuse examples..." << std::endl;

    doClass.set([](ComplexClass &m) {
        m.set_ic(100);
    });
    ComplexClass* m =doClass.get([](const ComplexClass& m) {
        return &const_cast<ComplexClass&>(m);
    });
    std::cout << "Value before: " << m->get_ic() << std::endl;
    m->set_ic(200);
    ComplexClass m2 = doClass.get([](ComplexClass complexClass) {
        return complexClass;
    });
    std::cout <<  "Misused by return of address of reference " << m2.get_ic() << std::endl;

    Asm::DataObject<int> do71(31);
    std::atomic<int> *tmp51 = do71.get([](const std::atomic<int> &i) {    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
        return &const_cast<std::atomic<int>&>(i);
    });
    std::cout << "Value before: " << *tmp51 << std::endl;
    *tmp51 = 32;
    do71.get([](const int i) {
        std::cout << "Misused by return of address of reference " << i << std::endl;
    });

    std::cout << "Atomic tests..." << std::endl;

    int tmp_atomic = 0;
    tmp_atomic = doInt.get([](const std::atomic_int& i) {
        return std::atomic_load(&i);
    });
    std::cout << "tmp_atomic actual value " << tmp_atomic << std::endl;
    tmp_atomic = doInt.get([](const int i) {
        return i;
    });
    std::cout << "tmp_atomic actual value with implizit cast " << tmp_atomic << std::endl;
    doInt.set([](std::atomic_int& i) {
        ++i;
    });
    doInt.get([&tmp_atomic](const int i) {
        tmp_atomic = i;
    });
    std::cout << "tmp_atomic actual value after atomic inc with overloaded operator " << tmp_atomic << std::endl;

    // Posible but should only be used if default ctor has been implemented
    Asm::DataObject<int> do72;

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
