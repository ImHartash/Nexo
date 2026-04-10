#include "CSocks5Server.hpp"

CSocks5Server::CSocks5Server(boost::asio::io_context& IOContext, short nPort)
	: m_Acceptor(IOContext, tcp::endpoint(tcp::v4(), nPort)) { }

void CSocks5Server::Start() {
	this->Listen();
}

void CSocks5Server::Listen() {
	m_Acceptor.async_accept(
		[this](boost::system::error_code Error, tcp::socket ClientSocket) {
			if (!Error) std::make_shared<CSocks5Session>(std::move(ClientSocket))->Start();
			Listen();
		});
}