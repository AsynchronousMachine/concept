#include "testcases/DataObjectAccess.cpp"
#include "testcases/DataObjectReactor.cpp"
#include "testcases/DataObjectTimer.cpp"

#include <cstdlib>

int main() {

	runDOAccessExamples();
	runDOReactorExamples();

#ifdef __linux__
	runDOTimerExample();
#endif // __linux__

	exit(0);
}
