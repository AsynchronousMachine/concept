//#pragma once
#include "../asm/asm.hpp"
#include "../datatypes/global_datatypes.hpp"

class MyModuleX
{
private:
	MyComplexDOType init;

public:
	MyModuleX() :
		intInputDO(1), // No serializer
		stringInputDO("init", Asm::default_serializer), // use default implementation for primitive data type
		/*
		TODO init wird als copy übergeben, suche nach einer Möglichkeit per Referenz
		*/
		myDO(init, &MyComplexDOType::serializeMe, &MyComplexDOType::deserializeMe, &init), // function pointer
		myDO2(
			MyComplexDOType{ 1, 2, "12" },
		//	//MyComplexDOType{ inputCounter{1}; outputCounter{2}; message{"12"}; },
			&MyComplexDOType::serializeMe, &MyComplexDOType::deserializeMe, true), // function pointer
		messageLengthDO(0, [&](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {messageLengthDO.get([&value](int i) {value.SetInt(i); }); }, [&](rapidjson::Value& value) {messageLengthDO.set([&value](int& i) {i = value.GetInt(); }); }), // lambda expression, does the same as default implementation
		messageDO("")
	{
		
		std::cout << "MyModuleX Konstruktor " << &init << std::endl;
	}

	Asm::DataObject<int> intInputDO;
	Asm::DataObject<std::string> stringInputDO;
	Asm::DataObject<MyComplexDOType> myDO;
	Asm::DataObject<MyComplexDOType> myDO2;
	Asm::DataObject<int> messageLengthDO;
	Asm::DataObject<std::string> messageDO;

	//mögliche Verlinkungen für myDO
	//Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::string>> InInt;
	//Asm::Link<Asm::DataObject<std::string>, Asm::DataObject<std::string>> InString;
	//Asm::Link<Asm::DataObject<std::string>, Asm::DataObject<int>> OutInt;
	//Asm::Link<Asm::DataObject<std::string>, Asm::DataObject<std::string>> OutString;

	void initdo() {
		std::cout << "initdo get1 " << myDO.get([](auto& i) {std::cout << "This before: " << &i << std::endl;  return i.inputCounter; }) << std::endl;
		init.inputCounter = 42;
		myDO.set([this](MyComplexDOType& i) {std::cout << "This set: " <<  &i << std::endl; i = init;});
		std::cout << "initdo get2 " << myDO.get([](auto& i) {std::cout << "This after: " << &i << std::endl; return i.inputCounter; }) << std::endl;
	}

	void initdo2() {
		std::cout << "initdo2 get1 " << myDO2.get([](auto& i) {std::cout << "This before: " << &i << i.outputCounter << std::endl;  return i.inputCounter; }) << std::endl;
		init.inputCounter = 42;
		myDO2.set([this](MyComplexDOType& i) {std::cout << "This set: " << &i << std::endl; i = init; });
		std::cout << "initdo2 get2 " << myDO2.get([](auto& i) {std::cout << "This after: " << &i << i.inputCounter << std::endl; return i.outputCounter; }) << std::endl;
	}
};