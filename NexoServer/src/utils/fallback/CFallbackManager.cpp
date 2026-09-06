#include "CFallbackManager.hpp"
#include "logger/CLogger.hpp"

bool CFallbackManager::m_bEnabled = false;
std::string CFallbackManager::m_strFallbackContent = "";

void CFallbackManager::Initialize(bool bEnabled, const std::string& strHtmlFallback) {
	m_bEnabled = bEnabled;
	
	std::ifstream fin(strHtmlFallback);
	if (fin.is_open()) {
		m_strFallbackContent = std::string(
			std::istreambuf_iterator<char>(fin),
			std::istreambuf_iterator<char>()
		);
		LOG_INFO("Fallback HTML loaded (%zu bytes)", m_strFallbackContent.size());
	}
	else {
		LOG_INFO("Failed to load fallback HTML file: %s",
			strHtmlFallback.c_str());
		m_strFallbackContent = "<html><body><h1>Welcome</h1></body></html>";
	}
}

std::string CFallbackManager::BuildResponse(const std::string& strTarget) {
	if (!m_bEnabled) 
		return "";

	if (strTarget == "/" || strTarget == "/index.html")
		return Build200();

	if (strTarget == "/404")
		return Build404();

	return Build404();
}

std::string CFallbackManager::Build200() {
	return "HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html; charset=utf-8\r\n"
		"Content-Length: " +
		std::to_string(m_strFallbackContent.size()) + "\r\n"
		"Connection: close\r\n"
		"Server: nginx/1.24.0\r\n"
		"\r\n" +
		m_strFallbackContent;
}

std::string CFallbackManager::Build404() {
	std::string strBody =
		"<html><head><title>404 Not Found</title></head>"
		"<body><center><h1>404 Not Found</h1></center>"
		"<hr><center>nginx/1.24.0</center></body></html>";

	return "HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html; charset=utf-8\r\n"
		"Content-Length: " + std::to_string(strBody.size()) + "\r\n"
		"Server: nginx/1.24.0\r\n"
		"Connection: close\r\n\r\n" + strBody;
}
