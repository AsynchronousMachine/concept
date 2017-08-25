/*
** Definitions of DataObject and LinkObject handler functions used for communication
*/

#include <iostream>

#include "../../external/rapidjson/include/rapidjson/rapidjson.h"
#include "../../external/rapidjson/include/rapidjson/document.h"
#include "../../external/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../external/rapidjson/include/rapidjson/writer.h"
#include "../../external/rapidjson/include/rapidjson/error/en.h"

#include "../asm/asm.hpp"
#include "../maker/maker_reflection.hpp"
#include "../modules/global_modules.hpp"

#include "ReceiveHandler.hpp"

namespace Asm {

void do_handler(boost::asio::ip::tcp::socket& socket, size_t len, std::array<char, Asm::TcpServer::MAX_BUFFER_SIZE>& buffer)
{
    rapidjson::Document rjdoc_in;
    rapidjson::Document rjdoc_out(rapidjson::kObjectType);
    rapidjson::Value rjval_out;
    rapidjson::StringBuffer rjsb;
    rapidjson::Writer<rapidjson::StringBuffer> rjw(rjsb);

    buffer[len] = 0; //Terminate it with 0x00  definitely

    if(rjdoc_in.ParseInsitu(&buffer[0]).HasParseError()) {
        std::cout << "do_handler has parsing error at offset " << rjdoc_in.GetErrorOffset() << std::endl;
        std::cout << ">>>" << rapidjson::GetParseError_En(rjdoc_in.GetParseError()) << std::endl;
        return;
    }

    for (rapidjson::Value::ConstMemberIterator itr = rjdoc_in.MemberBegin(); itr != rjdoc_in.MemberEnd(); ++itr) {
        const char* doName = itr->name.GetString();
        if (name_dataobjects.find(doName) != name_dataobjects.end()) {
            auto doInstance = name_dataobjects.at(doName);
            if (itr->value.IsNull()) {
                std::cout << "do_handler got read request" << std::endl;
                boost::apply_visitor([&](auto& d) { d.serialize(rjval_out, rjdoc_out.GetAllocator()); }, doInstance);
                rjdoc_out.AddMember(rapidjson::StringRef(doName), rjval_out, rjdoc_out.GetAllocator());
            }
            else {
                std::cout << "do_handler got write request" << std::endl;
                boost::apply_visitor([&](auto& d) { d.deserialize(const_cast<rapidjson::Value&>(itr->value)); }, doInstance);
            }
        }
    }

    std::cout << "Direct access to the DataObjects of the test case:" << std::endl;
    inModule.DOintOutput.get([](const int i) { std::cout << "inModule.DOintOutput got: " << i << std::endl; });
    inModule.DOstringOutput.get([](const std::string s) { std::cout << "inModuleDOstringOutput got: " << s << std::endl; });

    rjdoc_out.Accept(rjw);

    std::cout << "Stringify and output the created DOM:" << std::endl;
    std::cout << rjsb.GetString() << std::endl;

    socket.write_some(boost::asio::buffer(rjsb.GetString(), rjsb.GetSize()));
}

void lo_handler(boost::asio::ip::tcp::socket& socket, size_t len, std::array<char, Asm::TcpServer::MAX_BUFFER_SIZE>& buffer)
{
    rapidjson::Document rjdoc_in;

    buffer[len] = 0; //Terminate it with 0x00  definitely

    if(rjdoc_in.ParseInsitu(&buffer[0]).HasParseError()) {
        std::cout << "lo_handler has parsing error at offset " << rjdoc_in.GetErrorOffset() << std::endl;
        std::cout << ">>>" << rapidjson::GetParseError_En(rjdoc_in.GetParseError()) << std::endl;
        return;
    }

    for (rapidjson::Value::ConstMemberIterator itr = rjdoc_in.MemberBegin(); itr != rjdoc_in.MemberEnd(); ++itr) {
        const char* linkName = itr->name.GetString();
        if (name_links.find(linkName) != name_links.end()) {
			auto linkInstance = name_links.at(linkName);
            const rapidjson::Value& v = itr->value;
            if (v.IsArray()) {
                if (v.GetArray().Size() == 3) {
                    std::cout << "link_handler got set request" << std::endl;
                    boost::apply_visitor([&](auto& l) {l.set(v.GetArray()[0].GetString(), name_dataobjects.at(v.GetArray()[1].GetString()), name_dataobjects.at(v.GetArray()[2].GetString())); }, linkInstance);
                }
                else {
                    std::cout << "link_handler got clear request" << std::endl;
                    boost::apply_visitor([&](auto& l) {l.clear(v.GetArray()[0].GetString(), name_dataobjects.at(v.GetArray()[1].GetString())); }, linkInstance);
                }
            }

        }
    }
}

}
