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

namespace dcpp {
class SearchResult;
}

/**
 * Search result parameter extraction — extracted from Search::parseSearchResult_gui()
 * and Search::getGroupingColumn().
 * No GTK dependency — only dcpp types and standard C++.
 *
 * StringMap keys produced by paramsFromResult():
 *   "Filename", "Path", "File Order", "Type", "Size", "Exact Size",
 *   "Icon", "Shared", "Nick", "CID", "Slots", "Connection", "Hub",
 *   "Hub URL", "IP", "Real Size", "TTH" (files only),
 *   "Slots Order", "Free Slots"
 */
namespace gtk_search {

/// Grouping types for search results (mirrors Search::GroupType enum).
enum class GroupType {
    NONE = 0,
    FILENAME,
    FILEPATH,
    SIZE,
    CONNECTION,
    TTH,
    NICK,
    HUB,
    TYPE
};

/// Get the display column name for a grouping type.
/// Returns empty string for NONE.
std::string groupingColumn(GroupType type);

/// Parse a SearchResult into a parameter map.
/// The userNick and hubName parameters should be pre-resolved
/// (e.g. via gtk_util::getNicks / getHubNames) since the SearchResult
/// may not carry the full nick when the user is offline.
///
/// @param result   The search result to extract params from.
/// @param nick     Pre-resolved nick string for the result's user.
/// @param conn     Pre-resolved connection string.
/// @param shared   Whether the file is shared locally ("1" or "0").
std::map<std::string, std::string> paramsFromResult(
    dcpp::SearchResult &result,
    const std::string &nick,
    const std::string &conn,
    bool isSharedLocally);

/// Build the "File Order" sort key from a filename and whether it's a directory.
/// Directories sort as "d" + name, files as "f" + name.
std::string fileOrderKey(const std::string &filename, bool isDirectory);

/// Build the "Slots Order" sort key from free slots and total slots.
/// Lower values sort first (more free slots).
std::string slotsOrderKey(int freeSlots, int totalSlots);

} // namespace gtk_search
