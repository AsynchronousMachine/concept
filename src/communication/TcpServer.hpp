#pragma once

#ifdef __linux__
#include <pthread.h>
#include <sys/syscall.h>
#endif

#include <iostream>
#include <string>
#include <functional>
#include <thread>

#include <boost/asio.hpp>

namespace Asm {

class TcpServer {

    public:
        static constexpr int MAX_BUFFER_SIZE = 1000000;

        using cb_type = std::function<void(boost::asio::ip::tcp::socket&, size_t, std::array<char, MAX_BUFFER_SIZE>&)>;

    private:
        bool _run_state;
        unsigned short _port;
        cb_type _cb;
        std::thread _thrd;

        void stop() {
            _run_state = false;

            try {
                boost::asio::io_service io_service;
                boost::asio::ip::tcp::socket socket{io_service};
                boost::asio::ip::tcp::resolver resolver{io_service};
                boost::asio::ip::tcp::resolver::query query{"127.0.0.1", std::to_string(_port)};

                boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

                boost::asio::connect(socket, endpoint_iterator);

                std::cout << "TcpServer did connect to stop" << std::endl;
            } catch (std::exception& e) {
                std::cerr << "TcpServer stop: " << e.what() << std::endl;
            }
        }

        void run() {
            std::array<char, MAX_BUFFER_SIZE> _buffer;

    #ifdef __linux__
            std::cout << "TcpServer-THRD has TID-" << syscall(SYS_gettid) << std::endl;
    #endif

            try {
                boost::asio::io_service io_service;
                boost::asio::ip::tcp::socket socket{io_service};
                boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address_v6::any(), _port};
                boost::asio::ip::tcp::endpoint endpoint_peer;
                boost::asio::ip::tcp::acceptor acceptor{io_service, endpoint};

                //acceptor.set_option(boost::asio::socket_base::linger{true, 3});

                while(_run_state) {
                    std::cout << "TcpServer wait for connection" << std::endl;

                    acceptor.accept(socket, endpoint_peer);

                    std::cout << "TcpServer got connection from " << endpoint_peer.address().to_string() << " " << endpoint_peer.port() << std::endl;

                    if(!_run_state)
                        break;

                    size_t len = socket.read_some(boost::asio::buffer(_buffer));

                    if(_cb)
                        _cb(socket, len, _buffer);

                    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
                    socket.close();
                }

                std::cout << "TcpServer got stop" << std::endl;
            } catch (std::exception& e) {
                std::cerr << "TcpServer run: " << e.what() << std::endl;
            }
        }

    public:
        TcpServer(unsigned short port, cb_type cb) : _run_state(true), _port(port), _cb(cb) {
            _thrd = std::thread([this]{ TcpServer::run(); });
        };

        // Non-copyable
        TcpServer(const TcpServer&) = delete;
        TcpServer &operator=(const TcpServer&) = delete;

        // Non-movable
        TcpServer(TcpServer&&) = delete;
        TcpServer &operator=(TcpServer&&) = delete;

        ~TcpServer() {
            std::cout << "Delete TcpServer" << std::endl;
            stop();
            _thrd.join();
        }
};

}
