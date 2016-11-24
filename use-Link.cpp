#include "asm.hpp"

/*
	Definition part
*/


Asm::DataObject<int> do1(0);
Asm::DataObject<std::string> do2("init");

void action(Asm::DataObject<int> &doSource, Asm::DataObject<std::string> &doTarget) {
	//do action to convert the output datatype of the source DO to the input datatype of the target DO
	int newValue = doSource.get([](int i) {return i; });
	doTarget.set([&](std::string& s) {s = std::to_string(newValue); });
}

Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::string>> link(action);

void useSetLink() {
	link.set("Link1",(boost::any) &do1, (boost::any) &do2);
}

void useClearLink() {
	link.clear("Link1", (boost::any) &do1);
}

void run() {
	//1. create reactor at program start (one global instance for all)
	Asm::DataObjectReactor *rptr = new Asm::DataObjectReactor();
	//2. set links
	useSetLink();
	//3. set value at the initial DO
	do1.set([](int &i) { i = 3; });
	//4. trigger dataflow
	rptr->trigger(do1);
	//wait until trigger is executed
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
	std::cout << "check do2 content = " << do2.get([](std::string s) {return s; }) << std::endl;
	//5. delete reactor at program end
	delete rptr;
}
