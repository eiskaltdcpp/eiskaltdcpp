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

#include "EmoticonLoader.h"
#include <dcpp/stdinc.h>
#include <dcpp/File.h>
#include <dcpp/SimpleXML.h>
#include <dcpp/Text.h>
#include <dcpp/Util.h>
#include <dcpp/debug.h>

#include <algorithm>
#include <filesystem>
#include <sys/stat.h>

using namespace std;
using namespace dcpp;

namespace gtk_emoticon {

vector<string> listPacks(const string &emoticonDir)
{
    vector<string> packs;

    try {
        // Ensure trailing separator so File::findFiles returns correct paths
        string dir = emoticonDir;
        if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
            dir += '/';

        StringList files = File::findFiles(dir, "*.xml");
        for (auto &it : files) {
            // Use std::filesystem to reliably extract just the filename,
            // regardless of mixed separator styles (e.g. Windows paths
            // containing both '/' and '\\').
            string file = std::filesystem::path(it).filename().string();
            string::size_type pos = file.rfind('.');
            if (pos != string::npos) {
                packs.push_back(file.substr(0, pos));
            }
        }
    } catch (const Exception &) {
        // Directory doesn't exist or isn't readable
    }

    return packs;
}

EmotPack loadPack(const string &xmlFilePath,
                  const string &emoticonDir,
                  bool checkFileExists)
{
    EmotPack pack;

    if (xmlFilePath.empty())
        return pack;

    set<string> filter;  // deduplicate emoticon names

    try {
        SimpleXML xml;
        xml.fromXML(File(xmlFilePath, File::READ, File::OPEN).read());

        if (xml.findChild("emoticons-map")) {
            string mapName = xml.getChildAttrib("name");
            pack.packName = mapName;
            string path = emoticonDir;
            if (!path.empty() && path.back() != '/' && path.back() != '\\')
                path += '/';
            path += mapName + "/";

            xml.stepIn();

            while (xml.findChild("emoticon")) {
                string emotFile = xml.getChildAttrib("file");
                string emotPath = path + emotFile;

                if (checkFileExists) {
                    struct stat st;
                    if (stat(emotPath.c_str(), &st) != 0)
                        continue;
                }

                EmotEntry entry;
                entry.file = emotFile;

                xml.stepIn();

                while (xml.findChild("name")) {
                    string emotName = xml.getChildAttrib("text");

                    if (emotName.empty() || filter.count(emotName))
                        continue;

                    // Check UTF-8 length limit (approximation: byte length up to 4x char count)
                    // Match original behavior: limit by character count
                    size_t charCount = 0;
                    for (size_t i = 0; i < emotName.size(); ) {
                        unsigned char c = static_cast<unsigned char>(emotName[i]);
                        if (c < 0x80) i += 1;
                        else if (c < 0xE0) i += 2;
                        else if (c < 0xF0) i += 3;
                        else i += 4;
                        charCount++;
                    }

                    if (charCount > static_cast<size_t>(MAX_NAME_LENGTH))
                        continue;

                    entry.names.push_back(emotName);
                    filter.insert(emotName);
                }

                if (!entry.names.empty()) {
                    pack.entries.push_back(std::move(entry));
                    pack.fileCount++;
                }
                xml.stepOut();
            }
            xml.stepOut();
        }
    } catch (const Exception &e) {
        dcdebug("EmoticonLoader: %s...\n", e.getError().c_str());
        pack.entries.clear();
        pack.fileCount = 0;
    }

    return pack;
}

set<string> collectTriggers(const EmotPack &pack)
{
    set<string> triggers;
    for (const auto &entry : pack.entries) {
        for (const auto &name : entry.names) {
            triggers.insert(name);
        }
    }
    return triggers;
}

} // namespace gtk_emoticon
