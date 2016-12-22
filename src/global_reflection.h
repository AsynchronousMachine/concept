#pragma once
#include <map>
#include <unordered_map>
#include <functional>

#include <boost/any.hpp>

#include "../external/rapidjson/include/rapidjson/document.h"

#include "asm.hpp"

using module_name_map = std::unordered_map<void*, std::string>;
using name_module_map = std::unordered_map<std::string, boost::any>;
using dataobject_name_map = std::unordered_map<void*, std::string>;
using name_dataobject_map = std::unordered_map<std::string, boost::any>;
using registerlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any, boost::any)>>;
using unregisterlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any)>>;
using doserialize_map = std::unordered_map<std::string, Asm::serializeFnct>;
using dodeserialize_map = std::unordered_map<std::string, Asm::deserializeFnct>;
using print_module_map = std::map<std::string, std::string>;

extern const module_name_map module_names;
extern const name_module_map modules;
extern const dataobject_name_map do_names;
extern const name_dataobject_map dos;
extern const registerlink_map set_links;
extern const unregisterlink_map clear_links;
extern const doserialize_map do_serialize;
extern const dodeserialize_map do_deserialize;
extern const print_module_map print_modules;
