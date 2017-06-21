/*
** Test cases for dealing with DataObjects regarding de-/serialization.
*/

#include "../asm/asm.hpp"

#include "../modules/global_modules.hpp"

#include "../maker/maker_reflection.hpp"

#include "../../external/rapidjson/include/rapidjson/rapidjson.h"
#include "../../external/rapidjson/include/rapidjson/document.h"
#include "../../external/rapidjson/include/rapidjson/filewritestream.h"
#include "../../external/rapidjson/include/rapidjson/prettywriter.h"

void runDOSerializationExamples() {
    std::cout << "===================================================================" << std::endl;
    std::cout << "Run DataObject de-/serialization samples ..." << std::endl;

    // Non standard test for the public de-/serialization interface of a DataObject
    {
        std::cout << "Non standard test for the public de-/serialization interface of a DataObject ..." << std::endl;

        rapidjson::Document rjdoc(rapidjson::kObjectType);
        rapidjson::Value rjval;

        serModule.doInt.serialize(rjval, rjdoc.GetAllocator());
        std::cout << "serModule.doInt " << rjval.GetInt() << std::endl;

        rjval.SetInt(rjval.GetInt() + 1);

        serModule.doInt.deserialize(rjval);

        serModule.doInt.serialize(rjval, rjdoc.GetAllocator());
        std::cout << "serModule.doInt " << rjval.GetInt() << std::endl;

        // Optionally you will add the json value to a json document to form a higher order data structure.
        // Naming the json value is mandatory."SerializeModule.serModule.doInt"
        rjdoc.AddMember("serModule.doInt", rjval, rjdoc.GetAllocator());
        // A"SerializeModule.serModule.doInt"lternate interface for AddMember
        // rapidjson::Value rjkey;
        // rjkey.SetString("serModule.doInt", rjdoc.GetAllocator());
        // rjdoc.AddMember(rjkey, rjval, rjdoc.GetAllocator());

        //std::cout << "serModule.doInt from json document " << rjdoc["serModule.doInt"].GetInt() << std::endl;

        std::cout << "Stringify and output the created DOM:" << std::endl;
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        rjdoc.Accept(pw);
        std::cout << sb.GetString() << std::endl;
    }

    // But as depicted before this is not the typical usage for de-/serialization.
    // The usage is from outside and therefore the name and types of the
    // involved DataObjects must be wellknown.
    // For this reason a reflection interface is also available.
    {
        std::cout << std::endl << "Typical usage for the public de-/serialization interface of a DataObject ..." << std::endl;

        rapidjson::Document rjdoc(rapidjson::kObjectType);
        rapidjson::Value rjval(rapidjson::kObjectType);

        std::cout << "This is for humans:" << std::endl;
        for(const auto& m : print_modules) {
            std::cout << "Instance: " << m.first << std::endl << m.second << std::endl;
        }

        // Wellknown name by reflection gives access to generic interface to DataObject
        boost::apply_visitor([&](auto& d) { d.serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doInt"));
        rjdoc.AddMember("SerializeModule.serModule.doInt", rjval, rjdoc.GetAllocator());
        boost::apply_visitor([&](auto& d) { d.serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doDouble"));
        rjdoc.AddMember("SerializeModule.serModule.doDouble", rjval, rjdoc.GetAllocator());
        boost::apply_visitor([&](auto& d) { d.serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doString"));
        rjdoc.AddMember("SerializeModule.serModule.doString", rjval, rjdoc.GetAllocator());
        boost::apply_visitor([&](auto& d) { d.serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doBool"));
        rjdoc.AddMember("SerializeModule.serModule.doBool", rjval, rjdoc.GetAllocator());
        boost::apply_visitor([&](auto& d) { d.serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doString2"));
        rjdoc.AddMember("SerializeModule.serModule.doString2", rjval, rjdoc.GetAllocator());
        boost::apply_visitor([&](auto& d) { d.serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doString3"));
        rjdoc.AddMember("SerializeModule.serModule.doString3", rjval, rjdoc.GetAllocator());
        boost::apply_visitor([&](auto& d) { d.serialize(rjval, rjdoc.GetAllocator()); }, name_dataobjects.at("SerializeModule.serModule.doMyComplexDOType"));
        rjdoc.AddMember("SerializeModule.serModule.doMyComplexDOType", rjval, rjdoc.GetAllocator());

        std::cout << "Stringify and output the created DOM:" << std::endl;
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        rjdoc.Accept(pw);
        std::cout << sb.GetString() << std::endl;

        // Wellknown name by reflection gives access to generic interface to DataObject
        rapidjson::Value& rjdoint = rjdoc["SerializeModule.serModule.doInt"];
        rjdoint.SetInt(10);
        boost::apply_visitor([&](auto& d) { d.deserialize(rjdoint); }, name_dataobjects.at("SerializeModule.serModule.doInt"));

        std::cout << "Deserialized serModule.doInt " << serModule.doInt.get([](int i) {return i;}) << std::endl;

        rapidjson::Value& rjdoMCDOT = rjdoc["SerializeModule.serModule.doMyComplexDOType"];
        rjdoMCDOT["amount outputs"].SetInt(20);
        boost::apply_visitor([&](auto& d) { d.deserialize(rjdoMCDOT); }, name_dataobjects.at("SerializeModule.serModule.doMyComplexDOType"));

        std::cout << "Deserialized serModule.doMyComplexDOType.amount_outputs " << serModule.doMyComplexDOType.get([](const MyComplexDOType& d) {return d.outputCounter;}) << std::endl;
    }

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
