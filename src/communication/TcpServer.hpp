/*
** Definition of TCP server used for communication
*/

#pragma once

#ifdef __linux__
#include <pthread.h>
#include <sys/syscall.h>
#endif

#include <functional>
#include <memory>
#include <future>
#include <iostream>
#include <list>
#include <string>
#include <thread>

#include <boost/asio.hpp>

#include "../asm/asm.hpp"
#include "../logger/logger.hpp"

namespace Asm {

class TcpServer {

  public:
    static constexpr int max_buffer_size = 100000;

    using cb_type = std::function<void(boost::asio::ip::tcp::socket&, size_t, std::array<char, max_buffer_size>&)>;

  private:
    bool _run_state = true;
    cb_type _cb = nullptr;
    unsigned short _port;
    std::thread _thrd;

    void stop() {
        _run_state = false;

        try {
            boost::asio::io_service io_service;
            boost::asio::ip::tcp::socket socket{io_service};
            boost::asio::ip::tcp::resolver resolver{io_service};
            boost::asio::ip::tcp::resolver::query query{"::1", std::to_string(_port)};

            boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

            boost::asio::connect(socket, endpoint_iterator);

            Logger::pLOG->info("TcpServer did connect to stop");
        } catch (std::exception &e) {
            Logger::pLOG->error("TcpServer exception in stop: {}", e.what());
        }
    }

    void run(auto thisptr) {
#ifdef __linux__
        Logger::pLOG->info("TcpServer-THRD has TID-{}", syscall(SYS_gettid));
#endif

        try {
            boost::asio::io_service io_service;
            boost::asio::ip::tcp::socket socket{io_service};
            boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address_v6::any(), _port};
            boost::asio::ip::tcp::endpoint endpoint_peer;
            boost::asio::ip::tcp::acceptor acceptor{io_service, endpoint};
        
            while (_run_state) {
                Logger::pLOG->trace("TcpServer wait for connection, with socket id {}", (void*)&socket);

                acceptor.accept(socket, endpoint_peer);

                Logger::pLOG->trace("TcpServer got connection from {} {}", endpoint_peer.address().to_string(), endpoint_peer.port());

                if (!_run_state)
                    break;

                std::shared_ptr<boost::asio::ip::tcp::socket>  sock{std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket))};
                Logger::pLOG->trace("Make shared socket with id {}", (void*)(sock.get()));
                
                Asm::pDOR->trigger([thisptr, sock]() mutable { std::invoke(&Asm::TcpServer::handle_connection, thisptr, sock); });

                Logger::pLOG->info("TcpServer job enqueued");
            }

            Logger::pLOG->info("TcpServer got stop");
        } catch (std::exception &e) {
            Logger::pLOG->error("TcpServer exception in run: {}", e.what());
        }
    }

    void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
        auto overAllLength = 0;
        auto receivedLength = 0;
        boost::system::error_code ec;
        std::array<char, max_buffer_size> buffer;
        
        do {
            receivedLength = socket->read_some(boost::asio::buffer(&buffer[overAllLength], max_buffer_size - overAllLength), ec);
            overAllLength += receivedLength;
        } while (!ec && receivedLength > 0);

        if (_cb)
            _cb(*socket, overAllLength, buffer);

        // Swallow all faults from end points
        socket->close(ec);
        Logger::pLOG->trace("Closed socket with id {}", (void*)(socket.get()));
    }

  public:
    TcpServer(unsigned short port, cb_type cb) : _cb(cb), _port(port) {
        _thrd = std::thread([this] { TcpServer::run(this); });
    };

    // Non-copyable
    TcpServer(const TcpServer &) = delete;
    TcpServer &operator=(const TcpServer &) = delete;

    // Non-movable
    TcpServer(TcpServer &&) = delete;
    TcpServer &operator=(TcpServer &&) = delete;

    ~TcpServer() {
        Logger::pLOG->info("Delete TcpServer");
        stop();
        _thrd.join();
    }
};

}

