/*
** Test cases for showing the usage of DataObjects and their
** relation to the DataObjectReactor.
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include <string>
#include <atomic>

#include "../asm/asm.hpp"

static struct CustomStruct {
    int i;
} cs;

class CustomClass {
  public:
    int inputCounter;
};

void runDOReactorExamples() {
    std::cout << "===================================================================" << std::endl;
    std::cout << "Run DataObjectReactor test .." << std::endl;

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
        std::cout << "Triggered link [doExInt->doExStruct] with values doExInt: " << doExIntVal << ", doExStruct.i: " << doExStructVal.i << std::endl;
#ifdef __linux__
        std::cout << "Use TID-" << syscall(SYS_gettid) << " for datalink test1" << std::endl;
#endif
    });

    Asm::pDOR->trigger(doExInt);
    boost::this_thread::sleep_for(boost::chrono::seconds(3));
    doExInt.unregisterLink("doExInt->doExStruct");

    doExInt.set([](std::atomic<int> &i) { i = 321; }); // Notice that in [doExInt->doExStruct] value will be updated!

    doExInt.registerLink("doExInt->doExClass", doExClass, [](Asm::DataObject<int>& triggeredDoInt, Asm::DataObject<CustomClass>& triggeredDoClass) {
        int doExIntVal = triggeredDoInt.get([](int i) { return i; });
        const CustomClass doExClassVal = triggeredDoClass.get([](const CustomClass& complexClass) { return complexClass; });
        std::cout << "Triggered link [doExInt->doExClass] with values doExInt: " << doExIntVal << ", doExClass.inputCounter: " << doExClassVal.inputCounter << std::endl;

#ifdef __linux__
        std::cout << "Use TID- " << syscall(SYS_gettid) << " for datalink test2" << std::endl;
#endif
    });

    Asm::pDOR->trigger(doExInt);
    boost::this_thread::sleep_for(boost::chrono::seconds(3)); // wait for links to be executed

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
