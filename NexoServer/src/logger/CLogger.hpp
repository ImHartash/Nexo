#pragma once
#include <string>
#include <cstdio>

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

// 23:29:40 INFO - Message
class CLogger {
public:
	CLogger();
	~CLogger();

	bool Initialize();
	std::string GetTimeStamp();

	void Log(const char* szColor, const char* szLogLevel, const char* szFormat, ...);

private:
	bool m_bInitialized;
};

extern CLogger g_Logger;

#ifdef _DEBUG
#define LOG_INFO(fmt, ...)  g_Logger.Log(COLOR_LIGHT_CYAN, "INFO", fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...)  g_Logger.Log(COLOR_LIGHT_YELLOW, "WARN", fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) g_Logger.Log(COLOR_LIGHT_RED, "ERROR", fmt, __VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)  ((void)0)
#define LOG_WARN(fmt, ...)  ((void)0)
#define LOG_ERROR(fmt, ...) ((void)0)
#endif