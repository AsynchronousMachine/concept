#pragma once

#include "../asm/dataobjectreactor.hpp"
#include "../asm/dataobject.hpp"
#include "../asm/linkobject.hpp"

#include <string>


//well known Reactor
std::unique_ptr<Asm::DataObjectReactor> rptr(new Asm::DataObjectReactor());


//Hint: all DOs in this example are not serializeable to keep it simple, look at DataObjectSerialize.cpp for this use case

class InputModule
{
public:
	InputModule() :
		DOintOutput(0),
		DOstringOutput("")
	{}

	//DataObject is never a target in a LinkObject, only the source -> is an output of the module
	Asm::DataObject<int> DOintOutput;
	Asm::DataObject<std::string> DOstringOutput;
};

class ProcessModule
{
private:
	//handles the data flow from the source to the target DataObject, including converting the data type if necessary
	//if the data change in the target has to be published use the setAndTrigger function with the responsible reactor else use only the set function  
	void actionInt(Asm::DataObject<int>& doSource, Asm::DataObject<MyComplexDOType>& doTarget)
	{
		//combined action: first increase inputCounter
		doTarget.set([&doSource](MyComplexDOType& cc) { cc.inputCounter = cc.inputCounter + 1; });
		doTarget.setAndTrigger([&doSource](MyComplexDOType& cc) { cc.message = doSource.get([](int i) {return std::to_string(i); }); }, *rptr);
	}

	void actionString(Asm::DataObject<std::string>& doSource, Asm::DataObject<MyComplexDOType>& doTarget)
	{
		//combined action: first increase inputCounter
		doTarget.set([&doSource](MyComplexDOType& cc) { cc.inputCounter = cc.inputCounter + 1; });
		doTarget.setAndTrigger([&doSource](MyComplexDOType& cc) { cc.message = doSource.get([](std::string s) {return s; }); }, *rptr);
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

//All modules must be introduced in global_modules.hpp and global_modules.cpp by the developer, because this is the starting place for the map_maker
//part from global_modules.cpp

InputModule inModule;
ProcessModule processModule;
OutputModule outModule;

//map_maker generates the maps in maker_reflection.cpp
//part from maker_reflection.cpp

std::unordered_map<std::string, Asm::data_variant> name_dataobjects_uc2
{
	{ "InputModule.inModul.DOintOutput", inModule.DOintOutput },
	{ "InputModule.inModul.DOstringOutput", inModule.DOstringOutput },
	{ "ProcessModule.processModule.DOcomplexInOut", processModule.DOcomplexInOut },
	{ "OutputModule.outModul.DOintInput", outModule.DOintInput },
	{ "OutputModule.outModul.DOstringInput", outModule.DOstringInput },
};


void printline()
{
	MyComplexDOType data = processModule.DOcomplexInOut.get([](auto s) {return s; });
	std::cout << "Process: " << data.message << " In: " << data.inputCounter << " Out: " << data.outputCounter << " --> " << outModule.DOstringInput.get([](const std::string& s) {return s; }) << " length: " << outModule.DOintInput.get([](const int& i) {return i; }) << std::endl;
}


void runModuleUsageExample() {

	//Link DataObjects:
	/*
			InputModule.inModul.DOintOutput     ---|													|---> OutputModule.outModul.DOintInput
												   |--->  ProcessModule.processModule.DOcomplexInOut ---|
			InputModule.inModul.DOstringOutput  ---|													|---> OutputModule.outModul.DOstringInput					
	*/

	processModule.LinkInt.set("Int", name_dataobjects_uc2.at("InputModule.inModul.DOintOutput"), name_dataobjects_uc2.at("ProcessModule.processModule.DOcomplexInOut"));
	processModule.LinkString.set("String", name_dataobjects_uc2.at("InputModule.inModul.DOstringOutput"), name_dataobjects_uc2.at("ProcessModule.processModule.DOcomplexInOut"));

	//name may be the same for different LinkObjects, but must not for the same LinkObject
	outModule.LinkInt.set("Int", name_dataobjects_uc2.at("ProcessModule.processModule.DOcomplexInOut"), name_dataobjects_uc2.at("OutputModule.outModul.DOintInput"));
	outModule.LinkString.set("String", name_dataobjects_uc2.at("ProcessModule.processModule.DOcomplexInOut"), name_dataobjects_uc2.at("OutputModule.outModul.DOstringInput"));

	//set value and trigger chain
	boost::get<Asm::DataObject<int> &>(name_dataobjects_uc2.at("InputModule.inModul.DOintOutput")).setAndTrigger([](std::atomic<int> &i) { i = 556; }, *rptr.get());
	//wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(2));
	printline();

	inModule.DOstringOutput.setAndTrigger([](auto &s) { s = "Hallo"; }, *rptr);
	//wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(2));
	printline();

	for (int ii = 1; ii < 10; ii++)
	{
		boost::get<Asm::DataObject<int> &>(name_dataobjects_uc2.at("InputModule.inModul.DOintOutput")).setAndTrigger([&](std::atomic<int> &i) { i = std::pow(10, ii); }, *rptr);
		//wait till reactor has processed
		boost::this_thread::sleep_for(boost::chrono::seconds(2)); 
		printline();
	}

	//remove this Link again
	outModule.LinkString.clear("String", name_dataobjects_uc2.at("ProcessModule.processModule.DOcomplexInOut"));

	boost::get<Asm::DataObject<int> &>(name_dataobjects_uc2.at("InputModule.inModul.DOintOutput")).setAndTrigger([](std::atomic<int> &i) { i = 557; }, *rptr.get());
	//wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(2));
	printline();

};

