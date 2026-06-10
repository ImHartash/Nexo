#include "CServer.hpp"
#include "session/CSession.hpp"

CServer::CServer(boost::asio::io_context& IOContext, uint16_t nPort) 
	: m_Acceptor(IOContext, tcp::endpoint(tcp::v4(), nPort)), m_SSLContext(ssl::context::tls_server) {
	m_SSLContext.use_certificate_chain_file("server.crt");
	m_SSLContext.use_private_key_file("server.key", ssl::context::pem);
}

void CServer::Listen() {
	AcceptConnection();
}

void CServer::AcceptConnection() {
	m_Acceptor.async_accept(
		[this](boost::system::error_code Error, tcp::socket ClientSocket) {
			if (!Error) std::make_shared<CSession>(std::move(ClientSocket), m_SSLContext)->Start();
			AcceptConnection();
		});
}
