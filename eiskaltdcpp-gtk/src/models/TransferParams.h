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
 */

#pragma once

#include <cstdint>
#include <map>
#include <string>

/**
 * Transfer parameter helpers — extracted from Transfers::getParams_client().
 * These are pure functions that assemble StringMap entries from primitive values.
 * No GTK dependency — only standard C++.
 *
 * The original getParams_client() reads from dcpp Transfer/ConnectionQueueItem
 * objects and calls WulforUtil::getNicks/getHubNames. We extract the pure
 * computation here — callers resolve nicks/hubNames before calling.
 *
 * CQI params keys: "CID", "User", "Hub Name", "Failed", "Hub URL"
 *
 * Transfer params keys: "CID", "Filename", "User", "Hub Name", "Path",
 *   "Size", "Download Position", "Speed", "Progress Hidden", "IP",
 *   "Time Left", "Target", "Hub URL", "Encryption"
 */
namespace gtk_transfer {

/// Build a StringMap for a ConnectionQueueItem.
/// @param cid      Base32 CID of the user
/// @param nick     Resolved display nick
/// @param hubName  Resolved hub name
/// @param hubUrl   Hub hint URL
std::map<std::string, std::string> paramsFromCQI(
    const std::string &cid,
    const std::string &nick,
    const std::string &hubName,
    const std::string &hubUrl);

/// Transfer type constants (mirrors dcpp::Transfer::Type).
enum class TransferType { FILE, FULL_LIST, PARTIAL_LIST, TREE };

/// Build a StringMap for an active Transfer.
/// @param cid        Base32 CID
/// @param nick       Resolved display nick
/// @param hubName    Resolved hub name
/// @param path       Transfer path (target file)
/// @param type       Transfer type (affects "Filename" display)
/// @param size       Total transfer size in bytes
/// @param pos        Current position in bytes
/// @param avgSpeed   Average speed in bytes/sec
/// @param secondsLeft Estimated seconds remaining, -1 if unknown
/// @param remoteIp   Remote IP address
/// @param hubUrl     Hub URL
/// @param isSecure   Whether the connection is TLS-encrypted
/// @param cipherName TLS cipher name (only meaningful if isSecure)
std::map<std::string, std::string> paramsFromTransfer(
    const std::string &cid,
    const std::string &nick,
    const std::string &hubName,
    const std::string &path,
    TransferType type,
    int64_t size,
    int64_t pos,
    int64_t avgSpeed,
    int64_t secondsLeft,
    const std::string &remoteIp,
    const std::string &hubUrl,
    bool isSecure,
    const std::string &cipherName);

/// Build security/compression flag string for a download/upload tick.
/// @param secure  Connection is TLS
/// @param trusted TLS is trusted
/// @param tthCheck TTH check in progress (download only)
/// @param zTransfer zlib compressed transfer
std::string buildTransferFlags(bool secure, bool trusted,
                               bool tthCheck, bool zTransfer);

/// Compute the sort order prefix.
/// Downloads: "d", Uploads: "u", Completed/Failed: "w"
std::string sortOrderPrefix(bool isDownload, bool isCompleted);

/// Compute progress percentage as integer.
/// Returns 0 if size <= 0.
int progressPercent(int64_t pos, int64_t size);

} // namespace gtk_transfer
