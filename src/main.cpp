/*
** Global entry for all test cases
*/

/*
** Hint: for Windows systems "WIN32_LEAN_AND_MEAN" has to be defined in preprocessor else you will get boost/asio/detail/socket_types.hpp(24): fatal
*error C1189: #error:  WinSock.h has already been included
*/

#ifndef BUILD_TIMESTAMP
#error BUILD_TIMESTAMP is undefined
#endif

#ifndef VERSION
#error VERSION is undefined
#endif

#ifdef __linux__
#include <pthread.h>
#include <sys/syscall.h>
#endif

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#include <tbb/tbb.h>

#include "communication/ReceiveHandler.hpp"
#include "communication/TcpServer.hpp"
#include "logger/logger.hpp"

extern void runDOAccessExamples();
extern void runDOLocksExamples();
extern void runDOLinksExamples();
extern void runDOReactorExamples();
extern void runModuleUsageExamples();
extern void runDOTimerExamples();
extern void runDOSerializationExamples();
extern void runTBBUsageExamples();

struct Observer : tbb::task_scheduler_observer {
    static constexpr int RT_PRIO = 30; // Realtime priority
    int _rt_prio;

    Observer(bool b = true, int rt_prio = RT_PRIO) : _rt_prio(rt_prio) { observe(b); }

    void on_scheduler_entry(bool) {
#ifdef __linux__
        pthread_t pid = pthread_self();
        pid_t tid = syscall(SYS_gettid);

        // The thread name is a meaningful C language string, whose length is
        // restricted to 16 characters, including the terminating null byte ('\0')
        std::string s = "TBB-TID-" + std::to_string(tid);
        Logger::pLOG->info("Created {}", s);

        if (pthread_setname_np(pid, s.data()))
            Logger::pLOG->warn("Could not set name for {}", s);

        struct sched_param param = {};
        param.sched_priority = _rt_prio;

        if (pthread_setschedparam(pid, SCHED_FIFO, &param))
            Logger::pLOG->warn("Could not set realtime parameter for {}", s);
#endif
    }

    void on_scheduler_exit(bool) {
#ifdef __linux__
        char tn[20];
        pthread_getname_np(pthread_self(), &tn[0], sizeof(tn));
        Logger::pLOG->info("TBB-Exit: {}", tn);
#endif
    }
};

static void load_config(std::string fn, unsigned port) {
    boost::asio::ip::tcp::resolver::query query{"::1", std::to_string(port)};
    boost::asio::ip::tcp::iostream tcpstr;

    std::ifstream ifstr(fn);
    if (!ifstr) {
        Logger::pLOG->info("Configuration file {} not found", fn);
        return;
    }

    for (int cnt : {5, 4, 3, 2, 1, 0}) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        tcpstr.connect(query);

        if (tcpstr)
            break;

        if (cnt == 0) {
            Logger::pLOG->error("No connection for configuration file {}: {}", fn, tcpstr.error().message());
            return;
        }
    }

    tcpstr << ifstr.rdbuf();
    tcpstr.flush();
}

int main() {
    std::cout << "This is ASM version " << VERSION << ", build time " << BUILD_TIMESTAMP << "." << std::endl;
    Observer observer;
    tbb::task_scheduler_init tbb_init;

    Logger::pLOG->trace("Trace log active");
    Logger::pLOG->debug("Debug log active");
    Logger::pLOG->info("Info log active");
    Logger::pLOG->warn("Warning log active");
    Logger::pLOG->error("Error log active");
    Logger::pLOG->critical("Critical log active");

    Logger::pLOG->info("TID of main: {}", syscall(SYS_gettid));

    // Wait for all previous instantiation processes to finish
    std::this_thread::sleep_for(std::chrono::seconds(3));

    Logger::pLOG->info("TBB threads, max availablse: {}", tbb::task_scheduler_init::default_num_threads());

    runDOAccessExamples();
    runDOLocksExamples();
    runDOLinksExamples();
    runDOReactorExamples();
    runDOTimerExamples();
    runModuleUsageExamples();
    runDOSerializationExamples();
    runTBBUsageExamples();

    // Should be treaded as singletons
    std::unique_ptr<Asm::TcpServer> pDOServer = std::make_unique<Asm::TcpServer>(9600, Asm::do_handler);
    std::unique_ptr<Asm::TcpServer> pLOServer = std::make_unique<Asm::TcpServer>(9601, Asm::lo_handler);

    // Load configuration for dataobjects
    load_config("./src/testcases/do_config.json", 9600);
    // Load configuration for linkobjects
    load_config("./src/testcases/lo_config.json", 9601);

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'q\' for quit tests!" << std::endl;
    char c;
    std::cin >> c;

    exit(0);
}
