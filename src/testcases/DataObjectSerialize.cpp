#include "../asm/dataobject.hpp"

#include "../datatypes/MyComplexDOType.hpp"

/*
In the empty constructur of this class you find different examples how data objects of a module can be serialized.
*/
class SerializeModule
{
private:
	//private DOs are ignored by the map_maker tool and will not be serialized by the system
	Asm::DataObject<std::list<double>> doPrivate;

	//example for a serialization function
	void testSerFn4doFunc(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
		doString3.get([&value, &allocator](std::string s) {value.SetString(s.c_str(), allocator); });
	}

	//example for a deserialization function
	void testDeSerFn4doFunc(rapidjson::Value& value) {
		doString3.set([&value](std::string& s) {s = value.GetString(); });
	}

protected:
	//protected DOs are ignored by the map_maker tool and will not be serialized by the system
	Asm::DataObject<std::map<std::string, double>> doProtected;

public:
	SerializeModule() :
		//no serialization at all if only one parameter is used for the content
		doPrivate(std::list<double>{ {1.1, 2.2, 3.3}}),
		doProtected(std::map<std::string, double>{ {"42", 22.0}, { "43", 23.0 }, { "44", 24.0 }}),
		//only for primitive datatypes the build in serialization of the DataObject can be activated by a boolean value as second parameter of the DataObject constructor
		//it does not matter if it is true or false; this is only a dummy value for selecting this constructor
		//please use "Asm::default_serializer" for a better semantic understanding
		doInt(3, Asm::default_serializer),
		doDouble(1.1, true),
		doString("0-1", false),
		//you can set your own (de)serialization function as lambda expression (not highly recommended for complex datatypes)
		doBool(false, [&](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {doBool.get([&value](bool b) { value.SetBool(b); }); }, [&](rapidjson::Value& value) {doBool.set([&value](std::atomic<bool>& b) {b = value.GetBool(); }); }),
		doString2("dummy", [&](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {doString2.get([&](std::string s) { value.SetString(s.c_str(), allocator); }); }, [&](rapidjson::Value& value) {doString2.set([&value](std::string s) {s = value.GetString(); }); }),
		//function pointer
		doString3("test", &SerializeModule::testSerFn4doFunc, &SerializeModule::testDeSerFn4doFunc, this),
		//function pointer on the content
		//boolean is again a dummy value for selecting the right constructor
		//This is the prefered way to do serialization on complex types
		doMyComplexDOType(MyComplexDOType{ 1, 2, "12" }, &MyComplexDOType::serializeMe, &MyComplexDOType::deserializeMe, true)
	{}

	Asm::DataObject<int> doInt;
	Asm::DataObject<double> doDouble;
	Asm::DataObject<std::string> doString;
	Asm::DataObject<bool> doBool;
	Asm::DataObject<std::string> doString2;
	Asm::DataObject<std::string> doString3;
	Asm::DataObject<MyComplexDOType> doMyComplexDOType;
};
