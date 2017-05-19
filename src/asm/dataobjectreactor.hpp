#pragma once

#include <functional>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/lexical_cast.hpp>


#ifdef __linux__
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

#include <tbb/tbb.h>

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
		// Realtime priority
		static constexpr int RT_PRIO = 30;

		//TBB
		tbb::concurrent_bounded_queue<std::function<void()>> _tbbExecutionQueue; ///< contains links to be executed by @see run

		struct Threadpool
		{
			boost::thread_group _tg;
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

					struct sched_param param{};
					param.sched_priority = RT_PRIO;

					if (pthread_setschedparam(t->native_handle(), SCHED_FIFO, &param))
						std::cout << "Could not set realtime parameter" << std::endl;
#endif
				}

				std::cout << "Have " << _tg.size() << " thread/s running" << std::endl << std::endl;
			}

			~Threadpool()
			{
				std::cout << "Delete threadpool" << std::endl;
				_tg.interrupt_all();
				_tg.join_all();
			}

			void thrd(unsigned inst) { _f(inst); }
		};

		struct Threadpool _tp;

		// Call all DOs which are linked to that DOs which have been triggered like DO2.CALL(&DO1) / DO1 ---> DO2
		// These method is typically private and called with in a thread related to a priority
		// This thread is typically waiting on a synchronization element
		void run(unsigned inst)
		{
			std::function<void()> f;

			try {
				
#ifdef __linux__
				std::cout << "Tid of " << syscall(SYS_gettid) << " for dataobject reactor instance " << inst << std::endl;
#endif

				for (;;)
				{
					_tbbExecutionQueue.pop(f); // Pop of concurrent_bounded_queue waits if queue empty
					f();
				}
			}catch(tbb::user_abort abortException){
				std::cout << "Ending ASM-TP with instance " << inst << std::endl;
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

		~DataObjectReactor() {
			_tbbExecutionQueue.abort(); // Stops waiting of pop() in @see run
			std::cout << std::endl << "Delete reactor" << std::endl;
		}

		// Announce the change of the content of a dataobject to the reactor
		template <class D>
		void trigger(DataObject<D>& d)
		{
			{
				boost::lock_guard<boost::mutex> lock(d._mtx_links);

				if (d._links.empty())
					return;

				for (auto &p : d._links)
					_tbbExecutionQueue.push(p.second); // Queue is synchronized by tbb
			}
		}
	};
}
