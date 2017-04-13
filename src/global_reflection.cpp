#include "global_reflection.h"


// Get out the module name for humans
const id_string_map module_name
{
	{&m1, "Module1.m1"},
	{&m2, "Module2.m2"},
	{&m3, "Module3.m3"},
};

// Get out the DO name for humans
const id_string_map do_names
{
	{ &m1.do1, "Module1.m1.do1" },
	{ &m1.do2, "Module1.m1.do2" },
	{ &m2.do1, "Module2.m2.do1" },
	{ &m2.do2, "Module2.m2.do2" },
	{ &m3.doUInt, "Module3.m3.doUInt" },
	{ &m3.doInt, "Module3.m3.doInt" },
	{ &m3.doDouble, "Module3.m3.doDouble" },
	{ &m3.doBool, "Module3.m3.doBool" },
	{ &m3.doString, "Module3.m3.doString" },
	{ &m3.doFunc, "Module3.m3.doFunc" },
	{ &m3.doFuncNo, "Module3.m3.doFuncNo" },
};

// Get out by name all modules, all their dataobjects with type and their links for humans
const print_module_map print_modules
{
	{ "m1","Module1::m1\n  |> Asm::DataObject<int> do1\n  |> Asm::DataObject<std::string> do2\n" },
	{ "m2","Module2::m2\n  |> Asm::DataObject<int> do1\n  |> Asm::DataObject<std::map<std::string, double> > do2\n  |> Asm::Link<Asm::DataObject<int>, Asm::DataObject<int> > link1\n  |> Asm::Link<Asm::DataObject<std::string>, Asm::DataObject<std::map<std::string, double> > > link2\n  |> Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::map<std::string, double> > > linkComplex\n" },
	{ "m3","Module3::m3\n  |> Asm::DataObject<unsigned int> doUInt\n  |> Asm::DataObject<int> doInt\n  |> Asm::DataObject<double> doDouble\n  |> Asm::DataObject<_Bool> doBool\n  |> Asm::DataObject<std::string> doString\n  |> Asm::DataObject<std::string> doFunc\n  |> Asm::DataObject<int> doFuncNo\n  |> Asm::Link<Asm::DataObject<int>, Asm::DataObject<int> > link2Int\n  |> Asm::Link<Asm::DataObject<int>, Asm::DataObject<std::string> > link2String\n" },
};

const name_dataobject_map name_dataobjects
{
	{"Module1.m1.do1", m1.do1},
	{"Module1.m1.do2", m1.do2},
	{"Module2.m2.do1", m2.do1},
	{"Module2.m2.do2", m2.do2},
	{"Module3.m3.doUInt", m3.doUInt},
	{"Module3.m3.doInt", m3.doInt},
	{"Module3.m3.doDouble", m3.doDouble},
	{"Module3.m3.doBool", m3.doBool},
	{"Module3.m3.doString", m3.doString},
	{"Module3.m3.doFunc", m3.doFunc},
	{"Module3.m3.doFuncNo", m3.doFuncNo},
};

const name_link_map name_links
{
	{ "Module2.link1", m2.link1 },
};



