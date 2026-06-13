#include <iostream>
#include <locale>
#include "server/CServer.hpp"
#include "logger/CLogger.hpp"
#include "config/CConfigManager.hpp"
#include "config/ServerConfiguration.hpp"

int main(int argc, char* argv[]) {
	try {
		std::locale::global(std::locale("C"));
		CConfigManager Config = { "config/server_configuration.toml" };

		if (!g_Logger.Initialize()) {
			std::cout << "Failed to initialize log tools." << std::endl;
		}

		if (!Config.IsFileExists()) {
			Config.CreateDefault();
			LOG_INFO("Configuration file created. Please, restart Nexo to continue...");
			return 0;
		}

		if (!Config.LoadFromFile()) {
			LOG_ERROR("Failed to load configuration from file.");
			return 1;
		}

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