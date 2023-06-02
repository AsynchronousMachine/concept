/*
** Test cases for showing the access to DataObjects.
** The following code samples exclude the optional interface for Datalinks
** and ser- and deserialization of the content of a DataObject
*/

#include <string>
#include <atomic>

#include "../logger/logger.hpp"
#include "../asm/asm.hpp"

// Helper for simple DO access
static void fi(int i) {
    Logger::pLOG->trace("{}", i);
}

// Helper for DO access with return value
static int fir(int i) {
    return i + 1;
}

// Alternate helper for DO access, based on a callable class
static struct cb {
    void operator() (int i) {
        Logger::pLOG->trace("{}", i);
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
        Logger::pLOG->trace("ComplexClass default CTOR");
    }
    ComplexClass(int i, int j, std::string s) : inputCounter(i), outputCounter(j), message(s) {
        Logger::pLOG->trace("ComplexClass initializer list CTOR");
    }
    ComplexClass(const ComplexClass& cc) : inputCounter(cc.inputCounter), outputCounter(cc.outputCounter), message(cc.message) {
        Logger::pLOG->trace("ComplexClass copy CTOR");
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
static Asm::DataObject<ComplexStruct> doStruct3(ComplexStruct{});
static Asm::DataObject<ComplexClass> doClass(ComplexClass{ 1, 2, "step1" });
static Asm::DataObject<ComplexClass> doClass2(ComplexClass{});

void runDOAccessExamples() {
    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("Run DataObject handling samples ..");

    Asm::DataObject<ComplexClass> doClass1(ComplexClass{ 1, 2, "step1" }); // Will use DataObject(D content)
    doClass1.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    Asm::DataObject<ComplexClass> doClass2(ComplexClass{}); // Will use DataObject(D content)
    doClass2.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    Asm::DataObject<ComplexClass> doClass3(*new ComplexClass({ 3, 4, "step2" })); // Will use DataObject(D content)
    doClass3.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    Asm::DataObject<ComplexClass> doClass4(*new ComplexClass); // Will use DataObject(D content)
    doClass4.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    Asm::DataObject<ComplexClass> doClass5(ComplexClass({})); // will use DataObject(D content)
    doClass5.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    Asm::DataObject<ComplexClass> doClass6(*new ComplexClass({})); // will use DataObject(D content)
    doClass6.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    Asm::DataObject<ComplexClass> doClass7; // will use DataObject()
    doClass7.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    Asm::DataObject<ComplexClass> doClass8{}; // will use DataObject()
    doClass8.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
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
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    doClass2.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    doClass3.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    doClass4.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    doClass5.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    doClass6.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    doClass7.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    doClass8.get([](const ComplexClass& c) {
        Logger::pLOG->trace("inputcounter: {}", c.get_ic());
    });

    // Access content consistently
    Logger::pLOG->trace("DO simple examples...");

    doInt.set([](std::atomic<int> &i) {
        i = 1; // Refer to, std::atomic::operator=
    });
    doInt.get(fi);

    doInt.set([](std::atomic<int> &i) {
        ++i; // Refer to, std::atomic::operator++
    });
    doInt.get(cbi);

    Logger::pLOG->trace("{}", doInt.get(fir));

    // Complex DO
    Logger::pLOG->trace("DO vector examples...");

    // Will not compile due to difficulties to select the correct ctor
    // Asm::DataObject<std::vector<int>> doVector3({});
    // Use this as solution for an empty Vector
    Asm::DataObject<std::vector<int>> doVector(std::vector<int> {});
    doVector.set([](std::vector<int> &v) {
        v = std::vector<int> { 1, 2, 3 };
    });

    doVector.get([](const std::vector<int> &v) {
        Logger::pLOG->trace("{} , {}", v[0], v[1]);
    });

    Asm::DataObject<std::vector<int>> doVector2({ 1, 2 });

    // More complex DO
    Logger::pLOG->trace("DO map examples...");

    int tmp = 0;
    Asm::DataObject<std::map<std::string, int>> doMap(std::map<std::string, int> { {"3", 23}, { "4", 24 }});
    doMap.get([](const std::map<std::string, int> &v) {
        for (auto& m : v) {
            Logger::pLOG->trace("{} has value {}", m.first, m.second);
        }
    });

    doMap.set([](std::map<std::string, int> &v) {
        v = std::map<std::string, int> { { "1", 42 },{ "2", 43 } };
    });

    doMap.get([](const std::map<std::string, int> &v) {
        Logger::pLOG->trace("{} , {}", v.at("1"), v.at("2"));
    });

    doMap.set([](std::map<std::string, int> &v) {
        v["1"] = v.at("1") + 1;
    });

    doMap.get([](const std::map<std::string, int> &v) {
        Logger::pLOG->trace("{} , {}", v.at("1"), v.at("2"));
    });

    doMap.get([&tmp](const std::map<std::string, int> &v) {
        tmp = v.at("1") + 2;
    });

    Logger::pLOG->trace("{}", tmp);
    doMap.get([](const std::map<std::string, int> &v) {
        for (auto& m : v) {
            Logger::pLOG->trace("{} has value {}", m.first, m.second);
        }
    });

    doMap.set([](std::map<std::string, int> &v) {
        v["1"] = v.at("1") + 3;
    });

    tmp = doMap.get([](const std::map<std::string, int> &v) {
        return (v.at("1") + 2);
    });

    Logger::pLOG->trace("{}", tmp);

    // DO as none standard type
    Logger::pLOG->trace("DO struct/class examples...");

    Asm::DataObject<ComplexStruct> doComplexStruct({ 1, "step1" });
    doComplexStruct.get([](const ComplexStruct &m) {
        Logger::pLOG->trace("{} has value {}", m.i, m.s);
    });

    doComplexStruct.set([](ComplexStruct &m) {
        m.i = 2;
        m.s = "step2";
    });

    ComplexStruct cs;
    doComplexStruct.get([&cs](const ComplexStruct &m) {
        cs = m;
    });

    Logger::pLOG->trace("{} has value {}", cs.i, cs.s);
    doComplexStruct.set([](ComplexStruct &m) {
        m.i = 3;
        m.s = "step3";
    });

    cs = doComplexStruct.get([](const ComplexStruct &m) {
        return m;
    });

    Logger::pLOG->trace("{} has value {}", cs.i, cs.s);

    doClass.get([](const ComplexClass &m) {
        Logger::pLOG->trace("{} has value {}", m.get_ic(), m.get_m());
    });

    doClass.set([](ComplexClass &m) {
        m.set_ic(2);
        m.set_m("step2");
    });

    ComplexClass cc;
    doClass.get([&cc](const ComplexClass &m) {
        cc = m;
    });

    Logger::pLOG->trace("{} has value {}", cc.get_ic(), cc.get_m());

    doClass.set([](ComplexClass &m) {
        m.set_ic(3);
        m.set_m("step3");
    });

    cc = doClass.get([](const ComplexClass &m) {
        return m;
    });

    Logger::pLOG->trace("{} has value {}", cc.get_ic(), cc.get_m());

    // Possible misuse by return of address of reference
    Logger::pLOG->trace("Misuse examples...");

    doClass.set([](ComplexClass &m) {
        m.set_ic(100);
    });

    ComplexClass* m =doClass.get([](const ComplexClass& m) {
        return &const_cast<ComplexClass&>(m);
    });

    Logger::pLOG->trace("Value before: {}", m->get_ic());

    m->set_ic(200);
    ComplexClass m2 = doClass.get([](ComplexClass complexClass) {
        return complexClass;
    });

    Logger::pLOG->trace("Misused by return of address of reference {}", m2.get_ic());

    Asm::DataObject<int> do71(31);
    std::atomic<int> *tmp51 = do71.get([](const std::atomic<int> &i) {
        return &const_cast<std::atomic<int>&>(i);
    });

    Logger::pLOG->trace("Value before: {}", tmp51->load());
    *tmp51 = 32;
    do71.get([](const int i) {
        Logger::pLOG->trace("Misused by return of address of reference {}", i);
    });

    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("Run atomic tests ..");

    int tmp_atomic = doInt.get([](const std::atomic_int& i) {
        return i.load();
    });

    Logger::pLOG->trace("tmp_atomic actual value {}", tmp_atomic);

    tmp_atomic = doInt.get([](const int i) {
        return i;
    });

    Logger::pLOG->trace("tmp_atomic actual value with implizit cast {}", tmp_atomic);

    doInt.set([](std::atomic_int& i) {
        ++i;
    });

    doInt.get([&tmp_atomic](const int i) {
        tmp_atomic = i;
    });

    Logger::pLOG->trace("tmp_atomic actual value after atomic ++ with overloaded operator {}", tmp_atomic);

    doInt.set([](std::atomic_int& i) {
        //i.store(i.load() + 21); // Do not do this in that way
        i.fetch_add(21); // Does everything atomic, the right way
    });

    Logger::pLOG->trace("Actual value after atomic fetch_add {}", doInt.get([](const int i) { return i; }));

    doInt.set([](std::atomic_int& i) {
        //i.store(i.load() + 21); // Do not do this in that way
        i += 21; // Does everything atomic, the right way with overloaded operator
    });

    Logger::pLOG->trace("Actual value after atomic += with overloaded operator {}", doInt.get([](const int i) { return i; }));

    // Atomic long long handling
    Asm::DataObject<int64_t> do72{-8000000000};
    do72.set([](std::atomic_int64_t& a) { ++a; });
    Logger::pLOG->trace("Actual value after atomic ++ with overloaded operator {}", do72.get([](const int64_t i) { return i; }));

    Asm::DataObject<uint64_t> do73{9000000000};
    do73.set([](std::atomic_uint64_t& a) { --a; });
    Logger::pLOG->trace("Actual value after atomic -- with overloaded operator {}", do73.get([](const uint64_t i) { return i; }));


    Logger::pLOG->trace("===================================================================");
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
