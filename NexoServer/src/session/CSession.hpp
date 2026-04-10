#pragma once
#include <memory>
#include <boost/asio.hpp>
#include "headers/nexo.hpp"

namespace net = boost::asio;
using net::ip::tcp;
using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;

class CSession : public std::enable_shared_from_this<CSession> {
public:
	CSession(tcp::socket Socket);
	void Start();

private:
	awaitable<void> HandleSession();

	awaitable<void> RelayClientToServer();
	awaitable<void> RelayServerToClient();

	void CloseSockets();

	tcp::socket m_ClientSocket;
	tcp::socket m_TargetSocket;
	NexoProtocolHeader_t m_Header;
	std::string m_strTargetAddress;
	uint16_t m_nTargetPort;
};