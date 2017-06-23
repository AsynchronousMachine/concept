#pragma once

#include "../asm/asm.hpp"

//Hint: all DOs in this example are not serializeable to keep it simple, look at DataObjectSerialize.cpp for this use case

class TBBModule
{
private:
	// Handles the data flow from the source to the target DataObject, including converting the data type if necessary
	// If the data change in the target has to be published use the setAndTrigger function with the responsible reactor else use only the set function
	void actionInt(Asm::DataObject<int>& doSource, Asm::DataObject<MyComplexDOType>& doTarget);
    void pf(const tbb::blocked_range<size_t>& r);

public:
	TBBModule();

	//DataObject is used as source and as target within LinkObjects -> is an inout parameter of the module
	Asm::DataObject<MyComplexDOType> DOcomplexInOut;
	Asm::LinkObject<Asm::DataObject<int>, Asm::DataObject<MyComplexDOType>> LinkInt;
};
