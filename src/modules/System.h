#pragma once
#include <iostream>
#include <cstdio>

#include "../asm/asm.hpp"
#include "maker_reflection.hpp"

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
		for (auto map_iter : name_dataobjects)
		{
			boost::apply_visitor([&](auto& d) { d.serialize(json_value, doc.GetAllocator()); }, map_iter.second);
			if (!json_value.IsNull()) {
				key = map_iter.first;
				//json_key.SetString(StringRef(*key.c_str)); // always the last key!!!
				json_key.SetString(key.c_str(), doc.GetAllocator());
				doc.AddMember(json_key, json_value, doc.GetAllocator());
			}
		}

		std::FILE* fp = std::fopen("output-variant.json", "wb"); // non-Windows use "w"
		char writeBuffer[65536];
		FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		PrettyWriter<FileWriteStream> writer(os);
		doc.Accept(writer);
		fclose(fp);
	}

	void deserialize()
	{
		using namespace rapidjson;
		std::FILE* fp = std::fopen("output-variant.json", "rb"); // non-Windows use "r"
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

		//iterate over all DO-keys from json file
		for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
		{
			//const char* doName = itr->name.GetString();
			Value& v = doc[itr->name.GetString()];
			try {
				boost::apply_visitor([&](auto& d) { d.deserialize(v); }, name_dataobjects.at(itr->name.GetString()));
			}
			catch (const std::out_of_range& e) {
				std::cout << "Missing " << itr->name.GetString() << std::endl;
			}
			
		}
	}

	void walkthrough()
	{
		using namespace rapidjson;
		std::FILE* fp = std::fopen("output-test.json", "rb"); // non-Windows use "r"
		char readBuffer[65536];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));
		Document doc, answer;
		doc.ParseStream(is);
		fclose(fp);

		if (doc.HasParseError() == true)
		{
			std::cout << "Parsing error" << std::endl;
			return;
		}

		//answer.SetObject();
		Value answer_key(kStringType);
		Value answer_value;


		Value& v = doc["do"];
		
		for (Value::ConstMemberIterator itr = v.MemberBegin(); itr != v.MemberEnd(); ++itr)
		{
			
			const char* doName = itr->name.GetString();
			printf("Type of member %s %d\n", doName, itr->value.IsNull());
			auto doInstance = name_dataobjects.at(doName);
			if (itr->value.IsNull())
			{
				//init first time
				if(answer.IsNull())
					answer.SetObject();

				// request value
				boost::apply_visitor([&](auto& d) { d.serialize(answer_value, answer.GetAllocator()); }, doInstance);
				answer_key.SetString(doName, answer.GetAllocator());
				answer.AddMember(answer_key, answer_value, answer.GetAllocator());
			}
			else
			{
				// set value
				boost::apply_visitor([&](auto& d) { d.deserialize(v[itr->name.GetString()]); }, doInstance);
			}
		}

		// test write answer to file
		std::FILE* fp_test = std::fopen("output-test-answer.json", "wb"); // non-Windows use "w"
		char writeBuffer[65536];
		FileWriteStream os(fp_test, writeBuffer, sizeof(writeBuffer));
		PrettyWriter<FileWriteStream> writer(os);
		answer.Accept(writer);
		fclose(fp_test);		
	}
};