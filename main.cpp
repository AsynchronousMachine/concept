#include <iostream>
#include <string>

class DataObjectBase
{
    typedef void (CallbackType)(DataObjectBase*);

    protected:
        std::string _name;
        DataObjectBase* _ptr=nullptr;
        CallbackType* _cb=nullptr;

        DataObjectBase() {}
        DataObjectBase(std::string& name) : _name(name) {}

    public:
        virtual ~DataObjectBase() {}
        void setName(std::string& name) { _name = name; }
        const std::string& getName() const { return _name; }
        void lock() { std::cout << "Locked" << std::endl; }
        void unlock() { std::cout << "Unlocked" << std::endl; }
        void registerLink(DataObjectBase* ptr) { _ptr = ptr; std::cout << "Link " << ptr->getName() << " to " << getName() << std::endl; }
        void registerCallback(CallbackType* cb=nullptr) { _cb = cb; }
        DataObjectBase* getLinkedDO() { return _ptr; }
        void call(DataObjectBase* ptr) { (*_cb)(ptr); }
};



template <typename T> class DataObject : public DataObjectBase
{
    T _content;

    public:
        DataObject() {}
        DataObject(std::string& name) : DataObjectBase(name) {}
        virtual ~DataObject() {}
        T& get() { return _content; }
};

class AsynchronousMachine
{
    protected:
        DataObjectBase* _ptr;

    public:
        void trigger(DataObjectBase* ptr) { _ptr = ptr; std::cout << "Trigger DO " << ptr->getName() << std::endl; }
        void execute() { _ptr->getLinkedDO()->call(_ptr); }
};

void my_cb(DataObjectBase* ptr)
{
    std::cout << "Got DO.name: " << ptr->getName() << std::endl;
    std::cout << "Got DO.value: " << dynamic_cast<DataObject<int>*>(ptr)->get() << std::endl;
}

int main(void)
{
    AsynchronousMachine asm1;

    std::string h("Hello");
    std::string w("World");

    DataObject<int> do1;
    DataObject<int> do2(w);

    do1.setName(h);

    std::cout << do1.getName() << std::endl;
    std::cout << do2.getName() << std::endl;

    // Link together: do1<int> -------> do2<int>
    do2.registerCallback(my_cb);
    do1.registerLink(&do2);

    // Access content due to locked DO
    do1.lock();
    int& integer = do1.get();
    integer = 10;
    do1.unlock();
    asm1.trigger(&do1); // Because of changed content of do1

    std::cout << integer << std::endl;

    do1.lock();
    do1.get() = 7;
    do1.unlock();
    //asm1.trigger(&do1); // Because of changed content of do1

    std::cout << do1.get() << std::endl;

    do1.lock();
    int tmp = do1.get();
    do1.unlock();
    //asm1.trigger(&do1); // Because of changed content of do1

    std::cout << tmp << std::endl;

    // Simulate the job of ASM with do2.call(&do1)
    asm1.execute();

    return 0;
}

