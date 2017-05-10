#pragma once

#include <boost/variant.hpp>

#include "dataobject.hpp"
#include "maker_do.hpp"
#include "linkobject.hpp"
#include "../datatypes/global_datatypes.hpp"



namespace Asm {

	using link_variant = boost::variant<EmptyLinkObject&, LinkObject<Asm::DataObject<int>, DataObject<int>>&>;
}