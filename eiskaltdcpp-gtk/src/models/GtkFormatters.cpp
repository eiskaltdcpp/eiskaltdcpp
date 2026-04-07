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

#include "GtkFormatters.h"
#include <dcpp/stdinc.h>
#include <dcpp/Util.h>
#include <cstdio>

using namespace dcpp;

namespace gtk_fmt {

std::string formatSpeed(int64_t bytesPerSec)
{
    if (bytesPerSec < 0)
        return std::string();

    return Util::formatBytes(bytesPerSec) + "/s";
}

std::string formatSize(int64_t bytes)
{
    return Util::formatBytes(bytes);
}

std::string formatTimeLeft(int64_t seconds)
{
    return Util::formatSeconds(seconds);
}

std::string formatProgress(int64_t pos, int64_t size)
{
    double pct = 0.0;
    if (size > 0) {
        pct = static_cast<double>(pos) * 100.0 / static_cast<double>(size);
        if (pct > 100.0)
            pct = 100.0;
        if (pct < 0.0)
            pct = 0.0;
    }

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f%%", pct);
    return std::string(buf);
}

std::string formatExactSize(int64_t bytes)
{
    return Util::formatExactSize(bytes);
}

} // namespace gtk_fmt
