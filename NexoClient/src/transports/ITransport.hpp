#pragma once
#include "enums/transport.hpp"
#include <boost/asio.hpp>
#include <string>

namespace net = boost::asio;
using net::awaitable;

class ITransport {
public:
	virtual ~ITransport() = default;

	virtual awaitable<EHandshakeResult> Handshake() = 0;

	virtual awaitable<size_t> ReadExact(net::mutable_buffer Buffer) = 0;
	virtual awaitable<size_t> ReadSome(net::mutable_buffer Buffer) = 0;

	virtual awaitable<size_t> Write(net::const_buffer Buffer) = 0;
	virtual void Close() = 0;
};