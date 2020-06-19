/*
 * Copyright (C) 2001-2019 Jacek Sieka, arnetheduck on gmail point com
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

#pragma once

#include <cstdint>
#include <string>

namespace dcpp {

using std::string;

class SFVReader {
public:
    /** @see load */
    SFVReader(const string& aFileName) : crc32(0), crcFound(false) { load(aFileName); }

    /**
     * Search for a CRC32 file in all .sfv files in the directory of fileName.
     * Each SFV file has a number of lines containing a filename and its CRC32 value
     * in the form:
     * filename.ext<whitespaces>xxxxxxxx
     * where the x's represent the file's crc32 value. Lines starting with ';' are
     * considered comments but we throw away any lines that do not start with
     * a file name that resides in the same directory as the .sfv file anyway.
     * https://en.wikipedia.org/wiki/Simple_file_verification
     */
    void load(const string& fileName) noexcept;

    bool hasCRC() const noexcept { return crcFound; }
    uint32_t getCRC() const noexcept { return crc32; }

private:

    uint32_t crc32;
    bool crcFound;

    bool tryFile(const string& sfvFile, const string& fileName);

};

} // namespace dcpp
