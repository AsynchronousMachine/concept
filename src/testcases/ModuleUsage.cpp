#include "../asm/dataobjectreactor.hpp"
#include "../asm/dataobject.hpp"
#include "../asm/linkobject.hpp"

#include "../modules/global_modules.hpp"
#include "../maker/maker_reflection.hpp"

#include <string>






static void printline()
{
	MyComplexDOType data = processModule.DOcomplexInOut.get([](auto s) {return s; });
	std::cout << "Process: " << data.message << " In: " << data.inputCounter << " Out: " << data.outputCounter << " --> " << outModule.DOstringInput.get([](const std::string& s) {return s; }) << " length: " << outModule.DOintInput.get([](const int& i) {return i; }) << std::endl;
}


void runModuleUsageExample() {

	//Link DataObjects:
	/*
			InputModule.inModule.DOintOutput     ---|													|---> OutputModule.outModule.DOintInput
												   |--->  ProcessModule.processModule.DOcomplexInOut ---|
			InputModule.inModule.DOstringOutput  ---|													|---> OutputModule.outModule.DOstringInput
	*/

    std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "ModuleUsage tests..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
	name_dataobjects.at("InputModule.inModule.DOintOutput");
	name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut");
	processModule.LinkInt.set("Int", name_dataobjects.at("InputModule.inModule.DOintOutput"), name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));
	processModule.LinkString.set("String", name_dataobjects.at("InputModule.inModule.DOstringOutput"), name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));

	//name may be the same for different LinkObjects, but must not for the same LinkObject
	outModule.LinkInt.set("Int", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"), name_dataobjects.at("OutputModule.outModule.DOintInput"));
	outModule.LinkString.set("String", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"), name_dataobjects.at("OutputModule.outModule.DOstringInput"));

	//set value and trigger chain
	boost::get<Asm::DataObject<int> &>(name_dataobjects.at("InputModule.inModule.DOintOutput")).setAndTrigger([](std::atomic<int> &i) { i = 556; }, *Asm::pDOR.get());
	//wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(2));
	printline();

	inModule.DOstringOutput.setAndTrigger([](auto &s) { s = "Hallo"; }, *Asm::pDOR);
	//wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(2));
	printline();

	for (int ii = 1; ii < 10; ii++)
	{
		boost::get<Asm::DataObject<int> &>(name_dataobjects.at("InputModule.inModule.DOintOutput")).setAndTrigger([&](std::atomic<int> &i) { i = std::pow(10, ii); }, *Asm::pDOR);
		//wait till reactor has processed
		boost::this_thread::sleep_for(boost::chrono::seconds(2));
		printline();
	}

	//remove this Link again
	outModule.LinkString.clear("String", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));

	boost::get<Asm::DataObject<int> &>(name_dataobjects.at("InputModule.inModule.DOintOutput")).setAndTrigger([](std::atomic<int> &i) { i = 557; }, *Asm::pDOR.get());
	//wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(2));
	printline();

};
