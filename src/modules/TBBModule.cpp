/*
** Example for a TBB task usage within a Link
*/

#ifdef __linux_
#include <sys/syscall.h>
#endif

#include <tbb/tbb.h>

#include "../logger/logger.hpp"

#include "TBBModule.hpp"

void TBBModule::pf(const tbb::blocked_range<size_t>& r) {
#ifdef __linux__
    Logger::pLOG->trace("TID of pf: {}", syscall(SYS_gettid));
#endif

    boost::this_thread::sleep_for(boost::chrono::seconds(1));
    Logger::pLOG->trace("{}/{}", r.begin(), r.end());
    Logger::pLOG->trace("Job done ...");
};

void TBBModule::actionInt(Asm::DataObject<int>& doSource, Asm::DataObject<MyComplexDOType>&) {
#ifdef __linux__
    Logger::pLOG->trace("TID of TBBModule::actionInt: {}", syscall(SYS_gettid));
#endif

    tbb::parallel_for(tbb::blocked_range<size_t>(0, 16, tbb::info::default_concurrency()), 
                      [this](const tbb::blocked_range<size_t>& r) { TBBModule::pf(r); },
                      tbb::simple_partitioner());

    Logger::pLOG->trace("Got {} in TBBModule::actionInt", doSource.get([](int i) { return i; }));
}

TBBModule::TBBModule() :
    DOcomplexInOut(MyComplexDOType{ .inputCounter=0, .outputCounter=0, .message="Default" }),
    // The callback function for a Link can only be set in the constructor
    // Guideline: the Link is defined in the same module where its target DataObject is,
    // the same applies to the implementation of the callback function
    LinkInt(&TBBModule::actionInt, this) {
}
