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

/**
 * Pure formatting functions for the GTK frontend.
 * No GTK dependency — only standard C++ and dcpp::Util.
 *
 * These wrap dcpp::Util formatters with GTK-specific conventions
 * (e.g., speed suffixed with "/s").
 */
namespace gtk_fmt {

/// Format bytes-per-second as a human-readable speed string, e.g. "1.23 MiB/s"
/// Returns empty string for negative values.
std::string formatSpeed(int64_t bytesPerSec);

/// Format a byte count as a human-readable size string, e.g. "4.56 GiB"
std::string formatSize(int64_t bytes);

/// Format a duration in seconds as a human-readable time string, e.g. "2h 15m"
std::string formatTimeLeft(int64_t seconds);

/// Format download/upload progress as a percentage string, e.g. "45.2%"
/// Returns "0.0%" if size <= 0. Clamps to 100.0%.
std::string formatProgress(int64_t pos, int64_t size);

/// Format an exact byte count as "N B" with no unit conversion.
/// Useful for sizes that should always show exact counts.
std::string formatExactSize(int64_t bytes);

} // namespace gtk_fmt
