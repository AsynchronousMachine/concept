#pragma once

#include <string>
#include <iostream>
#include <experimental/filesystem>

#include "../asm/asm.hpp"
#include "../maker/maker_reflection.hpp"

#include "../../external/rapidjson/include/rapidjson/prettywriter.h"
#include "../../external/rapidjson/include/rapidjson/filewritestream.h"
#include "../../external/rapidjson/include/rapidjson/filereadstream.h"
#include "../../external/rapidjson/include/rapidjson/error/en.h"

class SystemModule {
  private:

    void emptySer(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
        std::cout << "Serialize not implemented" << std::endl;
    }

    void emptyDeser(rapidjson::Value& value) {
        std::cout << "Deserialize not implemented" << std::endl;
    }

    void runSerializeAll(rapidjson::Value& value) {
        if (value.IsBool() && value.GetBool()) {
            std::cout << "Dump <system-dump.json> to disk" << std::endl;
            using namespace rapidjson;
            Document doc; // Null
            doc.SetObject();

            std::string key;
            Value json_key(kStringType);
            Value json_value;

            //iterate over all public DOs
            for (auto map_iter : name_dataobjects) {
                boost::apply_visitor([&](auto& d) {
                    d.serialize(json_value, doc.GetAllocator());
                }, map_iter.second);
                if (!json_value.IsNull()) {
                    key = map_iter.first;
                    //json_key.SetString(StringRef(*key.c_str)); // always the last key!!!
                    json_key.SetString(key.c_str(), doc.GetAllocator());
                    doc.AddMember(json_key, json_value, doc.GetAllocator());
                }
            }

            std::FILE* fp = std::fopen("system-dump.json", "wb"); // non-Windows use "w"
            char writeBuffer[1000000];
            FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
            PrettyWriter<FileWriteStream> writer(os);
            doc.Accept(writer);
            fclose(fp);
        }
    }

    void runDeserializeAll(rapidjson::Value& value) {
        if (value.IsBool() && value.GetBool()) {
            std::cout << "Try to read <system-dump.json> from disk" << std::endl;

            if (!std::experimental::filesystem::exists("system-dump.json"))
                return;

            using namespace rapidjson;
            std::FILE* fp = std::fopen("system-dump.json", "rb"); // non-Windows use "r"
            char readBuffer[65536];
            FileReadStream is(fp, readBuffer, sizeof(readBuffer));
            Document doc;
            doc.ParseStream(is);
            fclose(fp);

            if (doc.HasParseError() == true) {
                std::cout << "Deserialize has parsing error at offset " << doc.GetErrorOffset() << std::endl;
                std::cout << ">>>" << GetParseError_En(doc.GetParseError()) << std::endl;
                return;
            }

            //iterate over all DO-keys from json file
            for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
                //const char* doName = itr->name.GetString();
                Value& v = doc[itr->name.GetString()];
                try {
                    boost::apply_visitor([&](auto& d) {
                        d.deserialize(v);
                    }, name_dataobjects.at(itr->name.GetString()));
                } catch (const std::exception& e) {
                    std::cerr << "Deserialize exeption: " << e.what() << std::endl;
                    std::cout << "Missing " << itr->name.GetString() << std::endl;
                }

            }
        }
    }

    void runPrintModules(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
        rapidjson::Value rjvalue;

        value.SetObject();

        std::cout << "This is for humans:" << std::endl;

        for(const auto& m : print_modules) {
            rjvalue.SetString(m.second.c_str(), allocator);
            value.AddMember(rapidjson::StringRef(m.first.c_str()), rjvalue, allocator);

            std::cout << m.first << std::endl << m.second << std::endl;
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
