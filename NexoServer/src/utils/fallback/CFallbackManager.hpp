#pragma once
#include <string>

class CFallbackManager {
public:
	static void Initialize(bool bEnabled, const std::string& strHtmlFallback);
	static std::string BuildResponse(const std::string& strTarget);

private:
	static std::string Build200();
	static std::string Build404();

	static bool m_bEnabled;
	static std::string m_strFallbackContent;
};