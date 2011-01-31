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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPLUSPLUS_DCPP_CID_H
#define DCPLUSPLUS_DCPP_CID_H

#include "Encoder.h"
#include "Util.h"

namespace dcpp {

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

    bool isZero() const { return find_if(cid, cid+SIZE, bind2nd(not_equal_to<uint8_t>(), 0)) == (cid+SIZE); }

    static CID generate() {
        uint8_t data[CID::SIZE];
        for(size_t i = 0; i < sizeof(data); ++i) {
            data[i] = (uint8_t)Util::rand();
        }
        return CID(data);
    }

private:
    uint8_t cid[SIZE];
};

} // namespace dcpp

namespace std { namespace tr1 {
template<>
struct hash<dcpp::CID> {
    size_t operator()(const dcpp::CID& rhs) const {
        size_t hvHash;
        memcpy(&hvHash, rhs.data(), sizeof(size_t));
        return hvHash;
    }
};
}
}

#endif // !defined(CID_H)
