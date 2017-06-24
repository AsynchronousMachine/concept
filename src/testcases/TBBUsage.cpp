/*
** Test cases for TTB used in Links within modules.
** The following code samples exclude the optional interface for
** ser- and deserialization of a DataObject
*/

#include <string>

#include "../asm/asm.hpp"

#include "../modules/global_modules.hpp"

#include "../maker/maker_reflection.hpp"

void runTBBUsageExamples() {
	std::cout << "===================================================================" << std::endl;
	std::cout << "Run TBB usage samples .." << std::endl;

	// Establish a Link for testing purpose
	// InputModule.inModule.DOintOutput |---> TBBModule.tbbModule.DOcomplexInOut
	tbbModule.LinkInt.set("TBB-Int", name_dataobjects.at("InputModule.inModule.DOintOutput"), name_dataobjects.at("TBBModule.tbbModule.DOcomplexInOut"));

	// Change content of DataObject and announce the change, eventually there are Links available to execute within the DataObjectReactor
	inModule.DOintOutput.setAndTrigger([](std::atomic<int>& i) { i = 10; }, *Asm::pDOR);

	// Wait till DataObjectReactor has processed
	boost::this_thread::sleep_for(boost::chrono::seconds(10));

	// Clear the Link after testing purpose
    tbbModule.LinkInt.clear("TBB-Int", name_dataobjects.at("InputModule.inModule.DOintOutput"));

	std::cout << "===================================================================" << std::endl;
	std::cout << "Enter \'n\' for next test!" << std::endl;
	char c;
	std::cin >> c;
};
