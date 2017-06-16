#include "../asm/asm.hpp"

static void tickTack(std::shared_ptr<Asm::TimerObject> timer){

    for(int i = 0; i < 5; ++i)
    {
        uint64_t t; // Amount of intervals between wait()s
        timer->wait(t); // Wait for the read in timer to return
        std::cout << (!(i%2) ? "tick (" : "tack (") << (t * timer->getInterval()) << "ms)" << std::endl;
    }
}

void runDOTimerExamples(){

    std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "DataObjectTimer tests..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

#ifdef __linux__

    std::shared_ptr<Asm::TimerObject> timer = std::make_unique<Asm::TimerObject>();
    std::cout << "Start timer in 3s with interval 1s" << std::endl;
    timer->setRelativeInterval(1000, 3000);
    tickTack(timer);

    std::cout << "Timer stop and restart..." << std::endl;
    timer->stop();
    timer->restart();
    tickTack(timer);

    std::cout << "Change interval to 0.5s and restart..." << std::endl;
    timer->stop();
    timer->setRelativeInterval(500, 3000);
    // No need to restart manually. done with setRelativeInterval
    tickTack(timer);
    timer->stop();


    std::cout << "TimerReactor test..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
    std::unique_ptr<Asm::DataObjectReactor> dataObjectReactor(new Asm::DataObjectReactor(4));
    // timer reactor uses object reactor for triggering links
    std::unique_ptr<Asm::TimerObjectReactor> timerReactor(new Asm::TimerObjectReactor(*dataObjectReactor.get()));

    // example do who's link will be triggered
    Asm::DataObject<Asm::TimerObject> dataObjectTimer;
    Asm::DataObject<int> dataObjectInt;

    // init interval with example capture of local timer
    dataObjectTimer.set([timer](Asm::TimerObject& to){
        to.setRelativeInterval(timer->getInterval(),0);
    });

    // link to be triggered in timerreactor by objectreactor every interval
    dataObjectTimer.registerLink("doTimer->doInt", dataObjectInt, [](Asm::DataObject<Asm::TimerObject>& timer, Asm::DataObject<int>& intVal){

        std::cout << "[doTimer->doInt] was triggered by timer-reactor." << std::endl;
        std::cout << "Tid of " << syscall(SYS_gettid) << " for timerlink test" << std::endl;
    });

    // add timer and let it be triggered triggerAmount=sleep/interval of timer
    timerReactor->registerTimer(dataObjectTimer);
    boost::this_thread::sleep_for(boost::chrono::seconds(5));
    timerReactor->unregisterTimer(dataObjectTimer);

    dataObjectTimer.unregisterLink("doTimer->doInt");

#endif//__linux__
#ifndef __linux__
    std::cout << "Linux only! Skipping DataObjectTimer tests..." << std::endl;
#endif//!__linux__

    boost::this_thread::sleep_for(boost::chrono::seconds(3));
}
