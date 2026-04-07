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

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Stateful model for a hub's user list, favorite tracking and chat history.
 *
 * Mirrors the data that Hub (hub.cc) stores in userMap, userIters,
 * userFavoriteMap, nickStore, history vectory, totalShared counter.
 *
 * No GTK dependency.
 */
namespace gtk_hub {

/// A single hub user.
struct UserInfo {
    std::string cid;
    std::string nick;
    std::string description;
    std::string tag;
    std::string connection;
    std::string ip;
    std::string email;
    std::string icon;
    std::string nickOrder;
    int64_t shared = 0;
    bool isFavorite = false;
};

class HubUserListModel {
public:
    static constexpr int MAX_HISTORY = 20;

    // ── User CRUD ──

    /// Add or update a user from a ParamMap.
    /// Keys expected: "CID", "Nick", "Description", "Tag", "Connection",
    /// "IP", "eMail", "Icon", "Nick Order", "Shared".
    /// Returns true if a new user was added, false if updated in place.
    bool addOrUpdateUser(const std::map<std::string, std::string> &params);

    /// Remove a user by CID. Returns true if found and removed.
    bool removeUser(const std::string &cid);

    /// Find a user by CID. Returns nullptr if not found.
    const UserInfo *findByCid(const std::string &cid) const;

    /// Find a user by nick. Returns nullptr if not found.
    const UserInfo *findByNick(const std::string &nick) const;

    /// All users.
    std::vector<const UserInfo *> allUsers() const;

    /// Number of users.
    size_t userCount() const { return users_.size(); }

    /// Total shared bytes across all users.
    int64_t totalShared() const { return totalShared_; }

    /// Clear the entire user list.
    void clearUsers();

    // ── Favorite tracking ──

    /// Mark a user as a favorite. Returns false if CID not found.
    bool setFavorite(const std::string &cid, bool isFavorite);

    /// Check if a user is a favorite.
    bool isFavorite(const std::string &cid) const;

    /// Get all favorite user CIDs.
    std::vector<std::string> favoriteCids() const;

    // ── Chat history ──

    /// Add a sent message to the history.
    void addHistoryEntry(const std::string &line);

    /// Navigate history backward (older). Returns empty string at boundary.
    std::string historyPrev();

    /// Navigate history forward (newer). Returns empty string at boundary.
    std::string historyNext();

    /// Reset history index to the end (after current entry is used).
    void historyReset();

    /// Current history size (not counting the trailing sentinel).
    size_t historySize() const;

private:
    // CID -> UserInfo
    std::unordered_map<std::string, UserInfo> users_;
    // Nick -> CID (for fast nick lookups)
    std::unordered_map<std::string, std::string> nickToCid_;
    // Running total
    int64_t totalShared_ = 0;

    // Chat history with trailing empty sentinel
    std::vector<std::string> history_{""};
    int historyIndex_ = 0;
};

} // namespace gtk_hub
