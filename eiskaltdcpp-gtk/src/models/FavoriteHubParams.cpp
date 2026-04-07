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
#include <dcpp/SettingsManager.h>
#include <dcpp/Util.h>
#include <dcpp/HubEntry.h>

#include "FavoriteHubParams.h"

using std::string;
using std::map;

namespace gtk_favhub {

map<string, string> paramsFromEntry(const dcpp::FavoriteHubEntry &entry)
{
    map<string, string> params;

    params["Auto Connect"] = entry.getConnect() ? "1" : "0";
    params["Name"] = entry.getName();
    params["Description"] = entry.getHubDescription();
    params["Nick"] = entry.getNick(false); // Don't display default nick
    params["Password"] = entry.getPassword();
    params["Address"] = entry.getServer();
    params["User Description"] = entry.getUserDescription();
    params["Encoding"] = entry.getEncoding();
    params["Mode"] = dcpp::Util::toString(entry.getMode());
    params["Search Interval"] = dcpp::Util::toString(entry.getSearchInterval());
    params["Disable Chat"] = entry.getDisableChat() ? "1" : "0";
    params["External IP"] = entry.getExternalIP();
    params["Internet IP"] = entry.getUseInternetIP() ? "1" : "0";

    return params;
}

bool validateHubEntry(const string &name, const string &address, string &errorMsg)
{
    if (name.empty()) {
        errorMsg = "Hub name cannot be empty";
        return false;
    }
    if (address.empty()) {
        errorMsg = "Hub address cannot be empty";
        return false;
    }
    // Basic address format check
    if (address.find("://") == string::npos &&
        address.find('.') == string::npos &&
        address.find(':') == string::npos)
    {
        errorMsg = "Invalid hub address format";
        return false;
    }
    return true;
}

} // namespace gtk_favhub
