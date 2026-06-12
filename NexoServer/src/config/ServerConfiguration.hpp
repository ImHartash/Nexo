#pragma once
#include <cstdint>
#include <string>

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
}