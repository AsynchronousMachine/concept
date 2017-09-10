/*
** Test cases for DataObjects used within modules.
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include <string>

#include "../logger/logger.hpp"
#include "../asm/asm.hpp"
#include "../modules/global_modules.hpp"
#include "../maker/maker_reflection.hpp"

static void printline()
{
	MyComplexDOType data = processModule.DOcomplexInOut.get([](auto s) { return s; });

	Logger::pLOG->trace("Process: {}  In: {} Out: {} --> {} Length: {}",
                        data.message, data.inputCounter, data.outputCounter,
                        outModule.DOstringInput.get([](const std::string& s) { return s; }), outModule.DOintInput.get([](const int& i) { return i; }));
}

void runModuleUsageExamples() {

	//Linked global DataObjects:
	/*
			InputModule.inModule.DOintOutput    ---|													 |---> OutputModule.outModule.DOintInput
												   |--->  ProcessModule.processModule.DOcomplexInOut --->|
			InputModule.inModule.DOstringOutput ---|													 |---> OutputModule.outModule.DOstringInput
	*/

    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("Run module usage samples ..");

	processModule.LinkInt.set("Int", name_dataobjects.at("InputModule.inModule.DOintOutput"), name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));
	processModule.LinkString.set("String", name_dataobjects.at("InputModule.inModule.DOstringOutput"), name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));

	// Link name may be the same for different LinkObjects, but must not for the same LinkObject
	outModule.LinkInt.set("Int", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"), name_dataobjects.at("OutputModule.outModule.DOintInput"));
	outModule.LinkString.set("String", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"), name_dataobjects.at("OutputModule.outModule.DOstringInput"));

	// Set value and trigger chain
	boost::get<Asm::DataObject<int> &>(name_dataobjects.at("InputModule.inModule.DOintOutput")).setAndTrigger([](std::atomic<int> &i) { i = 556; }, *Asm::pDOR);
	// wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
	printline();

	inModule.DOstringOutput.setAndTrigger([](auto &s) { s = "Hallo"; }, *Asm::pDOR);
	// Wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
	printline();

	for (int ii = 1; ii < 3; ii++)
	{
		boost::get<Asm::DataObject<int> &>(name_dataobjects.at("InputModule.inModule.DOintOutput")).setAndTrigger([&](std::atomic<int> &i) { i = std::pow(10, ii); }, *Asm::pDOR);
		// Wait till reactor has processed
		boost::this_thread::sleep_for(boost::chrono::seconds(3));
		printline();
	}

	// Remove this link again
	processModule.LinkInt.clear("Int", name_dataobjects.at("InputModule.inModule.DOintOutput"));
	processModule.LinkString.clear("String", name_dataobjects.at("InputModule.inModule.DOstringOutput"));
	outModule.LinkInt.clear("Int", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));
	outModule.LinkString.clear("String", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));

	boost::get<Asm::DataObject<int> &>(name_dataobjects.at("InputModule.inModule.DOintOutput")).setAndTrigger([](std::atomic<int> &i) { i = 557; }, *Asm::pDOR.get());
	// Wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
	printline();

    Logger::pLOG->trace("===================================================================");
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
};
