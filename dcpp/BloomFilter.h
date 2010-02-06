/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_BLOOM_FILTER_H
#define DCPLUSPLUS_DCPP_BLOOM_FILTER_H

#include "ZUtils.h"

namespace dcpp {

template<size_t N>
class BloomFilter {
public:
	BloomFilter(size_t tableSize) { table.resize(tableSize); }
	~BloomFilter() { }

	void add(const string& s) { xadd(s, N); }
	bool match(const StringList& s) const {
		for(StringList::const_iterator i = s.begin(); i != s.end(); ++i) {
			if(!match(*i))
				return false;
		}
		return true;
	}
	bool match(const string& s) const {
		if(s.length() >= N) {
			string::size_type l = s.length() - N;
			for(string::size_type i = 0; i <= l; ++i) {
				if(!table[getPos(s, i, N)]) {
					return false;
				}
			}
		}
		return true;
	}
	void clear() {
		size_t s = table.size();
		table.clear();
		table.resize(s);
	}
#ifdef TESTER
	void print_table_status() {
		int tot = 0;
		for (unsigned int i = 0; i < table.size(); ++i) if (table[i] == true) ++tot;

		std::cout << "table status: " << tot << " of " << table.size()
			<< " filled, for an occupancy percentage of " << (100.*tot)/table.size()
			<< "%" << std::endl;
	}
#endif
private:
	void xadd(const string& s, size_t n) {
		if(s.length() >= n) {
			string::size_type l = s.length() - n;
			for(string::size_type i = 0; i <= l; ++i) {
				table[getPos(s, i, n)] = true;
			}
		}
	}

	/* This is roughly how boost::hash does it */
	size_t getPos(const string& s, size_t i, size_t l) const {
		size_t h = 0;
		const char* c = s.data() + i;
		const char* end = s.data() + i + l;
		for(; c < end; ++c) {
			h ^= *c + 0x9e3779b9 + (h<<6) + (h>>2);
		}
		return (h % table.size());
	}

	vector<bool> table;
};

} // namespace dcpp

#endif // !defined(BLOOM_FILTER_H)
