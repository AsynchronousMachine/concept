#pragma once
#include "../asm.hpp"

class Module1
{
private:
	//private DOs are ignored by the map_maker tool
	Asm::DataObject<int> doPrivate;

public:
	Module1() :	do1(1),
		do2("42"),
		doPrivate(-2) {}


	Asm::DataObject<int> do1;
	Asm::DataObject<std::string> do2;

	void deserialize(std::string js) { std::cout << "Got: " << js << std::endl; }
	std::string serialize() { return "{do1:2}"; }
};