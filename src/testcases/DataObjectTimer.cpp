/*
** Test cases for showing almost all aspects of dealing with TimerObjects.
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include "../logger/logger.hpp"
#include "../asm/asm.hpp"

static void tickTack(Asm::TimerObject& timer) {
    for (int i = 0; i < 3; ++i) {
        uint64_t t; // Amount of intervals between wait()s

        timer.wait(t); // Wait for the timer to return

        if(!(i%2))
            Logger::pLOG->trace("tick ({}ms)", t * timer.getInterval());
        else
            Logger::pLOG->trace("tack ({}ms)", t * timer.getInterval());
    }
}

void runDOTimerExamples() {
    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("Run TimerObject handling samples ..");

    Asm::TimerObject timer;
    Logger::pLOG->trace("Start timer in 3s with interval 2s");
    timer.setRelativeInterval(2000, 3000);
    tickTack(timer);

    Logger::pLOG->trace("Timer stop and restart...");
    timer.stop();
    timer.restart();
    tickTack(timer);

    Logger::pLOG->trace("Change interval to 1s with interval 1s and restart...");
    timer.stop();
    timer.setRelativeInterval(1000, 1000); // No need to restart manually. done with setRelativeInterval
    tickTack(timer);
    timer.stop();

    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("TimerReactor tests ..");

    // Create the instances for test
    Asm::DataObject<Asm::TimerObject> dataObjectTimer;
    Asm::DataObject<int> dataObjectInt;

    dataObjectTimer.registerLink("doTimer->doInt", dataObjectInt, [](Asm::DataObject<Asm::TimerObject>& to, Asm::DataObject<int>& i){
        Logger::pLOG->trace("[doTimer->doInt] was triggered by TOR");
#ifdef __linux__
        Logger::pLOG->trace("Tid of {} for TOR test", syscall(SYS_gettid));
#endif
    });

    // Add TimerObject and let it trigger
    Asm::pTOR->registerTimer(dataObjectTimer);

    // Let timer start
    dataObjectTimer.set([](Asm::TimerObject& to){ to.setRelativeInterval(500,0); });

    boost::this_thread::sleep_for(boost::chrono::seconds(3));

    dataObjectTimer.set([](Asm::TimerObject& to){ to.stop(); });

    Asm::pTOR->unregisterTimer(dataObjectTimer);

    dataObjectTimer.unregisterLink("doTimer->doInt");

    Logger::pLOG->trace("===================================================================");
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
