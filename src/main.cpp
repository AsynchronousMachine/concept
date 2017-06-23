/*
** Global entry for all test cases
*/

#include <pthread.h>
#include <sys/syscall.h>

#include <cstdlib>
#include <iostream>

#include <boost/thread/thread.hpp>

#include <tbb/tbb.h>

extern void runDOAccessExamples();
extern void runDOReactorExamples();
extern void runModuleUsageExamples();
extern void runDOTimerExamples();
extern void runDOSerializationExamples();
extern void runTBBUsageExamples();

struct Observer : tbb::task_scheduler_observer
{
    static constexpr int RT_PRIO = 30; // Realtime priority
    int _rt_prio;

    Observer(bool b = true, int rt_prio = RT_PRIO) : _rt_prio(rt_prio) { observe(b); }

    void on_scheduler_entry(bool)
    {
#ifdef __linux__
        pthread_t pid = pthread_self();
        pid_t tid = syscall(SYS_gettid);

        //The thread name is a meaningful C language string, whose length is
        //restricted to 16 characters, including the terminating null byte ('\0')
        std::string s = "TBB-TID-" + std::to_string(tid);
        std::cout << s << std::endl;

        if(pthread_setname_np(pid, s.data()))
            std::cout << "Could not set tbb names" << std::endl;

        struct sched_param param = {};
        param.sched_priority = _rt_prio;

        if(pthread_setschedparam(pid, SCHED_FIFO, &param))
            std::cout << "Could not set realtime parameter" << std::endl;
#endif
    }

    void on_scheduler_exit(bool) { std::cout << "Off:" << pthread_self() << std::endl; }
};

int main() {
    Observer observer;
    tbb::task_scheduler_init tbb_init;

    // Wait for all instantiation processes to finish
    boost::this_thread::sleep_for(boost::chrono::seconds(5));

#ifdef __linux__
    std::cout << "TID of main: " << syscall(SYS_gettid)<< std::endl;
#endif

    std::cout << "TBB threads, max available: " << tbb_init.default_num_threads() << std::endl;

    runDOAccessExamples();
    runDOReactorExamples();
    runModuleUsageExamples();
    runDOSerializationExamples();
	runTBBUsageExamples();

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'q\' for quit tests!" << std::endl;
    char c;
    std::cin >> c;

    exit(0);
}
