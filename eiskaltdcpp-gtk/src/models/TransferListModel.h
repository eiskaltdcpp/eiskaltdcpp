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
#include <vector>

/**
 * Stateful model for active transfers (uploads + downloads).
 *
 * Mirrors the data that Transfers (transfers.cc) stores in GtkTreeStore.
 * Supports parent/child grouping for segmented downloads (multiple
 * connections downloading the same file).
 *
 * No GTK dependency.
 */
namespace gtk_transfer {

/// A single transfer connection entry.
struct TransferEntry {
    std::string cid;
    std::string user;
    std::string hub;
    std::string filename;
    std::string path;
    std::string ip;
    std::string target;
    std::string tmpTarget;
    std::string status;
    std::string encryption;
    std::string hubUrl;
    std::string sortOrder;
    int64_t size = 0;
    int64_t downloadPosition = 0;
    int64_t speed = 0;
    int64_t timeLeft = 0;
    int progressPercent = 0;
    bool isDownload = true;
    bool failed = false;
};

/// Aggregated summary for a parent (file) with multiple transfer connections.
struct ParentSummary {
    std::string target;
    std::string users;      // Comma-separated user names
    std::string hubs;       // Comma-separated hub names
    int64_t totalSpeed = 0;
    int64_t totalSize = 0;
    int64_t totalPosition = 0;
    int activeCount = 0;
    int totalCount = 0;
    int progressPercent = 0;
    int64_t timeLeft = 0;
    bool finished = false;
    std::string status;
};

class TransferListModel {
public:
    // ── CRUD ──

    /// Add or update a transfer from a StringMap.
    /// Keys: "CID", "User", "Hub Name", "Filename", "Path", "Size",
    /// "Speed", "IP", "Target", "tmpTarget", "Status", "Encryption",
    /// "Hub URL", "Sort Order", "Download Position", "Progress",
    /// "Time Left", "Download" ("1"/"0"), "Failed" ("1"/"0").
    void addOrUpdate(const std::map<std::string, std::string> &params,
                     bool isDownload);

    /// Remove a transfer by CID and direction. Returns true if found.
    bool remove(const std::string &cid, bool isDownload);

    /// Find a transfer. Returns nullptr if not found.
    const TransferEntry *find(const std::string &cid, bool isDownload) const;

    /// All download transfers.
    std::vector<const TransferEntry *> downloads() const;

    /// All upload transfers.
    std::vector<const TransferEntry *> uploads() const;

    /// All transfers (downloads + uploads).
    std::vector<const TransferEntry *> allTransfers() const;

    /// Total transfer count.
    size_t count() const { return downloads_.size() + uploads_.size(); }

    /// Clear everything.
    void clear();

    // ── Parent/child grouping for segmented downloads ──

    /// Get a summary for all downloads targeting a given file.
    /// Returns an empty summary if no matching downloads found.
    ParentSummary parentSummary(const std::string &target) const;

    /// Get all unique targets that have multiple active downloads.
    std::vector<std::string> segmentedTargets() const;

    /// Mark a parent (target) as finished.
    void finishTarget(const std::string &target, const std::string &status);

private:
    // CID -> TransferEntry
    std::map<std::string, TransferEntry> downloads_;
    std::map<std::string, TransferEntry> uploads_;
    // target -> finished status (for completed segmented downloads)
    std::map<std::string, std::string> finishedTargets_;
};

} // namespace gtk_transfer
