#pragma once
#include "sessions/socks5session/CSocks5Session.hpp"

class CSocks5Server {
public:
	CSocks5Server(boost::asio::io_context& IOContext, short nPort);
	void Start();

private:
	void Listen();

	tcp::acceptor m_Acceptor;
};