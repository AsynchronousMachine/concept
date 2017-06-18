/*
** Test cases for dealing with DataObjects regarding de-/serialization.
*/

#include "../asm/asm.hpp"

#include "../modules/global_modules.hpp"

#include "../../external/rapidjson/include/rapidjson/rapidjson.h"
#include "../../external/rapidjson/include/rapidjson/document.h"
#include "../../external/rapidjson/include/rapidjson/filewritestream.h"
#include "../../external/rapidjson/include/rapidjson/prettywriter.h"

void runDOSerializationExamples() {
    std::cout << "===================================================================" << std::endl;
    std::cout << "Run DataObject de-/serialization samples ..." << std::endl;

    // Non standard test for the public de-/serialization interface of a DataObject
    rapidjson::Document rjdoc(rapidjson::kObjectType);
    rapidjson::Value rjval;

    serModule.doInt.serialize(rjval, rjdoc.GetAllocator());
    std::cout << "serModule.doInt " << rjval.GetInt() << std::endl;

    rjval.SetInt(rjval.GetInt() + 1);

    serModule.doInt.deserialize(rjval);

    serModule.doInt.serialize(rjval, rjdoc.GetAllocator());
    std::cout << "serModule.doInt " << rjval.GetInt() << std::endl;

    // Optionally you will add the json value to a json document to form a higher order data structure.
    // Naming the json value is mandatory.
    rjdoc.AddMember("serModule.doInt", rjval, rjdoc.GetAllocator());
    // Alternate interface for AddMember
    // rapidjson::Value rjkey;
    // rjkey.SetString("serModule.doInt", rjdoc.GetAllocator());
    // rjdoc.AddMember(rjkey, rjval, rjdoc.GetAllocator());

    //std::cout << "serModule.doInt from json document " << rjdoc["serModule.doInt"].GetInt() << std::endl;

    // Stringify and output the created DOM
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
    rjdoc.Accept(pw);
    std::cout << sb.GetString() << std::endl;

    // But this is not the typical usage for de-/serialization.
    // The usage is from outside and therefore the name and types of the
    // involved DataObjects must be wellknown.
    // For this reason a reflection interface is also available.





//    Asm::DataObject<double> doDouble;
//    Asm::DataObject<std::string> doString;
//    Asm::DataObject<bool> doBool;
//    Asm::DataObject<std::string> doString2;
//    Asm::DataObject<std::string> doString3;
//    Asm::DataObject<MyComplexDOType> doMyComplexDOType;

    std::cout << "===================================================================" << std::endl;
    std::cout << "Enter \'n\' for next test!" << std::endl;
    char c;
    std::cin >> c;
}
