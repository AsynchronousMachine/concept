#include "TBBModule.hpp"

// Example for a TBB implementation

void TBBModule::actionInt(Asm::DataObject<int>& doSource, Asm::DataObject<MyComplexDOType>& doTarget)
{
	//combined action: first increase inputCounter
	doTarget.set([&doSource](MyComplexDOType& cc) { cc.inputCounter = cc.inputCounter + 1; });
	doTarget.setAndTrigger([&doSource](MyComplexDOType& cc) { cc.message = doSource.get([](int i) {return std::to_string(i); }); }, *Asm::pDOR);
}
