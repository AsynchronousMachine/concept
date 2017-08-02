#include "../asm/asm.hpp"

class InputModule
{
public:
	InputModule() :
		DOintOutput(0, Asm::default_serializer),
		DOstringOutput("", Asm::default_serializer)
	{}

	//DataObject is never a target in a LinkObject, only the source -> is an output of the module
	Asm::DataObject<int> DOintOutput;
	Asm::DataObject<std::string> DOstringOutput;
};
