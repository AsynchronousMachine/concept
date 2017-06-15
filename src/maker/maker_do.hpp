#pragma once

#include <boost/variant.hpp>

#include "../asm/asm.hpp"
#include "../datatypes/global_datatypes.hpp"

namespace Asm {

using data_variant = boost::variant<
                     Asm::DataObject<MyComplexDOType>&,
                     Asm::DataObject<int>&,
                     Asm::DataObject<std::string>&,
                     Asm::DataObject<bool>&,
                     Asm::DataObject<double>&
                     >;
}
