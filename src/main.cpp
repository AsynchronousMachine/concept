#include "testcases/DataObjectAccess.cpp"
#include "testcases/DataObjectReactor.cpp"
#include "testcases/DataObjectTimer.cpp"
#include "testcases/ModuleUsage.cpp"

#include <cstdlib>


std::unique_ptr<Asm::DataObjectReactor> Asm::rptr(new Asm::DataObjectReactor());

std::unique_ptr<Asm::TimerObjectReactor> Asm::trptr(new Asm::TimerObjectReactor(*rptr));

int main() {
	runDOAccessExamples();
	runDOReactorExamples();
	runModuleUsageExample();
	runDOTimerExample();

	exit(0);
}
