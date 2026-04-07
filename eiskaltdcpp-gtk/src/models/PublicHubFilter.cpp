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
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "dcpp/stdinc.h"
#include "PublicHubFilter.h"

#include <algorithm>
#include <cctype>

namespace gtk_pubhub {

namespace {

std::string toLowerStr(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s)
        out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}

bool containsCI(const std::string &haystack, const std::string &needleLower) {
    if (needleLower.empty()) return true;
    std::string lower = toLowerStr(haystack);
    return lower.find(needleLower) != std::string::npos;
}

} // anonymous namespace

PublicHubFilter::PublicHubFilter(const std::string &pattern)
    : pattern_(pattern), patternLower_(toLowerStr(pattern))
{
}

void PublicHubFilter::setPattern(const std::string &pattern) {
    pattern_ = pattern;
    patternLower_ = toLowerStr(pattern);
}

bool PublicHubFilter::matches(const HubInfo &hub) const {
    if (patternLower_.empty()) return true;
    return containsCI(hub.name, patternLower_) ||
           containsCI(hub.description, patternLower_) ||
           containsCI(hub.server, patternLower_);
}

std::vector<size_t> PublicHubFilter::filter(const std::vector<HubInfo> &hubs) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < hubs.size(); ++i) {
        if (matches(hubs[i]))
            indices.push_back(i);
    }
    return indices;
}

size_t PublicHubFilter::countMatching(const std::vector<HubInfo> &hubs) const {
    size_t count = 0;
    for (const auto &h : hubs)
        if (matches(h)) ++count;
    return count;
}

std::vector<HubInfo> PublicHubFilter::filterCopy(const std::vector<HubInfo> &hubs) const {
    std::vector<HubInfo> result;
    for (const auto &h : hubs)
        if (matches(h)) result.push_back(h);
    return result;
}

PublicHubFilter::Stats PublicHubFilter::stats(const std::vector<HubInfo> &hubs) const {
    Stats s;
    for (const auto &h : hubs) {
        if (matches(h)) {
            ++s.matchedHubs;
            s.totalUsers += h.users;
        }
    }
    return s;
}

} // namespace gtk_pubhub
