#include "CSession.hpp"
#include <boost/asio/experimental/awaitable_operators.hpp>
#include "logger/CLogger.hpp"
#include "config/ServerConfiguration.hpp"

using namespace boost::asio::experimental::awaitable_operators;

CSession::CSession(tcp::socket Socket, ssl::context& SSLContext, std::atomic<int>& nActiveConnections)
	: m_ClientSocket(std::move(Socket), SSLContext), m_TargetSocket(m_ClientSocket.get_executor()),
	m_strTargetAddress(), m_nTargetPort(0), m_Header(0), m_nActiveConnections(nActiveConnections),
	m_TimeoutTimer(m_ClientSocket.get_executor()) { }

CSession::~CSession() {
	boost::system::error_code Error;
	m_ClientSocket.shutdown(Error);
	m_ClientSocket.lowest_layer().close(Error);
	m_TargetSocket.close(Error);
	m_nActiveConnections--;

	LOG_INFO("Session closed successfully.");
}

void CSession::Start() {
	auto Self = shared_from_this();
	co_spawn(m_ClientSocket.get_executor(),
		[this, Self]() -> awaitable<void> {
			co_await HandleSession();
		}, detached);
}

awaitable<void> CSession::HandleSession() {
	try {
		// TLS Handshake
		co_await m_ClientSocket.async_handshake(ssl::stream_base::server, use_awaitable);

		// Reading and checking header
		co_await net::async_read(m_ClientSocket,
			net::buffer(&m_Header, sizeof(m_Header)), use_awaitable);
		
		if (!IsValidUUID(m_Header.nUUID)) {
			LOG_INFO("Unauthorized connection. Reseting...");
			co_return;
		}

		if (m_Header.nVersion != 0x01 || m_Header.nCommand != 0x01) {
			LOG_WARN("Invalid Nexo header (ver=%d, cmd=%d)", m_Header.nVersion, m_Header.nCommand);
			co_return;
		}

		// Getting address
		std::vector<char> vecAddressBuffer(m_Header.nAddressSize);
		co_await net::async_read(m_ClientSocket,
			net::buffer(vecAddressBuffer), use_awaitable);

		m_strTargetAddress = std::string(vecAddressBuffer.begin(), vecAddressBuffer.end());
		m_nTargetPort = ntohs(m_Header.nPort);

		LOG_INFO("Connecting to %s:%d", m_strTargetAddress.c_str(), m_nTargetPort);

		// Connection to target server
		tcp::resolver Resolver(m_ClientSocket.get_executor());
		auto Endpoints = co_await Resolver.async_resolve(
			m_strTargetAddress, std::to_string(m_nTargetPort), use_awaitable);

		co_await net::async_connect(m_TargetSocket, Endpoints, use_awaitable);
		LOG_INFO("Connected to target server %s:%d", m_strTargetAddress.c_str(), m_nTargetPort);

		// Starting translation
		co_await (RelayClientToServer() || RelayServerToClient() || WaitForTimeout());
		LOG_INFO("Relay finished, closing session...");
	}
	catch (std::exception& e) {
		LOG_WARN("Session error: %s", e.what());
	}
}

awaitable<void> CSession::RelayClientToServer() {
	try {
		std::array<char, 8192> arrBuffer;
		for (;;) {
			uint64_t nBufferSize = co_await m_ClientSocket.async_read_some(
				net::buffer(arrBuffer), use_awaitable);
			ResetTimer();
			co_await net::async_write(m_TargetSocket,
				net::buffer(arrBuffer.data(), nBufferSize), use_awaitable);
		}
	}
	catch (const boost::system::system_error& e) {
		if (e.code() != net::error::eof &&
			e.code() != net::error::connection_reset &&
			e.code() != net::error::connection_aborted &&
			e.code() != net::error::operation_aborted &&
			e.code().category() != boost::system::system_category() &&
			e.code().value() != 1236) {
			LOG_WARN("Client2Server relay error: %s", e.what());
		}
		/*CloseSockets();*/
	}
}

awaitable<void> CSession::RelayServerToClient() {
	try {
		std::array<char, 8192> arrBuffer;
		for (;;) {
			uint64_t nBufferSize = co_await m_TargetSocket.async_read_some(
				net::buffer(arrBuffer), use_awaitable);
			ResetTimer();
			co_await net::async_write(m_ClientSocket,
				net::buffer(arrBuffer.data(), nBufferSize), use_awaitable);
		}
	}
	catch (const boost::system::system_error& e) {
		if (e.code() != net::error::eof &&
			e.code() != net::error::connection_reset &&
			e.code() != net::error::connection_aborted &&
			e.code() != net::error::operation_aborted &&
			e.code().category() != boost::system::system_category() &&
			e.code().value() != 1236) {
			LOG_WARN("Server2Client relay error: %s", e.what());
		}
		/*CloseSockets();*/
	}
}

void CSession::CloseSockets() {
	boost::system::error_code Error;
	m_ClientSocket.shutdown(Error);
	m_ClientSocket.lowest_layer().close(Error);
	m_TargetSocket.close(Error);
	m_nActiveConnections--;

	LOG_INFO("Session closed successfully.");
}

void CSession::ResetTimer() {
	m_TimeoutTimer.expires_after(std::chrono::seconds(Config::Limits.nTimeoutSeconds));
}

awaitable<void> CSession::WaitForTimeout() {
	m_TimeoutTimer.expires_after(std::chrono::seconds(Config::Limits.nTimeoutSeconds));
	co_await m_TimeoutTimer.async_wait(use_awaitable);
	LOG_INFO("Session timed out.");
	/*CloseSockets();*/
}

bool CSession::IsValidUUID(const uint8_t* pReceivedUUID) {
	for (auto& User : Config::Users) {
		if (!User.bEnabled) continue;

		if (std::memcmp(pReceivedUUID, User.ByteUUID.data(), 16) == 0)
			return true;
	}

	return false;
}
