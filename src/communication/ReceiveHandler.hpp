#pragma once

#include <boost/asio.hpp>

#include "TcpServer.hpp"

//#include "../../external/rapidjson/include/rapidjson/document.h"
//#include "../../external/rapidjson/include/rapidjson/filewritestream.h"
//#include "../../external/rapidjson/include/rapidjson/prettywriter.h"
//
//#include "../../external/rapidjson/include/rapidjson/rapidjson.h"
//#include "../../external/rapidjson/include/rapidjson/writer.h"
//#include "../../external/rapidjson/include/rapidjson/stringbuffer.h"


namespace Asm {

extern void do_handler(boost::asio::ip::tcp::socket& socket, size_t len, std::array<char, Asm::TcpServer::MAX_BUFFER_SIZE>& buffer);
extern void lo_handler(boost::asio::ip::tcp::socket& socket, size_t len, std::array<char, Asm::TcpServer::MAX_BUFFER_SIZE>& buffer);

}

//namespace Asm {
//
//	void receivedDOHandler(char* json, int size, boost::asio::ip::tcp::socket* socket) {
//		//std::cout << "receivedJson" << std::endl;
//
//		using namespace rapidjson;
//
//		Document doc, answer;
//		doc.Parse(json, size);
//
//		if (doc.HasParseError() == true) {
//			std::cout << "Parsing error" << std::endl;
//			return;
//		}
//
//		Value answer_key(kStringType);
//		Value answer_value;
//
//		for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
//
//			const char* doName = itr->name.GetString();
//			if (name_dataobjects.find(doName) != name_dataobjects.end()) {
//				auto doInstance = name_dataobjects.at(doName);
//				if (itr->value.IsNull()) {
//					//init first time
//					if (answer.IsNull())
//						answer.SetObject();
//
//					// request value
//					boost::apply_visitor([&](auto& d) { d.serialize(answer_value, answer.GetAllocator()); }, doInstance);
//					answer_key.SetString(doName, answer.GetAllocator());
//					answer.AddMember(answer_key, answer_value, answer.GetAllocator());
//				}
//				else {
//					// set value
//					boost::apply_visitor([&](auto& d) { d.deserialize(const_cast<rapidjson::Value&>(itr->value)); }, doInstance);
//				}
//			}
//		}
//
//		// test write answer to file
//		//std::FILE* fp_test = std::fopen("receivedDO-answer.json", "wb"); // non-Windows use "w"
//		//char writeBuffer[65536];
//		//FileWriteStream os(fp_test, writeBuffer, sizeof(writeBuffer));
//		//PrettyWriter<FileWriteStream> writer(os);
//		//answer.Accept(writer);
//		//fclose(fp_test);
//
//		StringBuffer buf;
//		Writer<StringBuffer> writer(buf);
//		answer.Accept(writer);
//
//		socket->write_some(boost::asio::buffer(buf.GetString(), buf.GetSize()));
//	}
//
//
//
//	void receivedLinkHandler(char* json, int size, boost::asio::ip::tcp::socket* socket) {
//		//std::cout << "receivedLink\n" << std::endl;
//		//std::cout << json << std::endl;
//
//		using namespace rapidjson;
//
//		Document doc, answer;
//		doc.Parse(json, size);
//
//		if (doc.HasParseError() == true) {
//			std::cout << json << std::endl;
//			std::cout << "Parsing error: " << doc.GetParseError() << std::endl;
//			return;
//		}
//
//de
//		for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
//			const char* linkName = itr->name.GetString();
//			if (name_links.find(linkName) != name_links.end()) {
//				auto linkInstance = name_links.at(linkName);
//				const Value& av = itr->value;
//				if (av.IsArray()) {
//					if (av.GetArray().Size() == 3) {
//						boost::apply_visitor([&](auto& l) {l.set(av.GetArray()[0].GetString(), name_dataobjects.at(av.GetArray()[1].GetString()), name_dataobjects.at(av.GetArray()[2].GetString()));  }, linkInstance);
//					}
//					else {
//						boost::apply_visitor([&](auto& l) {l.clear(av.GetArray()[0].GetString(), name_dataobjects.at(av.GetArray()[1].GetString()));  }, linkInstance);
//					}
//				}
//			}
//		}
//
//		//for debugging to file
//
//		//std::FILE* fp = std::fopen("receivedLink-debug.json", "wb"); // non-Windows use "w"
//		//char writeBuffer[65536];
//		//FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
//		//PrettyWriter<FileWriteStream> writer(os);
//		//doc.Accept(writer);
//		//fclose(fp);
//	}
//}
