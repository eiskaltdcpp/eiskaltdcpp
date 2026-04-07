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
 * Widget-independent display formatting for ADL Search rules.
 *
 * Extracts the column formatting logic from ADLSearch::show_gui()
 * and setSearch_gui() into pure functions.  No GTK dependency.
 *
 * The SourceType → string and SizeType → string mappings live in dcpp
 * (ADLSearch.cpp).  This module provides the *display column* formatting
 * that the GTK view uses — i.e. converting rule data to the strings shown
 * in each column.
 */
namespace gtk_adl {

/// Source type values (mirrors dcpp::ADLSearch::SourceType without the dcpp dep)
enum class SourceType {
    OnlyFile    = 0,
    OnlyDirectory,
    FullPath
};

/// Size type values (mirrors dcpp::ADLSearch::SizeType)
enum class SizeType {
    Bytes      = 0,
    KibiBytes,
    MebiBytes,
    GibiBytes
};

/// Display-ready representation of an ADL search rule row.
struct AdlRuleDisplay {
    std::string searchString;
    std::string sourceTypeStr;   ///< "Filename", "Directory", or "Full Path"
    std::string destDir;
    std::string minSizeStr;      ///< e.g. "100 MiB" or "" if negative
    std::string maxSizeStr;
    bool isActive     = false;
    bool isAutoQueue  = false;
    // Raw values for hidden sort columns
    int64_t     minSize   = -1;
    int64_t     maxSize   = -1;
    SourceType  sourceType = SourceType::OnlyFile;
    SizeType    sizeType  = SizeType::Bytes;
};

/// Convert a SourceType to its display string.
std::string sourceTypeToString(SourceType t);

/// Parse a display string back to SourceType.
SourceType  stringToSourceType(const std::string &s);

/// Convert a SizeType to its display string.
std::string sizeTypeToString(SizeType t);

/// Parse a display string back to SizeType.
SizeType    stringToSizeType(const std::string &s);

/// Format a size column value.
/// If fileSize < 0, returns "".
/// Otherwise returns e.g. "100 MiB".
std::string formatSizeColumn(int64_t fileSize, SizeType sizeType);

/// Build a complete display row from raw rule data.
AdlRuleDisplay makeDisplay(const std::string &searchString,
                           bool isActive,
                           bool isAutoQueue,
                           SourceType sourceType,
                           const std::string &destDir,
                           int64_t minFileSize,
                           int64_t maxFileSize,
                           SizeType sizeType);

} // namespace gtk_adl
