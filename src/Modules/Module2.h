#pragma once
#include "../asm.hpp"
#include "../global_reflection.h"
class Module2
{
private:
	Asm::DataObject<int> doSave; // Access is fully thread safe

	void action1(Asm::DataObject<int>& doSource, Asm::DataObject<int>& doTarget)
	{
		int oldvalue = doTarget.get([](int i)->int {return i; });
		int newValue = doSource.get([](int i)->int {return i; });

		doTarget.set([newValue](int& i) { i = newValue; });

		int setvalue;
		doTarget.get([&setvalue](int i) {setvalue = i; });
		
		std::cout << "Update value " << do_names.at(&doTarget) << ": " << oldvalue << " -> " << newValue << "(" << setvalue << ")" << std::endl;
	}

	void action2(Asm::DataObject<std::string>& doSource, Asm::DataObject<std::map<std::string, double>>& doTarget)
	{
		std::string s = doSource.get([](std::string s) { std::cout << "Got DO.value: " << s << std::endl; return s; });
		std::cout << "Has DO.value: " << doTarget.get([&s](const std::map<std::string, double>& m) { return m.at(s); }) << std::endl;
	}



protected:
	//protected DOs are ignored by the map_maker tool
	Asm::DataObject<std::list<double>> doProtected;

public:
	Module2() :
		do1(2, Asm::default_serializer),
		do2(std::map<std::string, double>{ {"42", 22.0}, { "43", 23.0 }, { "44", 24.0 }}),
		doProtected(std::list<double>{ {1.1, 2.2, 3.3}}),
		doSave(0),
		link1(&Module2::action1, this),
		link2(&Module2::action2, this),
		linkComplex(actionStatic) {}

	Asm::DataObject<int> do1;
	Asm::DataObject<std::map<std::string, double> > do2;

	Asm::Link<Asm::DataObject<int>, Asm::DataObject<int>> link1;
	Asm::Link<Asm::DataObject<std::string>, Asm::DataObject<std::map<std::string, double>>> link2;
	Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::map<std::string, double>>> linkComplex;

	void actionComplex(Asm::DataObject<int>& doSource, Asm::DataObject<std::map<std::string, double>>& doTarget)
	{
		int newValue = doSource.get([](int i)->int {return i; });
		doTarget.set([newValue](std::map<std::string, double>& map) { map[std::to_string(newValue)] = (double)newValue; });
		doSave.set([](int& i) { ++i; });
		doProtected.set([newValue](std::list<double>& list) {list.push_back((double)newValue); });

		std::cout << "actionComplex i=" << doSave.get([](int i) {return i; }) << " do2.size= "
			<< doProtected.get([](std::list<double> list) ->int { return (int)list.size(); })
			<< std::endl;
	}

	static void actionStatic(Asm::DataObject<int>& doSource, Asm::DataObject<std::map<std::string, double>>& doTarget){}
};