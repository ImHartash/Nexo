#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct Socks5Auth_t {
	uint8_t nVersion;
	uint8_t nMethods;
	// after this going Methods, but its size is dynamic
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Socks5Request_t {
	uint8_t nVersion;
	uint8_t nCommand;
	uint8_t nReserve;
	uint8_t nATYP;
};
#pragma pack(pop)