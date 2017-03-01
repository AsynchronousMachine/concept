#pragma once
#include <iostream>
#include <cstdio>

#include "../asm.hpp"
#include "../global_reflection.h"

#include "../../external/rapidjson/include/rapidjson/prettywriter.h"
#include "../../external/rapidjson/include/rapidjson/filewritestream.h"
#include "../../external/rapidjson/include/rapidjson/filereadstream.h"

class System
{
public:
	void serialize()
	{
		using namespace rapidjson;
		Document doc; // Null
		doc.SetObject();

		std::string key;
		Value json_key(kStringType);
		Value json_value;

		//iterate over all public DOs
		for (auto map_iter : dataobject_map)
		{
			boost::apply_visitor([&](auto& d) { d.serialize(json_value, doc.GetAllocator()); }, map_iter.second);
			if (!json_value.IsNull()) {
				key = map_iter.first;
				//json_key.SetString(StringRef(*key.c_str)); // always the last key!!!
				json_key.SetString(key.c_str(), doc.GetAllocator());
				doc.AddMember(json_key, json_value, doc.GetAllocator());
			}
		}

		std::FILE* fp = std::fopen("output.json", "w"); // Windows use "wb"
		char writeBuffer[65536];
		FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		PrettyWriter<FileWriteStream> writer(os);
		doc.Accept(writer);
		fclose(fp);
	}


	void deserialize()
	{
		using namespace rapidjson;
		std::FILE* fp = std::fopen("output.json", "r"); // Windows use "rb"
		char readBuffer[65536];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));
		Document doc;
		doc.ParseStream(is);
		fclose(fp);

		if (doc.HasParseError() == true)
		{
			std::cout << "Parsing error" << std::endl;
			return;
		}

		//iterate over all public DOs
		for (auto map_iter : dataobject_map)
		{
			std::string key = map_iter.first;
			Value& v = doc[key.c_str()];
			boost::apply_visitor([&](auto& d) { d.deserialize(v); }, map_iter.second);
		}
	}

};