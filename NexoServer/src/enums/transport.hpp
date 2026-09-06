#pragma once
#include <cstdint>

enum class ETransportType : int32_t {
	TYPE_INVALID = -1,
	TYPE_TLS,
	TYPE_WEBSOCKET
};

enum class ETransportSide : int8_t {
	SIDE_INVALID = -1,
	SIDE_CLIENT,
	SIDE_SERVER
};

enum class EHandshakeResult : int8_t {
	HR_SUCCESS,
	HR_ERROR,
	HR_FALLBACK
};