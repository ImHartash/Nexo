#include "CConfigManager.hpp"
#include <filesystem>
#include <toml++/toml.hpp>
#include "ClientConfiguration.hpp"
#include "logger/CLogger.hpp"
#include "utils/utils.hpp"

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
		if (auto local = TomlTable["local"]) {
			if (auto host = local["host"].value<std::string>()) {
				Config::Local.strLocalHost = *host;
			}
			if (auto port = local["port"].value<uint16_t>()) {
				Config::Local.nPort = *port;
			}
		}

		if (auto server = TomlTable["server"]) {
			if (auto server_host = server["server_host"].value<std::string>()) {
				Config::Server.strServerHost = *server_host;
			}
			if (auto server_port = server["server_port"].value<uint16_t>()) {
				Config::Server.nPort = *server_port;
			}
			if (auto uuid = server["uuid"].value<std::string>()) {
				if (!Utils::ParseUUID(*uuid, Config::Server.UUID)) {
					Config::Server.UUID = std::array<uint8_t, 16>();
				}
			}
		}

		if (auto tls = TomlTable["tls"]) {
			if (auto verify_cert = tls["verify_cert"].value<bool>()) {
				Config::TLS.bVerifyCert = *verify_cert;
			}
			if (auto tls_sni = tls["tls_sni"].value<bool>()) {
				Config::TLS.strServerNameIndicator = *tls_sni;
			}
		}

		if (auto connection = TomlTable["connection"]) {
			if (auto timeout_seconds = connection["timeout_seconds"].value<int>()) {
				Config::Connection.nTimeoutSeconds = *timeout_seconds;
			}
			if (auto retry_attempts = connection["retry_attempts"].value<int>()) {
				Config::Connection.nRetryAttempts = *retry_attempts;
			}
			if (auto retry_delay_ms = connection["retry_delay_ms"].value<int>()) {
				Config::Connection.nRetryDelayMS = *retry_delay_ms;
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

		LOG_INFO("Successfully loaded config from file.");
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

	fout << R"(# Nexo Client Configuration
[local]
host = "127.0.0.1"
port = 6578

[server]
server_host = "your-domain-or-ip.com"
server_port = 443
uuid = "550e8400-e29b-41d4-a716-446655440000"

[tls]
verify_cert = false
tls_sni = "example.com"

[connection]
timeout_seconds = 10
retry_attempts = 3
retry_delay_ms = 1000

[log]
level = "info"
file = "logs/nexo_client.log"
)";

	LOG_INFO("Created default config: %s", m_strConfigPath.c_str());
}
