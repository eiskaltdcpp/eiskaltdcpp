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
#include <vector>

namespace dcpp {
class FinishedFileItem;
class FinishedUserItem;
}

/**
 * Finished transfer parameter extraction — extracted from
 * FinishedTransfers::getFinishedParams_client().
 * No GTK dependency — only dcpp types and standard C++.
 *
 * For files — StringMap keys:
 *   "Filename", "Time", "Path", "Nicks", "Transferred",
 *   "Speed", "CRC Checked", "Target", "Elapsed Time", "full"
 *
 * For users — StringMap keys:
 *   "Time", "Nick", "Hub", "Files", "Transferred",
 *   "Speed", "CID", "Elapsed Time"
 */
namespace gtk_finished {

/// Extract display parameters from a FinishedFileItem.
/// @param item      The finished file item.
/// @param filePath  The file target path.
/// @param nicks     Pre-resolved comma-separated nicks of the users involved.
std::map<std::string, std::string> paramsFromFile(
    const dcpp::FinishedFileItem &item,
    const std::string &filePath,
    const std::string &nicks);

/// Extract display parameters from a FinishedUserItem.
/// @param item    The finished user item.
/// @param nick    Pre-resolved display nick.
/// @param hub     Pre-resolved hub name.
/// @param cid     CID base32 string.
std::map<std::string, std::string> paramsFromUser(
    const dcpp::FinishedUserItem &item,
    const std::string &nick,
    const std::string &hub,
    const std::string &cid);

/// Aggregate totals for a collection of finished items.
struct Totals {
    int files = 0;
    int64_t bytes = 0;
    int64_t avgSpeed = 0;
};

/// Compute totals from a list of (transferred, averageSpeed) pairs.
Totals computeTotals(const std::vector<std::pair<int64_t, int64_t>> &items);

} // namespace gtk_finished
