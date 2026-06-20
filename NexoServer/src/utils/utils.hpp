#pragma once
#include <array>
#include <string>
#include <cstdint>

namespace Utils {
	bool ParseUUID(const std::string& strUUID, std::array<uint8_t, 16>& OutBytes);
}