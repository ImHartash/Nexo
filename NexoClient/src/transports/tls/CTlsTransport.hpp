#pragma once
#include "transports/ITransport.hpp"
#include "enums/transport.hpp"
#include <boost/asio/ssl.hpp>

class CTlsTransport : public ITransport {
public:
	explicit CTlsTransport(
		net::ip::tcp::socket ClientSocket, 
		net::ssl::context& SslContext, 
		ETransportSide TransportSide);

	// Overrides
	awaitable<EHandshakeResult> Handshake() override;

	awaitable<size_t> ReadExact(net::mutable_buffer Buffer) override;
	awaitable<size_t> ReadSome(net::mutable_buffer Buffer) override;

	awaitable<size_t> Write(net::const_buffer Buffer) override;
	void Close() override;

	// Class methods
	awaitable<size_t> ReadUntil(net::streambuf& Buffer, std::string strDelimiter);

private:
	net::ssl::stream<net::ip::tcp::socket> m_TlsSocket;
	ETransportSide m_TransportSide;
};