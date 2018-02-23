/*
** Test cases for showing the usage of DataObjects with relation to LINKs.
** The following code samples exclude ser- and deserialization of
** the content of a DataObject
*/

#include <future>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include "../asm/asm.hpp"
#include "../logger/logger.hpp"

constexpr auto maxRepeat = 1000000;

// A dataobject content
class MyLockDOType {
    int _counter;

  public:
    MyLockDOType(int c) : _counter(c) {}

    // The type class of the dataobject knows how to (de)serialize itself
    void serializeMe(rapidjson::Value &, rapidjson::Document::AllocatorType &) { ++_counter; }

    void deserializeMe(rapidjson::Value &) { ++_counter; }

    void operator++() { ++_counter; }

    int get() const { return _counter; }
};

void runDOLocksExamples() {
    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("Run DataObjectLocks dealing samples ..");

    // Dummy types
    rapidjson::Document rjdoc(rapidjson::kObjectType);
    rapidjson::Value rjval;

    // The dataobject instance
    Asm::DataObject<MyLockDOType> doMyLockDOType(1, &MyLockDOType::serializeMe, &MyLockDOType::deserializeMe);

    Logger::pLOG->trace("Counter init value {}", doMyLockDOType.get([](const auto &a) { return a.get(); }));

    //    doMyLockDOType.serialize(rjval, rjdoc.GetAllocator());
    //    doMyLockDOType.deserialize(rjval);
    //    doMyLockDOType.set([](auto &a) { ++a; });

    auto f1 = std::async(std::launch::async, [&] {
        for (auto i = 0; i < maxRepeat; ++i)
            doMyLockDOType.serialize(rjval, rjdoc.GetAllocator());
    });
    auto f2 = std::async(std::launch::async, [&] {
        for (auto i = 0; i < maxRepeat; ++i)
            doMyLockDOType.deserialize(rjval);
    });
    auto f3 = std::async(std::launch::async, [&] {
        for (auto i = 0; i < maxRepeat; ++i)
            doMyLockDOType.set([](auto &a) { ++a; });
    });

    f1.wait();
    f2.wait();
    f3.wait();

    Logger::pLOG->trace("Counter value after concurrency {}", doMyLockDOType.get([](const auto &a) { return a.get(); }));

    Logger::pLOG->trace("===================================================================");
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
