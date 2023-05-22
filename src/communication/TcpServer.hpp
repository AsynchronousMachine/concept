/*
** Definition of TCP server used for communication
*/

#pragma once

#ifdef __linux__
#include <pthread.h>
#include <sys/syscall.h>
#endif

#include <functional>
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

    using cb_type = std::function<void(boost::asio::ip::tcp::socket &, size_t, std::array<char, max_buffer_size> &)>;

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
            // std::list<std::future<void>> f;
            boost::asio::io_service io_service;
            boost::asio::ip::tcp::socket socket{io_service};
            boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address_v6::any(), _port};
            boost::asio::ip::tcp::endpoint endpoint_peer;
            boost::asio::ip::tcp::acceptor acceptor{io_service, endpoint};

            while (_run_state) {
                //                    // Clean up remaining futures
                //                    for (auto it = f.cbegin(); it != f.cend(); ) {
                //                        if (it->valid()) {
                //                            auto s = it->wait_for(std::chrono::seconds(0));
                //                            if (s == std::future_status::ready) {
                //                                Logger::pLOG->trace("Found remaining future to clean up");
                //                                it = f.erase(it);
                //                                continue;
                //                            }
                //                        }

                //                        Logger::pLOG->trace("Found pending future");
                //                        ++it;
                //                    }

                Logger::pLOG->trace("TcpServer wait for connection");

                acceptor.accept(socket, endpoint_peer);

                Logger::pLOG->trace("TcpServer got connection from {} {}", endpoint_peer.address().to_string(), endpoint_peer.port());

                if (!_run_state)
                    break;

                boost::asio::ip::tcp::socket s{std::move(socket)};
                Asm::pDOR->trigger([thisptr, &s] { std::invoke(&TcpServer::handle_connection, thisptr, std::move(s)); });

                Logger::pLOG->info("TcpServer job enqueued");
                // f.emplace_back(std::async(std::launch::async, std::mem_fn(&TcpServer::handle_connection), thisptr, std::move(socket)));
            }

            Logger::pLOG->info("TcpServer got stop");
        } catch (std::exception &e) {
            Logger::pLOG->error("TcpServer exception in run: {}", e.what());
        }
    }

    void handle_connection(boost::asio::ip::tcp::socket &&socket) {
        auto overAllLength = 0;
        auto receivedLength = 0;
        boost::system::error_code ec;
        std::array<char, max_buffer_size> buffer;

        do {
            receivedLength = socket.read_some(boost::asio::buffer(&buffer[overAllLength], max_buffer_size - overAllLength), ec);
            overAllLength += receivedLength;
        } while (!ec && receivedLength > 0);

        if (_cb)
            _cb(socket, overAllLength, buffer);

        try {
            socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
            socket.close();
        } catch (std::exception &e) {
            // E.g. in the case the endpoint has already been closed
            Logger::pLOG->trace("TcpServer exception: {}", e.what());
        }
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
