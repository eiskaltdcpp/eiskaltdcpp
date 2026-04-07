/*
 * Copyright (C) 2004-2010 Jens Oknelid, paskharen@gmail.com
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
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include <dcpp/stdinc.h>
#include "GtkWulforUtil.h"
#include <dcpp/Util.h>
#include <dcpp/typedefs.h>
#include <dcpp/CID.h>
#include <dcpp/ClientManager.h>
#include <dcpp/DCContext.h>
#include "extra/magnet.h"

#include <algorithm>
#include <cctype>
#include <cstring>

using namespace std;
using namespace dcpp;

namespace {

/// Case-insensitive prefix match (replaces g_ascii_strncasecmp)
bool hasPrefixCI(const string &text, const char *prefix, size_t len)
{
    if (text.size() < len)
        return false;
    for (size_t i = 0; i < len; ++i) {
        if (std::tolower(static_cast<unsigned char>(text[i])) !=
            std::tolower(static_cast<unsigned char>(prefix[i])))
            return false;
    }
    return true;
}

} // anonymous namespace

namespace gtk_util {

vector<int> splitString(const string &str, const string &delimiter)
{
    string::size_type loc, len, pos = 0;
    vector<int> array;

    if (!str.empty() && !delimiter.empty())
    {
        while ((loc = str.find(delimiter, pos)) != string::npos)
        {
            len = loc - pos;
            array.push_back(Util::toInt(str.substr(pos, len)));
            pos = loc + delimiter.size();
        }
        len = str.size() - pos;
        array.push_back(Util::toInt(str.substr(pos, len)));
    }
    return array;
}

string linuxSeparator(const string &ps)
{
    string str = ps;
    for (auto it = str.begin(); it != str.end(); ++it)
        if ((*it) == '\\')
            (*it) = '/';
    return str;
}

string windowsSeparator(const string &ps)
{
    string str = ps;
    for (auto it = str.begin(); it != str.end(); ++it)
        if ((*it) == '/')
            (*it) = '\\';
    return str;
}

static const string magnetSignature = "magnet:?";

string makeMagnet(const string &name, int64_t size, const string &tth)
{
    if (name.empty() || tth.empty())
        return string();

    // other clients can return paths with different separators, so we should catch both cases
    string::size_type i = name.find_last_of("/\\");
    string path = (i != string::npos) ? name.substr(i + 1) : name;

    return magnetSignature + "xt=urn:tree:tiger:" + tth + "&xl=" + Util::toString(size) + "&dn=" + Util::encodeURI(path);
}

bool splitMagnet(const string &magnet, string &name, int64_t &size, string &tth)
{
    name = "Unknown";
    size = 0;
    tth = string();

    StringMap params;
    if (magnet::parseUri(magnet, params)) {
        // Name of file or directory (or search keywords if name is not set)
        name = params["dn"];
        if (name.empty() && !params["kt"].empty()) {
            name = params["kt"];
        }

        // BitTorrent magnet links are quite popular nowadays...
        if (magnet.find("urn:btih:") == string::npos && magnet.find("urn:btmh:") == string::npos) {
            tth = params["xt"];
        }

        // Size of file or directory
        if (!params["xl"].empty()) {
            size = Util::toInt64(params["xl"]);
        }
        if (!params["dl"].empty()) { // this size is more valuable if it is set
            size = Util::toInt64(params["dl"]);
        }

        return true;
    }
    return false;
}

bool splitMagnet(const string &magnet, string &line)
{
    string name;
    string tth;
    int64_t size;

    if (splitMagnet(magnet, name, size, tth)) {
        // BitTorrent magnet links are quite popular nowadays...
        if (magnet.find("urn:btih:") != string::npos || magnet.find("urn:btmh:") != string::npos) {
            line = name + " (BitTorrent)";
        } else {
            line = name + " (" + Util::formatBytes(size) + ")";
        }
    }
    else {
        return false;
    }

    return true;
}

bool isMagnet(const string &text)
{
    return hasPrefixCI(text, magnetSignature.c_str(), magnetSignature.length());
}

bool isLink(const string &text)
{
    return hasPrefixCI(text, "http://", 7) ||
           hasPrefixCI(text, "https://", 8) ||
           hasPrefixCI(text, "www.", 4) ||
           hasPrefixCI(text, "ftp://", 6) ||
           hasPrefixCI(text, "sftp://", 7) ||
           hasPrefixCI(text, "irc://", 6) ||
           hasPrefixCI(text, "ircs://", 7) ||
           hasPrefixCI(text, "im:", 3) ||
           hasPrefixCI(text, "mailto:", 7) ||
           hasPrefixCI(text, "news:", 5);
}

bool isHubURL(const string &text)
{
    return hasPrefixCI(text, "dchub://", 8) ||
           hasPrefixCI(text, "nmdcs://", 8) ||
           hasPrefixCI(text, "adc://", 6) ||
           hasPrefixCI(text, "adcs://", 7);
}

// --- dcpp ClientManager lookups ---

string getNicks(dcpp::DCContext &ctx, const string &cid, const string &hintUrl)
{
    return dcpp::Util::toString(
        ctx.getClientManager()->getNicks(dcpp::CID(cid), hintUrl));
}

string getHubNames(dcpp::DCContext &ctx, const string &cid, const string &hintUrl)
{
    dcpp::StringList hubs =
        ctx.getClientManager()->getHubNames(dcpp::CID(cid), hintUrl);
    if (hubs.empty())
        return "Offline";
    return dcpp::Util::toString(hubs);
}

vector<string> getHubAddress(dcpp::DCContext &ctx, const string &cid, const string &hintUrl)
{
    return ctx.getClientManager()->getHubs(dcpp::CID(cid), hintUrl);
}

} // namespace gtk_util
