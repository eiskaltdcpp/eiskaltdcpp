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

/**
 * Download queue parameter helpers — extracted from DownloadQueue::getQueueParams_client().
 * No GTK dependency — only dcpp types and standard C++.
 *
 * StringMap keys produced by the full paramsFromQueueItem():
 *   "Filename", "Path", "Target", "Users", "Status",
 *   "Size Sort", "Size", "Exact Size",
 *   "Downloaded Sort", "Downloaded",
 *   "Priority", "Errors", "Added", "TTH"
 */
namespace gtk_queue {

/// Map a QueueItem::Priority value to a display string.
/// Values: PAUSED → "Paused", LOWEST → "Lowest", LOW → "Low",
///         HIGH → "High", HIGHEST → "Highest", default → "Normal"
std::string priorityString(int priority);

/// Format the status string.
/// @param online    Number of online sources
/// @param total     Total number of sources
/// @param isRunning Whether the queue item is actively downloading
std::string statusString(int online, int total, bool isRunning);

/// Format the "Downloaded" display string.
/// @param downloaded  Bytes downloaded so far
/// @param size        Total file size
/// Returns e.g. "1.23 MiB (45.67%)"
std::string downloadedString(int64_t downloaded, int64_t size);

/// Build a source error string from error flags.
/// @param nick       Nick of the bad source
/// @param flags      QueueItem::Source flags
/// Returns e.g. "nick (File not available)"
/// Flag constants from QueueItem::Source:
///   FLAG_FILE_NOT_AVAILABLE, FLAG_PASSIVE, FLAG_CRC_FAILED,
///   FLAG_BAD_TREE, FLAG_SLOW_SOURCE, FLAG_NO_TTHF
std::string sourceErrorString(const std::string &nick, int flags);

} // namespace gtk_queue
