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

#include "dcpp/stdinc.h"
#include "ShareBrowserModel.h"

#include <algorithm>
#include <cctype>
#include <numeric>

namespace gtk_share {

namespace {

/// Case-insensitive string compare.
int strcmpCI(const std::string &a, const std::string &b) {
    size_t n = std::min(a.size(), b.size());
    for (size_t i = 0; i < n; ++i) {
        int ca = std::tolower(static_cast<unsigned char>(a[i]));
        int cb = std::tolower(static_cast<unsigned char>(b[i]));
        if (ca != cb) return ca - cb;
    }
    if (a.size() < b.size()) return -1;
    if (a.size() > b.size()) return 1;
    return 0;
}

template <typename T>
int compare(T a, T b) {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

} // anonymous namespace

int compareItems(const ShareItem &a, const ShareItem &b, SortColumn col) {
    // Directories always sort before files
    if (a.type == ItemType::DIRECTORY && b.type != ItemType::DIRECTORY)
        return -1;
    if (a.type != ItemType::DIRECTORY && b.type == ItemType::DIRECTORY)
        return 1;

    // Same type — compare by column
    switch (col) {
        case SortColumn::SIZE:
        case SortColumn::EXACT_SIZE:
            return compare(a.size, b.size);

        case SortColumn::FILENAME:
        case SortColumn::TYPE:
        case SortColumn::TTH:
        default:
            return strcmpCI(a.name, b.name);
    }
}

int64_t computeTotalSize(const std::vector<ShareItem> &items) {
    int64_t total = 0;
    for (const auto &item : items)
        total += item.size;
    return total;
}

std::string fileOrderKey(const ShareItem &item) {
    return (item.type == ItemType::DIRECTORY ? "d" : "f") + item.name;
}

std::string typeString(const ShareItem &item) {
    if (item.type == ItemType::DIRECTORY)
        return "Directory";

    auto dot = item.name.rfind('.');
    if (dot == std::string::npos || dot + 1 >= item.name.size())
        return "File";

    std::string ext = item.name.substr(dot + 1);
    for (auto &c : ext)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return ext;
}

} // namespace gtk_share
