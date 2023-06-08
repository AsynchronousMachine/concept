/*
** Definitions of DataObject and LinkObject handler functions used for communication
*/

#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "../asm/asm.hpp"
#include "../logger/logger.hpp"
#include "../maker/maker_reflection.hpp"
#include "../modules/global_modules.hpp"

#include "ReceiveHandler.hpp"

namespace Asm {

void do_handler(boost::asio::ip::tcp::socket &socket, size_t len, std::array<char, Asm::TcpServer::max_buffer_size> &buffer) {
    boost::system::error_code ec;
    rapidjson::Document rjdoc_in;
    rapidjson::Document rjdoc_out(rapidjson::kObjectType);
    rapidjson::Value rjval_out;
    rapidjson::StringBuffer rjsb;
    rapidjson::Writer<rapidjson::StringBuffer> rjw(rjsb);

    if (len >= Asm::TcpServer::max_buffer_size) // Terminate it with 0x00 definitely
        buffer[Asm::TcpServer::max_buffer_size - 1] = 0;
    else
        buffer[len] = 0;

    if (rjdoc_in.ParseInsitu(&buffer[0]).HasParseError()) {
        Logger::pLOG->error("do_handler has parsing error at offset {}", rjdoc_in.GetErrorOffset());
        Logger::pLOG->error(">{}", rapidjson::GetParseError_En(rjdoc_in.GetParseError()));
        return;
    }

    for (rapidjson::Value::ConstMemberIterator itr = rjdoc_in.MemberBegin(); itr != rjdoc_in.MemberEnd(); ++itr) {
        const char *doName = itr->name.GetString();
        if (name_dataobjects.find(doName) != name_dataobjects.end()) {
            auto doInstance = name_dataobjects.at(doName);
            if (itr->value.IsNull()) {
                Logger::pLOG->trace("do_handler got read request for {}", doName);
                boost::apply_visitor([&](auto &d) { d.serialize(rjval_out, rjdoc_out.GetAllocator()); }, doInstance);
                rjdoc_out.AddMember(rapidjson::StringRef(doName), rjval_out, rjdoc_out.GetAllocator());
            } else {
                Logger::pLOG->trace("do_handler got write request for {}", doName);
                boost::apply_visitor([&](auto &d) { d.deserialize(const_cast<rapidjson::Value &>(itr->value)); }, doInstance);
            }
        }
    }

    Logger::pLOG->info("Direct access to the DataObjects of the test case:");
    inModule.DOintOutput.get([](const int i) { Logger::pLOG->info("inModule.DOintOutput got: {}", i); });
    inModule.DOstringOutput.get([](const std::string s) { Logger::pLOG->info("inModuleDOstringOutput got: {}", s); });

    rjdoc_out.Accept(rjw);

    Logger::pLOG->info("Stringify and output the created DOM:");
    Logger::pLOG->info("{}", rjsb.GetString());

    // Swallow all faults from entpoints
    socket.write_some(boost::asio::buffer(rjsb.GetString(), rjsb.GetSize()), ec);
}

void lo_handler(boost::asio::ip::tcp::socket &, size_t len, std::array<char, Asm::TcpServer::max_buffer_size> &buffer) {
    rapidjson::Document rjdoc_in;

    if (len >= Asm::TcpServer::max_buffer_size) // Terminate it with 0x00 definitely
        buffer[Asm::TcpServer::max_buffer_size - 1] = 0;
    else
        buffer[len] = 0;

    if (rjdoc_in.ParseInsitu(&buffer[0]).HasParseError()) {
        Logger::pLOG->error("lo_handler has parsing error at offset {}", rjdoc_in.GetErrorOffset());
        Logger::pLOG->error(">{}", rapidjson::GetParseError_En(rjdoc_in.GetParseError()));
        return;
    }

    for (rapidjson::Value::ConstMemberIterator itr = rjdoc_in.MemberBegin(); itr != rjdoc_in.MemberEnd(); ++itr) {
        const char *linkName = itr->name.GetString();
        if (name_links.find(linkName) != name_links.end()) {
            auto linkInstance = name_links.at(linkName);
            const rapidjson::Value &v = itr->value;
            if (v.IsArray()) {
                if (v.GetArray().Size() == 3) {
                    Logger::pLOG->info("link_handler got set request");
                    boost::apply_visitor(
                        [&](auto &l) {
                            l.set(v.GetArray()[0].GetString(), name_dataobjects.at(v.GetArray()[1].GetString()),
                                  name_dataobjects.at(v.GetArray()[2].GetString()));
                        },
                        linkInstance);
                } else {
                    Logger::pLOG->info("link_handler got clear request");
                    boost::apply_visitor([&](auto &l) { l.clear(v.GetArray()[0].GetString(), name_dataobjects.at(v.GetArray()[1].GetString())); },
                                         linkInstance);
                }
            }
        }
    }
}
}
