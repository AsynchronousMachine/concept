/*
**
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include "../asm/asm.hpp"

class OutputModule
{
private:
	//here nothing is triggered, end of the chain
	void actionInt(Asm::DataObject<MyComplexDOType>& doSource, Asm::DataObject<int>& doTarget)
	{
		doTarget.set([&doSource](std::atomic<int>& i) { i = doSource.get([](MyComplexDOType cc) {return cc.message.length(); }); });
		//combined action: increase outputCounter in the source
		doSource.set([](MyComplexDOType& cc) { cc.outputCounter = cc.outputCounter + 1; });
#ifdef __linux__
        std::cout << "Use TID-" << syscall(SYS_gettid) << " for OutputModule/actionInt" << std::endl;
#endif
	}

	void actionString(Asm::DataObject<MyComplexDOType>& doSource, Asm::DataObject<std::string>& doTarget)
	{
		doTarget.set([&](std::string& s) { s = doSource.get([](MyComplexDOType cc) {return cc.message; }); });
		//combined action: increase outputCounter in the source
		doSource.set([](MyComplexDOType& cc) { cc.outputCounter = cc.outputCounter + 1; });
#ifdef __linux__
        std::cout << "Use TID-" << syscall(SYS_gettid) << " for OutputModule/actionString" << std::endl;
#endif
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
