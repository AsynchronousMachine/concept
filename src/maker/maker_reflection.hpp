#pragma once

#include "../asm/asm.hpp"

using id_string_map = std::unordered_map<void*, std::string>;
using name_dataobject_map = std::unordered_map<std::string, Asm::data_variant>;
using name_link_map = std::unordered_map<std::string, Asm::link_variant>;
using print_module_map = std::map<std::string, std::string>;

extern const id_string_map module_name;
extern const id_string_map do_names;
extern const name_dataobject_map name_dataobjects;
extern const name_link_map name_links;
extern const print_module_map print_modules;
