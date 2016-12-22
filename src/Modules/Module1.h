#pragma once
#include "../asm.hpp"

class Module1
{
private:
	//private DOs are ignored by the map_maker tool
	Asm::DataObject<int> doPrivate;

public:
	Module1() :	
		do1(1, Asm::default_serializer),
		do2("42", Asm::default_serializer),
		doPrivate(-2) {}


	Asm::DataObject<int> do1;
	Asm::DataObject<std::string> do2;
};