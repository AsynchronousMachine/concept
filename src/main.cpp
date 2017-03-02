//#include "global_instances.h"
#include "global_reflection.cpp"


int main() {

	serializetest();

	auto DO2 = getDO("bla");
	std::cout << do_names.at(DO2) << ":" << std::endl;
	std::cout << DO2->get([](std::string i) {return i; }) << std::endl;
	
	auto list = getAllDOs();
	boost::hana::for_each(list, [&](const auto& x) {x->get([](auto i) {return i; }); });

	std::cout <<  boost::hana::at(list, boost::hana::size_c<0>)->get([](auto i) {return i; });
	

	exit(0);
}
