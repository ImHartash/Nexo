#include "CLogger.hpp"
#include <ctime>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#else
	#include <unistd.h>   // для isatty (опционально)
#endif

CLogger g_Logger;

CLogger::CLogger() : m_bInitialized(false) {}
CLogger::~CLogger() = default;

bool CLogger::Initialize() {
#ifndef _DEBUG
	return true;
#endif
#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;

	GetConsoleMode(hConsole, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hConsole, dwMode);

	m_bInitialized = true;

	SetConsoleTitleA("Nexo");
	return true;
#else
	return true;
#endif
}

std::string CLogger::GetTimeStamp() {
	time_t Now = time(nullptr);
	tm TimeInfo;
	localtime_s(&TimeInfo, &Now);

	char Buffer[80];
	strftime(Buffer, sizeof(Buffer), "%H:%M:%S", &TimeInfo);

	return std::string(Buffer);
}

void CLogger::Log(const char* szColor, const char* szLogLevel, const char* szFormat, ...) {
#ifdef _DEBUG
	std::string strTimeStamp = GetTimeStamp();
	printf("%s%s %s%s - ", strTimeStamp.c_str(), szColor, szLogLevel, COLOR_RESET);

	va_list args;
	va_start(args, szFormat);
	vprintf(szFormat, args);
	va_end(args);

	printf("%s\n", COLOR_RESET);
#else
	(void)szColor; (void)szLogLevel; (void)szFormat;
#endif
}
