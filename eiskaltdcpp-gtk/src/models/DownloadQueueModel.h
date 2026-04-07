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
 * Stateful model for the download queue (directory tree + file list).
 *
 * Mirrors the data that DownloadQueue (downloadqueue.cc) stores in
 * dirStore (GtkTreeStore) + fileStore (GtkListStore) + sources map.
 *
 * The model maintains:
 * - A flat list of queue items keyed by target path
 * - A computed directory tree
 * - Source tracking (nick -> CID) per target
 *
 * No GTK dependency.
 */
namespace gtk_queue {

/// A single queue item.
struct QueueEntry {
    std::map<std::string, std::string> params;
    std::string target;   // Full target path (unique key)
    std::string directory; // Parent directory
    int priority = 0;
    int64_t size = 0;
    int64_t downloaded = 0;
};

/// Directory tree node.
struct DirNode {
    std::string name;
    std::string fullPath;
    std::vector<DirNode> children;
    int fileCount = 0;
    int64_t totalSize = 0;
};

/// Aggregate queue statistics.
struct QueueStats {
    int items = 0;
    int64_t totalSize = 0;
    int64_t totalDownloaded = 0;
};

class DownloadQueueModel {
public:
    // ── Item CRUD ──

    /// Add a queue item from a StringMap.
    /// Keys: "Target", "Filename", "Path", "Size", "Downloaded",
    /// "Priority", "Status", "Users", etc.
    void addItem(const std::map<std::string, std::string> &params);

    /// Update a queue item by target. Merges params into existing.
    bool updateItem(const std::string &target,
                    const std::map<std::string, std::string> &params);

    /// Remove a queue item by target. Returns true if found and removed.
    bool removeItem(const std::string &target);

    /// Find an item by target. Returns nullptr if not found.
    const QueueEntry *findItem(const std::string &target) const;

    /// All items in the queue.
    std::vector<const QueueEntry *> allItems() const;

    /// Items in a specific directory.
    std::vector<const QueueEntry *> itemsInDir(const std::string &dir) const;

    /// Number of items.
    size_t itemCount() const { return items_.size(); }

    /// Clear all items.
    void clear();

    // ── Directory tree ──

    /// Build a directory tree from all queue items.
    DirNode directoryTree() const;

    /// Get all unique directory paths.
    std::vector<std::string> directories() const;

    // ── Source tracking ──

    /// Set the sources for a target. Map is nick -> CID.
    void setSources(const std::string &target,
                    const std::map<std::string, std::string> &nickToCid);

    /// Set the bad sources for a target.
    void setBadSources(const std::string &target,
                       const std::map<std::string, std::string> &nickToCid);

    /// Get sources for a target.
    const std::map<std::string, std::string> *
    getSources(const std::string &target) const;

    /// Get bad sources for a target.
    const std::map<std::string, std::string> *
    getBadSources(const std::string &target) const;

    // ── Stats ──

    QueueStats stats() const;

    /// Update the priority for a target. Returns true if found.
    bool updatePriority(const std::string &target, int priority);

private:
    std::map<std::string, QueueEntry> items_;  // target -> entry
    std::unordered_map<std::string, std::map<std::string, std::string>> sources_;
    std::unordered_map<std::string, std::map<std::string, std::string>> badSources_;
};

} // namespace gtk_queue
