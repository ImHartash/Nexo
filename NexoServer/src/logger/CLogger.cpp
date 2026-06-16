#include "CLogger.hpp"
#include <ctime>
#include <filesystem>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#else
	#include <unistd.h>   // для isatty (опционально)
#endif

CLogger g_Logger;

CLogger::CLogger() : m_bInitialized(false), m_FileStream(), m_Mutex(), m_nMinLogLevel(0) {}
CLogger::~CLogger() = default;

bool CLogger::Initialize(const std::string& strFilePath, const std::string& strLevel) {
	if (strLevel == "info") m_nMinLogLevel = 0;
	else if (strLevel == "warn") m_nMinLogLevel = 1;
	else if (strLevel == "error") m_nMinLogLevel = 2;

	std::filesystem::create_directories(
		std::filesystem::path(strFilePath).parent_path()
	);

	m_FileStream.open(strFilePath, std::ios::app);

	m_bInitialized = true;

#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;

	GetConsoleMode(hConsole, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hConsole, dwMode);

	SetConsoleTitleA("Nexo");
	return true;
#else
	return true;
#endif
}

std::string CLogger::GetTimeStamp() {
	time_t Now = time(nullptr);
	tm TimeInfo;

#ifdef _WIN32
	localtime_s(&TimeInfo, &Now);
#else
	localtime_r(&Now, &TimeInfo);
#endif

	char Buffer[80];
	strftime(Buffer, sizeof(Buffer), "%H:%M:%S", &TimeInfo);

	return std::string(Buffer);
}

void CLogger::Log(int nLogLevel, const char* szColor, const char* szLogLevel, const char* szFormat, ...) {
	if (nLogLevel < m_nMinLogLevel) return;

	va_list args;
	va_start(args, szFormat);
	char szMessageBuffer[2048];
	vsnprintf(szMessageBuffer, sizeof(szMessageBuffer), szFormat, args);
	va_end(args);

	std::string strTimestamp = GetTimeStamp();

	std::lock_guard<std::mutex> Lock(m_Mutex);

	printf("%s%s %s%s - %s\n", strTimestamp.c_str(), szColor, szLogLevel, COLOR_RESET, szMessageBuffer);

	if (m_FileStream.is_open()) {
		m_FileStream << strTimestamp << " " << szLogLevel
			<< " - " << szMessageBuffer << "\n";
		m_FileStream.flush();
	}
}
