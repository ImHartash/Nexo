#include "CSocks5Session.hpp"
#include <boost/asio/experimental/awaitable_operators.hpp>
#include "logger/CLogger.hpp"
#include "headers/nexo.hpp"

using namespace boost::asio::experimental::awaitable_operators;

CSocks5Session::CSocks5Session(tcp::socket ClientSocket) 
	: m_ClientSocket(std::move(ClientSocket)), m_UpstreamSocket(m_ClientSocket.get_executor()),
	m_strHostName(), m_nHostPort(0) {
	LOG_INFO("Session opened from %s", m_ClientSocket.remote_endpoint().address().to_string().c_str());
}

CSocks5Session::~CSocks5Session() {
	LOG_INFO("Session closed.");
}

void CSocks5Session::Start() {
	auto Self = shared_from_this();
	co_spawn(m_ClientSocket.get_executor(),
		[this, Self]() -> awaitable<void> {
			co_await HandleSession();
		}, detached);
}

awaitable<void> CSocks5Session::HandleSession() {
	try {
		co_await ReadSocksAuth();
		co_await ReadSocksRequest();
		co_await ConnectToUpstream();
	}
	catch (std::exception& e) {
		LOG_WARN("Session error: %s.", e.what());
	}
}

awaitable<void> CSocks5Session::ReadSocksAuth() {
	Socks5Auth_t AuthHeader;
	co_await net::async_read(m_ClientSocket, net::buffer(&AuthHeader, sizeof(Socks5Auth_t)), use_awaitable);

	if (AuthHeader.nVersion != 0x05) throw std::runtime_error("invalid socks version");
	if (AuthHeader.nMethods == 0x00) throw std::runtime_error("no auth methods offered");

	std::vector<uint8_t> vecMethods(AuthHeader.nMethods);
	co_await net::async_read(m_ClientSocket, net::buffer(vecMethods), use_awaitable);
	if (std::find(vecMethods.begin(), vecMethods.end(), 0x00) == vecMethods.end()) {
		uint8_t nResponse[2] = { 0x05, 0xFF };
		co_await net::async_write(m_ClientSocket, net::buffer(nResponse), use_awaitable);
		throw std::runtime_error("required method not found");
	}

	uint8_t nResponse[2] = { 0x05, 0x00 };
	co_await net::async_write(m_ClientSocket, net::buffer(nResponse), use_awaitable);
}

awaitable<void> CSocks5Session::ReadSocksRequest() {
	Socks5Request_t Request;
	co_await net::async_read(m_ClientSocket, net::buffer(&Request, sizeof(Socks5Request_t)), use_awaitable);
	if (Request.nVersion != 0x05) throw std::runtime_error("invalid socks version");
	if (Request.nCommand != 0x01) throw std::runtime_error("usupported socks command");

	if (Request.nATYP == 0x01) {
		std::vector<uint8_t> vecAddressBytes(4);
		co_await net::async_read(m_ClientSocket, net::buffer(vecAddressBytes), use_awaitable);
		m_strHostName = std::to_string(vecAddressBytes[0]) + "." +
			std::to_string(vecAddressBytes[1]) + "." +
			std::to_string(vecAddressBytes[2]) + "." +
			std::to_string(vecAddressBytes[3]);
	}
	else if (Request.nATYP == 0x03) {
		uint8_t nDomainLenght;
		co_await net::async_read(m_ClientSocket, net::buffer(&nDomainLenght, 1), use_awaitable);
		if (nDomainLenght <= 0) throw std::runtime_error("failed to read lenght of domain");

		std::vector<char> vecDomainBuffer(nDomainLenght);
		co_await net::async_read(m_ClientSocket, net::buffer(vecDomainBuffer), use_awaitable);
		m_strHostName = std::string(vecDomainBuffer.begin(), vecDomainBuffer.end());
	}
	else if (Request.nATYP == 0x04) {
		// not implemented.
		throw std::runtime_error("address type not implemented");
	}
	else {
		throw std::runtime_error("invalid address type");
	}

	uint16_t nNetworkPort;
	co_await net::async_read(m_ClientSocket, net::buffer(&nNetworkPort, 2), use_awaitable);
	m_nHostPort = ntohs(nNetworkPort);
}

awaitable<void> CSocks5Session::ConnectToUpstream() {
	tcp::resolver Resolver(m_ClientSocket.get_executor());
	auto Endpoints = co_await Resolver.async_resolve("127.0.0.1", "7777", use_awaitable);
	co_await net::async_connect(m_UpstreamSocket, Endpoints, use_awaitable);
	LOG_INFO("Client connected to Nexo server 127.0.0.1:7777");

	NexoProtocolHeader_t Header;
	Header.nVersion = 0x01;
	Header.nCommand = 0x01;
	Header.nPort = htons(m_nHostPort);
	Header.nAddressSize = static_cast<uint8_t>(m_strHostName.size());

	co_await net::async_write(m_UpstreamSocket,
		net::buffer(&Header, sizeof(Header)), use_awaitable);
	co_await net::async_write(m_UpstreamSocket,
		net::buffer(m_strHostName), use_awaitable);

	uint8_t nResponse[10] = { 0 };
	nResponse[0] = 0x05;
	nResponse[1] = 0x00;
	nResponse[2] = 0x00;
	nResponse[3] = 0x01;

	co_await net::async_write(m_ClientSocket, net::buffer(nResponse), use_awaitable);

	LOG_INFO("Starting relay...");
	co_await (RelayClientToUpstream() || RelayUpstreamToClient());
	LOG_INFO("Relay stopped, closing session.");
}

awaitable<void> CSocks5Session::RelayClientToUpstream() {
	try {
		std::array<char, 8192> arrBuffer;
		for (;;) {
			uint64_t nBufferSize = co_await m_ClientSocket.async_read_some(
				net::buffer(arrBuffer), use_awaitable);
			co_await net::async_write(m_UpstreamSocket,
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
			LOG_WARN("Client2Proxy error: %s", e.what());
		}
	}

	CloseSockets();
}

awaitable<void> CSocks5Session::RelayUpstreamToClient() {
	try {
		std::array<char, 8192> arrBuffer;
		for (;;) {
			uint64_t nBufferSize = co_await m_UpstreamSocket.async_read_some(
				net::buffer(arrBuffer), use_awaitable);
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
			LOG_WARN("Proxy2Client error: %s", e.what());
		}
	}

	CloseSockets();
}

void CSocks5Session::CloseSockets() {
	boost::system::error_code Error;
	m_UpstreamSocket.close(Error);
	m_ClientSocket.close(Error);
}
