#pragma once
#include <map>
#include <unordered_map>
#include <functional>

#include <boost/any.hpp>

#include "../external/rapidjson/include/rapidjson/document.h"

#include "asm.hpp"

using module_name_map = std::unordered_map<void*, std::string>;
using name_module_map = std::unordered_map<std::string, boost::any>;
using dataobject_name_map = std::unordered_map<void*, std::string>;
using name_dataobject_map = std::unordered_map<std::string, boost::any>;
using registerlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any, boost::any)>>;
using unregisterlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any)>>;
using doserialize_map = std::unordered_map<std::string, Asm::serializeFnct>;
using dodeserialize_map = std::unordered_map<std::string, Asm::deserializeFnct>;
using print_module_map = std::map<std::string, std::string>;

extern const module_name_map module_names;
extern const name_module_map modules;
extern const dataobject_name_map do_names;
extern const name_dataobject_map dos;
extern const registerlink_map set_links;
extern const unregisterlink_map clear_links;
extern const doserialize_map do_serialize;
extern const dodeserialize_map do_deserialize;
extern const print_module_map print_modules;

#include "global_instances.h"  //von main.cpp kopiert

#include <boost/hana.hpp>

const auto dataobject_map = boost::hana::make_map(
	boost::hana::make_pair(BOOST_HANA_STRING("Module1.m1.do1"), &m1.do1),
	boost::hana::make_pair(BOOST_HANA_STRING("Module1.m1.do2"), &m1.do2),
	boost::hana::make_pair(BOOST_HANA_STRING("Module2.m2.do1"), &m2.do1),
	boost::hana::make_pair(BOOST_HANA_STRING("Module2.m2.do2"), &m2.do2),
	boost::hana::make_pair(BOOST_HANA_STRING("Module3.m3.doUInt"), &m3.doUInt),
	boost::hana::make_pair(BOOST_HANA_STRING("Module3.m3.doInt"), &m3.doInt),
	boost::hana::make_pair(BOOST_HANA_STRING("Module3.m3.doDouble"), &m3.doDouble),
	boost::hana::make_pair(BOOST_HANA_STRING("Module3.m3.doBool"), &m3.doBool),
	boost::hana::make_pair(BOOST_HANA_STRING("Module3.m3.doString"), &m3.doString),
	boost::hana::make_pair(BOOST_HANA_STRING("Module3.m3.doFunc"), &m3.doFunc),
	boost::hana::make_pair(BOOST_HANA_STRING("Module3.m3.doFuncNo"), &m3.doFuncNo)
);

/*
die übliche Aufteilung wie bei den anderen Maps ist nicht möglich

Problem 1:

Der Zugriff auf dataobject_map funktioniert nur innerhalb dieser Datei.
Deshalb musste serializetest hier implementiert werden.
*/

void serializetest()
{
	using namespace rapidjson;
	Document doc; // Null
	doc.SetObject();

	std::string key;
	Value json_key(kStringType);
	Value json_value;

	//iterate over all public DOs
	boost::hana::for_each(dataobject_map, [&](const auto& x)
	{
		boost::hana::second(x)->serialize(json_value, doc.GetAllocator());
		if (!json_value.IsNull()) {
			key = do_names.at(boost::hana::second(x)); // get the std::string and not BOOST_HANA_STRING 
			json_key.SetString(key.c_str(), doc.GetAllocator());
			doc.AddMember(json_key, json_value, doc.GetAllocator());
		}
	});

	FILE* fp = fopen("outputhana.json", "wb"); // non-Windows use "w"
	char writeBuffer[65536];
	FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	PrettyWriter<FileWriteStream> writer(os);
	doc.Accept(writer);
	fclose(fp);
}

/*
Versuch Workaround über Methoden:

Problem 2:
std::string funktioniert nicht innerhalb einer boost::hana::map
BOOST_HANA_STRING() lässt sich nicht mit einem Parameter bzw. einer Variablen aufrufen
Gibt es einen Weg, den key zur Laufzeit aus einem std::string Parameter für die Abfrage mit boost::hana::find() bzw. boost::hana::at_key() zu generieren?
*/

auto getDO(const std::string doName2get)
{
	//return boost::hana::at_key(dataobject_map, BOOST_HANA_STRING(doName2get)); //geht nicht
	return boost::hana::at_key(dataobject_map, BOOST_HANA_STRING("Module1.m1.do2")); //so nicht verwendbar
}



auto getAllDOs()
{
	return boost::hana::values(dataobject_map);
}
