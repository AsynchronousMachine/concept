#pragma once

#include <boost/variant.hpp>

#include "../asm/dataobject.hpp"
#include "../asm/linkobject.hpp"
#include "../datatypes/global_datatypes.hpp"

namespace Asm {

using link_variant = boost::variant<
                     Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<int> >&, 
                     Asm::LinkObject<Asm::DataObject<MyComplexDOType>, Asm::DataObject<std::string> >&, 
                     Asm::LinkObject<Asm::DataObject<int>, Asm::DataObject<MyComplexDOType> >&, 
                     Asm::LinkObject<Asm::DataObject<std::string>, Asm::DataObject<MyComplexDOType> >&
                     >;
}
