/*
** Test cases for showing the usage of DataObjects with relation to LINKs.
** The following code samples exclude ser- and deserialization of
** the content of a DataObject
*/

#include <string>

#include "../logger/logger.hpp"
#include "../asm/asm.hpp"

// A dataobject link / callback
static void do2Link(Asm::DataObject<int>& src, Asm::DataObject<std::string>& dest) {
    boost::this_thread::sleep_for(boost::chrono::seconds(3));
    Logger::pLOG->trace("In DO2, got {}, have {}", src.get([](int a){ return a; }), dest.get([](const std::string& a){ return a; }));
}

void runDOLinksExamples() {
    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("Run DataObjectLinks handling samples ..");

/*
** DO1 ------LINK1-------> DO2    DO1 is linked to DO2
**  |
**  +--------LINK3-------> DO3    Also DO1 is linked to DO3
*/
    // The dataobject instances
    Asm::DataObject<int> do1(10);
    Asm::DataObject<std::string> do2("11");
    Asm::DataObject<double> do3(22.2);

    do1.registerLink("LINK1", do2, do2Link);
    do1.registerLink("LINK3", do3, [](Asm::DataObject<int>& src, Asm::DataObject<double>& dest)
                                   { boost::this_thread::sleep_for(boost::chrono::seconds(3));
                                     Logger::pLOG->trace("In DO3, got {}, have {}", src.get([](int a){ return a; }), dest.get([](double a){ return a; }));
                                   });

    // This calls all links / callbacks registered in "do1" within the actual thread context
    do1.exec();

    // This calls all links / callbacks registered in "do1" within the reactor context
    do1.set([](std::atomic<int> &i) { i = 100; });
    Asm::pDOR->trigger(do1);
    boost::this_thread::sleep_for(boost::chrono::seconds(6));

    do1.unregisterLink("LINK1");
    do1.unregisterLink("LINK2");

    Logger::pLOG->trace("===================================================================");
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
