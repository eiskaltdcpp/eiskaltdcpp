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

#include "SearchResultParams.h"

/**
 * Stateful model for search results with grouping and filtering.
 *
 * Mirrors the data that Search (search.cc) stores in
 * results (unordered_map<CID, vector<SearchResultPtr>>) + GtkTreeStore.
 *
 * The model maintains a flat list of result entries and provides
 * on-demand grouping and filtering without GTK dependency.
 */
namespace gtk_search {

/// A single search result entry.
struct ResultEntry {
    std::map<std::string, std::string> params;
    std::string cid;    // User CID for dedup
    std::string file;   // Full file path for dedup
};

/// A grouped set of results sharing a common key.
struct ResultGroup {
    std::string groupKey;
    std::vector<const ResultEntry *> entries;
};

/// Filter criteria for search results.
struct SearchFilter {
    std::string text;           // Text to match in Filename/Path
    bool onlyFreeSlots = false; // Require Free Slots >= 1
    bool hideShared = false;    // Hide locally shared results
    int64_t minSize = 0;        // Minimum size (0 = no filter)
    int64_t maxSize = 0;        // Maximum size (0 = no filter)
    std::string fileType;       // File type extension filter (empty = any)
};

class SearchResultsModel {
public:
    // ── CRUD ──

    /// Add a result entry from a parsed params map.
    /// Duplicates (same CID + file path) are silently ignored.
    /// Returns true if added, false if duplicate.
    bool addResult(const std::map<std::string, std::string> &params);

    /// Clear all results.
    void clear();

    /// Number of results (before filtering).
    size_t resultCount() const { return results_.size(); }

    /// Number of dropped duplicates.
    int droppedCount() const { return droppedCount_; }

    /// All results as flat list.
    std::vector<const ResultEntry *> allResults() const;

    // ── Grouping ──

    /// Set the grouping mode.
    void setGroupBy(GroupType type) { groupBy_ = type; }

    /// Get the current grouping mode.
    GroupType groupBy() const { return groupBy_; }

    /// Get results grouped by the current grouping mode.
    /// Returns a single group with empty key when GroupType::NONE.
    std::vector<ResultGroup> groupedResults() const;

    // ── Filtering ──

    /// Set the filter criteria.
    void setFilter(const SearchFilter &filter) { filter_ = filter; }

    /// Get the current filter.
    const SearchFilter &filter() const { return filter_; }

    /// Test whether a single entry passes the current filter.
    bool passes(const ResultEntry &entry) const;

    /// Get all results that pass the current filter (flat).
    std::vector<const ResultEntry *> filteredResults() const;

private:
    std::vector<ResultEntry> results_;
    // CID -> set of file paths (for dedup)
    std::unordered_map<std::string, std::vector<std::string>> cidFiles_;
    GroupType groupBy_ = GroupType::NONE;
    SearchFilter filter_;
    int droppedCount_ = 0;
};

} // namespace gtk_search
