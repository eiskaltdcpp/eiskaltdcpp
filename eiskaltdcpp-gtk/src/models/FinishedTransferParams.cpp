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
#include <dcpp/HintedUser.h>
#include <dcpp/FinishedItem.h>
#include <dcpp/Util.h>

#include "FinishedTransferParams.h"

using std::string;
using std::map;
using std::vector;
using std::pair;

namespace gtk_finished {

map<string, string> paramsFromFile(
    const dcpp::FinishedFileItem &item,
    const string &filePath,
    const string &nicks)
{
    map<string, string> params;

    params["Filename"] = dcpp::Util::getFileName(filePath);
    params["Time"] = dcpp::Util::formatTime("%Y-%m-%d %H:%M:%S", item.getTime());
    params["Path"] = dcpp::Util::getFilePath(filePath);
    params["Nicks"] = nicks;
    params["Transferred"] = dcpp::Util::toString(item.getTransferred());
    params["Speed"] = dcpp::Util::toString(item.getAverageSpeed());
    params["CRC Checked"] = item.getCrc32Checked() ? "Yes" : "No";
    params["Target"] = filePath;
    params["Elapsed Time"] = dcpp::Util::toString(item.getMilliSeconds());
    params["full"] = item.isFull() ? "1" : "0";

    return params;
}

map<string, string> paramsFromUser(
    const dcpp::FinishedUserItem &item,
    const string &nick,
    const string &hub,
    const string &cid)
{
    map<string, string> params;

    params["Time"] = dcpp::Util::formatTime("%Y-%m-%d %H:%M:%S", item.getTime());
    params["Nick"] = nick;
    params["Hub"] = hub;

    // Join files into a comma-separated list
    string files;
    for (const auto &file : item.getFiles()) {
        if (!files.empty())
            files += ", ";
        files += file;
    }
    params["Files"] = files;

    params["Transferred"] = dcpp::Util::toString(item.getTransferred());
    params["Speed"] = dcpp::Util::toString(item.getAverageSpeed());
    params["CID"] = cid;
    params["Elapsed Time"] = dcpp::Util::toString(item.getMilliSeconds());

    return params;
}

Totals computeTotals(const vector<pair<int64_t, int64_t>> &items)
{
    Totals t;
    t.files = static_cast<int>(items.size());
    int64_t totalSpeed = 0;
    for (const auto &p : items) {
        t.bytes += p.first;
        totalSpeed += p.second;
    }
    if (t.files > 0)
        t.avgSpeed = totalSpeed / t.files;
    return t;
}

} // namespace gtk_finished
