#pragma once

#include <boost/asio.hpp>

namespace Asm {

	using namespace boost::asio;

//#define MAX_DATA_LENGTH 65507


	class TCP_SyncClient {

	private:

		io_service _service;
		ip::tcp::endpoint _endpoint;

	public:
		TCP_SyncClient(const std::string& ip_address, int port)
			: _endpoint(ip::address::from_string(ip_address), port)
		{};

		void send(std::string msg) {
			ip::tcp::socket _socket(_service);
			_socket.connect(_endpoint);
			_socket.write_some(boost::asio::buffer(msg));
			_socket.close();
		}

		std::string sendAndRead(std::string msg) {
			ip::tcp::socket _socket(_service);
			_socket.connect(_endpoint);
			_socket.write_some(boost::asio::buffer(msg));
			char _readBuffer[MAX_DATA_LENGTH];
			int bytes = _socket.read_some(boost::asio::buffer(_readBuffer));
			std::string response(_readBuffer, bytes);

			_socket.close();
			return response;
		}

	};
}
