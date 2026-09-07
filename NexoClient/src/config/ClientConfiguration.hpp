#pragma once
#include <cstdint>
#include <string>
#include <array>
#include "enums/transport.hpp"

namespace Config {
	inline struct LocalConfig_t {
		std::string strLocalHost = "127.0.0.1";
		uint16_t nPort = 6578;
	} Local;

	inline struct ServerConfig_t {
		std::string strServerHost = "example.com";
		uint16_t nPort = 443;
		ETransportType Transport = ETransportType::TYPE_INVALID;
		std::array<uint8_t, 16> UUID{};
	} Server;

	inline struct TLSConfig_t {
		bool bVerifyCert = false;
		std::string strServerNameIndicator = "example.com";
	} TLS;

	inline struct WebSocketConfig_t {
		std::string strPath = "/enter/your/path/";
	} Websocket;

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