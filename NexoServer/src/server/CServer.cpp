#include "CServer.hpp"
#include "session/CSession.hpp"

CServer::CServer(boost::asio::io_context& IOContext, uint16_t nPort) : m_Acceptor(IOContext, tcp::endpoint(tcp::v4(), nPort)) {}

void CServer::Listen() {
	AcceptConnection();
}

void CServer::AcceptConnection() {
	m_Acceptor.async_accept(
		[this](boost::system::error_code Error, tcp::socket ClientSocket) {
			if (!Error) std::make_shared<CSession>(std::move(ClientSocket))->Start();
			AcceptConnection();
		});
}
