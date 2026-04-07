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
#include "AdlSearchDisplay.h"

#include <sstream>

namespace gtk_adl {

std::string sourceTypeToString(SourceType t) {
    switch (t) {
        case SourceType::OnlyFile:      return "Filename";
        case SourceType::OnlyDirectory: return "Directory";
        case SourceType::FullPath:      return "Full Path";
    }
    return "Filename";
}

SourceType stringToSourceType(const std::string &s) {
    if (s == "Directory")  return SourceType::OnlyDirectory;
    if (s == "Full Path")  return SourceType::FullPath;
    return SourceType::OnlyFile;  // default
}

std::string sizeTypeToString(SizeType t) {
    switch (t) {
        case SizeType::Bytes:     return "B";
        case SizeType::KibiBytes: return "KiB";
        case SizeType::MebiBytes: return "MiB";
        case SizeType::GibiBytes: return "GiB";
    }
    return "B";
}

SizeType stringToSizeType(const std::string &s) {
    if (s == "KiB") return SizeType::KibiBytes;
    if (s == "MiB") return SizeType::MebiBytes;
    if (s == "GiB") return SizeType::GibiBytes;
    return SizeType::Bytes;  // default
}

std::string formatSizeColumn(int64_t fileSize, SizeType sizeType) {
    if (fileSize < 0) return "";
    std::ostringstream oss;
    oss << fileSize << " " << sizeTypeToString(sizeType);
    return oss.str();
}

AdlRuleDisplay makeDisplay(const std::string &searchString,
                           bool isActive,
                           bool isAutoQueue,
                           SourceType sourceType,
                           const std::string &destDir,
                           int64_t minFileSize,
                           int64_t maxFileSize,
                           SizeType sizeType) {
    AdlRuleDisplay d;
    d.searchString  = searchString;
    d.isActive      = isActive;
    d.isAutoQueue   = isAutoQueue;
    d.sourceType    = sourceType;
    d.sourceTypeStr = sourceTypeToString(sourceType);
    d.destDir       = destDir;
    d.minSize       = minFileSize;
    d.maxSize       = maxFileSize;
    d.sizeType      = sizeType;
    d.minSizeStr    = formatSizeColumn(minFileSize, sizeType);
    d.maxSizeStr    = formatSizeColumn(maxFileSize, sizeType);
    return d;
}

} // namespace gtk_adl
