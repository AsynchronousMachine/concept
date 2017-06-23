#include <sys/syscall.h>

#include <tbb/tbb.h>

#include "TBBModule.hpp"

// Example for a TBB implementation
void TBBModule::pf(const tbb::blocked_range<size_t>& r)
{
#ifdef __linux__
    std::cout << "TID of pf: " << syscall(SYS_gettid)<< std::endl;
#endif

    boost::this_thread::sleep_for(boost::chrono::seconds(1));
    std::cout << r.begin() << "/"<< r.end() << std::endl;
    std::cout << "Job done ..." << std::endl;
};

void TBBModule::actionInt(Asm::DataObject<int>& doSource, Asm::DataObject<MyComplexDOType>& doTarget)
{
#ifdef __linux__
    std::cout << "TID of TBBModule::actionInt: " << syscall(SYS_gettid)<< std::endl;
#endif

    tbb::parallel_for(tbb::blocked_range<size_t>(0, 16, 2), [this](const tbb::blocked_range<size_t>& r){ this->pf(r); }, tbb::simple_partitioner());

    std::cout << "Got " << doSource.get([](int i) { return i; }) << " in TBBModule" << std::endl;
}

TBBModule::TBBModule() :
		DOcomplexInOut(MyComplexDOType{ 0, 0, "" }),
		// The callback funtion for a link can only be set in the constructor
		// Guideline: the link is defined in the same module where its target DataObject is, the same applies to the implementation of the callback function
		LinkInt(&TBBModule::actionInt, this)
	{}
