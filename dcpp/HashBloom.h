/*
* Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef HASHBLOOM_H_
#define HASHBLOOM_H_

#include "MerkleTree.h"

namespace dcpp {
/**
 * According to http://www.eecs.harvard.edu/~michaelm/NEWWORK/postscripts/BloomFilterSurvey.pdf
 * the optimal number of hashes k is (m/n)*ln(2), m = number of bits in the filter and n = number
 * of items added. The largest k that we can get from a single TTH value depends on the number of
 * bits we need to address the bloom structure, which in turn depends on m, so the optimal size
 * for our filter is m = n * k / ln(2) where n is the number of TTH values, or in our case, number of
 * files in share since each file is identified by one TTH value. We try that for each even dividend
 * of the key size (2, 3, 4, 6, 8, 12) and if m fits within the bits we're able to address (2^(keysize/k)),
 * we can use that value when requesting the bloom filter.
 */
class HashBloom {
public:
    HashBloom() : k(0), h(0) { }

    /** Return a suitable value for k based on n */
    static size_t get_k(size_t n, size_t h);
    /** Optimal number of bits to allocate for n elements when using k hashes */
    static uint64_t get_m(size_t n, size_t k);

    void add(const TTHValue& tth);
    bool match(const TTHValue& tth) const;
    void reset(size_t k, size_t m, size_t h);
    void push_back(bool v);

    void copy_to(ByteVector& v) const;
private:

    size_t pos(const TTHValue& tth, size_t n) const;

    std::vector<bool> bloom;
    size_t k;
    size_t h;
};

}

#endif /*HASHBLOOM_H_*/
