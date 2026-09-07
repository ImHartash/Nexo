#include "CWebSocketTransport.hpp"
#include "logger/CLogger.hpp"

CWebSocketTransport::CWebSocketTransport(
	net::ip::tcp::socket ClientSocket,
	net::ssl::context& SslContext,
	const std::string& strPath, const std::string& strHost,
	ETransportSide TransportSide)
	: m_WssSocket(std::move(ClientSocket), SslContext),
	m_strWssPath(strPath), m_strWssHost(strHost), m_TransportSide(TransportSide) {}

awaitable<EHandshakeResult> CWebSocketTransport::Handshake() {
	boost::system::error_code Error;

	co_await m_WssSocket.next_layer().async_handshake(
		(m_TransportSide == ETransportSide::SIDE_SERVER)
		? net::ssl::stream_base::server : net::ssl::stream_base::client,
		net::redirect_error(net::use_awaitable, Error)
	);

	if (Error) co_return EHandshakeResult::HR_ERROR;

	if (m_TransportSide == ETransportSide::SIDE_SERVER) {
		beast::flat_buffer Buffer;
		http::request<http::string_body> Request;

		co_await beast::http::async_read(
			m_WssSocket.next_layer(), Buffer, Request,
			net::redirect_error(net::use_awaitable, Error));

		if (Error) co_return EHandshakeResult::HR_ERROR;

		/*if (Request.target() != m_strWssPath) {
			std::string strResponse = CFallbackManager::BuildResponse(std::string(Request.target()));

			co_await net::async_write(m_WssSocket.next_layer(),
				net::buffer(strResponse), net::redirect_error(net::use_awaitable, Error));
			co_return EHandshakeResult::HR_FALLBACK;
		}*/

		co_await m_WssSocket.async_accept(Request,
			net::redirect_error(net::use_awaitable, Error));
	}
	else if (m_TransportSide == ETransportSide::SIDE_CLIENT) {
		co_await m_WssSocket.async_handshake(m_strWssHost, m_strWssPath,
			net::redirect_error(net::use_awaitable, Error));
	}
	else { co_return EHandshakeResult::HR_ERROR; }

	if (Error) co_return EHandshakeResult::HR_ERROR;

	m_WssSocket.binary(true);
	co_return EHandshakeResult::HR_SUCCESS;
}

awaitable<size_t> CWebSocketTransport::ReadExact(net::mutable_buffer Buffer) {
	size_t nTotalRead = 0;

	while (nTotalRead < Buffer.size()) {
		nTotalRead += 
			co_await ReadSome(Buffer + nTotalRead);
	}

	co_return nTotalRead;
}

awaitable<size_t> CWebSocketTransport::ReadSome(net::mutable_buffer Buffer) {
	boost::system::error_code Error;

	size_t nBytesRead = co_await m_WssSocket.async_read_some(
		Buffer, 
		net::redirect_error(net::use_awaitable, Error));

	if (Error) throw boost::system::system_error(Error);
	co_return nBytesRead;
}

awaitable<size_t> CWebSocketTransport::Write(net::const_buffer Buffer) {
	boost::system::error_code Error;

	size_t nBytesWrote = co_await m_WssSocket.async_write(
		Buffer,
		net::redirect_error(net::use_awaitable, Error));

	if (Error) throw boost::system::system_error(Error);
	co_return nBytesWrote;
}

void CWebSocketTransport::Close() {
	boost::system::error_code Error;
	
	m_WssSocket.next_layer().lowest_layer().shutdown(
		net::ip::tcp::socket::shutdown_both, Error);
	m_WssSocket.next_layer().lowest_layer().close(Error);
}
