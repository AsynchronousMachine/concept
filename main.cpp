#include <iostream>
#include <string>

// Concept of data object
//
// Data objects (aka DO) are container to hold data (content) and a callback function as a notifier
// DOs can be linked together
// DOs can be locked and unlocked to handle there content carefully
// The associated functions must be called if the content of the linked DO has been changed
//
// DO1 --------------> DO2    DO1 is linked to DO2
//        |
//        +----------> DO3    Also DO1 is linked to DO3

// Concept of reactor
//
// All registered callback functions which should be called within the reactor
// This decouples the changing and the notify process based on that
// This allows the introduction of priorities how important a change has been notified

// Everything that all DOs have in common
class DataObjectBase
{
    // Make it easier to handle the callback function
    typedef void (CallbackType)(DataObjectBase*);

    protected:
        // A data object should have a name to identify it by humans
        std::string _name;
        // This should be a at least a list to hold all data objects linked to that
        DataObjectBase* _ptr=nullptr;
        // This is a pointer to a function which is called if the content of data object linked to has been changed
        CallbackType* _cb=nullptr;
        // Some constructors
        DataObjectBase() {}
        DataObjectBase(std::string& name) : _name(name) {}

    public:
        // Necessary for a base class
        virtual ~DataObjectBase() {}
        // Set the DO name explicitly
        void setName(std::string& name) { _name = name; }
        // Get out the DO name for humans
        const std::string& getName() const { return _name; }
        // Lock it to have a consistent access to the content
        void lock() { std::cout << "Locked" << std::endl; }
        // Release the DO so others can also access the content
        void unlock() { std::cout << "Unlocked" << std::endl; }
        // Link a DO to that DO
        void registerLink(DataObjectBase* ptr) { _ptr = ptr; std::cout << "Link " << ptr->getName() << " to " << getName() << std::endl; }
        // Store a callback function it is called if the content of the DO linked to has been changed
        void registerCallback(CallbackType* cb=nullptr) { _cb = cb; }
        // Access to the DO which is linked to, needed by the reactor
        DataObjectBase* getLinkedDO() { return _ptr; }
        // Access to the registered callback function, needed by the reactor
        void call(DataObjectBase* ptr) { (*_cb)(ptr); }
};

// Template class for arbitrary  content
template <typename T> class DataObject : public DataObjectBase
{
    // Content to handle
    T _content;

    public:
        // Some constructors
        DataObject() {}
        DataObject(std::string& name) : DataObjectBase(name) {}
        // Necessary if someone want to inherit from that
        virtual ~DataObject() {}
        // General access to the content by reference
        T& get() { return _content; }
};

// A simple reactor
class AsynchronousMachine
{
    protected:
        // This should be a at least a list to hold all data objects which content has been changed
        DataObjectBase* _ptr;

    public:
        // Announce the change of content to the reactor
        void trigger(DataObjectBase* ptr) { _ptr = ptr; std::cout << "Trigger DO " << ptr->getName() << std::endl; }
        // Call the registered callback function to notify the DO like DO2.CALL(&DO1)
        void execute() { _ptr->getLinkedDO()->call(_ptr); }
};

// A callback function
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
    // Register a callback to call if content of DO1 has been changed
    do1.registerLink(&do2);

    // Access content consistently
    do1.lock();
    int& integer = do1.get();
    integer = 10;
    do1.unlock();
    asm1.trigger(&do1); // Because of changed content of do1

    std::cout << integer << std::endl;

    // Other variants of data access
    do1.lock();
    do1.get() = 7;
    do1.unlock();
    //asm1.trigger(&do1); // Because of changed content of do1

    std::cout << do1.get() << std::endl;

    // Other variants of data access
    do1.lock();
    int tmp = do1.get();
    do1.unlock();
    //asm1.trigger(&do1); // Because of changed content of do1

    std::cout << tmp << std::endl;

    // Simulate the job of ASM with dDO2.CALL(&DO1)
    // Should notify all callbacks if all DOs linked to
    asm1.execute();

    return 0;
}

