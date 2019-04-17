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

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "Encoder.h"

namespace dcpp {

using std::find_if;

class CID {
public:
    enum { SIZE = 192 / 8 };

    CID() { memset(cid, 0, sizeof(cid)); }
    explicit CID(const uint8_t* data) { memcpy(cid, data, sizeof(cid)); }
    explicit CID(const string& base32) { Encoder::fromBase32(base32.c_str(), cid, sizeof(cid)); }

    bool operator==(const CID& rhs) const { return memcmp(cid, rhs.cid, sizeof(cid)) == 0; }
    bool operator<(const CID& rhs) const { return memcmp(cid, rhs.cid, sizeof(cid)) < 0; }

    string toBase32() const { return Encoder::toBase32(cid, sizeof(cid)); }
    string& toBase32(string& tmp) const { return Encoder::toBase32(cid, sizeof(cid), tmp); }

    size_t toHash() const {
        // RVO should handle this as efficiently as reinterpret_cast version
        size_t cidHash;
        memcpy(&cidHash, cid, sizeof(size_t));
        return cidHash;
    }
    const uint8_t* data() const { return cid; }

    explicit operator bool() const { return find_if(cid, cid + SIZE, [](uint8_t c) { return c != 0; }) != cid + SIZE; }

    static CID generate();

private:
    uint8_t cid[SIZE];
};

} // namespace dcpp

namespace std {
template<>
struct hash<dcpp::CID> {
    size_t operator()(const dcpp::CID& cid) const {
        return cid.toHash();
    }
};
}
