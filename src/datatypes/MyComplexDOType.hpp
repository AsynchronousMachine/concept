//#pragma once
#include <string>
#include <iostream>

#include "../../external/rapidjson/include/rapidjson/document.h"

class MyComplexDOType
{
public:
	int inputCounter;
	int outputCounter;
	std::string message;

	//MyComplexDOType() :
	//	inputCounter(0),
	//	outputCounter(0),
	//	message("")
	//{}

	// The type class of the dataobject knows how to (de)serialize itself
	void serializeMe(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
	{
		using namespace rapidjson;
		Value json_key(kStringType);
		Value json_value;

		value.SetObject();

		json_key.SetString("amount inputs", allocator);
		json_value.SetInt(inputCounter);
		value.AddMember(json_key, json_value, allocator);

		json_key.SetString("amount outputs", allocator);
		json_value.SetInt(outputCounter);
		value.AddMember(json_key, json_value, allocator);

		json_key.SetString("message", allocator);
		json_value.SetString(message.c_str(), allocator);
		value.AddMember(json_key, json_value, allocator);
	}

	void deserializeMe(rapidjson::Value& value)
	{
		using namespace rapidjson;
		for (Value::ConstMemberIterator itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr)
		{
			//const char* doName = itr->name.GetString();
			std::string name = itr->name.GetString();
			Value& v = value[itr->name.GetString()];
			if (name=="amount inputs") 
			{
				inputCounter = v.GetInt();
			}
			else if (name == "amount outputs")
			{
				outputCounter = v.GetInt();
			}
			else if (name == "message")
			{
				std::cout << "deserializeMe: "<< v.GetString() << std::endl;
				message = std::string(v.GetString());
			};
		}
		std::cout << inputCounter << outputCounter << message << this << std::endl;
	}

};