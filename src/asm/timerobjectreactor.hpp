#pragma once

#include <unordered_map>

#include <boost/thread/thread.hpp>

#ifdef __linux__
#include <pthread.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

namespace Asm {

//#ifdef __linux__

	class TimerObjectReactor
	{
	private:
		// Max. epoll capacity, can be found at /proc/sys/fs/epoll/max_user_watches
		static constexpr size_t MAX_CAPACITY = 256;

		// File descriptor for epoll mechanism
		int _epfd;

		// File descriptor for epoll stop mechanism
		int _evtfd;

		// Reference to the event reactor
		DataObjectReactor &_dor;

		// Holds the timer thread reference
		boost::thread _thrd;

		// Holds all epoll file descriptor associated data
		std::unordered_map<int, DataObject<TimerObject>&> _notify;

		// Protects the access to epoll file descriptor associated data
		boost::mutex _mtx;

		// Threaded timer function mechanism
		void run()
		{

#ifdef __linux__
			std::cout << "Tid of " << syscall(SYS_gettid) << " for timer reactor" << std::endl;


			for (;;)
			{
				epoll_event evt[MAX_CAPACITY];

				int evt_cnt = ::epoll_wait(_epfd, &evt[0], MAX_CAPACITY, -1);

				if (evt_cnt <= 0)
				{
					std::cout << "Epoll wait error: " << std::strerror(errno) << std::endl;
					continue;
				}

				for (int i = 0; i < evt_cnt; i++)
				{
					if (evt[i].events & EPOLLIN)
					{
						uint64_t elapsed;

						if (::read(evt[i].data.fd, &elapsed, sizeof(elapsed)) != sizeof(elapsed))
						{
							std::cout << "Read timer returns wrong size: " << std::strerror(errno) << std::endl;
							continue;
						}

						if (evt[i].data.fd == _evtfd && elapsed > 0)
						{
							std::cout << "Read timer returns stop command" << std::endl;
							return;
						}

						std::cout << "Timer has fired" << std::endl;

						boost::unique_lock<boost::mutex> lock(_mtx);
						auto itr = _notify.find(evt[i].data.fd);
						if ( itr != _notify.end() )
						{
							lock.unlock();
							_dor.trigger(itr->second);
						}
					}
				}
			}
#endif
		}

	public:
		TimerObjectReactor(DataObjectReactor& dor) : _epfd(-1), _evtfd(-1), _dor(dor)
		{
#ifdef __linux__
			if ((_evtfd = ::eventfd(0, EFD_CLOEXEC)) < 0)
			{
				std::cout << "Eventfd file handle could not be created: " << std::strerror(errno) << std::endl;
				return;
			}

			if ((_epfd = ::epoll_create1(EPOLL_CLOEXEC)) < 0)
			{
				std::cout << "Epoll file handle could not be created: " << std::strerror(errno) << std::endl;
				close(_evtfd);
				return;
			}

			// Add it first to stop epoll_wait in case of destruction
			epoll_event evt;
			evt.events = EPOLLIN;
			evt.data.fd = _evtfd;

			if (::epoll_ctl(_epfd, EPOLL_CTL_ADD, _evtfd, &evt) < 0)
			{
				std::cout << "Epoll control error at ADD stop event: " << std::strerror(errno) << std::endl;
				close(_epfd);
				close(_evtfd);
				return;
			}

			_thrd = boost::thread([this]() { TimerObjectReactor::run(); });
#endif
		}

		// Non-copyable
		TimerObjectReactor(const TimerObjectReactor&) = delete;
		TimerObjectReactor &operator=(const TimerObjectReactor&) = delete;

		// Non-movable
		TimerObjectReactor(TimerObjectReactor&&) = delete;
		TimerObjectReactor &operator=(TimerObjectReactor&&) = delete;

		~TimerObjectReactor()
		{
			std::cout << "Delete timer reactor" << std::endl;

			uint64_t stop = 1;

#ifdef __linux__

			if (::write(_evtfd, &stop, sizeof(stop)) != sizeof(stop))
			{
				std::cout << "Write timer stop failed: " << std::strerror(errno) << std::endl;
			}

			_thrd.join();

			if (_evtfd >= 0)
				::close(_evtfd);

			if (_epfd >= 0)
				::close(_epfd);
#endif
		}

		bool registerTimer(DataObject<TimerObject>& dot)
		{
			int fd = dot.get([](const TimerObject& t) { return t._fd; });
#ifdef __linux__
			epoll_event evt;
			evt.events = EPOLLIN;
			evt.data.fd = fd;

			{
				boost::lock_guard<boost::mutex> lock(_mtx);
				_notify.insert({ fd, dot });
			}

			if (::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &evt) < 0)
			{
				// Delete it again in the case of error
				{
					boost::lock_guard<boost::mutex> lock(_mtx);
					_notify.erase(fd);
				}

				std::cout << "Epoll control error at ADD: " << std::strerror(errno) << std::endl;
				return false;
			}
#endif
			return true;
		}

		bool unregisterTimer(DataObject<TimerObject>& dot)
		{
			bool ret = true;

			int fd = dot.get([](const TimerObject& t) { return t._fd; });
#ifdef __linux__
			if (::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, 0) < 0)
			{
				std::cout << "Epoll control error at DEL: " << std::strerror(errno) << std::endl;
				ret = false;
			}

			// Erase it in any case
			{
				boost::lock_guard<boost::mutex> lock(_mtx);
				_notify.erase(fd);
			}
#endif
			return ret;
		}
	};


	//well known Reactor
	extern std::unique_ptr<TimerObjectReactor> trptr;
}
