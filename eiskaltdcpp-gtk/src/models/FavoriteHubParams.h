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

#include <map>
#include <string>

namespace dcpp {
class FavoriteHubEntry;
}

/**
 * Favorite hub parameter extraction — extracted from
 * FavoriteHubs::getFavHubParams_client().
 * No GTK dependency — only dcpp types and standard C++.
 *
 * StringMap keys produced:
 *   "Auto Connect", "Name", "Description", "Nick", "Password",
 *   "Address", "User Description", "Encoding", "Mode",
 *   "Search Interval", "Disable Chat", "External IP", "Internet IP"
 */
namespace gtk_favhub {

/// Extract display parameters from a FavoriteHubEntry.
std::map<std::string, std::string> paramsFromEntry(const dcpp::FavoriteHubEntry &entry);

/// Validate a hub entry for required fields.
/// Returns true if valid. On failure, errorMsg is set.
bool validateHubEntry(const std::string &name, const std::string &address,
                      std::string &errorMsg);

} // namespace gtk_favhub
