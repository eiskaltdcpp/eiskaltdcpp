/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "dcpp/stdinc.h"
#include "SettingsValidation.h"

#include <cctype>
#include <cstdlib>
#include <sstream>

namespace gtk_settings_validate {

// ── Port helpers ─────────────────────────────────────────────────────────────

bool isValidPort(int port) {
    return port > 0 && port <= 65535;
}

int adjustTlsPort(int tlsPort, int tcpPort) {
    return (tlsPort == tcpPort) ? tlsPort + 1 : tlsPort;
}

int adjustDhtPort(int dhtPort, int udpPort) {
    return (dhtPort == udpPort) ? dhtPort + 1 : dhtPort;
}

// ── Path helpers ─────────────────────────────────────────────────────────────

std::string ensureTrailingPathSep(const std::string &path) {
    if (path.empty()) return path;
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    if (path.back() != sep && path.back() != '/')
        return path + sep;
    return path;
}

ValidationResult validateDownloadDir(const std::string &path) {
    if (path.empty())
        return ValidationResult::fail("Download directory must not be empty");
    return ValidationResult::success();
}

// ── Nick helpers ─────────────────────────────────────────────────────────────

ValidationResult validateNick(const std::string &nick) {
    if (nick.empty())
        return ValidationResult::fail("Nick must not be empty");
    if (nick.front() == ' ' || nick.back() == ' ')
        return ValidationResult::fail("Nick must not have leading or trailing spaces");
    return ValidationResult::success();
}

// ── IP helpers ───────────────────────────────────────────────────────────────

bool isValidIpv4(const std::string &ip) {
    if (ip.empty()) return false;
    // Reject trailing dot
    if (ip.back() == '.') return false;

    int octets = 0;
    std::istringstream iss(ip);
    std::string part;
    while (std::getline(iss, part, '.')) {
        if (part.empty() || part.size() > 3) return false;
        for (char c : part)
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        int val = std::atoi(part.c_str());
        if (val < 0 || val > 255) return false;
        ++octets;
    }
    return octets == 4;
}

// ── Search type helpers ──────────────────────────────────────────────────────

bool isSearchTypeNameValid(const std::string &name) {
    return !name.empty();
}

bool isExtensionListValid(const std::vector<std::string> &exts) {
    return !exts.empty();
}

// ── Composite validation ─────────────────────────────────────────────────────

ValidationResult validateConnectionSettings(int tcpPort, int udpPort,
                                            int tlsPort,
                                            const std::string &externalIp,
                                            bool isActive) {
    if (!isValidPort(tcpPort))
        return ValidationResult::fail("TCP port is invalid (must be 1-65535)");
    if (!isValidPort(udpPort))
        return ValidationResult::fail("UDP port is invalid (must be 1-65535)");
    if (!isValidPort(tlsPort))
        return ValidationResult::fail("TLS port is invalid (must be 1-65535)");
    if (isActive && externalIp.empty())
        return ValidationResult::fail("External IP is required in active mode");
    if (isActive && !externalIp.empty() && !isValidIpv4(externalIp))
        return ValidationResult::fail("External IP is not a valid IPv4 address");
    return ValidationResult::success();
}

} // namespace gtk_settings_validate
