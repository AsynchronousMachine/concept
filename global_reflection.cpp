#include "global_reflection.h"

const name_module_map modules
{
	{"Module1.m1", &m1},
	{"Module2.m2", &m2},
};

// Get out the module name for humans
const module_name_map module_names
{
	{&m1, "Module1.m1"},
	{&m2, "Module2.m2"},
};

const name_dataobject_map dos
{
	{"Module1.m1.do1", &m1.do1},
	{"Module1.m1.do2", &m1.do2},
	{"Module2.m2.do1", &m2.do1},
	{"Module2.m2.do2", &m2.do2},
};

// Get out the DO name for humans
const dataobject_name_map do_names
{
	{&m1.do1, "Module1.m1.do1"},
	{&m1.do2, "Module1.m1.do2"},
	{&m2.do1, "Module2.m2.do1"},
	{&m2.do2, "Module2.m2.do2"},
};

const doset_map do_set
{
	{"Module1.m1.do1", [](const boost::any &a) {m1.do1.set(a); } },
	{"Module1.m1.do2", [](const boost::any &a) {m1.do2.set(a); } },
	{"Module2.m2.do1", [](const boost::any &a) {m2.do1.set(a); } },
	{"Module2.m2.do2", [](const boost::any &a) {m2.do2.set(a); } },
};

const registerlink_map set_links
{
	{"Module2.m2.link1", [](std::string name, boost::any a1, boost::any a2) {m2.link1.set(name, a1, a2); } },
	{"Module2.m2.link2", [](std::string name, boost::any a1, boost::any a2) {m2.link2.set(name, a1, a2); } },
	{"Module2.m2.linkComplex", [](std::string name, boost::any a1, boost::any a2) {m2.linkComplex.set(name, a1, a2); } },
};

const unregisterlink_map clear_links
{
	{"Module2.m2.link1", [](std::string name, boost::any a) {m2.link1.clear(name, a); } },
	{"Module2.m2.link2", [](std::string name, boost::any a) {m2.link2.clear(name, a); } },
	{"Module2.m2.linkComplex", [](std::string name, boost::any a) {m2.linkComplex.clear(name, a); } },
};

