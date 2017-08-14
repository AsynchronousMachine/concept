#pragma once

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace Asm {

	using namespace boost::asio;
	using namespace boost::posix_time;
	using boost::system::error_code;

#define MAX_DATA_LENGTH 1000000


	class TCP_SyncServer {

	private:
		char _readBuffer[MAX_DATA_LENGTH];
		std::function<void(char*, int, ip::tcp::socket*)> _readCallback;
		io_service _service;
		ip::tcp::endpoint _endpoint;
		boost::thread* _serverThread;
		bool _run = true;

		void unlockAccept() {
			try {
				ip::tcp::socket socketConnect(_service);
				ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), _endpoint.port());
				socketConnect.connect(ep); // close is called implicit by leaving the method
			}
			catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}

		void run() {
			ip::tcp::acceptor acceptor(_service, _endpoint);
			ip::tcp::socket socket(_service);

			while (_run) {
				try {					
					acceptor.accept(socket);

					boost::asio::socket_base::linger option(true, 30);
					socket.set_option(option);

					while (_run && socket.available() == 0)
						boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
					if (_run) {
						int bytes = socket.read_some(buffer(_readBuffer)); // read(sock, buffer(buff), ec); blocks untill buffer is full, not desired here
						_readCallback(_readBuffer, bytes, &socket);
					}
					socket.close();
				}
				catch (std::exception& e) {
					if (_run)
						std::cerr << e.what() << std::endl;
				}
			}
		}

		void start() {
			_serverThread = new boost::thread([&]() {
				run();
			});
		}

	public:
		TCP_SyncServer(int listenPort, std::function<void(char*, int, ip::tcp::socket*)> receiveCallback)
			: _readCallback(receiveCallback),
			_endpoint(ip::tcp::v6(), listenPort)
		{
			start();
		};

		~TCP_SyncServer() {
			_run = false;
			unlockAccept();
			_serverThread->join();
		}
	};
}