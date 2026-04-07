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

#include "FavoriteHubListModel.h"

#include <algorithm>

using std::string;
using std::map;
using std::vector;

namespace gtk_favhub {

// ── HubEntry convenience getters ──

string HubEntry::name() const
{
    auto it = params.find("Name");
    return (it != params.end()) ? it->second : string();
}

string HubEntry::address() const
{
    auto it = params.find("Address");
    return (it != params.end()) ? it->second : string();
}

// ── FavoriteHubListModel ──

bool FavoriteHubListModel::addHub(const map<string, string> &params)
{
    HubEntry entry;
    entry.params = params;

    auto acIt = params.find("Auto Connect");
    entry.autoConnect = (acIt != params.end() && acIt->second == "1");

    // Duplicate check by address
    string addr = entry.address();
    for (const auto &h : hubs_) {
        if (h.address() == addr)
            return false;
    }

    hubs_.push_back(std::move(entry));
    return true;
}

bool FavoriteHubListModel::removeHub(const string &address)
{
    auto it = std::remove_if(hubs_.begin(), hubs_.end(),
        [&address](const HubEntry &h) { return h.address() == address; });
    if (it == hubs_.end())
        return false;
    hubs_.erase(it, hubs_.end());
    return true;
}

bool FavoriteHubListModel::updateHub(const string &address,
                                      const map<string, string> &params)
{
    for (auto &h : hubs_) {
        if (h.address() == address) {
            h.params = params;
            auto acIt = params.find("Auto Connect");
            h.autoConnect = (acIt != params.end() && acIt->second == "1");
            return true;
        }
    }
    return false;
}

const HubEntry *FavoriteHubListModel::findByAddress(const string &address) const
{
    for (const auto &h : hubs_) {
        if (h.address() == address)
            return &h;
    }
    return nullptr;
}

vector<const HubEntry *> FavoriteHubListModel::allHubs() const
{
    vector<const HubEntry *> result;
    result.reserve(hubs_.size());
    for (const auto &h : hubs_)
        result.push_back(&h);
    return result;
}

} // namespace gtk_favhub
