/*
** Definition of TCP server used for communication
*/

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

#include "../logger/logger.hpp"

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

                Logger::pLOG->info("TcpServer did connect to stop");
            } catch (std::exception& e) {
                Logger::pLOG->error("TcpServer exception in stop: {}", e.what());
            }
        }

        void run() {
            std::array<char, MAX_BUFFER_SIZE> _buffer;

    #ifdef __linux__
            Logger::pLOG->info("TcpServer-THRD has TID-{}", syscall(SYS_gettid));
    #endif

            try {
                boost::asio::io_service io_service;
                boost::asio::ip::tcp::socket socket{io_service};
                boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address_v6::any(), _port};
                boost::asio::ip::tcp::endpoint endpoint_peer;
                boost::asio::ip::tcp::acceptor acceptor{io_service, endpoint};

                //acceptor.set_option(boost::asio::socket_base::linger{true, 3});

                while(_run_state) {
                    Logger::pLOG->trace("TcpServer wait for connection");

                    acceptor.accept(socket, endpoint_peer);

                    Logger::pLOG->trace("TcpServer got connection from {} {}", endpoint_peer.address().to_string(), endpoint_peer.port());

                    if(!_run_state)
                        break;

                    size_t len = socket.read_some(boost::asio::buffer(_buffer));

                    if(_cb)
                        _cb(socket, len, _buffer);

                    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
                    socket.close();
                }

                Logger::pLOG->info("TcpServer got stop");
            } catch (std::exception& e) {
                Logger::pLOG->error("TcpServer exception in run: {}", e.what());
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
            Logger::pLOG->info("Delete TcpServer");
            stop();
            _thrd.join();
        }
};

}
