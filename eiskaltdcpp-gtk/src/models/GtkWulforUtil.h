/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
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

#include <cstdint>
#include <string>
#include <vector>

namespace dcpp { class DCContext; }

/**
 * Pure utility functions extracted from WulforUtil.
 * No GTK dependency — only standard C++ and dcpp.
 *
 * The original WulforUtil mixes widget-dependent helpers (GdkPixbuf scaling,
 * GtkTreeModel iteration, icon registration, etc.) with pure data functions.
 * This header contains only the pure functions so they can be tested and
 * reused without any GTK dependency.
 */
namespace gtk_util {

/// Split a delimited string into a vector of ints.
/// Example: splitString("1,2,3", ",") → {1, 2, 3}
std::vector<int> splitString(const std::string &str, const std::string &delimiter);

/// Convert Windows path separators to Linux: '\\' → '/'
std::string linuxSeparator(const std::string &ps);

/// Convert Linux path separators to Windows: '/' → '\\'
std::string windowsSeparator(const std::string &ps);

/// Build a magnet URI from file name, size, and TTH.
/// Returns empty string if name or tth is empty.
/// Strips directory path from name before encoding.
std::string makeMagnet(const std::string &name, int64_t size, const std::string &tth);

/// Parse a magnet URI into its components.
/// Returns true if parsing succeeded.
/// On failure, name is set to "Unknown", size to 0, tth to empty.
bool splitMagnet(const std::string &magnet, std::string &name, int64_t &size, std::string &tth);

/// Parse a magnet URI into a human-readable summary line.
/// Format: "filename (size)" or "filename (BitTorrent)" for BT magnets.
bool splitMagnet(const std::string &magnet, std::string &line);

/// Test if text starts with "magnet:?" (case-insensitive).
bool isMagnet(const std::string &text);

/// Test if text starts with a recognized link prefix
/// (http://, https://, www., ftp://, sftp://, irc://, ircs://, im:, mailto:, news:)
bool isLink(const std::string &text);

/// Test if text starts with a DC hub URL prefix
/// (dchub://, nmdcs://, adc://, adcs://)
bool isHubURL(const std::string &text);

// --- dcpp ClientManager lookups (no GTK dependency) ---

/// Get comma-separated nicks for a user identified by CID + hint URL.
/// Falls back to CID string when user is not connected.
std::string getNicks(dcpp::DCContext &ctx, const std::string &cid, const std::string &hintUrl = std::string());

/// Get comma-separated hub names for a user identified by CID + hint URL.
/// Returns "Offline" when user is not connected to any hub.
std::string getHubNames(dcpp::DCContext &ctx, const std::string &cid, const std::string &hintUrl = std::string());

/// Get the list of hub addresses for a user identified by CID + hint URL.
std::vector<std::string> getHubAddress(dcpp::DCContext &ctx, const std::string &cid, const std::string &hintUrl = std::string());

} // namespace gtk_util
