#pragma once
#include <cstdint>

enum class ETransportType : int32_t {
	INVALID = -1,
	TLS,
	WEBSOCKET
};