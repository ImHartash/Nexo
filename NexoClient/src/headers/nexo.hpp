#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct NexoProtocolHeader_t {
	uint8_t nVersion;
	uint8_t nCommand;
	uint16_t nPort;
	uint8_t nAddressSize;
};
#pragma pack(pop)