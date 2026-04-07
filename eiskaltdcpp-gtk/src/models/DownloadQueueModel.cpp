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

#include "DownloadQueueModel.h"

#include <algorithm>
#include <set>

using std::string;
using std::map;
using std::vector;

namespace gtk_queue {

// ── Item CRUD ──

void DownloadQueueModel::addItem(const map<string, string> &params)
{
    auto targetIt = params.find("Target");
    if (targetIt == params.end() || targetIt->second.empty())
        return;

    QueueEntry entry;
    entry.params = params;
    entry.target = targetIt->second;

    auto pathIt = params.find("Path");
    if (pathIt != params.end())
        entry.directory = pathIt->second;

    auto sizeIt = params.find("Real Size");
    if (sizeIt != params.end())
        entry.size = dcpp::Util::toInt64(sizeIt->second);
    else {
        sizeIt = params.find("Size Sort");
        if (sizeIt != params.end())
            entry.size = dcpp::Util::toInt64(sizeIt->second);
    }

    auto dlIt = params.find("Downloaded Sort");
    if (dlIt != params.end())
        entry.downloaded = dcpp::Util::toInt64(dlIt->second);

    auto priIt = params.find("Priority");
    if (priIt != params.end())
        entry.priority = dcpp::Util::toInt(priIt->second);

    items_[entry.target] = std::move(entry);
}

bool DownloadQueueModel::updateItem(const string &target,
                                     const map<string, string> &params)
{
    auto it = items_.find(target);
    if (it == items_.end())
        return false;

    // Merge params
    for (const auto &p : params)
        it->second.params[p.first] = p.second;

    auto priIt = params.find("Priority");
    if (priIt != params.end())
        it->second.priority = dcpp::Util::toInt(priIt->second);

    auto dlIt = params.find("Downloaded Sort");
    if (dlIt != params.end())
        it->second.downloaded = dcpp::Util::toInt64(dlIt->second);

    return true;
}

bool DownloadQueueModel::removeItem(const string &target)
{
    sources_.erase(target);
    badSources_.erase(target);
    return items_.erase(target) > 0;
}

const QueueEntry *DownloadQueueModel::findItem(const string &target) const
{
    auto it = items_.find(target);
    return (it != items_.end()) ? &it->second : nullptr;
}

vector<const QueueEntry *> DownloadQueueModel::allItems() const
{
    vector<const QueueEntry *> result;
    result.reserve(items_.size());
    for (const auto &p : items_)
        result.push_back(&p.second);
    return result;
}

vector<const QueueEntry *> DownloadQueueModel::itemsInDir(const string &dir) const
{
    vector<const QueueEntry *> result;
    for (const auto &p : items_) {
        if (p.second.directory == dir)
            result.push_back(&p.second);
    }
    return result;
}

void DownloadQueueModel::clear()
{
    items_.clear();
    sources_.clear();
    badSources_.clear();
}

// ── Directory tree ──

DirNode DownloadQueueModel::directoryTree() const
{
    DirNode root;
    root.name = "/";
    root.fullPath = "/";

    // Collect all directories and their file counts/sizes
    map<string, std::pair<int, int64_t>> dirStats; // dir -> (count, size)
    for (const auto &p : items_) {
        const string &dir = p.second.directory;
        if (!dir.empty()) {
            dirStats[dir].first++;
            dirStats[dir].second += p.second.size;
        }
    }

    // Build tree from directory paths
    for (const auto &ds : dirStats) {
        const string &path = ds.first;

        // Split path into components
        vector<string> parts;
        string::size_type start = 0;
        while (start < path.size()) {
            auto pos = path.find('/', start);
            if (pos == string::npos) {
                string part = path.substr(start);
                if (!part.empty())
                    parts.push_back(part);
                break;
            }
            string part = path.substr(start, pos - start);
            if (!part.empty())
                parts.push_back(part);
            start = pos + 1;
        }

        // Navigate/create tree
        DirNode *current = &root;
        string builtPath;
        for (const auto &part : parts) {
            builtPath += "/" + part;

            auto childIt = std::find_if(current->children.begin(),
                current->children.end(),
                [&part](const DirNode &n) { return n.name == part; });

            if (childIt == current->children.end()) {
                DirNode child;
                child.name = part;
                child.fullPath = builtPath;
                current->children.push_back(std::move(child));
                current = &current->children.back();
            } else {
                current = &(*childIt);
            }
        }

        current->fileCount = ds.second.first;
        current->totalSize = ds.second.second;
    }

    // Compute root totals
    root.fileCount = static_cast<int>(items_.size());
    for (const auto &p : items_)
        root.totalSize += p.second.size;

    return root;
}

vector<string> DownloadQueueModel::directories() const
{
    std::set<string> dirs;
    for (const auto &p : items_) {
        if (!p.second.directory.empty())
            dirs.insert(p.second.directory);
    }
    return vector<string>(dirs.begin(), dirs.end());
}

// ── Source tracking ──

void DownloadQueueModel::setSources(const string &target,
                                     const map<string, string> &nickToCid)
{
    sources_[target] = nickToCid;
}

void DownloadQueueModel::setBadSources(const string &target,
                                        const map<string, string> &nickToCid)
{
    badSources_[target] = nickToCid;
}

const map<string, string> *
DownloadQueueModel::getSources(const string &target) const
{
    auto it = sources_.find(target);
    return (it != sources_.end()) ? &it->second : nullptr;
}

const map<string, string> *
DownloadQueueModel::getBadSources(const string &target) const
{
    auto it = badSources_.find(target);
    return (it != badSources_.end()) ? &it->second : nullptr;
}

// ── Stats ──

QueueStats DownloadQueueModel::stats() const
{
    QueueStats s;
    s.items = static_cast<int>(items_.size());
    for (const auto &p : items_) {
        s.totalSize += p.second.size;
        s.totalDownloaded += p.second.downloaded;
    }
    return s;
}

bool DownloadQueueModel::updatePriority(const string &target, int priority)
{
    auto it = items_.find(target);
    if (it == items_.end())
        return false;
    it->second.priority = priority;
    it->second.params["Priority"] = dcpp::Util::toString(priority);
    return true;
}

} // namespace gtk_queue
