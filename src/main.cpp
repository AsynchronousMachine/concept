/*
** Global entry for all test cases
*/

#include <cstdlib>
#include <iostream>

#include <boost/thread/thread.hpp>

extern void runDOAccessExamples();
extern void runDOReactorExamples();
extern void runModuleUsageExamples();
extern void runDOTimerExamples();
extern void runDOSerializationExamples();
extern void runTBBUsageExamples();

int main() {
    // Wait for all instantiation processes to finish
    boost::this_thread::sleep_for(boost::chrono::seconds(3));

    runDOAccessExamples();
    runDOReactorExamples();
    runModuleUsageExamples();
    runDOSerializationExamples();
	runTBBUsageExamples();	

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'q\' for quit tests!" << std::endl;
    char c;
    std::cin >> c;

    exit(0);
}
