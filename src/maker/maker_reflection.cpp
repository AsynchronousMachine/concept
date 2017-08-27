#include "maker_reflection.hpp"
#include "../modules/global_modules.hpp"

// Get out the module name for humans
const id_string_map module_name {
	{&inModule, "InputModule.inModule"},
	{&outModule, "OutputModule.outModule"},
	{&processModule, "ProcessModule.processModule"},
	{&serModule, "SerializeModule.serModule"},
	{&sysModule, "SystemModule.sysModule"},
	{&tbbModule, "TBBModule.tbbModule"},
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
	{&sysModule.serializeAll, "SystemModule.sysModule.serializeAll"},
	{&sysModule.deserializeAll, "SystemModule.sysModule.deserializeAll"},
	{&sysModule.printModules, "SystemModule.sysModule.printModules"},
	{&tbbModule.DOcomplexInOut, "TBBModule.tbbModule.DOcomplexInOut"},
};

// Get out by name all modules, all their dataobjects with type and their links for humans
const print_module_map print_modules {
	{"InputModule.inModule","  |> DataObject<int> DOintOutput\n  |> DataObject<std::string> DOstringOutput\n"},
	{"OutputModule.outModule","  |> DataObject<int> DOintInput\n  |> DataObject<std::string> DOstringInput\n  |> LinkObject<DataObject<MyComplexDOType>, DataObject<int> > LinkInt\n  |> LinkObject<DataObject<MyComplexDOType>, DataObject<std::string> > LinkString\n"},
	{"ProcessModule.processModule","  |> DataObject<MyComplexDOType> DOcomplexInOut\n  |> LinkObject<DataObject<int>, DataObject<MyComplexDOType> > LinkInt\n  |> LinkObject<DataObject<std::string>, DataObject<MyComplexDOType> > LinkString\n"},
	{"SerializeModule.serModule","  |> DataObject<int> doInt\n  |> DataObject<double> doDouble\n  |> DataObject<std::string> doString\n  |> DataObject<bool> doBool\n  |> DataObject<std::string> doString2\n  |> DataObject<std::string> doString3\n  |> DataObject<MyComplexDOType> doMyComplexDOType\n"},
	{"SystemModule.sysModule","  |> DataObject<bool> serializeAll\n  |> DataObject<bool> deserializeAll\n  |> DataObject<bool> printModules\n"},
	{"TBBModule.tbbModule","  |> DataObject<MyComplexDOType> DOcomplexInOut\n  |> LinkObject<DataObject<int>, DataObject<MyComplexDOType> > LinkInt\n"},
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
	{"SystemModule.sysModule.serializeAll", sysModule.serializeAll},
	{"SystemModule.sysModule.deserializeAll", sysModule.deserializeAll},
	{"SystemModule.sysModule.printModules", sysModule.printModules},
	{"TBBModule.tbbModule.DOcomplexInOut", tbbModule.DOcomplexInOut},
};

const name_link_map name_links {
	{"OutputModule.outModule.LinkInt", outModule.LinkInt},
	{"OutputModule.outModule.LinkString", outModule.LinkString},
	{"ProcessModule.processModule.LinkInt", processModule.LinkInt},
	{"ProcessModule.processModule.LinkString", processModule.LinkString},
	{"TBBModule.tbbModule.LinkInt", tbbModule.LinkInt},
};

