/*
** Test cases for showing almost all aspects of dealing with TimerObjects.
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include "../asm/asm.hpp"

static void tickTack(Asm::TimerObject& timer) {
    for (int i = 0; i < 3; ++i) {
        uint64_t t; // Amount of intervals between wait()s
        timer.wait(t); // Wait for the timer to return
        std::cout << (!(i%2) ? "tick (" : "tack (") << (t * timer.getInterval()) << "ms)" << std::endl;
    }
}

void runDOTimerExamples() {
    std::cout << "===================================================================" << std::endl;
    std::cout << "Run TimerObject handling samples .." << std::endl;

    Asm::TimerObject timer;
    std::cout << "Start timer in 3s with interval 2s" << std::endl;
    timer.setRelativeInterval(2000, 3000);
    tickTack(timer);

    std::cout << "Timer stop and restart..." << std::endl;
    timer.stop();
    timer.restart();
    tickTack(timer);

    std::cout << "Change interval to 1s with interval 1s and restart..." << std::endl;
    timer.stop();
    timer.setRelativeInterval(1000, 1000); // No need to restart manually. done with setRelativeInterval
    tickTack(timer);
    timer.stop();

    std::cout << "===================================================================" << std::endl;
    std::cout << "TimerReactor tests .." << std::endl;

    // Create the instances for test
    Asm::DataObject<Asm::TimerObject> dataObjectTimer;
    Asm::DataObject<int> dataObjectInt;
    dataObjectTimer.registerLink("doTimer->doInt", dataObjectInt, [](Asm::DataObject<Asm::TimerObject>& to, Asm::DataObject<int>& i){
        std::cout << "[doTimer->doInt] was triggered by TOR" << std::endl;
#ifdef __linux__
        std::cout << "Tid of " << syscall(SYS_gettid) << " for TOR test" << std::endl;
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

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
