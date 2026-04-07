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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * Widget-independent public hub list filter.
 *
 * Extracts the filter logic from PublicHubs::updateList_gui().
 * Uses simple case-insensitive substring matching (mirroring
 * dcpp::StringSearch) against hub name, description, and address.
 *
 * No GTK dependency.
 */
namespace gtk_pubhub {

/// Lightweight hub entry for filtering (avoids depending on dcpp::HubEntry).
struct HubInfo {
    std::string name;
    std::string description;
    std::string server;        ///< Address / host
    std::string country;
    std::string rating;
    float       reliability = 0.0f;
    int64_t     shared  = 0;
    int64_t     minShare = 0;
    int         users   = 0;
    int         minSlots = 0;
    int         maxHubs  = 0;
    int         maxUsers = 0;
};

/**
 * Case-insensitive substring match filter for public hub lists.
 *
 * The filter pattern is matched against Name, Description, and Server
 * (same as the original GTK code).  An empty pattern matches everything.
 */
class PublicHubFilter {
public:
    PublicHubFilter() = default;
    explicit PublicHubFilter(const std::string &pattern);

    /// Set / replace the current filter pattern.
    void setPattern(const std::string &pattern);

    /// Get the current pattern.
    const std::string &pattern() const { return pattern_; }

    /// Test whether a single hub matches the filter.
    bool matches(const HubInfo &hub) const;

    /// Filter a list — returns indices of matching hubs.
    std::vector<size_t> filter(const std::vector<HubInfo> &hubs) const;

    /// Count matching hubs.
    size_t countMatching(const std::vector<HubInfo> &hubs) const;

    /// Convenience: return filtered copies.
    std::vector<HubInfo> filterCopy(const std::vector<HubInfo> &hubs) const;

    /// Summary stats of last filterCopy / filter call.
    struct Stats {
        size_t matchedHubs  = 0;
        int    totalUsers   = 0;
    };
    Stats stats(const std::vector<HubInfo> &hubs) const;

private:
    std::string pattern_;         ///< Lowered pattern
    std::string patternLower_;    ///< Pre-lowered for matching
};

} // namespace gtk_pubhub
