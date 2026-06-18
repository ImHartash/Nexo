#pragma once
#include <string>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <cstdarg>

constexpr const char* COLOR_RESET = "\033[0m";
constexpr const char* COLOR_RED = "\033[31m";
constexpr const char* COLOR_GREEN = "\033[32m";
constexpr const char* COLOR_YELLOW = "\033[33m";
constexpr const char* COLOR_CYAN = "\033[36m";
constexpr const char* COLOR_LIGHT_RED = "\033[91m";
constexpr const char* COLOR_LIGHT_GREEN = "\033[92m";
constexpr const char* COLOR_LIGHT_YELLOW = "\033[93m";
constexpr const char* COLOR_LIGHT_CYAN = "\033[96m";
constexpr const char* COLOR_BOLD = "\033[1m";

enum class ELogLevel {
	Info = 0,
	Warn = 1,
	Error = 2,
};

// 23:29:40 INFO - Message
class CLogger {
public:
	CLogger();
	~CLogger();

	bool Initialize(const std::string& strFilePath, const std::string& strLevel);
	std::string GetTimeStamp();

	void Log(int nLogLevel, const char* szColor, const char* szLogLevel, const char* szFormat, ...);

private:
	bool m_bInitialized;
	std::ofstream m_FileStream;
	std::mutex m_Mutex;
	int m_nMinLogLevel;
};

extern CLogger g_Logger;

#define LOG_INFO(fmt, ...)  g_Logger.Log((int)ELogLevel::Info, COLOR_LIGHT_CYAN, "INFO", fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  g_Logger.Log((int)ELogLevel::Warn, COLOR_LIGHT_YELLOW, "WARN", fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) g_Logger.Log((int)ELogLevel::Error, COLOR_LIGHT_RED, "ERROR", fmt, ##__VA_ARGS__)