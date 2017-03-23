#pragma once
#include "../../external/rapidjson/include/rapidjson/document.h"

#include "../udp/udpServer.hpp"

namespace Asm {

	class UdpCommand {

	public:


		//UdpCommand() : port(9500) { runServer(); }

		UdpCommand(int port = 9500) :
			port(port),
			udpHandler4DO(new Asm::UdpServer(udpIOService1, port, [&](char *json) {receivedDO(json); })),
			udpHandler4Link(new Asm::UdpServer(udpIOService2, port + 1, [&](char *json) {receivedLink(json); }))
		{
			run();
		}

		void run()
		{
			boost::thread* t1 = new boost::thread([&]() {udpIOService1.run(); });
			t1->detach();
			boost::thread* t2 = new boost::thread([&]() {udpIOService2.run(); });
			t2->detach();
		}

		void receivedDO(char* json) {
			std::cout << "receivedJson" << std::endl;

			using namespace rapidjson;

			Document doc, answer;
			doc.Parse(json);

			if (doc.HasParseError() == true)
			{
				std::cout << "Parsing error" << std::endl;
				return;
			}

			//answer.SetObject();
			Value answer_key(kStringType);
			Value answer_value;

			for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
			{

				const char* doName = itr->name.GetString();
				if (dataobject_map.find(doName) != dataobject_map.end())
				{
					auto doInstance = dataobject_map.at(doName);
					if (itr->value.IsNull())
					{
						//init first time
						if (answer.IsNull())
							answer.SetObject();

						// request value
						boost::apply_visitor([&](auto& d) { d.serialize(answer_value, answer.GetAllocator()); }, doInstance);
						answer_key.SetString(doName, answer.GetAllocator());
						answer.AddMember(answer_key, answer_value, answer.GetAllocator());
					}
					else
					{
						// set value
						//boost::apply_visitor([&](auto& d) { d.deserialize(v[itr->name.GetString()]); }, doInstance);
						boost::apply_visitor([&](auto& d) { d.deserialize(const_cast<rapidjson::Value&>(itr->value)); }, doInstance);
						// -> Konvertierung von Argument 1 von "rapidjson::GenericObject<true,rapidjson::GenericValue<rapidjson::UTF8<char>,rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>>>" in "rapidjson::Value &" nicht möglich
					}
				}
			}

			// test write answer to file
			std::FILE* fp_test = std::fopen("output-test-answer.json", "wb"); // non-Windows use "w"
			char writeBuffer[65536];
			FileWriteStream os(fp_test, writeBuffer, sizeof(writeBuffer));
			PrettyWriter<FileWriteStream> writer(os);
			answer.Accept(writer);
			fclose(fp_test);

			/*
			std::FILE* fp = std::fopen("output-command.json", "wb"); // non-Windows use "w"
			char writeBuffer[65536];
			FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
			PrettyWriter<FileWriteStream> writer(os);
			doc.Accept(writer);
			fclose(fp);
			*/
		}


		void receivedLink(char* json) {
			std::cout << "receivedLink" << std::endl;

			using namespace rapidjson;

			Document doc, answer;
			doc.Parse(json);

			if (doc.HasParseError() == true)
			{
				std::cout << json << std::endl;
				std::cout << "Parsing error: " << doc.GetParseError() << std::endl;
				return;
			}


			for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
			{
				const char* linkName = itr->name.GetString();
				if (link_map.find(linkName) != link_map.end())
				{
					auto linkInstance = link_map.at(linkName);
					const Value& av = itr->value;
					if (av.IsArray()) {
						std::cout << "array found " << av.GetArray().Size() << std::endl;
						if (av.GetArray().Size() == 3)
						{
							boost::apply_visitor([&](auto& l) {l.set(av.GetArray()[0].GetString(), dataobject_map.at(av.GetArray()[1].GetString()), dataobject_map.at(av.GetArray()[2].GetString()));  }, linkInstance);
						}
						else
						{
							boost::apply_visitor([&](auto& l) {l.clear(av.GetArray()[0].GetString(), dataobject_map.at(av.GetArray()[1].GetString()));  }, linkInstance);
						}
					}
					//boost::apply_visitor([&](auto& l) {l.clear(av.GetArray()[0].GetString(),boost::get<Asm::DataObject<int>&>(dataobject_map.at(av.GetArray()[1].GetString())));  }, linkInstance);

					// set value
						//boost::apply_visitor([&](auto& d) { d.deserialize(v[itr->name.GetString()]); }, doInstance);
						//boost::apply_visitor([&](auto& d) { d.deserialize(const_cast<rapidjson::Value&>(itr->value)); }, doInstance);
						// -> Konvertierung von Argument 1 von "rapidjson::GenericObject<true,rapidjson::GenericValue<rapidjson::UTF8<char>,rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>>>" in "rapidjson::Value &" nicht möglich

				}
			}

			// test write answer to file

			/*
			std::FILE* fp_test = std::fopen("output-test-.json", "wb"); // non-Windows use "w"
			char writeBuffer[65536];
			FileWriteStream os(fp_test, writeBuffer, sizeof(writeBuffer));
			PrettyWriter<FileWriteStream> writer(os);
			answer.Accept(writer);
			fclose(fp_test);
			*/


			std::FILE* fp = std::fopen("output-test-LINK.json", "wb"); // non-Windows use "w"
			char writeBuffer[65536];
			FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
			PrettyWriter<FileWriteStream> writer(os);
			doc.Accept(writer);
			fclose(fp);

		}


	private:
		int port;
		boost::asio::io_service udpIOService1;
		boost::asio::io_service udpIOService2;
		//Asm::UdpServer* udpServer;
		std::shared_ptr<Asm::UdpServer> udpHandler4DO;
		std::shared_ptr<Asm::UdpServer> udpHandler4Link;
	};

}
