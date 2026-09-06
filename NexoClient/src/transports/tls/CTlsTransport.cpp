#include "CTlsTransport.hpp"

CTlsTransport::CTlsTransport(net::ip::tcp::socket ClientSocket,
	net::ssl::context& SslContext, ETransportSide TransportSide)
	: m_TlsSocket(std::move(ClientSocket), SslContext), m_TransportSide(TransportSide) { }

awaitable<EHandshakeResult> CTlsTransport::Handshake() {
	boost::system::error_code Error;

	co_await m_TlsSocket.async_handshake(
		(m_TransportSide == ETransportSide::SIDE_SERVER) 
		? net::ssl::stream_base::server : net::ssl::stream_base::client,
		net::redirect_error(net::use_awaitable, Error)
	);

	co_return Error ? EHandshakeResult::HR_ERROR : EHandshakeResult::HR_SUCCESS;
}

awaitable<size_t> CTlsTransport::ReadExact(net::mutable_buffer Buffer) {
	co_return co_await net::async_read(m_TlsSocket, Buffer, net::use_awaitable);
}

awaitable<size_t> CTlsTransport::ReadSome(net::mutable_buffer Buffer) {
	co_return co_await m_TlsSocket.async_read_some(Buffer, net::use_awaitable);
}

awaitable<size_t> CTlsTransport::Write(net::const_buffer Buffer) {
	co_return co_await net::async_write(m_TlsSocket, Buffer, net::use_awaitable);
}

void CTlsTransport::Close() {
	boost::system::error_code Error;
	m_TlsSocket.lowest_layer().shutdown(net::ip::tcp::socket::shutdown_both, Error);
	m_TlsSocket.lowest_layer().close(Error);
}

awaitable<size_t> CTlsTransport::ReadUntil(net::streambuf& Buffer, std::string strDelimiter) {
	boost::system::error_code Error;
	size_t nBytes = co_await net::async_read_until(
		m_TlsSocket, Buffer, strDelimiter, 
		net::redirect_error(net::use_awaitable, Error)
	);

	if (Error) throw boost::system::system_error(Error);
	else co_return nBytes;
}
