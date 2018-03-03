/*
** Declarations of DataObject and LinkObject handler functions used for communication
*/

#pragma once

#include <boost/asio.hpp>

#include "TcpServer.hpp"

namespace Asm {

extern void do_handler(boost::asio::ip::tcp::socket& socket, size_t len, std::array<char, Asm::TcpServer::max_buffer_size>& buffer);
extern void lo_handler(boost::asio::ip::tcp::socket& socket, size_t len, std::array<char, Asm::TcpServer::max_buffer_size>& buffer);

}
