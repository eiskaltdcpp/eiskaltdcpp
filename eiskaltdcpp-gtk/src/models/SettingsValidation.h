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

#pragma once

#include <string>
#include <vector>

/**
 * Widget-independent settings validation functions.
 *
 * Extracts the validation logic scattered throughout settingsdialog.cc
 * into pure, testable functions.  No GTK dependency.
 */
namespace gtk_settings_validate {

/// Result of a validation operation.
struct ValidationResult {
    bool ok = true;
    std::string error;

    static ValidationResult success() { return {true, ""}; }
    static ValidationResult fail(const std::string &msg) { return {false, msg}; }
};

// ── Port validation ──

/// A valid port is in the range [1, 65535].
bool isValidPort(int port);

/// Adjust TLS port if it conflicts with TCP port (must differ).
/// Returns the adjusted port.
int adjustTlsPort(int tlsPort, int tcpPort);

/// Adjust DHT port if it conflicts with UDP port (must differ).
/// Returns the adjusted port.
int adjustDhtPort(int dhtPort, int udpPort);

// ── Path validation ──

/// Ensure path has a trailing path separator.
/// Uses '/' on Linux, '\\' on Windows (compile-time).
std::string ensureTrailingPathSep(const std::string &path);

/// Validate a download directory: must be non-empty.
ValidationResult validateDownloadDir(const std::string &path);

// ── Nick validation ──

/// A valid nick is non-empty and does not contain leading/trailing spaces.
ValidationResult validateNick(const std::string &nick);

// ── IP validation ──

/// Basic IPv4 validation: four decimal octets separated by dots, each 0-255.
bool isValidIpv4(const std::string &ip);

// ── Search type validation ──

/// A valid search type name is non-empty.
bool isSearchTypeNameValid(const std::string &name);

/// A valid extension list is non-empty.
bool isExtensionListValid(const std::vector<std::string> &exts);

// ── Composite connection validation ──

/// Validate connection settings: ports must be valid, and if active mode,
/// an external IP should be set.
ValidationResult validateConnectionSettings(int tcpPort, int udpPort,
                                            int tlsPort,
                                            const std::string &externalIp,
                                            bool isActive);

} // namespace gtk_settings_validate
