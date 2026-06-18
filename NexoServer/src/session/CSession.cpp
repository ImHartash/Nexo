#include "CSession.hpp"
#include <boost/asio/experimental/awaitable_operators.hpp>
#include "logger/CLogger.hpp"
#include "config/ServerConfiguration.hpp"

using namespace boost::asio::experimental::awaitable_operators;

CSession::CSession(tcp::socket Socket, ssl::context& SSLContext, std::atomic<int>& nActiveConnections, 
	const std::string& strFallbackHTML)
	: m_ClientSocket(std::move(Socket), SSLContext), m_TargetSocket(m_ClientSocket.get_executor()),
	m_strTargetAddress(), m_nTargetPort(0), m_Header{}, m_bSocketsClosed(false), m_nActiveConnections(nActiveConnections),
	m_TimeoutTimer(m_ClientSocket.get_executor()), m_strFallbackHTML(strFallbackHTML) { }

CSession::~CSession() {}

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
	}
	catch (boost::system::system_error& e) {
		if (e.code().value() == 167772316) {
			LOG_INFO("Plain HTTP received, expected HTTPS. Closing.");
			co_return;
		}

		LOG_WARN("TLS handshake failed: %s", e.what());
		co_return;
	}

	try {
		// Reading first bytes to check it for UUID
		std::array<uint8_t, 16> UUIDBytes;
		co_await net::async_read(m_ClientSocket,
			net::buffer(UUIDBytes), use_awaitable);
		
		if (!IsValidUUID(UUIDBytes.data())) {
			co_await this->HandleFallbackSession(UUIDBytes);
			co_return;
		}

		constexpr size_t REST_SIZE = sizeof(NexoProtocolHeader_t) - 0x10;

		co_await net::async_read(m_ClientSocket,
			net::buffer(reinterpret_cast<uint8_t*>(&m_Header) + 0x10, REST_SIZE), use_awaitable);

		std::memcpy(m_Header.nUUID, UUIDBytes.data(), 16);

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

		bool bConnected = co_await this->ConnectWithTimeout(m_TargetSocket, Endpoints, 30);
		if (!bConnected) {
			LOG_WARN("Failed to connect to %s:%d", m_strTargetAddress.c_str(), m_nTargetPort);
			co_return;
		}

		LOG_INFO("Connected to target server %s:%d", m_strTargetAddress.c_str(), m_nTargetPort);

		// Starting translation
		co_await (RelayClientToServer() || RelayServerToClient() || WaitForTimeout());
		LOG_INFO("Relay finished, closing session...");
	}
	catch (std::exception& e) {
		LOG_WARN("Session error: %s", e.what());
	}
}

awaitable<void> CSession::HandleFallbackSession(const std::array<uint8_t, 16>& UUIDBytes) {
	if (!Config::Fallback.bEnabled) co_return;
	LOG_INFO("Fallback triggered - serving HTML page.");

	net::streambuf RequestBuffer;
	std::ostream RequestStream(&RequestBuffer);
	RequestStream.write(
		reinterpret_cast<const char*>(UUIDBytes.data()), 16);

	boost::system::error_code Error;
	co_await net::async_read_until(m_ClientSocket, RequestBuffer, "\r\n\r\n",
		net::redirect_error(use_awaitable, Error));

	std::string strResponse =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html; charset=utf-8\r\n"
		"Content-Length: " +
		std::to_string(m_strFallbackHTML.size()) + "\r\n"
		"Connection: close\r\n"
		"Server: nginx/1.24.0\r\n"
		"\r\n" +
		m_strFallbackHTML;

	co_await net::async_write(m_ClientSocket,
		net::buffer(strResponse), use_awaitable);

	LOG_INFO("Fallback response sent.");
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
		CloseSockets();
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
		CloseSockets();
	}
}

void CSession::CloseSockets() {
	if (m_bSocketsClosed) return;

	m_ClientSocket.async_shutdown([self = shared_from_this()](auto ec) {
		boost::system::error_code err;
		self->m_ClientSocket.lowest_layer().close(err);
		self->m_TargetSocket.close(err);
		self->m_nActiveConnections--;
		self->m_bSocketsClosed = true;
		LOG_INFO("Session closed successfully.");
	});
}

void CSession::ResetTimer() {
	m_TimeoutTimer.expires_after(std::chrono::seconds(Config::Limits.nTimeoutSeconds));
}

awaitable<void> CSession::WaitForTimeout() {
	m_TimeoutTimer.expires_after(std::chrono::seconds(Config::Limits.nTimeoutSeconds));
	co_await m_TimeoutTimer.async_wait(use_awaitable);
	LOG_INFO("Session timed out.");
	CloseSockets();
}

awaitable<bool> CSession::ConnectWithTimeout(tcp::socket& Socket, const tcp::resolver::results_type& Endpoints, int nTimeoutSeconds) {
	net::steady_timer Timer(co_await net::this_coro::executor);
	Timer.expires_after(std::chrono::seconds(nTimeoutSeconds));

	try {
		auto Result = co_await(
			net::async_connect(Socket, Endpoints, use_awaitable) || Timer.async_wait(use_awaitable));

		if (Result.index() == 0) {
			Timer.cancel();
			co_return true;
		}
		else {
			LOG_WARN("Failed to connect to the server: timeout.");
			co_return false;
		}
	}
	catch (std::exception&) {
		co_return false;
	}
}

bool CSession::IsValidUUID(const uint8_t* pReceivedUUID) {
	for (auto& User : Config::Users) {
		if (!User.bEnabled) continue;

		if (std::memcmp(pReceivedUUID, User.ByteUUID.data(), 16) == 0)
			return true;
	}

	return false;
}
