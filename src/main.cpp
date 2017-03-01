#include "global_instances.h"
#include "global_reflection.cpp"


int main() {
	sys.serialize();
	sys.deserialize();
	
	//WARNING: get is NOT usable with variant
	//boost::apply_visitor([&](auto& d) ->auto{return d.get([](auto i)->auto {return i; }); }, dataobject_map.at("Module1.m1.do1"));
	exit(0);
}
