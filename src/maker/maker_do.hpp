#pragma once

#include <variant>

#include "../asm/dataobject.hpp"
#include "../datatypes/global_datatypes.hpp"

namespace Asm {

using data_variant = std::variant<
                     Asm::DataObject<MyComplexDOType>*, 
                     Asm::DataObject<bool>*, 
                     Asm::DataObject<double>*, 
                     Asm::DataObject<int>*, 
                     Asm::DataObject<std::string>*
                     >;
}
