#include "asm.hpp"

// Test concept of dataobject, reactor and modules

class Module1
{
    public:
        // A module should have a constant name to identify it
        const std::string _name;

        // Only one constructor
        Module1(std::string name) : _name(name), do1("DO1", 1), do2("DO2", "42") {}

        Asm::DataObject<int> do1;
        Asm::DataObject<std::string> do2;
};

class Module2
{
    public:
        // A module should have a constant name to identify it
        const std::string _name;

        // Only one constructor
        Module2(std::string name) : _name(name), do1("DO1", 2), do2("DO2", std::map<std::string, double>{{"42", 22.0}, {"43", 23.0}, {"44", 24.0}}) {}

        Asm::DataObject<int> do1;
        Asm::DataObject<std::map<std::string, double>> do2;

        void link1(Asm::DataObject<int> &do1, Asm::DataObject<int> &do2)
        {
            std::cout << "My DO.name: " << do2.getName() << std::endl;
            std::cout << "Got DO.name: " << do1.getName() << std::endl;
            do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
            do2.get([](int i){ std::cout << "Had DO.value: " << i << std::endl; });
        }

        void link2(Asm::DataObject<std::string> &do1, Asm::DataObject<std::map<std::string, double>> &do2)
        {
            std::cout << "My DO.name: " << do2.getName() << std::endl;
            std::cout << "Got DO.name: " << do1.getName() << std::endl;
            std::string s = do1.get([](std::string s){ std::cout << "Got DO.value: " << s << std::endl; return s; });
            std::cout << "Has DO.value: " << do2.get([&s](const std::map<std::string, double> &m){ return m.at(s); }) << '\n';
        }
};

int main(void)
{
    Asm::Reactor *rptr = new Asm::Reactor(2);
    // Let it start
    boost::this_thread::sleep_for(boost::chrono::seconds(3));

    Module1 module1("Module1");
    Module2 module2("Module2");

    module1.do1.registerLink("Link1", module2.do1, [&module2](Asm::DataObject<int> &do1, Asm::DataObject<int> &do2){ module2.link1(do1, do2); });
    module1.do2.registerLink("Link2", module2.do2, [&module2](Asm::DataObject<std::string> &do1, Asm::DataObject<std::map<std::string, double>> &do2){ module2.link2(do1, do2); });

    // Usually now is time to announce the change of this DO to the reactor
    rptr->trigger(module1.do1);
    rptr->trigger(module1.do2);
    // Let it run
    boost::this_thread::sleep_for(boost::chrono::seconds(3));

    delete rptr;

    exit(0);
}

