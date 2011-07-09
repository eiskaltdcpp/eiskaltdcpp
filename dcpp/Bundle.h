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

#ifndef DCPLUSPLUS_DCPP_BUNDLE_H_
#define DCPLUSPLUS_DCPP_BUNDLE_H_

#include <string>
#include <set>

#include "TigerHash.h"
#include "MerkleTree.h"
#include "Pointer.h"

namespace dcpp {

using std::string;

/**
 * A bundle is a set of related files that can be searched for by a single hash value.
 *
 * The hash is defined as follows:
 * For each file in the set, ordered by name (byte-order, not linguistic), except those specially marked,
 * compute the compute the hash. Then calculate the combined hash value by passing the concatenated hashes
 * of each file through the hash function.
 */
class Bundle : public intrusive_ptr_base<Bundle> {
public:
        struct Entry {
                Entry() { }
                Entry(const string& name, int64_t size, TTHValue tth, bool include) : name(name), size(size), tth(tth), include(include) { }
                string name;
                int64_t size;
                TTHValue tth;
                bool include;

                friend bool operator<(const Entry &lhs, const Entry& rhs) { return lhs.name < rhs.name; }
        };

        TTHValue getHash() const;
        bool contains(const TTHValue& tth) const;

        string name;

        std::set<Entry> entries;
};

}

#endif /* DCPLUSPLUS_DCPP_BUNDLE_H_ */
