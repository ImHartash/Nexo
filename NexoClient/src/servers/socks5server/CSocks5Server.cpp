#include "CSocks5Server.hpp"
#include "config/ClientConfiguration.hpp"

CSocks5Server::CSocks5Server(boost::asio::io_context& IOContext, short nPort)
	: m_Acceptor(IOContext, tcp::endpoint(tcp::v4(), nPort)), m_SSLContext(ssl::context::tls_client) {
	if (Config::TLS.bVerifyCert) {
		m_SSLContext.set_verify_mode(ssl::verify_peer);
		m_SSLContext.set_default_verify_paths();
	}
	else {
		m_SSLContext.set_verify_mode(ssl::verify_none);
	}
}

void CSocks5Server::Start() {
	this->Listen();
}

void CSocks5Server::Listen() {
	m_Acceptor.async_accept(
		[this](boost::system::error_code Error, tcp::socket ClientSocket) {
			if (!Error) std::make_shared<CSocks5Session>(std::move(ClientSocket), m_SSLContext)->Start();
			Listen();
		});
}