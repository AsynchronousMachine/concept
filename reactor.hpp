//#include <forward_list>
//#include <type_traits>
//
//#include <boost/thread/shared_mutex.hpp>
//#include <boost/thread/null_mutex.hpp>
//#include <boost/thread/shared_lock_guard.hpp>

#include <queue>

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
// All registered callback functions (aka LINKS) should be called within the reactor
// This decouples the changing of data (content) and the notify process based on that
// This allows the introduction of priorities how important a change has been notified

// Concept of module
//
// This is not yet shown here

// A simple reactor
// To simulate the not implemented asynchron behaviour call the public function execute()
class AsynchronousMachine
{
    protected:
        // Protect the list of triggered DOs
        boost::mutex _triggeredDOs_mutex;

        // This should be a at least a queue to hold all data objects which content has been changed or rather triggered
        std::queue<notifier> _triggeredDOs;

    public:
        // Announce the change of content to the reactor
        template <class T>
        void trigger(DataObject<T> &do1)
        {
            boost::lock_guard<boost::mutex> lock(_triggeredDOs_mutex);
            if(!do1.links.empty())
            {
                _triggeredDOs.push([&do1](){ do1.notify_all(); });
                // Now trigger a synchronisation element like a counting semaphore to release a waiting thread
            }
        }

        // Call all DOs which are linked to that DOs which have been triggered like DO2.CALL(&DO1) / DO1 ---> DO2
        // These method is typically private and called with in a thread related with priority "prio"
        // This thread is waiting on a synchronisation element like a counting semaphore
        void execute()
        {
            bool empty = false;

            while(!empty)
            {
                notifier f;
                {
                    boost::lock_guard<boost::mutex> lock(_triggeredDOs_mutex);
                    if(!_triggeredDOs.empty())
                    {
                        f = _triggeredDOs.front();
                        _triggeredDOs.pop();
                    }
                    else
                    {
                        empty = true;
                        continue;
                    }

                }
                f();
            }
        }
};
