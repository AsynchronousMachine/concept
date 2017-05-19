#include "../asm/dataobject.hpp"
#include "../modules/maker_reflection.hpp"

#include <string>
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

    ComplexClass() : inputCounter(0), outputCounter(0), message("+") {
		std::cout << "ComplexClass() with: " << inputCounter << "|" << outputCounter << "|" << message << std::endl;
	}

    ComplexClass(int a, int b, std::string c) : inputCounter(a), outputCounter(b), message(c) {
		std::cout << "ComplexClass(int a, int b, std::string c) with: " << inputCounter << "|" << outputCounter << "|" << message << std::endl;
	}

	ComplexClass(const ComplexClass& cc) : inputCounter(cc.inputCounter), outputCounter(cc.outputCounter), message(cc.message) {
		std::cout << "ComplexClass(const ComplexClass& cc) with: " << inputCounter << "|" << outputCounter << "|" << message << std::endl;
	}
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

template<typename T>
class ato
{
	std::atomic<T> a;
public:
	ato(T i) : a(i) {}
};

void initializingExamples(){

	std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "DO initialize examples..." << std::endl;

	Asm::DataObject<ComplexClass> doClass1(ComplexClass{ 1, 2, "step1" }); // will use DataObject(D content)
	doClass1.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});

	Asm::DataObject<ComplexClass> doClass2(ComplexClass{}); // will use DataObject(D content)
	doClass2.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});

	Asm::DataObject<ComplexClass> doClass3(*new ComplexClass({ 3, 4, "step2" })); // will use DataObject(D content)
	doClass3.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});

	Asm::DataObject<ComplexClass> doClass4(*new ComplexClass); // will use DataObject(D content)
	doClass4.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});

	Asm::DataObject<ComplexClass> doClass5(ComplexClass({})); // will use DataObject(D content)
	doClass5.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});

	Asm::DataObject<ComplexClass> doClass6(*new ComplexClass({})); // will use DataObject(D content)
	doClass6.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});

	Asm::DataObject<ComplexClass> doClass7; // will use DataObject()
	doClass7.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});

	Asm::DataObject<ComplexClass> doClass8{}; // will use DataObject()
	doClass8.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});

	doClass1.set([](ComplexClass& c){c.inputCounter = 1;});
	doClass2.set([](ComplexClass& c){c.inputCounter = 2;});
	doClass3.set([](ComplexClass& c){c.inputCounter = 3;});
	doClass4.set([](ComplexClass& c){c.inputCounter = 4;});
	doClass5.set([](ComplexClass& c){c.inputCounter = 5;});
	doClass6.set([](ComplexClass& c){c.inputCounter = 6;});
	doClass7.set([](ComplexClass& c){c.inputCounter = 7;});
	doClass8.set([](ComplexClass& c){c.inputCounter = 8;});

	std::cout << "-----------------------------------------" << std::endl;

	doClass1.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});
	doClass2.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});
	doClass3.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});
	doClass4.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});
	doClass5.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});
	doClass6.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});
	doClass7.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});
	doClass8.get([](const ComplexClass& c){std::cout << "inputcounter: " << c.inputCounter << std::endl;});
}

void runDOAccessExamples()
{
	initializingExamples();

	// Access content consistently
	std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "DO simple examples..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

	//Hint: all DOs in this example are not serializeable
	Asm::DataObject<int> doInt(10);
	Asm::DataObject<std::string> doString("11");
	Asm::DataObject<ComplexStruct> doStruct(csInst);

	Asm::DataObject<ComplexClass> doClass(ComplexClass{ 1, 2, "step1" });
	doClass.get([](const ComplexClass& c){std::cout << c.inputCounter << std::endl;});

	doInt.get(fi);
	doInt.set([](std::atomic<int> &i) { i = 1; });
	doInt.get(fi);
	doInt.set([](std::atomic<int> &i) { i = i + 1; });
	doInt.get(cbi);
	std::cout << doInt.get(fir) << '\n';

	// Complex DO
	std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "DO vector examples..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

	Asm::DataObject<std::vector<int>> doVector({1, 2}); //init empty
	Asm::DataObject<std::vector<int>> doVector2({3, 4}); //init empty
	// Asm::DataObject<std::vector<int>> doVector3({}); //init empty <- not working as call of ambiguous overload initializer list
	doVector.set([](std::vector<int> &v) {v = std::vector<int>{ 1, 2, 3 }; });
	doVector.get([](const std::vector<int> &v) { std::cout << v[0] << ',' << v[1] << '\n'; });

	// More complex DO
	std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "DO map examples..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

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
	std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "DO struct/class examples..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

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
	std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "Misuse examples..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

	doClass.set([](ComplexClass &m) {m.inputCounter = 100; });
	ComplexClass* m =doClass.get([](const ComplexClass& m) {return &const_cast<ComplexClass&>(m);});
	std::cout << "Value before: " << m->inputCounter << std::endl;
	m->inputCounter = 200;
	ComplexClass m2 = doClass.get([](ComplexClass complexClass){return complexClass;});
	std::cout <<  "Misused by return of address of reference " << m2.inputCounter << std::endl;

	Asm::DataObject<int> do71(31);
	std::atomic<int> *tmp51 = do71.get([](const std::atomic<int> &i) {return &const_cast<std::atomic<int>&>(i); });
	std::cout << "Value before: " << *tmp51 << std::endl;
	*tmp51 = 32;
	do71.get([](const int i) {std::cout << "Misused by return of address of reference " << i << std::endl; });


	// Atomic Tests, atomic operations are still done within held dataobject mutex ?????
	std::cout << std::endl << "*****************************************" << std::endl;
	std::cout << "Atomic tests..." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

	int tmp_atomic = 0;
	tmp_atomic = doInt.get([](const std::atomic_int& i) {return std::atomic_load(&i); });
	//doInt.get([&tmp_atomic](const std::atomic_int& i) {tmp_atomic = std::atomic_load(&i);});
	std::cout << "tmp_atomic load init " << tmp_atomic << std::endl;
	doInt.set([](std::atomic_int& i) {++i; });
	//doInt.get([&tmp_atomic](const std::atomic_int& i) {tmp_atomic = std::atomic_load(&i);});
	tmp_atomic = doInt.get([](const std::atomic_int& i) {return std::atomic_load(&i); });
	std::cout << "tmp_atomic load after inc " << tmp_atomic << std::endl;

	boost::this_thread::sleep_for(boost::chrono::seconds(3));
}
