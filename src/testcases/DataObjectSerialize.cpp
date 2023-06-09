/*
** Test cases for dealing with DataObjects regarding de-/serialization.
*/
#include <variant>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include "../logger/logger.hpp"
#include "../asm/asm.hpp"
#include "../modules/global_modules.hpp"
#include "../maker/maker_reflection.hpp"

void runDOSerializationExamples() {
    Logger::pLOG->trace("===================================================================");
    Logger::pLOG->trace("Run DataObject de-/serialization samples ...");

    // Non standard test for the public de-/serialization interface of a DataObject
    {
        Logger::pLOG->trace("Non standard test for the public de-/serialization interface of a DataObject ...");

        rapidjson::Document rjdoc(rapidjson::kObjectType);
        rapidjson::Value rjval;

        serModule.doInt.serialize(rjval, rjdoc.GetAllocator());
        Logger::pLOG->trace("serModule.doInt {}", rjval.GetInt());
        rjval.SetInt(rjval.GetInt() + 1);
        serModule.doInt.deserialize(rjval);
        serModule.doInt.serialize(rjval, rjdoc.GetAllocator());
        Logger::pLOG->trace("serModule.doInt after increment {}", rjval.GetInt());

        serModule.doUnsignedInt.serialize(rjval, rjdoc.GetAllocator());
        Logger::pLOG->trace("serModule.doUnsignedInt {}", rjval.GetUint());
        rjval.SetUint(rjval.GetUint() + 1);
        serModule.doUnsignedInt.deserialize(rjval);
        serModule.doUnsignedInt.serialize(rjval, rjdoc.GetAllocator());
        Logger::pLOG->trace("serModule.doUnsignedInt after increment {}", rjval.GetUint());

        serModule.doInt64.serialize(rjval, rjdoc.GetAllocator());
        Logger::pLOG->trace("serModule.doInt64 {}", rjval.GetInt64());
        rjval.SetInt64(rjval.GetInt64() + 1);
        serModule.doInt64.deserialize(rjval);
        serModule.doInt64.serialize(rjval, rjdoc.GetAllocator());
        Logger::pLOG->trace("serModule.doInt64 after increment {}", rjval.GetInt64());

        serModule.doUInt64.serialize(rjval, rjdoc.GetAllocator());
        Logger::pLOG->trace("serModule.doInt64 {}", rjval.GetUint64());
        rjval.SetUint64(rjval.GetUint64() + 1);
        serModule.doUInt64.deserialize(rjval);
        serModule.doUInt64.serialize(rjval, rjdoc.GetAllocator());
        Logger::pLOG->trace("serModule.doUInt64 after increment {}", rjval.GetUint64());

        // Optionally you will add the json value to a json document to form a higher order data structure.
        // Naming the json value is mandatory."SerializeModule.serModule.doInt"
        rjdoc.AddMember("SerializeModule.serModule.doInt64", rjval, rjdoc.GetAllocator());

        Logger::pLOG->trace("Stringify and output the created DOM:");
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        rjdoc.Accept(pw);
        Logger::pLOG->trace("{}", sb.GetString());
    }

    // But as depicted before this is not the typical usage for de-/serialization.
    // The usage is from outside and therefore the name and types of the
    // involved DataObjects must be wellknown.
    // For this reason a reflection interface is also available.
    {
        Logger::pLOG->trace("Typical usage for the public de-/serialization interface of a DataObject ...");

        rapidjson::Document rjdoc(rapidjson::kObjectType);
        rapidjson::Value rjval(rapidjson::kObjectType);

        Logger::pLOG->trace("This is for humans:");
        for(const auto& m : print_modules) {
            Logger::pLOG->trace("{}\n{}", m.first, m.second);
        }

        // Wellknown name by reflection gives access to generic interface to DataObject
        std::visit([&](auto& d) { d->serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doInt"));
        rjdoc.AddMember("SerializeModule.serModule.doInt", rjval, rjdoc.GetAllocator());
        std::visit([&](auto& d) { d->serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doDouble"));
        rjdoc.AddMember("SerializeModule.serModule.doDouble", rjval, rjdoc.GetAllocator());
        std::visit([&](auto& d) { d->serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doString"));
        rjdoc.AddMember("SerializeModule.serModule.doString", rjval, rjdoc.GetAllocator());
        std::visit([&](auto& d) { d->serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doBool"));
        rjdoc.AddMember("SerializeModule.serModule.doBool", rjval, rjdoc.GetAllocator());
        std::visit([&](auto& d) { d->serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doString2"));
        rjdoc.AddMember("SerializeModule.serModule.doString2", rjval, rjdoc.GetAllocator());
        std::visit([&](auto& d) { d->serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doString3"));
        rjdoc.AddMember("SerializeModule.serModule.doString3", rjval, rjdoc.GetAllocator());
        std::visit([&](auto& d) { d->serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doMyComplexDOType"));
        rjdoc.AddMember("SerializeModule.serModule.doMyComplexDOType", rjval, rjdoc.GetAllocator());

        Logger::pLOG->trace("Stringify and output the created DOM:");
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        rjdoc.Accept(pw);
        Logger::pLOG->trace("{}", sb.GetString());

        // Wellknown name by reflection gives access to generic interface to DataObject
        rapidjson::Value& rjdoint = rjdoc["SerializeModule.serModule.doInt"];
        rjdoint.SetInt(10);
        std::visit([&](auto& d){ d->deserialize(rjdoint); }, name_dataobjects.at("SerializeModule.serModule.doInt"));

        Logger::pLOG->trace("Deserialized serModule.doInt {}", serModule.doInt.get([](int i) {return i;}));

        rapidjson::Value& rjdoMCDOT = rjdoc["SerializeModule.serModule.doMyComplexDOType"];
        rjdoMCDOT["amount outputs"].SetInt(20);
        std::visit([&](auto& d){ d->deserialize(rjdoMCDOT); }, name_dataobjects.at("SerializeModule.serModule.doMyComplexDOType"));

        Logger::pLOG->trace("Deserialized serModule.doMyComplexDOType.amount_outputs {}", serModule.doMyComplexDOType.get([](const MyComplexDOType& d) {return d.outputCounter;}));
    }

    Logger::pLOG->trace("===================================================================");
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
