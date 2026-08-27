#include "CConfigManager.hpp"
#include "ServerConfiguration.hpp"
#include "logger/CLogger.hpp"
#include "utils/utils.hpp"
#include <filesystem>
#include <toml++/toml.hpp>
#include <boost/algorithm/string.hpp>

namespace fs = std::filesystem;

CConfigManager::CConfigManager(const std::string& strConfigPath) 
	: m_strConfigPath(strConfigPath) {}

bool CConfigManager::IsFileExists() {
	return fs::exists(m_strConfigPath);
}

bool CConfigManager::LoadFromFile() {
	try {
		toml::table TomlTable = toml::parse_file(m_strConfigPath);
		
		// Parsing
		if (auto server = TomlTable["server"]) {
			if (auto port = server["port"].value<uint16_t>()) {
				Config::Server.nServerPort = *port;
			}
			if (auto cert = server["cert_file"].value<std::string>()) {
				Config::Server.strCertFilePath = *cert;
			}
			if (auto key = server["key_file"].value<std::string>()) {
				Config::Server.strKeyFilePath = *key;
			}
			if (auto transport = server["transport"].value<std::string>()) {
				boost::algorithm::to_lower(*transport);
				if (*transport == "tls") { Config::Server.Transport = ETransportType::TLS; }
				else if (*transport == "wss") { Config::Server.Transport = ETransportType::WEBSOCKET; }
			}
		}

		if (auto fallback = TomlTable["fallback"]) {
			if (auto enabled = fallback["enabled"].value<bool>()) {
				Config::Fallback.bEnabled = *enabled;
			}
			if (auto html_file = fallback["html_fallback"].value<std::string>()) {
				Config::Fallback.strHtmlFile = *html_file;
			}
		}

		if (auto limits = TomlTable["limits"]) {
			if (auto max_connections = limits["max_connections"].value<int>()) {
				Config::Limits.nMaxConnections = *max_connections;
			}
			if (auto timeout_seconds = limits["timeout_seconds"].value<int>()) {
				Config::Limits.nTimeoutSeconds = *timeout_seconds;
			}
		}

		if (auto websocket = TomlTable["websocket"]) {
			if (auto path = websocket["path"].value<std::string>()) {
				Config::Websocket.strPath = *path;
			}
		}

		if (auto log = TomlTable["log"]) {
			if (auto level = log["level"].value<std::string>()) {
				Config::Log.strLogLevel = *level;
			}
			if (auto file = log["file"].value<std::string>()) {
				Config::Log.strFilePath = *file;
			}
		}

		Config::Users.clear();

		if (auto users = TomlTable["users"].as_array()) {
			for (auto& UserNode : *users) {
				auto UserTable = UserNode.as_table();
				if (!UserTable) continue;

				Config::UserConfig_t User;
				User.strUsername = UserTable->get("name")->value_or(std::string(""));
				User.strUUID = UserTable->get("uuid")->value_or(std::string(""));
				User.bEnabled = UserTable->get("enabled")->value_or(true);

				if (!User.strUUID.empty()) {
					if (!Utils::ParseUUID(User.strUUID, User.ByteUUID)) {
						LOG_WARN("Skipping user '%s': invalid UUID", User.strUsername.c_str());
						continue;
					}
				}

				Config::Users.push_back(User);
			}
		}

		LOG_INFO("Successfully loaded config from file. Loaded %zu users.", Config::Users.size());
		return true;
	}
	catch (std::exception& e) {
		LOG_ERROR("Failed to load config. Exception: %s", std::string(e.what()).c_str());
		return false;
	}
}

void CConfigManager::CreateDefault() {
	fs::path ConfigPath(m_strConfigPath);
	fs::path ParentDirectory = ConfigPath.parent_path();
	if (!ParentDirectory.empty() && !fs::exists(ParentDirectory)) {
		fs::create_directories(ParentDirectory);
	}

	std::ofstream fout(m_strConfigPath);
	if (!fout.is_open()) {
		LOG_ERROR("Failed to open config for writing.");
		return;
	}

	fout << R"(# Nexo Server Configuration
[server]
port = 443
cert_file = "certs/server.crt"
key_file  = "certs/server.key"
transport = "tls"

[fallback]
enabled = false
html_file = "fallback/index.html"

[limits]
max_connections = 100
timeout_seconds = 60

[log]
log_level = "info"
file = "logs/nexo.log"

# Add users below. Each [[users]] block is one user.
[[users]]
name    = "test_user"
uuid    = "7f3d9200-a14c-52e8-b923-115566770011"
enabled = true
)";

	LOG_INFO("Created default config: %s", m_strConfigPath.c_str());
}
