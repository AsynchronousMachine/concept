#pragma once

#include <functional>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/lexical_cast.hpp>


#ifdef __linux__
#include <pthread.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

namespace Asm {

	template <typename D>
	class DataObject;


	// Concept of reactor
	//
	// All registered callback functions (aka LINKS) should be called within the reactor
	// This decouples the changing of data (content) and the notify process based on that
	// This allows the introduction of priorities how important a change has been notified

	// A simple reactor
	// To simulate the not implemented asynchronous behavior call the public function execute()
	class DataObjectReactor
	{
	private:
		// Max. queue capacity
		static constexpr size_t MAX_CAPACITY = 1024;

		// Realtime priority
		static constexpr int RT_PRIO = 30;

		// This is a circular buffer to hold all links from dataobjects which content has been changed
		boost::circular_buffer<std::function<void()>> _triggeredLinks{ MAX_CAPACITY };

		// Protect the circular buffer of triggered DOs
		boost::mutex _mtx;

		struct Threadpool
		{
			boost::thread_group _tg;
			boost::condition_variable _cv;
			std::function<void(unsigned inst)> _f;

			Threadpool(unsigned thrd_cnt, std::function<void(unsigned inst)> f) : _f(f)
			{
				unsigned cores = boost::thread::hardware_concurrency();

				std::cout << "Found " << cores << " cores" << std::endl;

				if (thrd_cnt == 0 || thrd_cnt > cores)
					thrd_cnt = cores;

				for (unsigned i = 0; i < thrd_cnt; ++i)
				{
					boost::thread *t = _tg.create_thread([this, i]() { Threadpool::thrd(i); });

					//The thread name is a meaningful C language string, whose length is
					//restricted to 16 characters, including the terminating null byte ('\0')
					std::string s = "ASM-TP" + boost::lexical_cast<std::string>(i);
					std::cout << s << std::endl;

#ifdef __linux__
					if (pthread_setname_np(t->native_handle(), s.data()))
						std::cout << "Could not set threadpool name" << std::endl;

					struct sched_param param = {};
					param.sched_priority = RT_PRIO;

					if (pthread_setschedparam(t->native_handle(), SCHED_FIFO, &param))
						std::cout << "Could not set realtime parameter" << std::endl;
#endif
				}

				std::cout << "Have " << _tg.size() << " thread/s running" << std::endl << std::endl;
			}

			~Threadpool()
			{
				std::cout << "Delete treadpool" << std::endl;
				_tg.interrupt_all();
				_tg.join_all();
			}

			void wait(boost::unique_lock<boost::mutex>& lock) { _cv.wait(lock); }

			void notify() { _cv.notify_one(); }

			void thrd(unsigned inst) { _f(inst); }
		};

		struct Threadpool _tp;

		// Call all DOs which are linked to that DOs which have been triggered like DO2.CALL(&DO1) / DO1 ---> DO2
		// These method is typically private and called with in a thread related to a priority
		// This thread is typically waiting on a synchronization element
		void run(unsigned inst)
		{
			std::function<void()> f;

			for (;;)
			{
				{
					boost::unique_lock<boost::mutex> lock(_mtx);

					while (_triggeredLinks.empty())
					{
						_tp.wait(lock);
						std::cout << ">>>" << inst << "<<<" << std::endl;
					}

					f = _triggeredLinks.front();
					_triggeredLinks.pop_front();
				}

				// Execute the link without holding the lock
				f();
			}
		}

	public:
		DataObjectReactor(unsigned thrd_cnt = 1) : _tp(thrd_cnt, [this](unsigned inst) { DataObjectReactor::run(inst); }) {}

		// Non-copyable
		DataObjectReactor(const DataObjectReactor&) = delete;
		DataObjectReactor &operator=(const DataObjectReactor&) = delete;

		// Non-movable
		DataObjectReactor(DataObjectReactor&&) = delete;
		DataObjectReactor &operator=(DataObjectReactor&&) = delete;

		~DataObjectReactor() { std::cout << std::endl << "Delete reactor" << std::endl; }

		// Announce the change of the content of a dataobject to the reactor
		template <class D>
		void trigger(DataObject<D>& d)
		{
			{
				boost::lock_guard<boost::mutex> lock(d._mtx_links);

				if (d._links.empty())
					return;

				for (auto &p : d._links)
					_triggeredLinks.push_back(p.second);
			}

			// Now trigger a synchronization element to release at least a waiting thread
			_tp.notify();
		}
	};
}
