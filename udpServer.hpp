#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#define MAX_UDP_DATA_LENGTH 65507

namespace Asm {

using boost::asio::ip::udp;

class UdpServer {

public:

    /**
     *  constructor for asynchronously receiving and sending udp messages
     *  @param udpIOService noncopyable asio io service
     *  @param listenPort portnumber to start receiving udp messages at
     *  @param receiveCallback function to be called if udp message was received
     */
	UdpServer(boost::asio::io_service &udpIOService, int listenPort, std::function<void(char*)> receiveCallback)
        :   m_receiveEndpoint(udp::endpoint(udp::v4(), listenPort)),
            m_udpSocket(udpIOService, m_receiveEndpoint),
            m_receiveCallback(receiveCallback)
	{
		receive();
	}

    /**
     *  constructor, creating udpsocket for sending only
     *  @param udpIOService noncopyable asio io service
     */
	UdpServer(boost::asio::io_service &udpIOService)
        :   m_udpSocket(udpIOService, udp::endpoint(udp::v4(), 0)){}

    /**
     * send message via udp to specified ipv4:port
     * @param destinationIP ipv4 to send message to e.g. \"127.0.0.1\"
     * @param destinationPort port to send message to
     * @param data pointer to data that shall be sent
     * @param dataLength amount of bytes to be sent
     */
	void sendTo(std::string destinationIP, int destinationPort, unsigned char *data, int dataLength){
	    udp::endpoint remoteEndpoint(boost::asio::ip::address::from_string(destinationIP), destinationPort);

		m_udpSocket.async_send_to(boost::asio::buffer(data, dataLength), remoteEndpoint, boost::bind(&UdpServer::handleSend, this,
	        	boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

    /**
     * send message via udp to specified ipv4:port
     * @param destinationIP ipv4 to send message to e.g. \"127.0.0.1\"
     * @param destinationPort port to send message to
     * @param data string that shall be sent
     */
	void sendTo(std::string destinationIP, int destinationPort, std::string data){
	    udp::endpoint remoteEndpoint(boost::asio::ip::address::from_string(destinationIP), destinationPort);

		m_udpSocket.async_send_to(boost::asio::buffer(data), remoteEndpoint, boost::bind(&UdpServer::handleSend, this,
	        	boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

private:

	udp::endpoint m_receiveEndpoint; ///< endpoint data will be received from
	udp::socket m_udpSocket; ///< udp socket instance
	char m_receiveBuffer[MAX_UDP_DATA_LENGTH]; ///< buffer, data will be stored into when receiving
	std::function<void(char*)> m_receiveCallback; ///< callback to be called if data was received

	/**
	 *	start asynchronous receive
	 *  received data will be stored in @see m_receiveBuffer
	 */
	void receive(){
		m_udpSocket.async_receive_from(boost::asio::buffer(m_receiveBuffer, MAX_UDP_DATA_LENGTH), m_receiveEndpoint, boost::bind(&UdpServer::handleReceive, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	/**
	 *	callback of @see receive when data was received
	 *  will call receive() at the end to keep io service running
	 *	@param error contains error code
	 *	@param bytes_transferred amount of bytes received as data
	 */
	void handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred) {

		if (!error || error == boost::asio::error::message_size) {

			m_receiveCallback(m_receiveBuffer);
	  		receive();
		}
	}

	/**
	 *	callback of @see sendTo
	 *	@param error contains error code
	 *	@param bytes_transferred amount of bytes sent
	 */
	void handleSend(const boost::system::error_code& error, std::size_t bytes_transferred){}
};
}

