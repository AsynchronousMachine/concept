/*
** Example for a TBB task usage within a Link
** All DataObjectss in this example are not serializeable to keep it simple
*/

#pragma once

#include "../asm/asm.hpp"

class TBBModule
{
private:
	// Handles the Link from the source of DataObject<int> to the target DataObject<MyComplexDOType>
	// This Link is typically passed to the CTOR of an LinkObject
	// If the data changes in the target usually this will be done by DataObject.setAndTrigger()
	// Afterwards the Link will be executed within the DataObjectReactor
	void actionInt(Asm::DataObject<int>& doSource, Asm::DataObject<MyComplexDOType>& doTarget);
	// This is the TBB task to execute in parallel
    void pf(const tbb::blocked_range<size_t>& r);

public:
	TBBModule();

	// DataObject, here in this sample the target
	Asm::DataObject<MyComplexDOType> DOcomplexInOut;
	// LinkObject towing source DataObject<int> and target DataObject<MyComplexDOType> together
	Asm::LinkObject<Asm::DataObject<int>, Asm::DataObject<MyComplexDOType>> LinkInt;
};
