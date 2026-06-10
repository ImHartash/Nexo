#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "headers/socks5.hpp"

namespace net = boost::asio;
namespace ssl = net::ssl;
using net::ip::tcp;
using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;

class CSocks5Session : public std::enable_shared_from_this<CSocks5Session> {
public:
	CSocks5Session(tcp::socket ClientSocket, ssl::context& SSLContext);
	~CSocks5Session();
	void Start();

private:
	awaitable<void> HandleSession();

	awaitable<void> ReadSocksAuth();
	awaitable<void> ReadSocksRequest();
	awaitable<void> ConnectToUpstream();

	awaitable<void> RelayClientToUpstream();
	awaitable<void> RelayUpstreamToClient();

	void CloseSockets();

	//ssl::context& m_SSLContext;
	tcp::socket m_ClientSocket;
	ssl::stream<tcp::socket> m_UpstreamSocket;
	
	std::string m_strHostName;
	uint16_t m_nHostPort;
};