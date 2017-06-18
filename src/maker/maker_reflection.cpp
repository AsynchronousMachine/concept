#include "maker_reflection.hpp"
#include "../modules/global_modules.hpp"

// Get out the module name for humans
const id_string_map module_name {
	{&inModule, "InputModule.inModule"},
	{&outModule, "OutputModule.outModule"},
	{&processModule, "ProcessModule.processModule"},
	{&serModule, "SerializeModule.serModule"},
};

// Get out the DO name for humans
const id_string_map do_names
{
	{&inModule.DOintOutput, "InputModule.inModule.DOintOutput"},
	{&inModule.DOstringOutput, "InputModule.inModule.DOstringOutput"},
	{&outModule.DOintInput, "OutputModule.outModule.DOintInput"},
	{&outModule.DOstringInput, "OutputModule.outModule.DOstringInput"},
	{&processModule.DOcomplexInOut, "ProcessModule.processModule.DOcomplexInOut"},
	{&serModule.doInt, "SerializeModule.serModule.doInt"},
	{&serModule.doDouble, "SerializeModule.serModule.doDouble"},
	{&serModule.doString, "SerializeModule.serModule.doString"},
	{&serModule.doBool, "SerializeModule.serModule.doBool"},
	{&serModule.doString2, "SerializeModule.serModule.doString2"},
	{&serModule.doString3, "SerializeModule.serModule.doString3"},
	{&serModule.doMyComplexDOType, "SerializeModule.serModule.doMyComplexDOType"},
};

// Get out by name all modules, all their dataobjects with type and their links for humans
const print_module_map print_modules {
	{"inModule","InputModule::inModule\n  |> Asm::DataObject<int> DOintOutput\n  |> Asm::DataObject<std::string> DOstringOutput\n"},
	{"outModule","OutputModule::outModule\n  |> Asm::DataObject<int> DOintInput\n  |> Asm::DataObject<std::string> DOstringInput\n  |> Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<int> > LinkInt\n  |> Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<std::string> > LinkString\n"},
	{"processModule","ProcessModule::processModule\n  |> Asm::DataObject<MyComplexDOType> DOcomplexInOut\n  |> Asm::LinkObject<Asm::DataObject<int>, Asm::DataObject<MyComplexDOType> > LinkInt\n  |> Asm::LinkObject<Asm::DataObject<std::string>, Asm::DataObject<MyComplexDOType> > LinkString\n"},
	{"serModule","SerializeModule::serModule\n  |> Asm::DataObject<int> doInt\n  |> Asm::DataObject<double> doDouble\n  |> Asm::DataObject<std::string> doString\n  |> Asm::DataObject<bool> doBool\n  |> Asm::DataObject<std::string> doString2\n  |> Asm::DataObject<std::string> doString3\n  |> Asm::DataObject<MyComplexDOType> doMyComplexDOType\n"},
};

// Besser Class::Instance|>DOs ???
const print_module_map print_modules2 {
	{"InputModule::inModule", "  |> Asm::DataObject<int> DOintOutput\n  |> Asm::DataObject<std::string> DOstringOutput"},
	{"OutputModule::outModule", "  |> Asm::DataObject<int> DOintInput\n  |> Asm::DataObject<std::string> DOstringInput\n  |> Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<int> > LinkInt\n  |> Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<std::string> > LinkString"},
	{"ProcessModule::processModule", "  |> Asm::DataObject<MyComplexDOType> DOcomplexInOut\n  |> Asm::LinkObject<Asm::DataObject<int>, Asm::DataObject<MyComplexDOType> > LinkInt\n  |> Asm::LinkObject<Asm::DataObject<std::string>, Asm::DataObject<MyComplexDOType> > LinkString"},
	{"SerializeModule::serModule", "  |> Asm::DataObject<int> doInt\n  |> Asm::DataObject<double> doDouble\n  |> Asm::DataObject<std::string> doString\n  |> Asm::DataObject<bool> doBool\n  |> Asm::DataObject<std::string> doString2\n  |> Asm::DataObject<std::string> doString3\n  |> Asm::DataObject<MyComplexDOType> doMyComplexDOType"},
};

const name_dataobject_map name_dataobjects {
	{"InputModule.inModule.DOintOutput", inModule.DOintOutput},
	{"InputModule.inModule.DOstringOutput", inModule.DOstringOutput},
	{"OutputModule.outModule.DOintInput", outModule.DOintInput},
	{"OutputModule.outModule.DOstringInput", outModule.DOstringInput},
	{"ProcessModule.processModule.DOcomplexInOut", processModule.DOcomplexInOut},
    {"SerializeModule.serModule.doInt", serModule.doInt},
	{"SerializeModule.serModule.doDouble", serModule.doDouble},
	{"SerializeModule.serModule.doString", serModule.doString},
	{"SerializeModule.serModule.doBool", serModule.doBool},
	{"SerializeModule.serModule.doString2", serModule.doString2},
	{"SerializeModule.serModule.doString3", serModule.doString3},
	{"SerializeModule.serModule.doMyComplexDOType", serModule.doMyComplexDOType},
};

const name_link_map name_links {
	{"OutputModule.outModule.LinkInt", outModule.LinkInt},
	{"OutputModule.outModule.LinkString", outModule.LinkString},
	{"ProcessModule.processModule.LinkInt", processModule.LinkInt},
	{"ProcessModule.processModule.LinkString", processModule.LinkString},
};

