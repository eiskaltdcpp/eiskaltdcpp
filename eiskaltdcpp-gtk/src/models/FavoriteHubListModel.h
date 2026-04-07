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

#include <map>
#include <string>
#include <vector>

/**
 * Stateful model for the favorite hub list.
 *
 * Mirrors the data that FavoriteHubs (favoritehubs.cc) stores in GtkListStore.
 * This model holds the in-memory list of entries with CRUD operations.
 * Actual persistence to Favorites.xml goes through dcpp::FavoriteManager.
 *
 * No GTK dependency.
 */
namespace gtk_favhub {

/// A single favorite hub entry in the model.
struct HubEntry {
    std::map<std::string, std::string> params;
    bool autoConnect = false;

    /// Convenience getters — delegate to params map.
    std::string name() const;
    std::string address() const;
};

class FavoriteHubListModel {
public:
    /// Add a hub entry. Duplicates by address are silently ignored.
    /// Returns true if added, false if duplicate.
    bool addHub(const std::map<std::string, std::string> &params);

    /// Remove a hub by address. Returns true if found and removed.
    bool removeHub(const std::string &address);

    /// Update an existing hub by address. Returns true if found.
    bool updateHub(const std::string &address,
                   const std::map<std::string, std::string> &params);

    /// Find a hub by address. Returns nullptr if not found.
    const HubEntry *findByAddress(const std::string &address) const;

    /// All hubs in insertion order.
    std::vector<const HubEntry *> allHubs() const;

    /// Number of hubs.
    size_t count() const { return hubs_.size(); }

    /// Clear all entries.
    void clear() { hubs_.clear(); }

private:
    std::vector<HubEntry> hubs_;
};

} // namespace gtk_favhub
