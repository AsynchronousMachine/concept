#include "global_reflection.h"

const name_module_map modules
{
	{"Module1.m1", &m1},
	{"Module2.m2", &m2},
	{"Module3.m3", &m3},
};

// Get out the module name for humans
const module_name_map module_names
{
	{&m1, "Module1.m1"},
	{&m2, "Module2.m2"},
	{&m3, "Module3.m3"},
};

const name_dataobject_map dos
{
	{"Module1.m1.do1", &m1.do1},
	{"Module1.m1.do2", &m1.do2},
	{"Module2.m2.do1", &m2.do1},
	{"Module2.m2.do2", &m2.do2},
	{"Module3.m3.doUInt", &m3.doUInt},
	{"Module3.m3.doInt", &m3.doInt},
	{"Module3.m3.doDouble", &m3.doDouble},
	{"Module3.m3.doBool", &m3.doBool},
	{"Module3.m3.doString", &m3.doString},
	{"Module3.m3.doFunc", &m3.doFunc},
	{"Module3.m3.doFuncNo", &m3.doFuncNo},
};

// Get out the DO name for humans
const dataobject_name_map do_names
{
	{&m1.do1, "Module1.m1.do1"},
	{&m1.do2, "Module1.m1.do2"},
	{&m2.do1, "Module2.m2.do1"},
	{&m2.do2, "Module2.m2.do2"},
	{&m3.doUInt, "Module3.m3.doUInt"},
	{&m3.doInt, "Module3.m3.doInt"},
	{&m3.doDouble, "Module3.m3.doDouble"},
	{&m3.doBool, "Module3.m3.doBool"},
	{&m3.doString, "Module3.m3.doString"},
	{&m3.doFunc, "Module3.m3.doFunc"},
	{&m3.doFuncNo, "Module3.m3.doFuncNo"},
};

const registerlink_map set_links
{
	{"Module2.m2.link1", [](std::string name, boost::any a1, boost::any a2) {m2.link1.set(name, a1, a2); } },
	{"Module2.m2.link2", [](std::string name, boost::any a1, boost::any a2) {m2.link2.set(name, a1, a2); } },
	{"Module2.m2.linkComplex", [](std::string name, boost::any a1, boost::any a2) {m2.linkComplex.set(name, a1, a2); } },
	{"Module3.m3.link2Int", [](std::string name, boost::any a1, boost::any a2) {m3.link2Int.set(name, a1, a2); } },
	{"Module3.m3.link2String", [](std::string name, boost::any a1, boost::any a2) {m3.link2String.set(name, a1, a2); } },
};

const unregisterlink_map clear_links
{
	{"Module2.m2.link1", [](std::string name, boost::any a) {m2.link1.clear(name, a); } },
	{"Module2.m2.link2", [](std::string name, boost::any a) {m2.link2.clear(name, a); } },
	{"Module2.m2.linkComplex", [](std::string name, boost::any a) {m2.linkComplex.clear(name, a); } },
	{"Module3.m3.link2Int", [](std::string name, boost::any a) {m3.link2Int.clear(name, a); } },
	{"Module3.m3.link2String", [](std::string name, boost::any a) {m3.link2String.clear(name, a); } },
};

const doserialize_map do_serialize
{
	{"Module1.m1.do1", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m1.do1.serialize(value, allocator); } },
	{"Module1.m1.do2", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m1.do2.serialize(value, allocator); } },
	{"Module2.m2.do1", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m2.do1.serialize(value, allocator); } },
	{"Module2.m2.do2", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m2.do2.serialize(value, allocator); } },
	{"Module3.m3.doUInt", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m3.doUInt.serialize(value, allocator); } },
	{"Module3.m3.doInt", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m3.doInt.serialize(value, allocator); } },
	{"Module3.m3.doDouble", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m3.doDouble.serialize(value, allocator); } },
	{"Module3.m3.doBool", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m3.doBool.serialize(value, allocator); } },
	{"Module3.m3.doString", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m3.doString.serialize(value, allocator); } },
	{"Module3.m3.doFunc", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m3.doFunc.serialize(value, allocator); } },
	{"Module3.m3.doFuncNo", [](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {m3.doFuncNo.serialize(value, allocator); } },
};

const dodeserialize_map do_deserialize
{
	{"Module1.m1.do1", [](rapidjson::Value& value) {m1.do1.deserialize(value); } },
	{"Module1.m1.do2", [](rapidjson::Value& value) {m1.do2.deserialize(value); } },
	{"Module2.m2.do1", [](rapidjson::Value& value) {m2.do1.deserialize(value); } },
	{"Module2.m2.do2", [](rapidjson::Value& value) {m2.do2.deserialize(value); } },
	{"Module3.m3.doUInt", [](rapidjson::Value& value) {m3.doUInt.deserialize(value); } },
	{"Module3.m3.doInt", [](rapidjson::Value& value) {m3.doInt.deserialize(value); } },
	{"Module3.m3.doDouble", [](rapidjson::Value& value) {m3.doDouble.deserialize(value); } },
	{"Module3.m3.doBool", [](rapidjson::Value& value) {m3.doBool.deserialize(value); } },
	{"Module3.m3.doString", [](rapidjson::Value& value) {m3.doString.deserialize(value); } },
	{"Module3.m3.doFunc", [](rapidjson::Value& value) {m3.doFunc.deserialize(value); } },
	{"Module3.m3.doFuncNo", [](rapidjson::Value& value) {m3.doFuncNo.deserialize(value); } },
};

const print_module_map print_modules
{
	{"m1","Module1::m1\n  |> Asm::DataObject<int> do1\n  |> Asm::DataObject<std::string> do2\n"},
	{"m2","Module2::m2\n  |> Asm::DataObject<int> do1\n  |> Asm::DataObject<std::map<std::string, double> > do2\n  |> Asm::Link<Asm::DataObject<int>, Asm::DataObject<int> > link1\n  |> Asm::Link<Asm::DataObject<std::string>, Asm::DataObject<std::map<std::string, double> > > link2\n  |> Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::map<std::string, double> > > linkComplex\n"},
	{"m3","Module3::m3\n  |> Asm::DataObject<unsigned int> doUInt\n  |> Asm::DataObject<int> doInt\n  |> Asm::DataObject<double> doDouble\n  |> Asm::DataObject<_Bool> doBool\n  |> Asm::DataObject<std::string> doString\n  |> Asm::DataObject<std::string> doFunc\n  |> Asm::DataObject<int> doFuncNo\n  |> Asm::Link<Asm::DataObject<int>, Asm::DataObject<int> > link2Int\n  |> Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::string> > link2String\n"},
};

