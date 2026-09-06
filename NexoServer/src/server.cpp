#include <iostream>
#include <locale>
#include "server/CServer.hpp"
#include "logger/CLogger.hpp"
#include "config/CConfigManager.hpp"
#include "config/ServerConfiguration.hpp"
#include "utils/fallback/CFallbackManager.hpp"

int main(int argc, char* argv[]) {
	try {
		std::locale::global(std::locale("C"));
		CConfigManager Config = { "config/server_configuration.toml" };

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
			std::cout << "Failed to initialize log tools.\n";
		}

		CFallbackManager::Initialize(Config::Fallback.bEnabled, Config::Fallback.strHtmlFile);

		boost::asio::io_context IOContext;
		CServer Server(IOContext, Config::Server.nServerPort);
		Server.Listen();
		LOG_INFO("Nexo server listening on port %u", Config::Server.nServerPort);
		IOContext.run();
	}
	catch (std::exception& e) {
		LOG_ERROR("Exception: %s", std::string(e.what()).c_str());
	}

	return 0;
}