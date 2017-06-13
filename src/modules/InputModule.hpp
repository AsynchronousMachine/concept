/*
**
** The following code samples exclude the optional interface for
** ser- and deserialization of the content of a DataObject
*/

#include "../asm/asm.hpp"

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
