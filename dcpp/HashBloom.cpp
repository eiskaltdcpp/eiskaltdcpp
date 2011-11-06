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

#include "HashBloom.h"

#include <cmath>

namespace dcpp {

size_t HashBloom::get_k(size_t n, size_t h) {
    for(size_t k = TTHValue::BITS/h; k > 1; --k) {
        uint64_t m = get_m(n, k);
        if(m >> 24 == 0) {
            return k;
        }
    }
    return 1;
}

uint64_t HashBloom::get_m(size_t n, size_t k) {
    uint64_t m = (static_cast<uint64_t>(ceil(static_cast<double>(n) * k / log(2.))));
    // 64-bit boundary as per spec
    return ((m + 63 )/ 64) * 64;
}

void HashBloom::add(const TTHValue& tth) {
    for(size_t i = 0; i < k; ++i) {
        bloom[pos(tth, i)] = true;
    }
}

bool HashBloom::match(const TTHValue& tth) const {
    if(bloom.empty()) {
        return false;
    }
    for(size_t i = 0; i < k; ++i) {
        if(!bloom[pos(tth, i)]) {
            return false;
        }
    }
    return true;
}

void HashBloom::push_back(bool v) {
    bloom.push_back(v);
}

void HashBloom::reset(size_t k_, size_t m, size_t h_) {
    bloom.resize(m);
    k = k_;
    h = h_;
}

size_t HashBloom::pos(const TTHValue& tth, size_t n) const {
    if((n+1)*h > TTHValue::BITS) {
        return 0;
    }

    uint64_t x = 0;

    size_t start = n * h;
    for(size_t i = 0; i < h; ++i) {
        size_t bit = start + i;
        size_t byte = bit / 8;
        size_t pos = bit % 8;

        if(tth.data[byte] & (1 << pos)) {
            x |= (1 << i);
        }
    }
    return x % bloom.size();
}

void HashBloom::copy_to(ByteVector& v) const {
    v.resize(bloom.size() / 8);
    for(size_t i = 0; i < bloom.size(); ++i) {
        v[i/8] |= bloom[i] << (i % 8);
    }
}

}
