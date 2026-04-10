#pragma once
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class CServer {
public:
	CServer(boost::asio::io_context& IOContext, uint16_t nPort);	
	void Listen();

private:
	void AcceptConnection();

	tcp::acceptor m_Acceptor;
};