#pragma once
#include <boost/variant.hpp>
#include "asm.hpp"

struct EmptyDataobject : boost::blank
{
    const std::string serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) { return "!!!Will never be used!!!"; };
    const void deserialize(rapidjson::Value& value) {};
    template <class Visitor>
    const int get(Visitor visitor) { return 0; }; // !!!Will never be used!!!
};

using dataobjects = boost::variant<EmptyDataobject&, Asm::DataObject<bool>&, Asm::DataObject<double>&, Asm::DataObject<int>&, Asm::DataObject<std::map<std::string, double> >&, Asm::DataObject<std::string>&, Asm::DataObject<unsigned int>&>;
using module_name_map = std::unordered_map<void*, std::string>;
using name_module_map = std::unordered_map<std::string, boost::any>;
using dataobject_name_map = std::unordered_map<void*, std::string>;
using name_dataobject_map = std::unordered_map<std::string, dataobjects>;
using registerlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any, boost::any)>>;
using unregisterlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any)>>;
using print_module_map = std::map<std::string, std::string>;

extern const module_name_map module_names;
extern const name_module_map modules;
extern const dataobject_name_map do_names;
extern const registerlink_map set_links;
extern const unregisterlink_map clear_links;
extern const print_module_map print_modules;
extern const name_dataobject_map dataobject_map;