#include <queue>

// Concept of reactor
//
// All registered callback functions (aka LINKS) should be called within the reactor
// This decouples the changing of data (content) and the notify process based on that
// This allows the introduction of priorities how important a change has been notified

// A simple reactor
// To simulate the not implemented asynchronous behavior call the public function execute()
class Reactor
{
    protected:
        // Protect the list of triggered DOs
        boost::mutex _triggeredDOs_mutex;

        // This should be a at least a queue to hold all data objects which content has been changed or rather triggered
        std::queue<std::function<void()>> _triggeredDOs;

    public:
        // Announce the change of content to the reactor
        template <class D1>
        void trigger(DataObject<D1> &do1)
        {
            boost::lock_guard<boost::mutex> lock(_triggeredDOs_mutex);
            if(!do1.links.empty())
            {
                boost::lock_guard<boost::mutex> lock(do1._link_mutex);

                std::for_each(do1.links.begin(), do1.links.end(), [this](std::function<void()> f){ _triggeredDOs.push(f); });
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
                std::function<void()> f;

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
