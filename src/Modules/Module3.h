#include "../asm.hpp"

class Module3
{
private:
	void link2IntMethod(Asm::DataObject<int>& doSource, Asm::DataObject<int>& doTarget)
	{
		doTarget.set([&doSource](int& i) { i = doSource.get([](int i) {return i; }); });
	}

	void link2StringMethod(Asm::DataObject<int>& doSource, Asm::DataObject<std::string>& doTarget)
	{
		doTarget.set([&doSource](std::string& s) { s = doSource.get([](int i) {return std::to_string(i); }); });
	}

	void testSerFn4doFunc(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
		doFunc.get([&value, &allocator](std::string s) {value.SetString(s.c_str(), allocator); });
	}

	void testDeSerFn4doFunc(rapidjson::Value& value) {
		doFunc.set([&value](std::string& s) {s = value.GetString(); });
	}

public:
	Module3() :
		doUInt(0, Asm::default_serializer),
		doInt(3, Asm::default_serializer),
		doDouble(1.1, Asm::default_serializer),
		doBool(false, [&](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {doBool.get([&value](bool b) { value.SetBool(b); }); }, [&](rapidjson::Value& value) {doBool.set([&value](bool& b) {b = value.GetBool(); }); }),
		doString("0-1"), // No serializer
		doFunc("test", &Module3::testSerFn4doFunc, &Module3::testDeSerFn4doFunc, this),
		doFuncNo(5, [&](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {doFuncNo.get([&value](int i) {value.SetInt(i); }); }, [&](rapidjson::Value& value) {doFuncNo.set([&value](int& i) {i = value.GetInt(); }); }),
		link2Int(&Module3::link2IntMethod, this),
		link2String(&Module3::link2StringMethod, this)
	{}


	Asm::DataObject<unsigned int> doUInt;
	Asm::DataObject<int> doInt;
	Asm::DataObject<double> doDouble;
	Asm::DataObject<bool> doBool;
	Asm::DataObject<std::string> doString;
	Asm::DataObject<std::string> doFunc;
	Asm::DataObject<int> doFuncNo;

	Asm::Link<Asm::DataObject<int>, Asm::DataObject<int>> link2Int;
	Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::string>> link2String;
};