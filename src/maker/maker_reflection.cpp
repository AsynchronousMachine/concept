#include "../modules/global_modules.hpp"
#include "maker_reflection.hpp"

// Get out the module name for humans
const id_string_map module_name {
	{&inModule, "InputModule.inModule"},
	{&outModule, "OutputModule.outModule"},
	{&processModule, "ProcessModule.processModule"},
};

// Get out the DO name for humans
const id_string_map do_names
{
	{&inModule.DOintOutput, "InputModule.inModule.DOintOutput"},
	{&inModule.DOstringOutput, "InputModule.inModule.DOstringOutput"},
	{&outModule.DOintInput, "OutputModule.outModule.DOintInput"},
	{&outModule.DOstringInput, "OutputModule.outModule.DOstringInput"},
	{&processModule.DOcomplexInOut, "ProcessModule.processModule.DOcomplexInOut"},
};

// Get out by name all modules, all their dataobjects with type and their links for humans
const print_module_map print_modules {
	{"inModule","InputModule::inModule\n  |> Asm::DataObject<int> DOintOutput\n  |> Asm::DataObject<std::string> DOstringOutput\n"},
	{"outModule","OutputModule::outModule\n  |> Asm::DataObject<int> DOintInput\n  |> Asm::DataObject<std::string> DOstringInput\n  |> Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<int> > LinkInt\n  |> Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<std::string> > LinkString\n"},
	{"processModule","ProcessModule::processModule\n  |> Asm::DataObject<MyComplexDOType> DOcomplexInOut\n  |> Asm::LinkObject<Asm::DataObject<int>, Asm::DataObject<MyComplexDOType> > LinkInt\n  |> Asm::LinkObject<Asm::DataObject<std::string>, Asm::DataObject<MyComplexDOType> > LinkString\n"},
};

const name_dataobject_map name_dataobjects {
	{"InputModule.inModule.DOintOutput", inModule.DOintOutput},
	{"InputModule.inModule.DOstringOutput", inModule.DOstringOutput},
	{"OutputModule.outModule.DOintInput", outModule.DOintInput},
	{"OutputModule.outModule.DOstringInput", outModule.DOstringInput},
	{"ProcessModule.processModule.DOcomplexInOut", processModule.DOcomplexInOut},
};

const name_link_map name_links {
	{"OutputModule.outModule.LinkInt", outModule.LinkInt},
	{"OutputModule.outModule.LinkString", outModule.LinkString},
	{"ProcessModule.processModule.LinkInt", processModule.LinkInt},
	{"ProcessModule.processModule.LinkString", processModule.LinkString},
};

