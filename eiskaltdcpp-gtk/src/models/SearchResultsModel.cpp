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

#include "SearchResultsModel.h"

#include <algorithm>
#include <cctype>
#include <sstream>

using std::string;
using std::map;
using std::vector;

namespace gtk_search {

namespace {

/// Case-insensitive substring search.
bool containsCI(const string &haystack, const string &needle)
{
    if (needle.empty())
        return true;
    auto it = std::search(haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
        });
    return it != haystack.end();
}

/// Get the grouping key for an entry based on GroupType.
string groupKeyFor(const ResultEntry &entry, GroupType type)
{
    auto get = [&entry](const char *key) -> string {
        auto it = entry.params.find(key);
        return (it != entry.params.end()) ? it->second : string();
    };

    switch (type) {
    case GroupType::NONE:       return "";
    case GroupType::FILENAME:   return get("Filename");
    case GroupType::FILEPATH:   return get("Path");
    case GroupType::SIZE:       return get("Real Size");
    case GroupType::CONNECTION: return get("Connection");
    case GroupType::TTH:        return get("TTH");
    case GroupType::NICK:       return get("Nick");
    case GroupType::HUB:        return get("Hub");
    case GroupType::TYPE:       return get("Type");
    default:                    return "";
    }
}

} // anonymous namespace

// ── CRUD ──

bool SearchResultsModel::addResult(const map<string, string> &params)
{
    auto cidIt = params.find("CID");
    string cid = (cidIt != params.end()) ? cidIt->second : "";

    // Derive file path for dedup
    string file;
    auto fnIt = params.find("Filename");
    auto pathIt = params.find("Path");
    if (fnIt != params.end())
        file = fnIt->second;
    if (pathIt != params.end())
        file = pathIt->second + file;

    // Duplicate check
    if (!cid.empty()) {
        auto &files = cidFiles_[cid];
        for (const auto &f : files) {
            if (f == file) {
                droppedCount_++;
                return false;
            }
        }
        files.push_back(file);
    }

    ResultEntry entry;
    entry.params = params;
    entry.cid = cid;
    entry.file = file;
    results_.push_back(std::move(entry));
    return true;
}

void SearchResultsModel::clear()
{
    results_.clear();
    cidFiles_.clear();
    droppedCount_ = 0;
}

vector<const ResultEntry *> SearchResultsModel::allResults() const
{
    vector<const ResultEntry *> result;
    result.reserve(results_.size());
    for (const auto &r : results_)
        result.push_back(&r);
    return result;
}

// ── Grouping ──

vector<ResultGroup> SearchResultsModel::groupedResults() const
{
    if (groupBy_ == GroupType::NONE) {
        ResultGroup single;
        for (const auto &r : results_)
            single.entries.push_back(&r);
        return {single};
    }

    // Group by the key
    map<string, ResultGroup> groups;
    for (const auto &r : results_) {
        string key = groupKeyFor(r, groupBy_);
        auto &group = groups[key];
        group.groupKey = key;
        group.entries.push_back(&r);
    }

    vector<ResultGroup> result;
    result.reserve(groups.size());
    for (auto &p : groups)
        result.push_back(std::move(p.second));
    return result;
}

// ── Filtering ──

bool SearchResultsModel::passes(const ResultEntry &entry) const
{
    auto get = [&entry](const char *key) -> string {
        auto it = entry.params.find(key);
        return (it != entry.params.end()) ? it->second : string();
    };

    // Free slots filter
    if (filter_.onlyFreeSlots) {
        string fs = get("Free Slots");
        if (fs.empty() || dcpp::Util::toInt(fs) < 1)
            return false;
    }

    // Hide shared filter
    if (filter_.hideShared) {
        if (get("Shared") == "1")
            return false;
    }

    // Size range filter
    if (filter_.minSize > 0 || filter_.maxSize > 0) {
        int64_t realSize = dcpp::Util::toInt64(get("Real Size"));
        if (filter_.minSize > 0 && realSize < filter_.minSize)
            return false;
        if (filter_.maxSize > 0 && realSize > filter_.maxSize)
            return false;
    }

    // File type filter
    if (!filter_.fileType.empty()) {
        string type = get("Type");
        if (!containsCI(type, filter_.fileType))
            return false;
    }

    // Text filter (matches against Filename and Path)
    if (!filter_.text.empty()) {
        // Support negative terms prefixed with '-'
        string filename = get("Filename");
        string path = get("Path");
        string combined = filename + " " + path;

        // Tokenize filter text
        std::istringstream iss(filter_.text);
        string token;
        while (iss >> token) {
            if (token.size() > 1 && token[0] == '-') {
                // Negative match — must NOT contain this
                string negTerm = token.substr(1);
                if (containsCI(combined, negTerm))
                    return false;
            } else {
                // Positive match — must contain this
                if (!containsCI(combined, token))
                    return false;
            }
        }
    }

    return true;
}

vector<const ResultEntry *> SearchResultsModel::filteredResults() const
{
    vector<const ResultEntry *> result;
    for (const auto &r : results_) {
        if (passes(r))
            result.push_back(&r);
    }
    return result;
}

} // namespace gtk_search
