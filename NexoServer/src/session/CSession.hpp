#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "headers/nexo.hpp"

namespace net = boost::asio;
namespace ssl = net::ssl;
using net::ip::tcp;
using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;

class CSession : public std::enable_shared_from_this<CSession> {
public:
	CSession(tcp::socket Socket, ssl::context& SSLContext, std::atomic<int>& nActiveConnections, const std::string& strFallbackHTML);
	~CSession();
	void Start();

	void CloseSockets();

private:
	awaitable<void> HandleSession();
	awaitable<void> HandleFallbackSession(const std::array<uint8_t, 16>& UUIDBytes);

	awaitable<void> RelayClientToServer();
	awaitable<void> RelayServerToClient();

	void ResetTimer();
	awaitable<void> WaitForTimeout();

	awaitable<bool> ConnectWithTimeout(tcp::socket& Socket, const tcp::resolver::results_type& Endpoints, int nTimeoutSeconds);

	// Utils for session
	bool IsValidUUID(const uint8_t* pReceivedUUID);

	ssl::stream<tcp::socket> m_ClientSocket;
	tcp::socket m_TargetSocket;
	NexoProtocolHeader_t m_Header;
	std::string m_strTargetAddress;
	uint16_t m_nTargetPort;

	bool m_bSocketsClosed;

	std::atomic<int>& m_nActiveConnections;
	net::steady_timer m_TimeoutTimer;

	const std::string& m_strFallbackHTML;
};