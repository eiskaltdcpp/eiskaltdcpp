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
 * Widget-independent share browser item model and comparison logic.
 *
 * Extracts the sorting / comparison logic from
 * ShareBrowser::ItemInfo::compareItems() and display helper logic from
 * ShareBrowser::updateFiles_gui() into pure functions.  No GTK dependency.
 */
namespace gtk_share {

/// Item type for share browser entries.
enum class ItemType {
    FILE,
    DIRECTORY
};

/// Lightweight representation of a share browser item.
struct ShareItem {
    std::string name;
    int64_t     size = 0;        ///< File size, or total directory size
    std::string tth;             ///< TTH hash string (files only)
    ItemType    type = ItemType::FILE;
    int64_t     timestamp = 0;   ///< Shared time (epoch) for files
};

/// Column identifiers for sorting (mirrors ShareBrowser column enum).
enum class SortColumn {
    FILENAME,
    TYPE,
    SIZE,
    EXACT_SIZE,
    TTH
};

/**
 * Compare two share items for sorting.
 *
 * Rule: directories always sort before files.
 * Within same type, comparison depends on column:
 * - FILENAME / TYPE / TTH: compare by name (case-insensitive)
 * - SIZE / EXACT_SIZE: compare by size
 *
 * Returns negative if a < b, 0 if equal, positive if a > b.
 */
int compareItems(const ShareItem &a, const ShareItem &b, SortColumn col);

/**
 * Compute total size of a list of items.
 */
int64_t computeTotalSize(const std::vector<ShareItem> &items);

/**
 * Generate the file-order sort key used by the GTK TreeView.
 *
 * Directories get "d" + name, files get "f" + name.
 * This ensures directories sort before files in an alphabetical sort.
 */
std::string fileOrderKey(const ShareItem &item);

/**
 * Generate a human-readable type string for the item.
 *
 * Directories → "Directory"
 * Files → lowercase extension (e.g. "txt", "mp3") or "File" if no extension.
 */
std::string typeString(const ShareItem &item);

} // namespace gtk_share
