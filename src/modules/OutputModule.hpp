#include "../asm/asm.hpp"

//Hint: all DOs in this example are not serializeable to keep it simple, look at DataObjectSerialize.cpp for this use case

class OutputModule
{
private:
	//here nothing is triggered, end of the chain
	void actionInt(Asm::DataObject<MyComplexDOType>& doSource, Asm::DataObject<int>& doTarget)
	{
		doTarget.set([&doSource](std::atomic<int>& i) { i = doSource.get([](MyComplexDOType cc) {return cc.message.length(); }); });
		//combined action: increase outputCounter in the source
		doSource.set([](MyComplexDOType& cc) { cc.outputCounter = cc.outputCounter + 1; });
	}

	void actionString(Asm::DataObject<MyComplexDOType>& doSource, Asm::DataObject<std::string>& doTarget)
	{
		doTarget.set([&](std::string& s) { s = doSource.get([](MyComplexDOType cc) {return cc.message; }); });
		//combined action: increase outputCounter in the source
		doSource.set([](MyComplexDOType& cc) { cc.outputCounter = cc.outputCounter + 1; });
	}

public:
	OutputModule() :
		DOintInput(0),
		DOstringInput(""),
		LinkInt(&OutputModule::actionInt, this),
		LinkString(&OutputModule::actionString, this)
	{}

	//DataObject is never a source in a LinkObject, only target -> is an input of the module
	Asm::DataObject<int> DOintInput;
	Asm::DataObject<std::string> DOstringInput;
	Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<int>> LinkInt;
	Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<std::string>> LinkString;
};
