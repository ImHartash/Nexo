#include "CServer.hpp"
#include "session/CSession.hpp"
#include "config/ServerConfiguration.hpp"
#include "logger/CLogger.hpp"
#include "transports/tls/CTlsTransport.hpp"
#include "transports/wss/CWebSocketTransport.hpp"

CServer::CServer(boost::asio::io_context& IOContext, uint16_t nPort) 
	: m_Acceptor(IOContext, tcp::endpoint(tcp::v4(), nPort)), m_SSLContext(ssl::context::tls_server) 
{
	m_SSLContext.use_certificate_chain_file(Config::Server.strCertFilePath);
	m_SSLContext.use_private_key_file(Config::Server.strKeyFilePath, ssl::context::pem);

	/*if (Config::Fallback.bEnabled) {
		std::ifstream fin(Config::Fallback.strHtmlFile);
		if (fin.is_open()) {
			m_strFallbackHTML = std::string(
				std::istreambuf_iterator<char>(fin),
				std::istreambuf_iterator<char>()
			);
			LOG_INFO("Fallback HTML loaded (%zu bytes)", m_strFallbackHTML.size());
		}
		else {
			LOG_INFO("Failed to load fallback HTML file: %s",
				Config::Fallback.strHtmlFile.c_str());
			m_strFallbackHTML = "<html><body><h1>Welcome</h1></body></html>";
		}
	}*/
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
					std::unique_ptr<ITransport> Transport = PrepareTransport(
						Config::Server.Transport, std::move(ClientSocket));

					if (!Transport)
						return;

					m_nActiveConnections++;
					auto Session = std::make_shared<CSession>(
						std::move(Transport), m_Acceptor.get_executor(), m_nActiveConnections);

					Session->Start();
				}
			}
			
			AcceptConnection();
		});
}

std::unique_ptr<ITransport> CServer::PrepareTransport(ETransportType TransportType, tcp::socket Socket) {
	if (TransportType == ETransportType::TYPE_TLS) {
		return std::make_unique<CTlsTransport>(std::move(Socket), m_SSLContext, ETransportSide::SIDE_SERVER);
	}
	else if (TransportType == ETransportType::TYPE_WEBSOCKET) {
		return std::make_unique<CWebSocketTransport>(std::move(Socket), m_SSLContext, 
			Config::Websocket.strPath, "localhost", ETransportSide::SIDE_SERVER);
	}
	else {
		LOG_INFO("Failed to prepare transport: invalid transport type, please, fix config and try again.");
		return nullptr;
	}
}
