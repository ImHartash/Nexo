#include "utils.hpp"
#include <algorithm>
#include "logger/CLogger.hpp"
#include "config/ServerConfiguration.hpp"

bool Utils::ParseUUID(const std::string& strUUID, std::array<uint8_t, 16>& OutBytes) {
    std::string strNormalizedUUID = strUUID;
    strNormalizedUUID.erase(
        std::remove(strNormalizedUUID.begin(), strNormalizedUUID.end(), '-'), strNormalizedUUID.end());

    if (strNormalizedUUID.size() != 32) {
        LOG_WARN("Failed to parse UUID: invalid length.");
        return false;
    }

    if (std::any_of(strNormalizedUUID.begin(), strNormalizedUUID.end(),
        [](char c) { return !std::isxdigit(static_cast<unsigned char>(c)); })) {
        LOG_WARN("Failed to parse UUID: invalid hex characters in UUID");
        return false;
    }

    for (int i = 0; i < 32; i += 2) {
        std::string strByte = strNormalizedUUID.substr(i, 2);
        OutBytes[i / 2] = static_cast<uint8_t>(std::stoi(strByte, nullptr, 16));
    }

    return true;
}

std::string Utils::GetFallbackResponse(std::string& strFallbackHTML) {
    return "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " +
        std::to_string(strFallbackHTML.size()) + "\r\n"
        "Connection: close\r\n"
        "Server: nginx/1.24.0\r\n"
        "\r\n" +
        strFallbackHTML;
}
