#include "global_reflection.h"

const name_module_map modules
{
	{"ModuleStart.mstart1", &mstart1},
	{"ModuleMiddle.mmid1", &mmid1},
	{"ModuleEnd.mend1", &mend1},
	{"ModuleEnd.mend2", &mend2},
};

// Get out the module name for humans
const module_name_map module_names
{
	{&mstart1, "ModuleStart.mstart1"},
	{&mmid1, "ModuleMiddle.mmid1"},
	{&mend1, "ModuleEnd.mend1"},
	{&mend2, "ModuleEnd.mend2"},
};

const name_dataobject_map dos
{
	{"ModuleStart.mstart1.doInit", &mstart1.doInit},
	{"ModuleEnd.mend1.doEnd1", &mend1.doEnd1},
	{"ModuleEnd.mend1.doEnd2", &mend1.doEnd2},
	{"ModuleEnd.mend2.doEnd1", &mend2.doEnd1},
	{"ModuleEnd.mend2.doEnd2", &mend2.doEnd2},
};

// Get out the DO name for humans
const dataobject_name_map do_names
{
	{&mstart1.doInit, "ModuleStart.mstart1.doInit"},
	{&mend1.doEnd1, "ModuleEnd.mend1.doEnd1"},
	{&mend1.doEnd2, "ModuleEnd.mend1.doEnd2"},
	{&mend2.doEnd1, "ModuleEnd.mend2.doEnd1"},
	{&mend2.doEnd2, "ModuleEnd.mend2.doEnd2"},
};

const doset_map do_set
{
	{"ModuleStart.mstart1.doInit", [](const boost::any &a) {mstart1.doInit.set(a); } },
	{"ModuleEnd.mend1.doEnd1", [](const boost::any &a) {mend1.doEnd1.set(a); } },
	{"ModuleEnd.mend1.doEnd2", [](const boost::any &a) {mend1.doEnd2.set(a); } },
	{"ModuleEnd.mend2.doEnd1", [](const boost::any &a) {mend2.doEnd1.set(a); } },
	{"ModuleEnd.mend2.doEnd2", [](const boost::any &a) {mend2.doEnd2.set(a); } },
};

const registerlink_map set_links
{
	{"ModuleMiddle.mmid1.link2SumInt", [](std::string name, boost::any a1, boost::any a2) {mmid1.link2SumInt.set(name, a1, a2); } },
	{"ModuleMiddle.mmid1.link2SumString", [](std::string name, boost::any a1, boost::any a2) {mmid1.link2SumString.set(name, a1, a2); } },
	{"ModuleEnd.mend1.link2Int", [](std::string name, boost::any a1, boost::any a2) {mend1.link2Int.set(name, a1, a2); } },
	{"ModuleEnd.mend1.link2String", [](std::string name, boost::any a1, boost::any a2) {mend1.link2String.set(name, a1, a2); } },
	{"ModuleEnd.mend2.link2Int", [](std::string name, boost::any a1, boost::any a2) {mend2.link2Int.set(name, a1, a2); } },
	{"ModuleEnd.mend2.link2String", [](std::string name, boost::any a1, boost::any a2) {mend2.link2String.set(name, a1, a2); } },
};

const unregisterlink_map clear_links
{
	{"ModuleMiddle.mmid1.link2SumInt", [](std::string name, boost::any a) {mmid1.link2SumInt.clear(name, a); } },
	{"ModuleMiddle.mmid1.link2SumString", [](std::string name, boost::any a) {mmid1.link2SumString.clear(name, a); } },
	{"ModuleEnd.mend1.link2Int", [](std::string name, boost::any a) {mend1.link2Int.clear(name, a); } },
	{"ModuleEnd.mend1.link2String", [](std::string name, boost::any a) {mend1.link2String.clear(name, a); } },
	{"ModuleEnd.mend2.link2Int", [](std::string name, boost::any a) {mend2.link2Int.clear(name, a); } },
	{"ModuleEnd.mend2.link2String", [](std::string name, boost::any a) {mend2.link2String.clear(name, a); } },
};

