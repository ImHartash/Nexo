#pragma once
#include <cstdint>
#include <string>
#include <array>

namespace Config {
	inline struct LocalConfig_t {
		std::string strLocalHost = "127.0.0.1";
		uint16_t nPort = 6578;
	} Local;

	inline struct ServerConfig_t {
		std::string strServerHost = "example.com";
		std::string strTransport = "tls";
		uint16_t nPort = 443;
		std::array<uint8_t, 16> UUID{};
	} Server;

	inline struct TLSConfig_t {
		bool bVerifyCert = false;
		std::string strServerNameIndicator = "example.com";
	} TLS;

	inline struct WebSocketConfig_t {
		std::string strWebsocketPath = "/enter/your/path/";
	} WebSocket;

	// WIP
	inline struct LogConfig_t {
		std::string strLogLevel = "info";
		std::string strFilePath = "logs/nexo_client.log";
	} Log;

	inline struct ConnectionConfig_t {
		int nTimeoutSeconds = 10;
		int nRetryAttempts = 3;
		int nRetryDelayMS = 1000;
	} Connection;
}