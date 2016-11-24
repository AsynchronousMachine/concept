#pragma once
#include <unordered_map>
#include <functional>

#include <boost/any.hpp>

using module_name_map = std::unordered_map<void*, std::string>;
using name_module_map = std::unordered_map<std::string, boost::any>;
using dataobject_name_map = std::unordered_map<void*, std::string>;
using name_dataobject_map = std::unordered_map<std::string, boost::any>;
using doset_map = std::unordered_map<std::string, std::function<void(const boost::any)>>;
using registerlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any, boost::any)>>;
using unregisterlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any)>>;

extern const module_name_map module_names;
extern const name_module_map modules;
extern const dataobject_name_map do_names;
extern const name_dataobject_map dos;
extern const doset_map do_set;
extern const registerlink_map set_links;
extern const unregisterlink_map clear_links;
