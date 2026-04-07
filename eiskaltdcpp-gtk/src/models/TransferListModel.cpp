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

#include "TransferListModel.h"

#include <set>

using std::string;
using std::map;
using std::vector;

namespace gtk_transfer {

namespace {

void applyParams(TransferEntry &e, const map<string, string> &params)
{
    auto get = [&params](const char *key) -> string {
        auto it = params.find(key);
        return (it != params.end()) ? it->second : string();
    };

    string v;
    v = get("CID");       if (!v.empty()) e.cid = v;
    v = get("User");      if (!v.empty()) e.user = v;
    v = get("Hub Name");  if (!v.empty()) e.hub = v;
    v = get("Filename");  if (!v.empty()) e.filename = v;
    v = get("Path");      if (!v.empty()) e.path = v;
    v = get("IP");        if (!v.empty()) e.ip = v;
    v = get("Target");    if (!v.empty()) e.target = v;
    v = get("tmpTarget"); if (!v.empty()) e.tmpTarget = v;
    v = get("Status");    if (!v.empty()) e.status = v;
    v = get("Encryption");if (!v.empty()) e.encryption = v;
    v = get("Hub URL");   if (!v.empty()) e.hubUrl = v;
    v = get("Sort Order");if (!v.empty()) e.sortOrder = v;

    v = get("Size");
    if (!v.empty()) e.size = dcpp::Util::toInt64(v);
    v = get("Download Position");
    if (!v.empty()) e.downloadPosition = dcpp::Util::toInt64(v);
    v = get("Speed");
    if (!v.empty()) e.speed = dcpp::Util::toInt64(v);
    v = get("Time Left");
    if (!v.empty()) e.timeLeft = dcpp::Util::toInt64(v);
    v = get("Progress");
    if (!v.empty()) e.progressPercent = dcpp::Util::toInt(v);

    v = get("Failed");
    if (!v.empty()) e.failed = (v == "1");
}

} // anonymous namespace

// ── CRUD ──

void TransferListModel::addOrUpdate(const map<string, string> &params,
                                     bool isDownload)
{
    auto cidIt = params.find("CID");
    if (cidIt == params.end() || cidIt->second.empty())
        return;

    auto &store = isDownload ? downloads_ : uploads_;
    auto &entry = store[cidIt->second];
    entry.isDownload = isDownload;
    applyParams(entry, params);
}

bool TransferListModel::remove(const string &cid, bool isDownload)
{
    auto &store = isDownload ? downloads_ : uploads_;
    return store.erase(cid) > 0;
}

const TransferEntry *TransferListModel::find(const string &cid,
                                              bool isDownload) const
{
    const auto &store = isDownload ? downloads_ : uploads_;
    auto it = store.find(cid);
    return (it != store.end()) ? &it->second : nullptr;
}

vector<const TransferEntry *> TransferListModel::downloads() const
{
    vector<const TransferEntry *> result;
    result.reserve(downloads_.size());
    for (const auto &p : downloads_)
        result.push_back(&p.second);
    return result;
}

vector<const TransferEntry *> TransferListModel::uploads() const
{
    vector<const TransferEntry *> result;
    result.reserve(uploads_.size());
    for (const auto &p : uploads_)
        result.push_back(&p.second);
    return result;
}

vector<const TransferEntry *> TransferListModel::allTransfers() const
{
    vector<const TransferEntry *> result;
    result.reserve(downloads_.size() + uploads_.size());
    for (const auto &p : downloads_)
        result.push_back(&p.second);
    for (const auto &p : uploads_)
        result.push_back(&p.second);
    return result;
}

void TransferListModel::clear()
{
    downloads_.clear();
    uploads_.clear();
    finishedTargets_.clear();
}

// ── Parent/child grouping ──

ParentSummary TransferListModel::parentSummary(const string &target) const
{
    ParentSummary summary;
    summary.target = target;

    // Check if this target is already finished
    auto finIt = finishedTargets_.find(target);
    if (finIt != finishedTargets_.end()) {
        summary.finished = true;
        summary.status = finIt->second;
    }

    std::set<string> userNames;
    std::set<string> hubNames;

    for (const auto &p : downloads_) {
        const TransferEntry &e = p.second;
        if (e.target == target) {
            summary.totalCount++;
            if (!e.failed) {
                summary.activeCount++;
                summary.totalSpeed += e.speed;
            }
            summary.totalPosition += e.downloadPosition;
            if (e.size > summary.totalSize)
                summary.totalSize = e.size;
            if (!e.user.empty())
                userNames.insert(e.user);
            if (!e.hub.empty())
                hubNames.insert(e.hub);
        }
    }

    // Compute progress
    if (summary.totalSize > 0) {
        summary.progressPercent = static_cast<int>(
            summary.totalPosition * 100 / summary.totalSize);
    }

    // Compute time left
    if (summary.totalSpeed > 0 && summary.totalSize > summary.totalPosition) {
        summary.timeLeft = (summary.totalSize - summary.totalPosition) / summary.totalSpeed;
    }

    // Join user/hub names
    for (const auto &u : userNames) {
        if (!summary.users.empty())
            summary.users += ", ";
        summary.users += u;
    }
    for (const auto &h : hubNames) {
        if (!summary.hubs.empty())
            summary.hubs += ", ";
        summary.hubs += h;
    }

    return summary;
}

vector<string> TransferListModel::segmentedTargets() const
{
    // Count downloads per target
    map<string, int> targetCounts;
    for (const auto &p : downloads_) {
        if (!p.second.target.empty())
            targetCounts[p.second.target]++;
    }

    vector<string> result;
    for (const auto &tc : targetCounts) {
        if (tc.second > 1)
            result.push_back(tc.first);
    }
    return result;
}

void TransferListModel::finishTarget(const string &target, const string &status)
{
    finishedTargets_[target] = status;
}

} // namespace gtk_transfer
