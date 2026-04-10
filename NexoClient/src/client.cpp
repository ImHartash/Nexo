#include <iostream>
#include <locale>
#include "logger/CLogger.hpp"
#include "servers/socks5server/CSocks5Server.hpp"

int main() {
	try {
		std::locale::global(std::locale("C"));
		if (!g_Logger.Initialize()) {
			std::cout << "Failed to initialize log tools." << std::endl;
		}

		boost::asio::io_context IOContext;
		CSocks5Server ProxyServer(IOContext, 6578);
		ProxyServer.Start();
		LOG_INFO("Local SOCKS5 proxy running on port 6578");
		LOG_INFO("Configure your browser to use HTTP proxy 127.0.0.1:6578");
		LOG_INFO("Or use `curl -v --socks5 127.0.0.1:6578 https://example.com`");
		IOContext.run();
	}
	catch (std::exception& e) {
		LOG_ERROR("Exception: %s", std::string(e.what()));
	}

	return 0;
}