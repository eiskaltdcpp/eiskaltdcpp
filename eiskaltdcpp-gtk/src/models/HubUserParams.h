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

#include <string>
#include <map>

namespace dcpp {
class Identity;
}

/**
 * Hub user parameter extraction — extracted from Hub::getParams_client().
 * Transforms an Identity into a ParamMap (StringMap) for display.
 * No GTK dependency — only dcpp types.
 *
 * StringMap keys produced:
 *   "Icon"        — icon name (e.g. "dc++-fw-op", "normal")
 *   "Nick Order"  — sort key: "O" + nick for operators, "U" + nick for users
 *   "Nick"        — display nick
 *   "Shared"      — bytes shared as string
 *   "Description" — user description
 *   "Tag"         — client tag
 *   "Connection"  — connection type string
 *   "IP"          — IP address
 *   "eMail"       — email address
 *   "CID"         — base32 CID
 */
namespace gtk_hub {

/// Determine the icon name for a hub user.
/// Logic: base is "dc++" if user supports the DC++ feature set (ADCS+SEGA+...),
/// else "normal". Suffix "-fw" if passive, "-op" if operator.
///
/// @param supportsDCPP  true if user supports the DC++ feature set
/// @param isPassive     true if user has the PASSIVE flag
/// @param isOp          true if user is an operator
std::string iconForUser(bool supportsDCPP, bool isPassive, bool isOp);

/// Produce a sort-order key for the nick column.
/// Operators sort first ("O" + nick), others get "U" + nick.
std::string nickOrderKey(const std::string &nick, bool isOp);

/// Full parameter extraction from an Identity.
/// Produces the same StringMap keys as Hub::getParams_client().
std::map<std::string, std::string> paramsFromIdentity(const dcpp::Identity &id);

} // namespace gtk_hub
