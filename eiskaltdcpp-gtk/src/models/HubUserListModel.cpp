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

#include "HubUserListModel.h"

#include <algorithm>

using std::string;
using std::map;
using std::vector;

namespace gtk_hub {

// ── User CRUD ──

bool HubUserListModel::addOrUpdateUser(const map<string, string> &params)
{
    auto cidIt = params.find("CID");
    if (cidIt == params.end() || cidIt->second.empty())
        return false;

    const string &cid = cidIt->second;
    auto existing = users_.find(cid);

    if (existing != users_.end()) {
        // Update existing user
        UserInfo &u = existing->second;

        // Handle nick change: update nickToCid_ mapping
        auto newNick = params.find("Nick");
        if (newNick != params.end() && newNick->second != u.nick) {
            nickToCid_.erase(u.nick);
            nickToCid_[newNick->second] = cid;
            u.nick = newNick->second;
        }

        // Update shared total
        auto sharedIt = params.find("Shared");
        if (sharedIt != params.end()) {
            int64_t newShared = dcpp::Util::toInt64(sharedIt->second);
            totalShared_ -= u.shared;
            totalShared_ += newShared;
            u.shared = newShared;
        }

        // Update remaining fields
        auto it = params.find("Description");
        if (it != params.end()) u.description = it->second;
        it = params.find("Tag");
        if (it != params.end()) u.tag = it->second;
        it = params.find("Connection");
        if (it != params.end()) u.connection = it->second;
        it = params.find("IP");
        if (it != params.end()) u.ip = it->second;
        it = params.find("eMail");
        if (it != params.end()) u.email = it->second;
        it = params.find("Icon");
        if (it != params.end()) u.icon = it->second;
        it = params.find("Nick Order");
        if (it != params.end()) u.nickOrder = it->second;

        return false; // updated, not added
    }

    // New user
    UserInfo u;
    u.cid = cid;

    auto it = params.find("Nick");
    if (it != params.end()) u.nick = it->second;
    it = params.find("Description");
    if (it != params.end()) u.description = it->second;
    it = params.find("Tag");
    if (it != params.end()) u.tag = it->second;
    it = params.find("Connection");
    if (it != params.end()) u.connection = it->second;
    it = params.find("IP");
    if (it != params.end()) u.ip = it->second;
    it = params.find("eMail");
    if (it != params.end()) u.email = it->second;
    it = params.find("Icon");
    if (it != params.end()) u.icon = it->second;
    it = params.find("Nick Order");
    if (it != params.end()) u.nickOrder = it->second;
    it = params.find("Shared");
    if (it != params.end()) u.shared = dcpp::Util::toInt64(it->second);

    totalShared_ += u.shared;
    nickToCid_[u.nick] = cid;
    users_.emplace(cid, std::move(u));

    return true; // newly added
}

bool HubUserListModel::removeUser(const string &cid)
{
    auto it = users_.find(cid);
    if (it == users_.end())
        return false;

    totalShared_ -= it->second.shared;
    nickToCid_.erase(it->second.nick);
    users_.erase(it);
    return true;
}

const UserInfo *HubUserListModel::findByCid(const string &cid) const
{
    auto it = users_.find(cid);
    return (it != users_.end()) ? &it->second : nullptr;
}

const UserInfo *HubUserListModel::findByNick(const string &nick) const
{
    auto it = nickToCid_.find(nick);
    if (it == nickToCid_.end())
        return nullptr;
    return findByCid(it->second);
}

vector<const UserInfo *> HubUserListModel::allUsers() const
{
    vector<const UserInfo *> result;
    result.reserve(users_.size());
    for (const auto &pair : users_)
        result.push_back(&pair.second);
    return result;
}

void HubUserListModel::clearUsers()
{
    users_.clear();
    nickToCid_.clear();
    totalShared_ = 0;
}

// ── Favorite tracking ──

bool HubUserListModel::setFavorite(const string &cid, bool isFav)
{
    auto it = users_.find(cid);
    if (it == users_.end())
        return false;
    it->second.isFavorite = isFav;
    return true;
}

bool HubUserListModel::isFavorite(const string &cid) const
{
    auto it = users_.find(cid);
    return (it != users_.end()) && it->second.isFavorite;
}

vector<string> HubUserListModel::favoriteCids() const
{
    vector<string> result;
    for (const auto &pair : users_) {
        if (pair.second.isFavorite)
            result.push_back(pair.first);
    }
    return result;
}

// ── Chat history ──

void HubUserListModel::addHistoryEntry(const string &line)
{
    if (line.empty())
        return;

    // Remove trailing sentinel
    if (!history_.empty() && history_.back().empty())
        history_.pop_back();

    history_.push_back(line);

    // Trim to max
    while (history_.size() > static_cast<size_t>(MAX_HISTORY))
        history_.erase(history_.begin());

    // Re-add sentinel and reset index
    history_.push_back("");
    historyIndex_ = static_cast<int>(history_.size()) - 1;
}

string HubUserListModel::historyPrev()
{
    if (historyIndex_ > 0) {
        --historyIndex_;
        return history_[historyIndex_];
    }
    return history_.empty() ? string() : history_[0];
}

string HubUserListModel::historyNext()
{
    if (historyIndex_ < static_cast<int>(history_.size()) - 1) {
        ++historyIndex_;
        return history_[historyIndex_];
    }
    return string();
}

void HubUserListModel::historyReset()
{
    historyIndex_ = static_cast<int>(history_.size()) - 1;
}

size_t HubUserListModel::historySize() const
{
    // Don't count the trailing sentinel
    if (!history_.empty() && history_.back().empty())
        return history_.size() - 1;
    return history_.size();
}

} // namespace gtk_hub
