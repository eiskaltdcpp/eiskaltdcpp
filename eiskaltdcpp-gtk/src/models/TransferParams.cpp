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
#include <dcpp/Util.h>

#include "TransferParams.h"

using std::string;
using std::map;

namespace gtk_transfer {

map<string, string> paramsFromCQI(
    const string &cid,
    const string &nick,
    const string &hubName,
    const string &hubUrl)
{
    map<string, string> params;
    params["CID"] = cid;
    params["User"] = nick;
    params["Hub Name"] = hubName;
    params["Failed"] = "0";
    params["Hub URL"] = hubUrl;
    return params;
}

map<string, string> paramsFromTransfer(
    const string &cid,
    const string &nick,
    const string &hubName,
    const string &path,
    TransferType type,
    int64_t size,
    int64_t pos,
    int64_t avgSpeed,
    int64_t secondsLeft,
    const string &remoteIp,
    const string &hubUrl,
    bool isSecure,
    const string &cipherName)
{
    map<string, string> params;

    params["CID"] = cid;

    // File name depends on transfer type
    switch (type) {
    case TransferType::FULL_LIST:
    case TransferType::PARTIAL_LIST:
        params["Filename"] = "File list";
        break;
    case TransferType::TREE:
        params["Filename"] = "TTH: " + dcpp::Util::getFileName(path);
        break;
    case TransferType::FILE:
    default:
        params["Filename"] = dcpp::Util::getFileName(path);
        break;
    }

    params["User"] = nick;
    params["Hub Name"] = hubName;
    params["Path"] = dcpp::Util::getFilePath(path);
    params["Size"] = dcpp::Util::toString(size);
    params["Download Position"] = dcpp::Util::toString(pos);
    params["Speed"] = dcpp::Util::toString(avgSpeed);
    params["Progress Hidden"] = dcpp::Util::toString(progressPercent(pos, size));
    params["IP"] = remoteIp;
    params["Time Left"] = secondsLeft > 0 ? dcpp::Util::toString(secondsLeft) : "-1";
    params["Target"] = path;
    params["Hub URL"] = hubUrl;
    params["Encryption"] = isSecure ? cipherName : "Plain";

    return params;
}

string buildTransferFlags(bool secure, bool trusted,
                          bool tthCheck, bool zTransfer)
{
    string flags;
    if (secure) {
        flags += trusted ? "[S]" : "[U]";
    }
    if (tthCheck)
        flags += "[T]";
    if (zTransfer)
        flags += "[Z]";
    return flags;
}

string sortOrderPrefix(bool isDownload, bool isCompleted)
{
    if (isCompleted)
        return "w";
    return isDownload ? "d" : "u";
}

int progressPercent(int64_t pos, int64_t size)
{
    if (size <= 0)
        return 0;
    return static_cast<int>(static_cast<double>(pos) * 100.0 / size);
}

} // namespace gtk_transfer
