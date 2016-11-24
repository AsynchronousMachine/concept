#include "asm.hpp"
#include "global_reflection.h"

/*
	Definition part
*/

class ModuleStart
{

public:
	ModuleStart() :
		doInit(1) {}

	Asm::DataObject<int> doInit;

	void deserialize(std::string js) { std::cout << "Got: " << js << std::endl; }
	std::string serialize() { return "{do1:2}"; }
};


class ModuleMiddle
{
private:
	int counter = 0;
	//private DOs are ignored by the map_maker tool, the same for links
	Asm::DataObject<int> doSumLinkInt;

	void link2SumIntMethod(Asm::DataObject<int> &doSource, Asm::DataObject<int> &doTarget)
	{
		counter++;
		doSumLinkInt.set([&](int &i) {i+= doSource.get([](int i) {return i; }); });
		doTarget.set([&](int &i) { i = doSource.get([](int i) {return i; }); });
	}

protected:
	//protected DOs are ignored by the map_maker tool, the same for links
	Asm::DataObject<std::string> doSumLinkString;

public:
	ModuleMiddle() :
		doSumLinkInt(0),
		doSumLinkString(0),
		link2SumInt(&ModuleMiddle::link2SumIntMethod, this),
		link2SumString(&ModuleMiddle::link2SumStringMethod, this)
		{}

	Asm::Link<Asm::DataObject<int>, Asm::DataObject<int>> link2SumInt;
	Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::string>> link2SumString;

	void link2SumStringMethod(Asm::DataObject<int> &doSource, Asm::DataObject<std::string> &doTarget)
	{
		doSumLinkString.set([&](std::string &s) {s = s.append(";").append(std::to_string(doSource.get([](int i) {return i; }))); });
		doTarget.set([&](std::string &s) { s = std::to_string(doSource.get([](int i) {return i; })); });
	}

	void deserialize(std::string js) { std::cout << "Got: " << js << std::endl; }
	std::string serialize() { return "{do1:2}"; }
};



class ModuleEnd
{
private:
	void link2IntMethod(Asm::DataObject<int> &doSource, Asm::DataObject<int> &doTarget)
	{
		doTarget.set([&](int &i) { i = doSource.get([](int i) {return i; }); });
	}

	void link2StringMethod(Asm::DataObject<int> &doSource, Asm::DataObject<std::string> &doTarget)
	{
		doTarget.set([&](std::string &s) { s = doSource.get([](int i) {return std::to_string(i); }); });
	}


public:
	ModuleEnd() :
		doEnd1(0),
		doEnd2("0"),
		link2Int(&ModuleEnd::link2IntMethod, this),
		link2String(&ModuleEnd::link2StringMethod, this){}


	Asm::DataObject<int> doEnd1;
	Asm::DataObject<std::string> doEnd2;

	Asm::Link<Asm::DataObject<int>, Asm::DataObject<int>> link2Int;
	Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::string>> link2String;

	void deserialize(std::string js) { std::cout << "Got: " << js << std::endl; }
	std::string serialize() { return "{do1:2}"; }
};




//template<typename D1>
//class ModuleGlobalCounter
//{
//private:
//	static int counter = 0;
//
//	void link2intMethod(Asm::DataObject<D1> &doSource, Asm::DataObject<D1> &doTarget)
//	{
//
//	}
//
//public:
//	ModuleGlobalCounter
//};









ModuleStart mstart1;
ModuleMiddle mmid1;
ModuleEnd mend1;
ModuleEnd mend2;


/*
	Example part
*/

void example4links() {
	// normal use case:
	set_links.at("ModuleEnd.mend1.link2Int")("Link-1", dos.at("ModuleStart.mstart1.doInit"), dos.at("ModuleEnd.mend1.doEnd1"));
	//set_links.at("ModuleEnd.mend1.link2Int")("Link-1", dos.at("ModuleStart.mstart1.doInit"), dos.at("ModuleEnd.mend1.doEnd2")); //does not work 

	set_links.at("ModuleEnd.mend2.link2String")("Link-2", dos.at("ModuleStart.mstart1.doInit"), dos.at("ModuleEnd.mend2.doEnd2"));

	// another instance link could be used, if the link action does not access instance variables
	set_links.at("ModuleEnd.mend1.link2Int")("Link-3", dos.at("ModuleStart.mstart1.doInit"), dos.at("ModuleEnd.mend2.doEnd1"));

	// also a link can be used to connect DOs from other modules
	set_links.at("ModuleMiddle.mmid1.link2SumInt")("Link-4", dos.at("ModuleStart.mstart1.doInit"), dos.at("ModuleEnd.mend2.doEnd1"));
	set_links.at("ModuleMiddle.mmid1.link2SumString")("Link-5", dos.at("ModuleStart.mstart1.doInit"), dos.at("ModuleEnd.mend1.doEnd2"));
}

template <typename D>
D any_caster(boost::any a)
{
	return boost::any_cast<Asm::DataObject<D>*>(a);
}

//template <>
//Asm::DataObject<int> any_caster(boost::any a)
//{
//	return boost::any_cast<Asm::DataObject<int>*>(a);
//}

void run() {
	//1. create reactor at program start (one global instance for all)
	Asm::DataObjectReactor *rptr = new Asm::DataObjectReactor();
	//2. set links
	example4links();
	//3. set value at the initial DO
	do_set.at("ModuleStart.mstart1.doInit")(21);
	//4. trigger dataflow
	//Asm::DataObject<int>  *d = do_set.at("ModuleStart.mstart1.doInit");
	//rptr->trigger(dos.at("ModuleStart.mstart1.doInit") );
	//wait until trigger is executed
	boost::this_thread::sleep_for(boost::chrono::seconds(3));
//	std::cout << "check do2 content = " << do2.get(getDO2) << std::endl;
	//5. delete reactor at program end
	delete rptr;
}