#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include "enums/transport.hpp"

namespace Config {
	inline struct ServerConfig_t {
		uint16_t nServerPort = 443;
		std::string strCertFilePath = "server.cert";
		std::string strKeyFilePath = "server.key";
		ETransportType Transport = ETransportType::INVALID;
	} Server;

	inline struct FallbackConfig_t {
		bool bEnabled = false;
		std::string strHtmlFile = "fallback/index.html";
	} Fallback;

	inline struct LimitsConfig_t {
		int nMaxConnections = 100;
		int nTimeoutSeconds = 60;
	} Limits;

	inline struct WebsocketConfig_t {
		std::string strPath = "/enter/your/path/";
	} Websocket;

	// WIP
	inline struct LogConfig_t {
		std::string strLogLevel = "info";
		std::string strFilePath = "logs/latest.log";
	} Log;

	// Users
	struct UserConfig_t {
		std::string strUsername = "";
		std::string strUUID = "";
		bool bEnabled = false;
		std::array<uint8_t, 16> ByteUUID{};
	};

	inline std::vector<UserConfig_t> Users;
}