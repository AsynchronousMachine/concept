#pragma once

#include <boost/variant.hpp>

#include "dataobject.hpp"
#include "../datatypes/global_datatypes.hpp"


namespace Asm {
	using data_variant = boost::variant<
		EmptyDataobject&,
		DataObject<bool>&,
		DataObject<double>&,
		DataObject<int>&,
		DataObject<std::map<std::string, double>>&,
		DataObject<std::string>&, Asm::DataObject<unsigned int>&, Asm::DataObject<MyComplexDOType>&, Asm::DataObject<MyComplexDOType&>&>;
}