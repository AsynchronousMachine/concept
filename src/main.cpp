#include "testcases/DataObjectAccess.cpp"
#include "testcases/DataObjectReactor.cpp"
#include "testcases/ModuleUsage.cpp"

#include <cstdlib>

int main() {

	runDOAccessExamples();
	runDOReactorExamples();
	runModuleUsageExample();

	exit(0);
}
