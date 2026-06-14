#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace Config {
	inline struct ServerConfig_t {
		uint16_t nServerPort = 443;
		std::string strCertFilePath = "server.cert";
		std::string strKeyFilePath = "server.key";
	} Server;

	inline struct FallbackConfig_t {
		bool bEnabled = false;
		std::string strHostName = "example.com";
		uint16_t nHostPort = 443;
	} Fallback;

	inline struct LimitsConfig_t {
		int nMaxConnections = 100;
		int nTimeoutSeconds = 60;
	} Limits;

	// WIP
	inline struct LogConfig_t {
		std::string strLogLevel = "info";
		std::string strFilePath = "logs/client.log";
	} Log;

	// Users
	struct UserConfig_t {
		std::string strUsename = "";
		std::string strUUID = "";
		bool bEnabled = false;
		std::array<uint8_t, 16> ByteUUID{};
	};

	inline std::vector<UserConfig_t> Users;
}