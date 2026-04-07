/*
 * Copyright © 2009-2010 freedcpp, https://github.com/eiskaltdcpp/freedcpp
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

#include <set>
#include <string>
#include <vector>

/**
 * Widget-independent emoticon pack loading.
 *
 * Parses emoticon XML pack files and provides the data
 * (emoticon names and image file paths) without any GTK dependency.
 * The UI layer is responsible for loading actual images (GdkPixbuf, etc.).
 */
namespace gtk_emoticon {

/// Maximum number of emoticons per pack (matches EMOTICONS_MAX in the original)
constexpr int EMOTICONS_MAX = 48;

/// Maximum length of a single emoticon text name
constexpr int MAX_NAME_LENGTH = 24;

/// Represents a single emoticon entry from a pack file
struct EmotEntry {
    std::vector<std::string> names;   ///< Text triggers for this emoticon (e.g., ":)", ":-)")
    std::string file;                  ///< Image filename within the pack directory
};

/// Result of loading an emoticon pack
struct EmotPack {
    std::string packName;              ///< Name of the pack (from pack directory name)
    std::vector<EmotEntry> entries;    ///< All emoticons in the pack
    int fileCount = 0;                 ///< Number of emoticons loaded
};

/// List available emoticon pack names from a base directory.
/// Looks for *.xml files and returns their basenames (without .xml extension).
std::vector<std::string> listPacks(const std::string &emoticonDir);

/// Parse an emoticon pack XML file.
/// Returns the parsed pack. entries will be empty on failure.
/// @param xmlFilePath   Full path to the .xml pack file
/// @param emoticonDir   Base directory containing emoticon pack subdirectories
/// @param checkFileExists  If true, skips entries whose image file doesn't exist on disk.
///                         Set to false for testing without actual image files.
EmotPack loadPack(const std::string &xmlFilePath,
                  const std::string &emoticonDir,
                  bool checkFileExists = true);

/// Collect all unique emoticon text triggers from a pack.
/// Useful for building a pattern-matching table.
std::set<std::string> collectTriggers(const EmotPack &pack);

} // namespace gtk_emoticon
