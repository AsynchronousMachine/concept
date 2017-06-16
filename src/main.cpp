/*
** Global entry for all test cases
*/

#include <cstdlib>
#include <iostream>

extern void runDOAccessExamples();
extern void runDOReactorExamples();
extern void runModuleUsageExamples();
extern void runDOTimerExamples();
extern void runDOSerializationExamples();

int main() {
    runDOAccessExamples();
    runDOReactorExamples();
    runModuleUsageExamples();
    runDOSerializationExamples();

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'q\' for quit tests!" << std::endl;
    char c;
    std::cin >> c;

    exit(0);
}
