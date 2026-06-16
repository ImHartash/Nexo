#include <iostream>
#include <locale>
#include "logger/CLogger.hpp"
#include "config/CConfigManager.hpp"
#include "config/ClientConfiguration.hpp"
#include "servers/socks5server/CSocks5Server.hpp"

int main() {
	try {
		std::locale::global(std::locale("C"));
		CConfigManager Config = { "config/client_configuration.toml" };

		if (!Config.IsFileExists()) {
			Config.CreateDefault();
			std::cout << "Configuration file created. Please, restart Nexo to continue...\n";
			return 0;
		}

		if (!Config.LoadFromFile()) {
			std::cout << "Failed to load configuration from file.\n";
			return 1;
		}

		if (!g_Logger.Initialize(Config::Log.strFilePath, Config::Log.strLogLevel)) {
			std::cout << "Failed to initialize log tools." << std::endl;
		}

		boost::asio::io_context IOContext;
		CSocks5Server ProxyServer(IOContext, Config::Local.nPort);
		ProxyServer.Start();
		LOG_INFO("Local SOCKS5 proxy running on port %u", Config::Local.nPort);
		LOG_INFO("Configure your browser to use HTTP proxy 127.0.0.1:%u", Config::Local.nPort);
		LOG_INFO("Or use `curl -v --socks5 127.0.0.1:%u https://example.com`", Config::Local.nPort);
		IOContext.run();
	}
	catch (std::exception& e) {
		LOG_ERROR("Exception: %s", std::string(e.what()));
	}

	return 0;
}