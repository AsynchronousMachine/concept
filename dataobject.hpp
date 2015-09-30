#include <forward_list>
#include <type_traits>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>

// Concept of data object
//
// Data objects (aka DO) are container to hold data (content) and callback functions (aka LINKS) as notifier
// DOs can be linked together via LINKS
// DOs can be locked and unlocked to handle there content consistently
// The associated callback functions (LINKS) are called if the content of the original DO has been changed
//
// DO1 ------LINK1-------> DO2    DO1 is linked to DO2
//        |
//        +--LINK3-------> DO3    Also DO1 is linked to DO3

// Concept of reactor
//
// This is not yet shown here

// Notifier type for the LINKS
using notifier = std::function<void()>;

// Template class for arbitrary  content
template <typename T> class DataObject
{
    friend class AsynchronousMachine; // This enables the reactor to call notify_all() from outside

    private:
        template <class M, class Enable = void>
        struct mutex
        {
            using type = void; //Invalid type for mutex
        };

        template <class M>
        struct mutex<M, std::enable_if_t<std::is_integral<M>::value>>
        {
            using type = boost::null_mutex;
        };

        template <class M>
        struct mutex<M, std::enable_if_t<!std::is_integral<M>::value>>
        {
            using type = boost::shared_mutex;
        };

        // Make it easier to name it
        using mutex_t = typename mutex<T>::type;

        // A data object should have a constant name to identify it
        const std::string _name;

        // Content for this DO
        T _content;

        // Mutable mutex_ member as it needs to be modified in the const member function get()
        mutable mutex_t _mutex;

        // Protect the list of linked DOs
        boost::mutex _link_mutex;

        // This should be a at least a simple list to hold all callbacks (LINKS) linked to that original DO
        // A mutex for exlusive access will properly also necessary
        std::forward_list<std::function<void(DataObject<T>&)>> links;

        // Only called by reactor
        void notify_all()
        {
            boost::lock_guard<boost::mutex> lock(_link_mutex);
            std::for_each(links.begin(), links.end(), [this](std::function<void(DataObject<T>&)> f){ f(*this); });
        }

    public:
        DataObject(const std::string name) : _name(name) {}
        DataObject(const std::string name, T content) : _name(name), _content(content) {}

        // Non-copyable
        DataObject(const DataObject&) = delete;
        DataObject &operator=(const DataObject&) = delete;

        // Non-movable
        DataObject(DataObject&&) = delete;
        DataObject &operator=(DataObject&&) = delete;

        // Necessary if someone want to inherit from that
        virtual ~DataObject() = default;

        template <class Visitor>
        void set(Visitor visitor)
        {
            // Exclusive lock for write access
            boost::lock_guard<mutex_t> lock(_mutex);
            visitor(_content);
        }

        // Const member function to avoid that a non-const reference is passed to the visitor
        // as that would allow the visitor to change the data_ member
        template <class Visitor>
        std::result_of_t<Visitor(T)> get(Visitor visitor) const
        {
            // Shared lock to support concurrent access from multiple visitors in different threads
            boost::shared_lock_guard<mutex_t> lock(_mutex);
            return visitor(_content);
        }

        // Get out the DO name for humans
        const std::string& getName() const { return _name; }

        // Link a DO to that DO
        template <typename D, typename C>
        void registerLink(DataObject<D> &d2, C cb)
        {
            boost::lock_guard<boost::mutex> lock(_link_mutex);
            links.push_front([cb, &d2](DataObject<T>& d1){ cb(d1, d2); });
        }

        // Remove a link to a DO by name
        void unregisterLink(std::string name)
        {
            boost::lock_guard<boost::mutex> lock(_link_mutex);
            /*todo*/
        }
};
