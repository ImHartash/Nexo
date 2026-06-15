#pragma once
#include <string>

class CConfigManager {
public:
	CConfigManager(const std::string& strConfigPath);

	bool IsFileExists();
	bool LoadFromFile();

	void CreateDefault();

private:
	std::string m_strConfigPath;
};