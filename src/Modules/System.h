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
		for (auto map_iter : do_serialize)
		{
			Asm::serializeFnct serializeDO = map_iter.second;
			serializeDO(json_value, doc.GetAllocator());
			if (!json_value.IsNull()) {
				key = map_iter.first;
				//json_key.SetString(StringRef(*key.c_str)); // always the last key!!!
				json_key.SetString(key.c_str(), doc.GetAllocator());
				doc.AddMember(json_key, json_value, doc.GetAllocator());
			}
		}

		FILE* fp = fopen("output.json", "wb"); // non-Windows use "w"
		char writeBuffer[65536];
		FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		PrettyWriter<FileWriteStream> writer(os);
		doc.Accept(writer);
		fclose(fp);
	}


	void deserialize()
	{
		using namespace rapidjson;
		FILE* fp = fopen("output.json", "rb"); // non-Windows use "r"
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
		for (Value::ConstMemberIterator iter = doc.MemberBegin(); iter != doc.MemberEnd(); ++iter)
		{
			std::string key = iter->name.GetString();
			Value& v = doc[key.c_str()];
			do_deserialize.at(key)(v);
		}
	}
};