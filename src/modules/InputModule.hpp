#include "../asm/dataobject.hpp"
#include "../asm/linkobject.hpp"

//Hint: all DOs in this example are not serializeable to keep it simple, look at DataObjectSerialize.cpp for this use case

class InputModule
{
public:
	InputModule() :
		DOintOutput(0),
		DOstringOutput("")
	{}

	//DataObject is never a target in a LinkObject, only the source -> is an output of the module
	Asm::DataObject<int> DOintOutput;
	Asm::DataObject<std::string> DOstringOutput;
};