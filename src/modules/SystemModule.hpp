#pragma once

#include <string>
#include <iostream>
#include <filesystem>
#include <variant>

#include <boost/core/demangle.hpp>

#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

#include "../logger/logger.hpp"
#include "../asm/asm.hpp"
#include "../maker/maker_reflection.hpp"

class SystemModule {
  private:

    void emptySer(rapidjson::Value&, rapidjson::Document::AllocatorType&) {
        Logger::pLOG->warn("Serialize not implemented");
    }

    void emptyDeser(rapidjson::Value&) {
        Logger::pLOG->warn("Deserialize not implemented");
    }

    void runSerializeAll(rapidjson::Value& value) {
        if (value.IsBool() && value.GetBool()) {
            Logger::pLOG->trace("Dump <system-dump.json> to disk");
            using namespace rapidjson;
            Document doc;
            doc.SetObject();

            std::string key;
            Value json_key(kStringType);
            Value json_value;

            Logger::pLOG->trace("Iterate over all public DOs");
            for (auto map_iter : name_dataobjects) {
                try {
                    Logger::pLOG->trace("Serialize DO {}", map_iter.first);
                    std::visit([&](auto& d) {
                        d->serialize(json_value, doc.GetAllocator());
                    }, map_iter.second);
                } catch (const std::exception& e) {
                    Logger::pLOG->error("Serialize exeption: {}", e.what());
                    Logger::pLOG->error("Handling DO {} / {}", map_iter.first, boost::core::demangle(typeid(map_iter.second).name()));
                }                    
                    
                if (!json_value.IsNull()) {
                    key = map_iter.first;
                    json_key.SetString(key.data(), doc.GetAllocator());
                    doc.AddMember(json_key, json_value, doc.GetAllocator());
                }
            }

            Logger::pLOG->trace("Try to write <system-dump.json>");
            std::FILE* fp = std::fopen("system-dump.json", "wb"); 
            char writeBuffer[1000000];
            FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
            PrettyWriter<FileWriteStream> writer(os);
            doc.Accept(writer);
            fclose(fp);
        }
    }

    void runDeserializeAll(rapidjson::Value& value) {
        if (value.IsBool() && value.GetBool()) {
            Logger::pLOG->trace("Try to read <system-dump.json> from disk");

            if (!std::filesystem::exists("system-dump.json"))
                return;

            Logger::pLOG->trace("Try to open <system-dump.json>");
            using namespace rapidjson;
            std::FILE* fp = std::fopen("system-dump.json", "rb");
            char readBuffer[65536];
            FileReadStream is(fp, readBuffer, sizeof(readBuffer));
            Document doc;
            doc.ParseStream(is);
            fclose(fp);

            if (doc.HasParseError() == true) {
                Logger::pLOG->error("Deserialize has parsing error at offset {}", doc.GetErrorOffset());
                Logger::pLOG->error(">{}", GetParseError_En(doc.GetParseError()));
                return;
            }

            Logger::pLOG->trace("Iterate over all DO-keys from json file");
            for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
                Value& v = doc[itr->name.GetString()];
                try {
                    Logger::pLOG->trace("Deserialize DO {}", itr->name.GetString());
                    std::visit([&](auto& d) { d->deserialize(v); }, name_dataobjects.at(itr->name.GetString()));
                } catch (const std::exception& e) {
                    Logger::pLOG->error("Deserialize exeption: {}", e.what());
                    Logger::pLOG->error("Handling DO {}", itr->name.GetString());
                }

            }
        }
    }

    void runPrintModules(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
        rapidjson::Value rjvalue;

        value.SetObject();

        Logger::pLOG->trace("This is for humans:");

        for(const auto& m : print_modules) {
            rjvalue.SetString(m.second.data(), allocator);
            value.AddMember(rapidjson::StringRef(m.first.data()), rjvalue, allocator);

            Logger::pLOG->trace("{}", m.first);
            Logger::pLOG->trace("{}", m.second);
        }
    }

  public:
    // This have to be public, else they are not in the maps build by the map maker tool
    Asm::DataObject<bool> serializeAll;
    Asm::DataObject<bool> deserializeAll;
    Asm::DataObject<bool> printModules;

    SystemModule() :
        serializeAll(false, &SystemModule::emptySer, &SystemModule::runSerializeAll, this),
        deserializeAll(false, &SystemModule::emptySer, &SystemModule::runDeserializeAll, this),
        printModules(false, &SystemModule::runPrintModules, &SystemModule::emptyDeser, this) {}
};
