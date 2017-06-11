#include "../asm/dataobject.hpp"
#include <atomic>
#include <string>

static struct CustomStruct {
    int i;
} cs;

class CustomClass {
  public:
    int inputCounter;
};

void runDOReactorExamples() {

    Asm::DataObject<int> doExInt(10);
    Asm::DataObject<CustomStruct> doExStruct(cs);
    Asm::DataObject<CustomClass> doExClass(CustomClass{ 1 });

    // DataObjectReactor Tests
    std::cout << std::endl << "*****************************************" << std::endl;
    std::cout << "DataObjectReactor tests..." << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::unique_ptr<Asm::DataObjectReactor> reactor = std::make_unique<Asm::DataObjectReactor>(4);

    doExInt.set([](std::atomic<int> &i) {
        i = 123;
    });
    doExStruct.set([](CustomStruct& m) {
        m.i = 456;
    });
    doExClass.set([](CustomClass &m) {
        m.inputCounter = 789;
    });

    // this shall be executed after [doExInt->doExClass]
    doExInt.registerLink("doExInt->doExStruct", doExStruct, [](Asm::DataObject<int>& triggeredDoInt, Asm::DataObject<CustomStruct>& triggeredDoStruct) {

        boost::this_thread::sleep_for(boost::chrono::seconds(3));
        int doExIntVal = triggeredDoInt.get([](int i) {
            return i;
        });
        const CustomStruct doExStructVal = triggeredDoStruct.get([](const CustomStruct& customStruct) {
            return customStruct;
        });
        std::cout << "Triggered link [doExInt->doExStruct] with values doExInt: " << doExIntVal << ", doExStruct.i: " << doExStructVal.i << std::endl;
    });
    reactor->trigger(doExInt);
    doExInt.unregisterLink("doExInt->doExStruct");

    doExInt.set([](std::atomic<int> &i) {
        i = 321;
    }); // notice that in [doExInt->doExStruct] value will be updated!
    doExInt.registerLink("doExInt->doExClass", doExClass, [](Asm::DataObject<int>& triggeredDoInt, Asm::DataObject<CustomClass>& triggeredDoClass) {

        int doExIntVal = triggeredDoInt.get([](int i) {
            return i;
        });
        const CustomClass doExClassVal = triggeredDoClass.get([](const CustomClass& complexClass) {
            return complexClass;
        });
        std::cout << "Triggered link [doExInt->doExClass] with values doExInt: " << doExIntVal << ", doExClass.inputCounter: " << doExClassVal.inputCounter << std::endl;

#ifdef __linux__
        std::cout << "Tid of " << syscall(SYS_gettid) << " for datalink test" << std::endl;
#endif

    });
    reactor->trigger(doExInt);
    // wait for links to be executed
    boost::this_thread::sleep_for(boost::chrono::seconds(5));
}
