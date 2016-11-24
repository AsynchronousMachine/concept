#include "global_instances.cpp"
#include "global_reflection.cpp"



class UseExample {

public:
	static void example_set_and_clear_links()
{
	// Module1.m1.do1 -----(Module2.m2.link1)-----------> Module2.m2.do1          Module1.m1.do1 is linked via Module2.m2.link1 to Module2.m2.do1
	//					|
	//					+--(Module2.m2.linkComplex)-----> Module2.m2.do2          Module1.m1.do1 is linked via Module2.m2.linkComplex to Module2.m2.do2, too

    /*
	JSON syntax:
	Array of link definitions where the JSON key is a free choosen name (which has to be unique for each source DO because it is used as key in the _links map of the DO) and 
	the JSON value is an array of the string representations of the link, source DO and target DO (in this order)
	string representation is the key of the maps build with the map maker tool
	default structure: <module class>.<module instance name>.<instance name>

	Example:
	[
		{"Link1": ["Module2.m2.link1", "Module1.m1.do1", "Module2.m2.do1"]},
		{"Link2": ["Module2.m2.linkComplex", "Module1.m1.do1", "Module2.m2.do2"]}
	]

	Warning: confusing but allowed
	[
		{"Link1": ["Module2.m2.link1", "Module1.m1.do1", "Module2.m2.do1"]},
		{"Link1": ["Module2.m2.link2", "Module1.m1.do2", "Module2.m2.do2"]}
	]
	*/
	set_links.at("Module2.m2.link1")("Link1", dos.at("Module1.m1.do1"), dos.at("Module2.m2.do1"));
	set_links.at("Module2.m2.linkComplex")("Link2", dos.at("Module1.m1.do1"), dos.at("Module2.m2.do2"));
	
	test_link_trigger<int>(m1.do1);

	/*
	JSON syntax:
	for deleting a link the target DO is missing

	Example:
	[
	{"Link1": ["Module2.m2.link1", "Module1.m1.do1"]},
	{"Link2": ["Module2.m2.linkComplex", "Module1.m1.do1"]}
	]
	*/
	clear_links.at("Module2.m2.link1")("Link1", dos.at("Module1.m1.do1"));
	clear_links.at("Module2.m2.linkComplex")("Link2", dos.at("Module1.m1.do1"));
}

	static void example_set_do_value()
	{
		/*
		JSON syntax:

		[{"Module1.m1": [{"do1": 42}, {"do2": "42"}]}]
		or
		[{"Module1.m1.do1": 42}, {"Module1.m1.do2": "42"}]

		advantage first proposal: JSON value could be used as input for deserialize method of the module
		advantage second proposal: each public DO can be set individual from everywhere by using the name_dataobject_map, the DO doesn't have to be within a module

		*/

		//Problem to solve: parse value as correct datatype
		//set visitor method call vs set boost:any call
		boost::any a = 42;

		Asm::DataObject<int> *d1 = boost::any_cast<Asm::DataObject<int>*>(dos.at("Module1.m1.do1"));
		d1->set(a);

		do_set.at("Module1.m1.do1")(a);


		m1.do1.set(a);

		// Usually now is time to announce the change of this DO to the reactor
		//rptr->trigger(m1.do1);
	}


template <class D>
static void test_link_trigger(Asm::DataObject<D> &d)
{
	Asm::DataObjectReactor *rptr = new Asm::DataObjectReactor(2);
	rptr->trigger(d);
	delete rptr;
}
};


void main() {
	m1.do1.set((boost::any)42);
	UseExample::example_set_and_clear_links();
	m1.do1.set((boost::any)55);
	UseExample::example_set_and_clear_links();
	exit(0);
}
