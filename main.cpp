#include "asm.hpp"

#include <boost/any.hpp>

// Test concept of dataobject, reactor and modules

struct Module1
{
    // A module should have a constant name to identify it
    const std::string _name;

    // Only one constructor
    Module1(std::string name) : _name(name), do1("DO1", 1), do2("DO2", "42") {}

    Asm::DataObject<int> do1;
    Asm::DataObject<std::string> do2;

    void deserialize(std::string js) { std::cout << "Got " << js << std::endl; }
    std::string serialize() { return "{do1:2}"; }
};

struct Module2
{
    // A module should have a constant name to identify it
    const std::string _name;

    // Only one constructor
    Module2(std::string name) : _name(name), do1("DO1", 2), do2("DO2", std::map<std::string, double>{{"42", 22.0}, {"43", 23.0}, {"44", 24.0}}) {}

    Asm::DataObject<int> do1;
    Asm::DataObject<std::map<std::string, double>> do2;

    void link1(Asm::DataObject<int> &do1, Asm::DataObject<int> &do2)
    {
        std::cout << "My Module.name: " << _name << std::endl;
        std::cout << "My DO.name: " << do2.getName() << std::endl;
        std::cout << "Got DO.name: " << do1.getName() << std::endl;
        do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
        do2.get([](int i){ std::cout << "Had DO.value: " << i << std::endl; });
    }

    void link2(Asm::DataObject<std::string> &do1, Asm::DataObject<std::map<std::string, double>> &do2)
    {
        std::cout << "My Module.name: " << _name << std::endl;
        std::cout << "My DO.name: " << do2.getName() << std::endl;
        std::cout << "Got DO.name: " << do1.getName() << std::endl;
        std::string s = do1.get([](std::string s){ std::cout << "Got DO.value: " << s << std::endl; return s; });
        std::cout << "Has DO.value: " << do2.get([&s](const std::map<std::string, double> &m){ return m.at(s); }) << '\n';
    }

    void deserialize(std::string js) { std::cout << "Got " << js << std::endl; }
    std::string serialize() { return "{do1:2}"; }
};

// Module helper function template
template <typename D1, typename M2, typename D2, typename CB>
void RegisterLink(std::string name, Asm::DataObject<D1> &d1, M2 &m2, Asm::DataObject<D2> &d2, CB cb)
{
    d1.registerLink(name, d2, [cb, &m2](Asm::DataObject<D1> &do1, Asm::DataObject<D2> &do2){ std::mem_fn(cb)(m2, do1, do2); });
}

int main(void)
{
    Asm::Reactor *rptr = new Asm::Reactor(2);
    // Let it start
    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    Module1 module1("Module1");
    Module2 module2("Module2");

    RegisterLink("Link1", module1.do1, module2, module2.do1, &Module2::link1);
    RegisterLink("Link2", module1.do2, module2, module2.do2, &Module2::link2);

    // Usually now is time to announce the change of this DO to the reactor
    rptr->trigger(module1.do1);
    rptr->trigger(module1.do2);
    // Let it run
    boost::this_thread::sleep_for(boost::chrono::seconds(3));

    std::cout << std::endl;

    //Reflection
    using deserialize_map = std::unordered_map<std::string, std::function<void(std::string)>>;
    using serialize_map   = std::unordered_map<std::string, std::function<std::string()>>;

    // This map should be built automatically
    const deserialize_map des_modules{{"Module1", [&module1](std::string js){ std::mem_fn(&Module1::deserialize)(module1, js); }},
                                      {"Module2", [&module2](std::string js){ std::mem_fn(&Module2::deserialize)(module2, js); }}
                                     };

    // This map should be built automatically
    const serialize_map s_modules{{"Module1", [&module1](){ return std::mem_fn(&Module1::serialize)(module1); }},
                                  {"Module2", [&module2](){ return std::mem_fn(&Module2::serialize)(module2); }}
                                 };

    // Sample access to deserialization/serialization
    std::cout << "+++Deserialize via JSON" << std::endl;
    for(auto it = des_modules.begin(); it != des_modules.end(); ++it)
    {
        std::cout << std::get<0>(*it) << std::endl;
        std::get<1>(*it)("{do1:1}");
    }

    std::cout << "+++Serialize via JSON" << std::endl;
    for(auto it = s_modules.begin(); it != s_modules.end(); ++it)
    {
        std::cout << std::get<0>(*it) << std::endl;
        std::cout << std::get<1>(*it)() << std::endl;
    }

    std::cout << "+++Serialize via JSON by key" << std::endl;
    std::cout << s_modules.at("Module1")() << std::endl;
    std::cout << s_modules.at("Module2")() << std::endl;

    //boost::any p_mod = &module1;
    //Module1 *p_mod1 = boost::any_cast<Module1*>(p_mod);
    //std::cout << p_mod1->_name << std::endl;

    //for(auto &m : modules)
    //    std::cout << std::get<std::string>(m) << std::endl;

    //for(auto &m : Modules)
    //    std::cout << m._name << std::endl;

    //std::for_each(Modules, [](auto &m){ std::cout << m._name << std::endl; });

    delete rptr;

    exit(0);
}

