#pragma once
#include "../ITransport.hpp"
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ssl.hpp>
#include <string>

namespace beast = boost::beast;
namespace ws = beast::websocket;
namespace http = beast::http;

class CWebSocketTransport : public ITransport {
public:
	explicit CWebSocketTransport(
		net::ip::tcp::socket ClientSocket,
		net::ssl::context& SslContext, 
		const std::string& strPath, const std::string& strHost,
		ETransportSide TransportSide);

	// Override methods
	awaitable<EHandshakeResult> Handshake() override;

	awaitable<size_t> ReadExact(net::mutable_buffer Buffer) override;
	awaitable<size_t> ReadSome(net::mutable_buffer Buffer) override;

	awaitable<size_t> Write(net::const_buffer Buffer) override;
	void Close() override;

private:
	ws::stream<net::ssl::stream<net::ip::tcp::socket>> m_WssSocket;
	std::string m_strWssPath;
	std::string m_strWssHost;
	ETransportSide m_TransportSide;
};