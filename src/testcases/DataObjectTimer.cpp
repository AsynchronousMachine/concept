#include "../asm/asm.hpp"

#ifdef __linux__

void tickTack(std::shared_ptr<Asm::TimerObject> timer){

    for(int i = 0; i < 5; ++i)
    {
        uint64_t t; // Amount of intervals between wait()s
        timer->wait(t); // Wait for the read in timer to return
        std::cout << (!(i%2) ? "tick (" : "tack (") << (t * timer->getInterval()) << "ms)" << std::endl;
    }
}

void runDOTimerExample(){

    std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "DataObjectTimer tests..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

    std::shared_ptr<Asm::TimerObject> timer(new Asm::TimerObject());
    std::cout << "Start timer in 3s with interval 1s" << std::endl;
    timer->setRelativeInterval(1000, 3000);
    tickTack(timer);

    std::cout << "Timer stop and restart..." << std::endl;
    timer->stop();
    timer->restart();
    tickTack(timer);
    timer->stop();

    std::cout << "Change interval to 0.5s and restart..." << std::endl;
    timer->setRelativeInterval(500, 3000);
    // No need to restart manually. done with setRelativeInterval
    tickTack(timer);
    timer->stop();
}

#endif//__linux__
