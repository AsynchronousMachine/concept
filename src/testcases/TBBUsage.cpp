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
	MyComplexDOType data = tbbModule.DOcomplexInOut.get([](auto s) {return s; });
	std::cout << "Process: " << data.message << " In: " << data.inputCounter << " Out: " << data.outputCounter << " --> " << outModule.DOstringInput.get([](const std::string& s) {return s; }) << " length: " << outModule.DOintInput.get([](const int& i) {return i; }) << std::endl;
}

void runTBBUsageExamples() {

	//Linked global DataObjects:
	/*
	InputModule.inModule.DOintOutput   |--->  TBBModule.tbbModule.DOcomplexInOut
	*/
	std::cout << "===================================================================" << std::endl;
	std::cout << "Run TBB usage samples .." << std::endl;

	name_dataobjects.at("InputModule.inModule.DOintOutput");
	name_dataobjects.at("TBBModule.tbbModule.DOcomplexInOut");
	tbbModule.LinkInt.set("Int", name_dataobjects.at("InputModule.inModule.DOintOutput"), name_dataobjects.at("TBBModule.tbbModule.DOcomplexInOut"));


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

	boost::get<Asm::DataObject<int> &>(name_dataobjects.at("InputModule.inModule.DOintOutput")).setAndTrigger([](std::atomic<int> &i) { i = 557; }, *Asm::pDOR.get());
	// Wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
	printline();

	std::cout << "===================================================================" << std::endl;
	std::cout << "Enter \'n\' for next test!" << std::endl;
	char c;
	std::cin >> c;
};