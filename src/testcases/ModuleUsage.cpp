/*
** Test cases for DataObjects used within modules.
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include <string>

#include "../asm/asm.hpp"
#include "../modules/global_modules.hpp"
#include "../maker/maker_reflection.hpp"

static void printline()
{
	MyComplexDOType data = processModule.DOcomplexInOut.get([](auto s) {return s; });
	std::cout << "Process: " << data.message << " In: " << data.inputCounter << " Out: " << data.outputCounter << " --> " << outModule.DOstringInput.get([](const std::string& s) {return s; }) << " length: " << outModule.DOintInput.get([](const int& i) {return i; }) << std::endl;
}

void runModuleUsageExamples() {

	//Linked global DataObjects:
	/*
			InputModule.inModule.DOintOutput    ---|													|---> OutputModule.outModule.DOintInput
												   |--->  ProcessModule.processModule.DOcomplexInOut ---|
			InputModule.inModule.DOstringOutput ---|													|---> OutputModule.outModule.DOstringInput
	*/
    std::cout << "===================================================================" << std::endl;
    std::cout << "Run module usage samples .." << std::endl;

	name_dataobjects.at("InputModule.inModule.DOintOutput");
	name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut");
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
	outModule.LinkString.clear("String", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));
    processModule.LinkInt.clear("Int", name_dataobjects.at("InputModule.inModule.DOintOutput"));
	outModule.LinkInt.clear("Int", name_dataobjects.at("ProcessModule.processModule.DOcomplexInOut"));

	boost::get<Asm::DataObject<int> &>(name_dataobjects.at("InputModule.inModule.DOintOutput")).setAndTrigger([](std::atomic<int> &i) { i = 557; }, *Asm::pDOR.get());
	// Wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
	printline();

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
};
