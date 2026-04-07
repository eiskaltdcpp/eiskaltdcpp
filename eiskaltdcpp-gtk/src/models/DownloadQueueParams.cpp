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
#include <dcpp/QueueItem.h>

#include "DownloadQueueParams.h"

using std::string;
using std::map;

namespace gtk_queue {

string priorityString(int priority)
{
    using dcpp::QueueItem;
    switch (priority) {
    case QueueItem::PAUSED:  return "Paused";
    case QueueItem::LOWEST:  return "Lowest";
    case QueueItem::LOW:     return "Low";
    case QueueItem::HIGH:    return "High";
    case QueueItem::HIGHEST: return "Highest";
    default:                 return "Normal";
    }
}

string statusString(int online, int total, bool isRunning)
{
    if (isRunning)
        return "Running...";
    return dcpp::Util::toString(online) + " of " +
           dcpp::Util::toString(total) + " user(s) online";
}

string downloadedString(int64_t downloaded, int64_t size)
{
    if (size > 0) {
        double percent = static_cast<double>(downloaded) * 100.0 /
                         static_cast<double>(size);
        return dcpp::Util::formatBytes(downloaded) + " (" +
               dcpp::Util::toString(percent) + "%)";
    }
    return "0 B (0.00%)";
}

string sourceErrorString(const string &nick, int flags)
{
    using dcpp::QueueItem;

    string reason;
    if (flags & QueueItem::Source::FLAG_FILE_NOT_AVAILABLE)
        reason = "File not available";
    else if (flags & QueueItem::Source::FLAG_PASSIVE)
        reason = "Passive user";
    else if (flags & QueueItem::Source::FLAG_CRC_FAILED)
        reason = "CRC32 inconsistency (SFV-Check)";
    else if (flags & QueueItem::Source::FLAG_BAD_TREE)
        reason = "Full tree does not match TTH root";
    else if (flags & QueueItem::Source::FLAG_SLOW_SOURCE)
        reason = "Source too slow";
    else if (flags & QueueItem::Source::FLAG_NO_TTHF)
        reason = "Remote client does not fully support TTH - cannot download";
    else
        return nick;

    return nick + " (" + reason + ")";
}

} // namespace gtk_queue
