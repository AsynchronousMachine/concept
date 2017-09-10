/*
** Test cases for showing the usage of DataObjects and their
** relation to the DataObjectReactor.
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include <string>
#include <atomic>

#include "../logger/logger.hpp"
#include "../asm/asm.hpp"

static struct CustomStruct {
    int i;
} cs;

class CustomClass {
  public:
    int inputCounter;
};

void runDOReactorExamples() {
    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("Run DataObjectReactor test ..");

    Asm::DataObject<int> doExInt(10);
    Asm::DataObject<CustomStruct> doExStruct(cs);
    Asm::DataObject<CustomClass> doExClass(CustomClass{ 1 });

    doExInt.set([](std::atomic<int> &i) { i = 123; });

    doExStruct.set([](CustomStruct& m) { m.i = 456; });

    doExClass.set([](CustomClass &m) { m.inputCounter = 789; });

    // this shall be executed after [doExInt->doExClass]
    doExInt.registerLink("doExInt->doExStruct", doExStruct, [](Asm::DataObject<int>& triggeredDoInt, Asm::DataObject<CustomStruct>& triggeredDoStruct) {
        int doExIntVal = triggeredDoInt.get([](int i) { return i; });
        const CustomStruct doExStructVal = triggeredDoStruct.get([](const CustomStruct& customStruct) { return customStruct; });
        Logger::pLOG->trace("Triggered link [doExInt->doExStruct] with values doExInt: {}, doExStruct.i: {}", doExIntVal, doExStructVal.i);
#ifdef __linux__
        Logger::pLOG->trace("Use TID-{} for datalink test1", syscall(SYS_gettid));
#endif
    });

    Asm::pDOR->trigger(doExInt);
    boost::this_thread::sleep_for(boost::chrono::seconds(3));
    doExInt.unregisterLink("doExInt->doExStruct");

    doExInt.set([](std::atomic<int> &i) { i = 321; }); // Notice that in [doExInt->doExStruct] value will be updated!

    doExInt.registerLink("doExInt->doExClass", doExClass, [](Asm::DataObject<int>& triggeredDoInt, Asm::DataObject<CustomClass>& triggeredDoClass) {
        int doExIntVal = triggeredDoInt.get([](int i) { return i; });
        const CustomClass doExClassVal = triggeredDoClass.get([](const CustomClass& complexClass) { return complexClass; });
        Logger::pLOG->trace("Triggered link [doExInt->doExClass] with values doExInt: {}, doExClass.inputCounter: {}", doExIntVal, doExClassVal.inputCounter);

#ifdef __linux__
        Logger::pLOG->trace("Use TID-{} for datalink test2", syscall(SYS_gettid));
#endif
    });

    Asm::pDOR->trigger(doExInt);
    boost::this_thread::sleep_for(boost::chrono::seconds(3)); // wait for links to be executed

    Logger::pLOG->trace("===================================================================");
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
