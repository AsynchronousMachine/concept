/*
**
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include "../logger/logger.hpp"
#include "../asm/asm.hpp"

class ProcessModule
{
private:
	//handles the data flow from the source to the target DataObject, including converting the data type if necessary
	//if the data change in the target has to be published use the setAndTrigger function with the responsible reactor else use only the set function
	void actionInt(Asm::DataObject<int>& doSource, Asm::DataObject<MyComplexDOType>& doTarget)
	{
		//combined action: first increase inputCounter
		doTarget.set([&doSource](MyComplexDOType& cc) { cc.inputCounter = cc.inputCounter + 1; });
		doTarget.setAndTrigger([&doSource](MyComplexDOType& cc) { cc.message = doSource.get([](int i) {return std::to_string(i); }); }, *Asm::pDOR);
#ifdef __linux__
        Logger::pLOG->trace("Use TID-{} for ProcessModule/actionInt", syscall(SYS_gettid));
#endif
	}

	void actionString(Asm::DataObject<std::string>& doSource, Asm::DataObject<MyComplexDOType>& doTarget)
	{
		//combined action: first increase inputCounter
		doTarget.set([&doSource](MyComplexDOType& cc) { cc.inputCounter = cc.inputCounter + 1; });
		doTarget.setAndTrigger([&doSource](MyComplexDOType& cc) { cc.message = doSource.get([](std::string s) {return s; }); }, *Asm::pDOR);
#ifdef __linux__
        Logger::pLOG->trace("Use TID-{} for ProcessModule/actionString", syscall(SYS_gettid));
#endif
	}

public:
	ProcessModule() :
		DOcomplexInOut(MyComplexDOType{ 0, 0, "" }),
		//the callback funtion for a link can only be set in the constructor
		//Guideline: the link is defined in the same module where its target DataObject is, the same applies to the implementation of the callback function
		LinkInt(&ProcessModule::actionInt, this),
		LinkString(&ProcessModule::actionString, this)
	{}

	//DataObject is used as source and as target within LinkObjects -> is an inout parameter of the module
	Asm::DataObject<MyComplexDOType> DOcomplexInOut;

	Asm::LinkObject<Asm::DataObject<int>, Asm::DataObject<MyComplexDOType>> LinkInt;
	Asm::LinkObject<Asm::DataObject<std::string>, Asm::DataObject<MyComplexDOType>> LinkString;
};
