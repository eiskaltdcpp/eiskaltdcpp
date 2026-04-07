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

#include <dcpp/stdinc.h>
#include <dcpp/OnlineUser.h>
#include <dcpp/AdcHub.h>
#include <dcpp/User.h>
#include <dcpp/Util.h>

#include "HubUserParams.h"

using std::string;
using std::map;

namespace gtk_hub {

string iconForUser(bool supportsDCPP, bool isPassive, bool isOp)
{
    string icon = supportsDCPP ? "dc++" : "normal";

    if (isPassive)
        icon += "-fw";
    if (isOp)
        icon += "-op";

    return icon;
}

string nickOrderKey(const string &nick, bool isOp)
{
    return (isOp ? "O" : "U") + nick;
}

map<string, string> paramsFromIdentity(const dcpp::Identity &id)
{
    map<string, string> params;

    // Determine DC++ feature support
    bool supportsDCPP =
        id.supports(dcpp::AdcHub::ADCS_FEATURE) &&
        id.supports(dcpp::AdcHub::SEGA_FEATURE) &&
        ((id.supports(dcpp::AdcHub::TCP4_FEATURE) &&
          id.supports(dcpp::AdcHub::UDP4_FEATURE)) ||
         id.supports(dcpp::AdcHub::NAT0_FEATURE));

    bool isPassive = id.getUser()->isSet(dcpp::User::PASSIVE);
    bool isOp = id.isOp();

    params["Icon"] = iconForUser(supportsDCPP, isPassive, isOp);
    params["Nick Order"] = nickOrderKey(id.getNick(), isOp);
    params["Nick"] = id.getNick();
    params["Shared"] = dcpp::Util::toString(id.getBytesShared());
    params["Description"] = id.getDescription();
    params["Tag"] = id.getTag();
    params["Connection"] = id.getConnection();
    params["IP"] = id.getIp();
    params["eMail"] = id.getEmail();
    params["CID"] = id.getUser()->getCID().toBase32();

    return params;
}

} // namespace gtk_hub
