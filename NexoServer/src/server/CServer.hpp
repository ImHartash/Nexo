#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace ssl = boost::asio::ssl;
using boost::asio::ip::tcp;

class CServer {
public:
	CServer(boost::asio::io_context& IOContext, uint16_t nPort);	
	void Listen();

private:
	void AcceptConnection();

	ssl::context m_SSLContext;
	tcp::acceptor m_Acceptor;
};