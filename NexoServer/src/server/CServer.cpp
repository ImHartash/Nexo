#include "CServer.hpp"
#include "session/CSession.hpp"
#include "config/ServerConfiguration.hpp"
#include "logger/CLogger.hpp"

CServer::CServer(boost::asio::io_context& IOContext, uint16_t nPort) 
	: m_Acceptor(IOContext, tcp::endpoint(tcp::v4(), nPort)), m_SSLContext(ssl::context::tls_server) {
	m_SSLContext.use_certificate_chain_file(Config::Server.strCertFilePath);
	m_SSLContext.use_private_key_file(Config::Server.strKeyFilePath, ssl::context::pem);
}

CServer::~CServer() {
	LOG_INFO("Nexo server successfully stopped.");
}

void CServer::Listen() {
	AcceptConnection();
}

void CServer::AcceptConnection() {
	m_Acceptor.async_accept(
		[this](boost::system::error_code Error, tcp::socket ClientSocket) {
			if (!Error) {
				if (m_nActiveConnections >= Config::Limits.nMaxConnections) {
					LOG_WARN("Max connections reached (%d), rejecting.",
						Config::Limits.nMaxConnections);
					ClientSocket.close();
				}
				else {
					m_nActiveConnections++;
					auto Session = std::make_shared<CSession>(
						std::move(ClientSocket), m_SSLContext, m_nActiveConnections);
					Session->Start();
				}
			}
			
			AcceptConnection();
		});
}
