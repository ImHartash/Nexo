#include "CSession.hpp"
#include <boost/asio/experimental/awaitable_operators.hpp>
#include "logger/CLogger.hpp"
#include "config/ServerConfiguration.hpp"
#include "transports/tls/CTlsTransport.hpp"
#include "transports/wss/CWebSocketTransport.hpp"
#include "utils/fallback/CFallbackManager.hpp"

using namespace boost::asio::experimental::awaitable_operators;

CSession::CSession(std::unique_ptr<ITransport> pTransport, net::any_io_executor Executor, 
	std::atomic<int>& nActiveConnections)
	: m_pTransport(std::move(pTransport)), m_Executor(Executor), m_TargetSocket(m_Executor),
	m_strTargetAddress(), m_nTargetPort(0), m_Header{}, m_bSocketsClosed(false), m_nActiveConnections(nActiveConnections),
	m_TimeoutTimer(m_Executor) { }

CSession::~CSession() {}

void CSession::Start() {
	auto Self = shared_from_this();
	co_spawn(m_Executor,
		[this, Self]() -> awaitable<void> {
			co_await HandleSession();
		}, detached);
}

void CSession::CloseSockets() {
	if (m_bSocketsClosed) return;
	this->m_bSocketsClosed = true;

	/*m_ClientSocket.async_shutdown([self = shared_from_this()](auto ec) {
		boost::system::error_code err;
		self->m_ClientSocket.lowest_layer().close(err);
		self->m_TargetSocket.close(err);
		self->m_nActiveConnections--;
		LOG_INFO("Session closed successfully.");
	});*/

	boost::system::error_code Error;

	m_pTransport->Close();
	m_TargetSocket.close(Error);
	m_nActiveConnections -= 1;

	LOG_INFO("Session closed successfully!");
}

awaitable<void> CSession::HandleSession() {
	struct SessionGuard_t {
		CSession* Self;
		~SessionGuard_t() { Self->CloseSockets(); }
	} Guard{ this };

	//try {
	//	// TLS Handshake
	//	co_await m_ClientSocket.async_handshake(ssl::stream_base::server, use_awaitable);
	//}
	//catch (boost::system::system_error& e) {
	//	if (e.code().value() == 167772316) {
	//		LOG_INFO("Plain HTTP received, expected HTTPS. Closing.");
	//		co_return;
	//	}

	//	LOG_WARN("TLS handshake failed: %s", e.what());
	//	co_return;
	//}

	EHandshakeResult HandshakeResult = co_await m_pTransport->Handshake();
	
	if (HandshakeResult == EHandshakeResult::HR_ERROR ||
		HandshakeResult == EHandshakeResult::HR_FALLBACK) {
		co_return;
	}

	try {
		// Reading first bytes to check it for UUID
		std::array<uint8_t, 16> UUIDBytes;
		co_await m_pTransport->ReadExact(net::buffer(UUIDBytes));
		
		if (!IsValidUUID(UUIDBytes.data())) {
			// TODO: Make Fallback Session pls (again) (ImHartash)
			// co_await this->HandleFallbackSession(UUIDBytes);
			co_return;
		}

		constexpr size_t REST_SIZE = sizeof(NexoProtocolHeader_t) - 0x10;

		co_await m_pTransport->ReadExact(net::buffer(reinterpret_cast<uint8_t*>(&m_Header) + 0x10, REST_SIZE));
		std::memcpy(m_Header.nUUID, UUIDBytes.data(), 16);

		if (m_Header.nVersion != 0x01 || m_Header.nCommand != 0x01) {
			LOG_WARN("Invalid Nexo header (ver=%d, cmd=%d)", m_Header.nVersion, m_Header.nCommand);
			co_return;
		}

		// Getting address
		std::vector<char> vecAddressBuffer(m_Header.nAddressSize);
		co_await m_pTransport->ReadExact(net::buffer(vecAddressBuffer));

		m_strTargetAddress = std::string(vecAddressBuffer.begin(), vecAddressBuffer.end());
		m_nTargetPort = ntohs(m_Header.nPort);

		LOG_INFO("Connecting to %s:%d", m_strTargetAddress.c_str(), m_nTargetPort);

		// Connection to target server
		tcp::resolver Resolver(m_Executor);
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
	if (!Config::Fallback.bEnabled || Config::Server.Transport != ETransportType::TYPE_TLS) co_return;
	LOG_INFO("Fallback triggered - serving HTML page.");

	CTlsTransport* pTLS = dynamic_cast<CTlsTransport*>(m_pTransport.get());
	if (!pTLS) {
		LOG_WARN("Failed to fallback request: transport not supported");
		co_return;
	}

	net::streambuf RequestBuffer;
	std::ostream RequestStream(&RequestBuffer);
	RequestStream.write(
		reinterpret_cast<const char*>(UUIDBytes.data()), 16);
	
	co_await pTLS->ReadUntil(RequestBuffer, "\r\n\r\n");

	http::request<http::string_body> Request;
	http::request_parser<http::string_body> Parser;

	boost::system::error_code ParseError;
	Parser.put(RequestBuffer.data(), ParseError);

	std::string strResponse;
	if (ParseError) {
		strResponse = CFallbackManager::BuildResponse("/404");
	}
	else {
		strResponse = CFallbackManager::BuildResponse(Request.target());
	}

	/*co_await net::async_write(m_ClientSocket,
		net::buffer(strResponse), use_awaitable);*/
	co_await pTLS->Write(net::buffer(strResponse));

	LOG_INFO("Fallback response sent.");

	// c. ON REMADE (Paulus ImHartash)
}

awaitable<void> CSession::RelayClientToServer() {
	try {
		std::array<char, 8192> arrBuffer;
		for (;;) {
			/*uint64_t nBufferSize = co_await m_ClientSocket.async_read_some(
				net::buffer(arrBuffer), use_awaitable);*/
			size_t nBytesRead = co_await m_pTransport->ReadSome(net::buffer(arrBuffer));
			ResetTimer();
			co_await net::async_write(m_TargetSocket,
				net::buffer(arrBuffer.data(), nBytesRead), use_awaitable);
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
	}
}

awaitable<void> CSession::RelayServerToClient() {
	try {
		std::array<char, 8192> arrBuffer;
		for (;;) {
			uint64_t nBytesRead = co_await m_TargetSocket.async_read_some(
				net::buffer(arrBuffer), use_awaitable);
			ResetTimer();
			/*co_await net::async_write(m_ClientSocket,
				net::buffer(arrBuffer.data(), nBufferSize), use_awaitable);*/
			co_await m_pTransport->Write(net::buffer(arrBuffer.data(), nBytesRead));
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
	}
}

void CSession::ResetTimer() {
	m_TimeoutTimer.expires_after(std::chrono::seconds(Config::Limits.nTimeoutSeconds));
}

awaitable<void> CSession::WaitForTimeout() {
	for (;;) {
		m_TimeoutTimer.expires_after(
			std::chrono::seconds(Config::Limits.nTimeoutSeconds));

		boost::system::error_code Error;
		co_await m_TimeoutTimer.async_wait(
			net::redirect_error(use_awaitable, Error));

		if (!Error) {
			LOG_INFO("Session timed out.");
			co_return;
		}
	}
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
