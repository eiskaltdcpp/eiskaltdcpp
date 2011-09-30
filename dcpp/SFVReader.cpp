/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"

#include "SFVReader.h"

#include "StringTokenizer.h"
#include "File.h"

#ifndef _WIN32
#include <dirent.h>
#include <fnmatch.h>
#endif

namespace dcpp {

bool SFVReader::tryFile(const string& sfvFile, const string& fileName) {

    string sfv = File(sfvFile, File::READ, File::OPEN).read();

    string::size_type i = 0;
    while( (i = Util::findSubString(sfv, fileName, i)) != string::npos) {
        // Either we're at the beginning of the file or the line...otherwise skip...
        if( (i == 0) || (sfv[i-1] == '\n') ) {
            string::size_type j = i + fileName.length() + 1;
            if(j < sfv.length() - 8) {
                sscanf(sfv.c_str() + j, "%x", &crc32);
                crcFound = true;
                return true;
            }
        }
        i += fileName.length();
    }

    return false;
}

void SFVReader::load(const string& fileName) noexcept {
    string path = Util::getFilePath(fileName);
    string fname = Util::getFileName(fileName);
    StringList files = File::findFiles(path, "*.sfv");

    for(StringIter i = files.begin(); i != files.end(); ++i) {
        try {
            if (tryFile(*i, fname)) {
                return;
            }
        } catch(const FileException&) {
            // Ignore...
        }
    }
}

} // namespace dcpp
