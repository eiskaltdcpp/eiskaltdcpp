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
#include <dcpp/SearchResult.h>
#include <dcpp/Util.h>

#include "SearchResultParams.h"
#include "GtkWulforUtil.h"

using std::string;
using std::map;

namespace gtk_search {

string groupingColumn(GroupType type)
{
    switch (type) {
    case GroupType::NONE:       return "";
    case GroupType::FILENAME:   return "Filename";
    case GroupType::FILEPATH:   return "Path";
    case GroupType::SIZE:       return "Size";
    case GroupType::CONNECTION: return "Connection";
    case GroupType::TTH:        return "TTH";
    case GroupType::NICK:       return "Nick";
    case GroupType::HUB:        return "Hub";
    case GroupType::TYPE:       return "Type";
    default:                    return "";
    }
}

string fileOrderKey(const string &filename, bool isDirectory)
{
    return (isDirectory ? "d" : "f") + filename;
}

string slotsOrderKey(int freeSlots, int totalSlots)
{
    return dcpp::Util::toString(-1000 * freeSlots - totalSlots);
}

map<string, string> paramsFromResult(
    dcpp::SearchResult &result,
    const string &nick,
    const string &conn,
    bool isSharedLocally)
{
    map<string, string> params;

    if (result.getType() == dcpp::SearchResult::TYPE_FILE)
    {
        string file = gtk_util::linuxSeparator(result.getFile());
        if (file.rfind('/') == string::npos)
        {
            params["Filename"] = file;
        }
        else
        {
            // Path was normalized to '/' by linuxSeparator —
            // must pass '/' explicitly so getFileName/getFilePath
            // work correctly on Windows where PATH_SEPARATOR is '\\'.
            params["Filename"] = dcpp::Util::getFileName(file, '/');
            params["Path"] = dcpp::Util::getFilePath(file, '/');
        }

        params["File Order"] = fileOrderKey(params["Filename"], false);
        params["Type"] = dcpp::Util::getFileExt(params["Filename"]);
        if (!params["Type"].empty() && params["Type"][0] == '.')
            params["Type"].erase(0, 1);
        params["Size"] = dcpp::Util::formatBytes(result.getSize());
        params["Exact Size"] = dcpp::Util::formatExactSize(result.getSize());
        params["Icon"] = "icon-file";
        params["Shared"] = isSharedLocally ? "1" : "0";
    }
    else
    {
        // Directory result — path normalized to '/' by linuxSeparator.
        string path = gtk_util::linuxSeparator(result.getFile());
        params["Filename"] = dcpp::Util::getLastDir(path, '/') + "/";
        string chopPath = path.substr(0, path.length() > 0 ? path.length() - 1 : 0);
        params["Path"] = dcpp::Util::getFilePath(chopPath, '/');
        if (params["Path"].find("/") == string::npos)
            params["Path"] = "";
        params["File Order"] = fileOrderKey(params["Filename"], true);
        params["Type"] = "Directory";
        params["Icon"] = "icon-directory";
        params["Shared"] = "0";
        if (result.getSize() > 0)
        {
            params["Size"] = dcpp::Util::formatBytes(result.getSize());
            params["Exact Size"] = dcpp::Util::formatExactSize(result.getSize());
        }
    }

    params["Nick"] = nick;
    params["CID"] = result.getUser() ?
        result.getUser()->getCID().toBase32() : "";
    params["Slots"] = result.getSlotString();
    params["Connection"] = conn;
    params["Hub"] = result.getHubName().empty() ?
        result.getHubURL() : result.getHubName();
    params["Hub URL"] = result.getHubURL();
    params["IP"] = result.getIP();
    params["Real Size"] = dcpp::Util::toString(result.getSize());
    if (result.getType() == dcpp::SearchResult::TYPE_FILE)
        params["TTH"] = result.getTTH().toBase32();

    params["Slots Order"] = slotsOrderKey(result.getFreeSlots(), result.getSlots());
    params["Free Slots"] = dcpp::Util::toString(result.getFreeSlots());

    return params;
}

} // namespace gtk_search
