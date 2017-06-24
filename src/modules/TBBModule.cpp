/*
** Example for a TBB task usage within a Link
*/

#ifdef __linux_
#include <sys/syscall.h>
#endif

#include <tbb/tbb.h>

#include "TBBModule.hpp"

void TBBModule::pf(const tbb::blocked_range<size_t>& r) {
#ifdef __linux__
    std::cout << "TID of pf: " << syscall(SYS_gettid)<< std::endl;
#endif

    boost::this_thread::sleep_for(boost::chrono::seconds(1));
    std::cout << r.begin() << "/"<< r.end() << std::endl;
    std::cout << "Job done ..." << std::endl;
};

void TBBModule::actionInt(Asm::DataObject<int>& doSource, Asm::DataObject<MyComplexDOType>& doTarget) {
#ifdef __linux__
    std::cout << "TID of TBBModule::actionInt: " << syscall(SYS_gettid)<< std::endl;
#endif

    tbb::parallel_for(tbb::blocked_range<size_t>(0, 16, tbb::task_scheduler_init::default_num_threads()),
                      [this](const tbb::blocked_range<size_t>& r) { TBBModule::pf(r); },
                      tbb::simple_partitioner());

    std::cout << "Got " << doSource.get([](int i) {
        return i;
    }) << " in TBBModule::actionInt" << std::endl;
}

TBBModule::TBBModule() :
    DOcomplexInOut(MyComplexDOType{ 0, 0, "" }),
    // The callback function for a Link can only be set in the constructor
    // Guideline: the Link is defined in the same module where its target DataObject is, the same applies to the implementation of the callback function
    LinkInt(&TBBModule::actionInt, this) {
}
