#include "../modules/global_modules.hpp"

#include "maker_reflection.hpp"

// Get out the module name for humans
const id_string_map module_name
{
	{&m1, "Module1.m1"}
};

// Get out the DO name for humans
const id_string_map do_names
{
	{ &m1.do1, "Module1.m1.do1" },
	{ &m1.do2, "Module1.m1.do2" }
};

// Get out by name all modules, all their dataobjects with type and their links for humans
const print_module_map print_modules
{
	{ "m1","Module1::m1\n  |> Asm::DataObject<int> do1\n  |> Asm::DataObject<std::string> do2\n" }
};

const name_dataobject_map name_dataobjects
{
	{"Module1.m1.do1", m1.do1},
	{"Module1.m1.do2", m1.do2}
};

const name_link_map name_links
{
};



