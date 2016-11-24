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

//get and set function must have exacly one parameter for the content of the DO (with the datatype of the DO)
//CAUTION: for set the parameter must be a reference (does not work without '&') 
int getDO1(int i) {
	return i;
}

std::string getDO1asstring(int i) {
	return std::to_string(i);
}

std::string getDO2(std::string s) {
	return s;
}

std::string newStringContent = "set-function";
//important: There is no parameter for the new value possilbe
void setDO2(std::string &s) {
	s = newStringContent;
}

/*
	Example part
*/

void useGetterAndSetter() {
	int do1Content;
	std::string do2Content;

	//writing and reading the content of a DO is only possible throught functions			
	do1Content = do1.get(getDO1);
	do1.get(getDO1asstring);  //function can return a different datatype
	// do1.get(getDO2); --compiler error: parameter has wrong datatype

	//as lambda expressions
	do1.get([](int i) {return i; });
	do1.get([&](int i) {do1Content = i; });

	//the function defines if a value is set by value or reference
	std::string newString = "test1 ";
	do2.set([=](std::string &s) { s = newString; }); // call by value
	do2.set([&](std::string &s) { s = newString; }); // call by reference
	do2.set(setDO2);
}

void useSetLink() {
	// Link together: do1<int> -------> do2<string>
	// without a Link class instance
	do1.registerLink("Link1", do2, action);
}

void useClearLink() {
	// Unlink: do1<int> -------> do2<string> (link name from set must be known)
	// without a Link class instance
	do1.unregisterLink("Link1");
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
	std::cout << "check do2 content = " << do2.get(getDO2) << std::endl;
	//5. delete reactor at program end
	delete rptr;
}
