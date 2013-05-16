/*
 * Copyright (C) 2003-2006 RevConnect, http://www.revconnect.com
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

#pragma once

#include "MerkleTree.h"

namespace dcpp {

struct SearchCore
{
    int32_t     sizeType;
    int64_t     size;
    int32_t     fileType;
    string      query;
    string      token;
    StringList  exts;
    std::unordered_set<void*>  owners;

    bool operator==(const SearchCore& rhs) const {
         return this->sizeType == rhs.sizeType &&
                this->size == rhs.size &&
                this->fileType == rhs.fileType &&
                this->query == rhs.query &&
                this->token == rhs.token;
    }
};

class SearchQueue
{
public:

    SearchQueue(uint32_t aInterval = 0)
        : interval(aInterval), lastSearchTime(0)
    {
    }

    bool add(const SearchCore& s);
    bool pop(SearchCore& s, uint64_t now);

    void clear()
    {
        Lock l(cs);
        searchQueue.clear();
    }

    bool cancelSearch(void* aOwner);

    /** return 0 means not in queue */
    uint64_t getSearchTime(void* aOwner, uint64_t now);

    /**
        by milli-seconds
        0 means no interval, no auto search and manual search is sent immediately
    */
    uint32_t interval;

private:
    deque<SearchCore>   searchQueue;
    uint64_t       lastSearchTime;
    CriticalSection cs;
};

}
