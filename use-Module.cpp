#include "asm.hpp"

class Module
{

public:
	Module() :
		do1(1),
		do2("42"),
		link([&](Asm::DataObject<int> &doSource, Asm::DataObject<std::string> &doTarget)
	{
		int newValue = doSource.get([](int i) {return i; });
		doTarget.set([&](std::string& s) {s = std::to_string(newValue); });
	}) {}


	Asm::DataObject<int> do1;
	Asm::DataObject<std::string> do2;

	Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::string>> link;

	void deserialize(std::string js) { std::cout << "Got: " << js << std::endl; }
	std::string serialize() { return "{do1:2}"; }
};

Module m;

void run() {
	//1. create reactor at program start (one global instance for all)
	Asm::DataObjectReactor *rptr = new Asm::DataObjectReactor();
	//2. set links
	m.link.set("Link", &m.do1, &m.do2);  //DOs are set as boost::any
	//3. set value at the initial DO
	m.do1.set([](int &i) { i = 3; });
	//4. trigger dataflow
	rptr->trigger(m.do1);
	//wait until trigger is executed
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
	std::cout << "check m.do2 content = " << m.do2.get([](std::string s) {return s; }) << std::endl;
	//5. delete reactor at program end
	delete rptr;
}