/*
** Global entry for all test cases
*/

#include <cstdlib>
#include <iostream>

extern void runDOAccessExamples();
extern void runDOReactorExamples();
extern void runModuleUsageExample();
extern void runDOTimerExample();

int main() {
    runDOAccessExamples();

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'q\' for quit tests!" << std::endl;
    char c;
    std::cin >> c;

    exit(0);
}
