#include "testcases/DataObjectAccess.cpp"
#include "testcases/DataObjectReactor.cpp"
#include "testcases/DataObjectTimer.cpp"

#include <cstdlib>

int main() {

	runDOAccessExamples();
	runDOReactorExamples();
	runDOTimerExample();

	exit(0);
}
