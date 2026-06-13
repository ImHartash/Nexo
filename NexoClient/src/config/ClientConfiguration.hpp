#pragma once
#include <cstdint>
#include <string>

namespace Config {
	inline struct LocalConfig_t {
		std::string strLocalHost = "127.0.0.1";
		uint16_t nPort = 6578;
	} Local;

	inline struct ServerConfig_t {
		std::string strServerHost = "example.com";
		uint16_t nPort = 443;
		std::string strUUID = "550e8400-e29b-41d4-a716-446655440000";
	} Server;

	inline struct TLSConfig_t {
		bool bVerifyCert = false;
		std::string strServerNameIndicator = "example.com";
	} TLS;

	// WIP
	inline struct LogConfig_t {
		std::string strLogLevel = "info";
		std::string strFilePath = "logs/nexo_client.log";
	} Log;

	inline struct ConnectionLog_t {
		int nTimeoutSeconds = 10;
		int nRetryAttempts = 3;
		int nRetryDelayMS = 1000;
	} Connection;
}