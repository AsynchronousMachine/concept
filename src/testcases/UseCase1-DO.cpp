#pragma once
#include "../asm/dataobject.hpp"
#include "../modules/maker_reflection.hpp"

#include <string>
//#include <vector>
#include <atomic>

//developer defined complex datatpyes for DO
struct ComplexStruct
{
	int i;
	std::string s;
} csInst;

class ComplexClass
{
public:
	int inputCounter;
	int outputCounter;
	std::string message;
};

// Helper for data access
void fi(int i) { std::cout << i << '\n'; }

// Helper for data access with return value
int fir(int i) { return i + 1; }

// Alternate helper for data access
struct cb
{
	void operator() (int i) { std::cout << i << '\n'; }
}
cbi;


//Hint: all DOs in this example are not serializeable
Asm::DataObject<int> doInt(10);
Asm::DataObject<std::string> doString("11");
Asm::DataObject<ComplexStruct> doStruct(csInst);
Asm::DataObject<ComplexClass> doClass(ComplexClass{ 1, 2, "step1" });

template<typename T>
class ato
{
	std::atomic<T> a;
public:
	ato(T i) : a(i) {}
};

void runcase1() {
	//First get a DataObject

	// Access content consistently
	doInt.get(fi);
	doInt.set([](std::atomic<int> &i) { i = 1; });
	doInt.get(fi);
	doInt.set([](std::atomic<int> &i) { i = i + 1; });
	doInt.get(cbi);
	std::cout << doInt.get(fir) << '\n';

	// Complex DO
	Asm::DataObject<std::vector<int>> doVector({}); //init empty
	doVector.set([](std::vector<int> &v) {v = std::vector<int>{ 1, 2, 3 }; });
	doVector.get([](const std::vector<int> &v) { std::cout << v[0] << ',' << v[1] << '\n'; });

	// More complex DO
	int tmp = 0;
	Asm::DataObject<std::map<std::string, int>> doMap(std::map<std::string, int>{ {"3", 23}, { "4", 24 }});
	doMap.get([](const std::map<std::string, int> &v) { for (auto& m : v) { std::cout << m.first << " has value " << m.second << '\n'; } });
	doMap.set([](std::map<std::string, int> &v) { v = std::map<std::string, int>{ { "1", 42 },{ "2", 43 } }; });
	doMap.get([](const std::map<std::string, int> &v) { std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
	doMap.set([](std::map<std::string, int> &v) { v["1"] = v.at("1") + 1; });
	doMap.get([](const std::map<std::string, int> &v) { std::cout << v.at("1") << ',' << v.at("2") << '\n'; });
	doMap.get([&tmp](const std::map<std::string, int> &v) { tmp = v.at("1") + 2; });
	std::cout << tmp << '\n';
	doMap.get([](const std::map<std::string, int> &v) { for (auto& m : v) { std::cout << m.first << " has value " << m.second << '\n'; } });
	doMap.set([](std::map<std::string, int> &v) { v["1"] = v.at("1") + 3; });
	tmp = doMap.get([](const std::map<std::string, int> &v) { return (v.at("1") + 2); });
	std::cout << tmp << '\n';

	// DO as none standard type
	Asm::DataObject<ComplexStruct> doComplexStruct({ 1, "step1" });
	doComplexStruct.get([](const ComplexStruct &m) {std::cout << m.i << " has value " << m.s << '\n'; });
	doComplexStruct.set([](ComplexStruct &m) {m.i = 2; m.s = "step2"; });
	ComplexStruct cs;
	doComplexStruct.get([&cs](const ComplexStruct &m) {cs = m; });
	std::cout << cs.i << " has value " << cs.s << '\n';
	doComplexStruct.set([](ComplexStruct &m) {m.i = 3; m.s = "step3"; });
	cs = doComplexStruct.get([](const ComplexStruct &m) {return m; });
	std::cout << cs.i << " has value " << cs.s << '\n';

	doClass.get([](const ComplexClass &m) {std::cout << m.inputCounter << " has value " << m.message << '\n'; });
	doClass.set([](ComplexClass &m) {m.inputCounter = 2; m.message = "step2"; });
	ComplexClass cc;
	doClass.get([&cc](const ComplexClass &m) {cc = m; });
	std::cout << cc.inputCounter << " has value " << cc.message << '\n';
	doClass.set([](ComplexClass &m) {m.inputCounter = 3; m.message = "step3"; });
	cc = doClass.get([](const ComplexClass &m) {return m; });
	std::cout << cc.inputCounter << " has value " << cc.message << '\n';


	// Possible misuse by return of address of reference
	Asm::DataObject<int> do7(3);
	const int *tmp5 = do7.get([](const int &i) {return &i; });
	std::cout << "*tmp5 " << *tmp5 << std::endl;
	*const_cast<int*>(tmp5) = 4; // Misuse
	do7.get([](const int i) {std::cout << "Misused by return of address of reference " << i << std::endl; });

	// Possible misuse by return of reference
	Asm::DataObject<int> do8(5);
	do8.get([](const int &i) {int& tmp = const_cast<int&>(i); tmp = 6; }); // Misuse
	do8.get([](const int i) {std::cout << "Misused by return of reference " << i << std::endl; });



	//ato<int> a(3);

	// Atomic Tests, atomic operations are still done within held dataobject mutex ?????
	int tmp_atomic = 0;
	tmp_atomic = doInt.get([](const std::atomic_int& i) {return std::atomic_load(&i); });
	//doInt.get([&tmp_atomic](const std::atomic_int& i) {tmp_atomic = std::atomic_load(&i);});
	std::cout << "tmp_atomic load init " << tmp_atomic << std::endl;
	doInt.set([](std::atomic_int& i) {++i; });
	//doInt.get([&tmp_atomic](const std::atomic_int& i) {tmp_atomic = std::atomic_load(&i);});
	tmp_atomic = doInt.get([](const std::atomic_int& i) {return std::atomic_load(&i); });
	std::cout << "tmp_atomic load after inc " << tmp_atomic << std::endl;


	// DataObjectReactor Tests
	std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "DataObjectReactor Tests..." << std::endl;
	std::shared_ptr<Asm::DataObjectReactor> reactor(new Asm::DataObjectReactor(4));

	doInt.set([](std::atomic<int> &i) { i = 123; });
	doStruct.set([](ComplexStruct &complexStruct){ complexStruct.i = 456; });
	doClass.set([](ComplexClass &m) {m.inputCounter = 789; });

	// this shall be executed after [doInt->doClass]
	doInt.registerLink("doInt->doStruct", doStruct, [](Asm::DataObject<int>& triggeredDoInt, Asm::DataObject<ComplexStruct>& triggeredDoStruct){

		boost::this_thread::sleep_for(boost::chrono::seconds(3));
		int doIntVal = triggeredDoInt.get([](int i){return i;});
		ComplexStruct doStructVal = triggeredDoStruct.get([](ComplexStruct complexStruct){return complexStruct;});
		std::cout << "Triggered link [doInt->doStruct] with values doInt: " << doIntVal << ", doStruct.i: " << doStructVal.i << std::endl;
	});
	reactor->trigger(doInt);
	doInt.unregisterLink("doInt->doStruct");

	doInt.set([](std::atomic<int> &i) { i = 321; }); // notice that in [doInt->doStruct] value will be updated!
	doInt.registerLink("doInt->doClass", doClass, [](Asm::DataObject<int>& triggeredDoInt, Asm::DataObject<ComplexClass>& triggeredDoClass){

		int doIntVal = triggeredDoInt.get([](int i){return i;});
		ComplexClass doClassVal = triggeredDoClass.get([](ComplexClass complexClass){return complexClass;});
		std::cout << "Triggered link [doInt->doClass] with values doInt: " << doIntVal << ", doClass.inputCounter: " << doClassVal.inputCounter << std::endl;
	});
	reactor->trigger(doInt);

	// wait for links to be executed
	boost::this_thread::sleep_for(boost::chrono::seconds(5));
}
