#include <mutex>

#include <boost/circular_buffer.hpp>

// Concept of reactor
//
// All registered callback functions (aka LINKS) should be called within the reactor
// This decouples the changing of data (content) and the notify process based on that
// This allows the introduction of priorities how important a change has been notified

// A simple reactor
// To simulate the not implemented asynchronous behavior call the public function execute()
class Reactor
{
    private:
        // Max. queue capacity
        static constexpr size_t MAX_CAPACITY = 1024;

        // This is a circular buffer to hold all links from dataobjects which content has been changed
        boost::circular_buffer<std::function<void()>> _triggeredDOs{MAX_CAPACITY};

        // Protect the list of triggered DOs
        std::mutex _triggeredDOs_mutex;

    public:
        // Announce the change of the content of a dataobject to the reactor
        template <class D>
        void trigger(DataObject<D> &d)
        {
            std::lock_guard<std::mutex> lock(_triggeredDOs_mutex);
            if(!d._links.empty())
            {
                std::lock_guard<std::mutex> lock(d._links_mutex);

                for(auto &p : d._links)
                    _triggeredDOs.push_front(p.second);
            }
            // Now trigger a synchronization element to release at least a waiting thread
        }

        // Call all DOs which are linked to that DOs which have been triggered like DO2.CALL(&DO1) / DO1 ---> DO2
        // These method is typically private and called with in a thread related to a priority
        // This thread is typically waiting on a synchronization element
        void execute()
        {
            std::function<void()> cb;

            for(;;)
            {
                {
                    std::lock_guard<std::mutex> lock(_triggeredDOs_mutex);

                    if(_triggeredDOs.empty())
                        break;

                    cb = _triggeredDOs.front();
                    _triggeredDOs.pop_front();
                }

                // Execute the callback without holding the lock
                cb();
            }

            std::cout << "----" << std::endl;
        }
};
