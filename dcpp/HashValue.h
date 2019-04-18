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

#include "FastAlloc.h"
#include "Encoder.h"

namespace dcpp {

template<class Hasher>
struct HashValue : FastAlloc<HashValue<Hasher> >{
    static const size_t BITS = Hasher::BITS;
    static const size_t BYTES = Hasher::BYTES;

    HashValue() { memset(data, 0, BYTES); }
    explicit HashValue(const uint8_t* aData) { memcpy(data, aData, BYTES); }
    explicit HashValue(const std::string& base32) { Encoder::fromBase32(base32.c_str(), data, BYTES); }

    bool operator!=(const HashValue& rhs) const { return !(*this == rhs); }
    bool operator==(const HashValue& rhs) const { return memcmp(data, rhs.data, BYTES) == 0; }
    bool operator<(const HashValue& rhs) const { return memcmp(data, rhs.data, BYTES) < 0; }

    std::string toBase32() const { return Encoder::toBase32(data, BYTES); }
    std::string& toBase32(std::string& tmp) const { return Encoder::toBase32(data, BYTES, tmp); }

    explicit operator bool() const { return find_if(data, data + BYTES, [](uint8_t c) { return c != 0; }) != data + BYTES; }

    uint8_t data[BYTES];
};

} // namespace dcpp

namespace std {
template<typename T>
struct hash<dcpp::HashValue<T> > {
    size_t operator()(const dcpp::HashValue<T>& rhs) const {
        // RVO should handle this as efficiently as reinterpret_cast
        size_t hvHash;
        memcpy(&hvHash, rhs.data, sizeof(size_t));
        return hvHash;
    }
};

}
