#include <iostream>
#include <locale>
#include "server/CServer.hpp"
#include "logger/CLogger.hpp"

int main(int argc, char* argv[]) {
	try {
		std::locale::global(std::locale("C"));
		if (!g_Logger.Initialize()) {
			std::cout << "Failed to initialize log tools." << std::endl;
		}

		uint16_t nServerPort = 7777;
		if (argc >= 2) nServerPort = std::stoi(argv[1]);

		boost::asio::io_context IOContext;
		CServer Server(IOContext, nServerPort);
		Server.Listen();
		LOG_INFO("Nexo server listening on port %u", nServerPort);
		IOContext.run();
	}
	catch (std::exception& e) {
		LOG_ERROR("Exception: %s", std::string(e.what()));
	}

	return 0;
}