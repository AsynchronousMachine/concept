#include "asm.hpp"

// Test concept of dataobject, reactor and modules

// The global reactors
Asm::DataObjectReactor *g_rptr;
Asm::TimerReactor *g_trptr;

class Module1
{
private:
    // A module should have a constant name to identify it
    const std::string _name;

    void timer_action1(Asm::DataObject<Asm::Timer> &do_timer1, Asm::DataObject<Asm::Timer> &do_timer2)
    {
        static unsigned cnt = 0;

        if(++cnt >= 5)
        {
            std::cout << "Timer tick: " << cnt << std::endl;
            std::cout << "Stop timer after 5 ticks" << std::endl;
            do_timer1.set([](Asm::Timer &t){ t.stop(); });
            g_trptr->unregisterTimer(do_timer1);
        }
    }

public:
    Module1(std::string name) : _name(name),
                                do1("DO1", 1),
                                do2("DO2", "42"),
                                do_timer1("DO-Timer1"),
                                timer_link1(&Module1::timer_action1, this)
    {
        timer_link1.set("Timer1-Link", &do_timer1, &do_timer1);
        g_trptr->registerTimer(do_timer1);
        do_timer1.set([](Asm::Timer &t){ t.setRelativeInterval(1000, 3000); });
    }

    Asm::DataObject<int> do1;
    Asm::DataObject<std::string> do2;
    Asm::DataObject<Asm::Timer> do_timer1;

    Asm::Link<Asm::DataObject<Asm::Timer>, Asm::DataObject<Asm::Timer>> timer_link1;

    void deserialize(std::string js) { std::cout << "Got: " << js << std::endl; }
    std::string serialize() { return "{do1:2}"; }

    // Get out the module name for humans
    const std::string& getName() const { return _name; }
};

class Module2
{
private:
    // A module should have a constant name to identify it
    const std::string _name;

    void action1(Asm::DataObject<int> &do1, Asm::DataObject<int> &do2)
    {
        std::cout << "My Module.name: " << _name << std::endl;
        std::cout << "My DO.name: " << do2.getName() << std::endl;
        std::cout << "Got DO.name: " << do1.getName() << std::endl;
        do1.get([](int i){ std::cout << "Got DO.value: " << i << std::endl; });
        do2.get([](int i){ std::cout << "Had DO.value: " << i << std::endl; });
    }

    void action2(Asm::DataObject<std::string> &do1, Asm::DataObject<std::map<std::string, double>> &do2)
    {
        std::cout << "My Module.name: " << _name << std::endl;
        std::cout << "My DO.name: " << do2.getName() << std::endl;
        std::cout << "Got DO.name: " << do1.getName() << std::endl;
        std::string s = do1.get([](std::string s){ std::cout << "Got DO.value: " << s << std::endl; return s; });
        std::cout << "Has DO.value: " << do2.get([&s](const std::map<std::string, double> &m){ return m.at(s); }) << '\n';
    }

public:
    Module2(std::string name) : _name(name),
                                do1("DO1", 2),
                                do2("DO2", std::map<std::string, double>{{"42", 22.0}, {"43", 23.0}, {"44", 24.0}}),
                                link1(&Module2::action1, this),
                                link2(&Module2::action2, this) {}

    Asm::DataObject<int> do1;
    Asm::DataObject<std::map<std::string, double>> do2;

    Asm::Link<Asm::DataObject<int>, Asm::DataObject<int>> link1;
    Asm::Link<Asm::DataObject<std::string>, Asm::DataObject<std::map<std::string, double>>> link2;

    void deserialize(std::string js) { std::cout << "Got: " << js << std::endl; }
    std::string serialize() { return "{do1:2}"; }

    // Get out the module name for humans
    const std::string& getName() const { return _name; }
};

int main(void)
{
    g_rptr = new Asm::DataObjectReactor(2);
    g_trptr = new Asm::TimerReactor(*g_rptr);

    // Let it start
    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    Module1 module1("Module1");
    Module2 module2("Module2");

#if 0
    //Reflection
    using dataobject_map = std::unordered_map<std::string, boost::any>;
    using registerlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any, boost::any)>>;
    using unregisterlink_map = std::unordered_map<std::string, std::function<void(std::string, boost::any)>>;
    using deserialize_map = std::unordered_map<std::string, std::function<void(std::string)>>;
    using serialize_map   = std::unordered_map<std::string, std::function<std::string()>>;

    // This map should be built automatically
    dataobject_map dos{//{"Module1.DO1", &module1.do1},
                       {module1.getName()+"."+module1.do1.getName(), &module1.do1}, //Alternate initialization
                       {"Module1.DO2", &module1.do2},
                       {"Module2.DO1", &module2.do1},
                       {"Module2.DO2", &module2.do2},
                      };

    // This map should be built automaticallystd::mem_fn(&Module1::deserialize)(module1, js);
    registerlink_map set_links{{"Module2.link1", [&module2](std::string name, boost::any a1, boost::any a2){auto l = std::mem_fn(&Module2::link1); l(module2).set(name, a1, a2);}},
                               {"Module2.link2", [&module2](std::string name, boost::any a1, boost::any a2){auto l = std::mem_fn(&Module2::link2); l(module2).set(name, a1, a2);}}
                              };

    // This map should be built automatically
    unregisterlink_map clear_links{{"Module2.link1", [&module2](std::string name, boost::any a){auto l = std::mem_fn(&Module2::link1); l(module2).clear(name, a);}},
                                   {"Module2.link2", [&module2](std::string name, boost::any a){auto l = std::mem_fn(&Module2::link2); l(module2).clear(name, a);}}
                                  };

    // This map should be built automatically
    const deserialize_map des_modules{{"Module1", [&module1](std::string js){std::mem_fn(&Module1::deserialize)(module1, js);}},
                                      {"Module2", [&module2](std::string js){std::mem_fn(&Module2::deserialize)(module2, js);}}
                                     };

    // This map should be built automatically
    const serialize_map s_modules{{"Module1", [&module1](){return std::mem_fn(&Module1::serialize)(module1);}},
                                  {"Module2", [&module2](){return std::mem_fn(&Module2::serialize)(module2);}}
                                 };

    // Test to set links by JSON description only
    // {
    // {Link1:["Module2.link1", "Module1.DO1", "Module2.DO1"]},
    // {Link2:["Module2.link2", "Module1.DO2", "Module2.DO2"]}
    // }
    set_links.at("Module2.link1")("Link1", dos.at("Module1.DO1"), dos.at("Module2.DO1"));
    set_links.at("Module2.link2")("Link2", dos.at("Module1.DO2"), dos.at("Module2.DO2"));

    // Test boost:any interface at specific dataobject
    boost::any a = 42;
    module1.do1.set(a);

    // Usually now is time to announce the change of this DO to the reactor
    g_rptr->trigger(module1.do1);
    g_rptr->trigger(module1.do2);
    // Let it run
    boost::this_thread::sleep_for(boost::chrono::seconds(3));

    std::cout << "+++" << std::endl;

    // Sample access to deserialization/serialization
    std::cout << "+++Deserialize via JSON" << std::endl;
    for(auto it = des_modules.begin(); it != des_modules.end(); ++it)
    {
        std::cout << "Set: " << std::get<0>(*it) << " to: " << "{do1:1}" << std::endl;
        std::get<1>(*it)("{do1:1}");
    }

    std::cout << "+++Deserialize via JSON by key" << std::endl;
    std::cout << "Set Module1: "; des_modules.at("Module1")("{do1:1}");
    std::cout << "Set Module1: "; des_modules.at("Module1")("{do2:\"action\"}");

    std::cout << "+++Serialize via JSON" << std::endl;
    for(auto it = s_modules.begin(); it != s_modules.end(); ++it)
    {
        std::cout << "From: " << std::get<0>(*it) << " got: " << std::get<1>(*it)() << std::endl;
    }

    std::cout << "+++Serialize via JSON by key" << std::endl;
    std::cout << "Got from  Module1: "; std::cout << s_modules.at("Module1")() << std::endl;
    std::cout << "Got from  Module2: "; std::cout << s_modules.at("Module2")() << std::endl;

    // Test to clear links by JSON description only
    // {
    // {Link2:["Module2.link2", "Module1.DO2"]}
    // }
    clear_links.at("Module2.link2")("Link2", dos.at("Module1.DO2"));

    delete g_rptr;
#endif

#if 0
    Asm::Timer timer1;

    std::cout << "Start with delay of 3s with intervall 2s" << std::endl;

    timer1.setRelativeInterval(2000, 3000);

    for(int i = 0; i < 5; ++i)
    {
        uint64_t t;
        timer1.Wait(t);

        std::cout << "Elapsed time " << t << std::endl;
    }

    std::cout << "Start with delay of 0s with intervall 2s" << std::endl;

    timer1.setRelativeInterval(2000, 0);

    for(int i = 0; i < 5; ++i)
    {
        uint64_t t;
        timer1.Wait(t);

        std::cout << "Elapsed time " << t << std::endl;
    }

    std::cout << "Timer stop and restart" << std::endl;

    timer1.stop();

    timer1.restart();

    for(int i = 0; i < 5; ++i)
    {
        uint64_t t;
        timer1.Wait(t);

        std::cout << "Elapsed time " << t << std::endl;
    }

    timer1.Stop();
#endif

#if 0
    Asm::Timer timer2;

    std::cout << "Register timer2" << std::endl;
    g_trptr->registerTimer(&timer2);

    std::cout << "Wait 3s" << std::endl;
    boost::this_thread::sleep_for(boost::chrono::seconds(3));

    std::cout << "Set intervall to 1s at timer2 with delay 0s" << std::endl;
    timer2.setRelativeInterval(1000, 0);

    std::cout << "Wait 6s" << std::endl;
    boost::this_thread::sleep_for(boost::chrono::seconds(6));

    std::cout << "Stop timer2" << std::endl;
    timer2.stop();

    std::cout << "Unregister timer2" << std::endl;
    g_trptr->unregisterTimer(&timer2);
#endif

    boost::this_thread::sleep_for(boost::chrono::seconds(15));


    delete g_trptr;
    delete g_rptr;

    exit(0);
}

