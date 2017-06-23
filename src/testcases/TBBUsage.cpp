/*
** Test cases for TTB used in DataObjects within modules.
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include <string>

#include "../asm/asm.hpp"

#include "../modules/global_modules.hpp"

#include "../maker/maker_reflection.hpp"

void runTBBUsageExamples() {

	//Linked global DataObjects:
	std::cout << "===================================================================" << std::endl;
	std::cout << "Run TBB usage samples .." << std::endl;

	// InputModule.inModule.DOintOutput   |--->  TBBModule.tbbModule.DOcomplexInOut
	tbbModule.LinkInt.set("TBB-Int", name_dataobjects.at("InputModule.inModule.DOintOutput"), name_dataobjects.at("TBBModule.tbbModule.DOcomplexInOut"));

	inModule.DOintOutput.setAndTrigger([](std::atomic<int>& i) { i = 10; }, *Asm::pDOR);

	// Wait till reactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(10));

    tbbModule.LinkInt.clear("TBB-Int", name_dataobjects.at("InputModule.inModule.DOintOutput"));

	std::cout << "===================================================================" << std::endl;
	std::cout << "Enter \'n\' for next test!" << std::endl;
	char c;
	std::cin >> c;
};
