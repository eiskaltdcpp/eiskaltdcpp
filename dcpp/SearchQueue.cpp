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

#include "stdinc.h"

#include "SearchQueue.h"

#include "TimerManager.h"
#include "QueueManager.h"
#include "SearchManager.h"

namespace dcpp {

bool SearchQueue::add(const SearchCore& s)
{
    dcassert(s.owners.size() == 1);
    dcassert(interval >= 10000); // min interval is 15 seconds

    Lock l(cs);

    for(deque<SearchCore>::iterator i = searchQueue.begin(); i != searchQueue.end(); i++)
    {
        // check dupe
        if(*i == s) {
            void* aOwner = *s.owners.begin();
            i->owners.insert(aOwner);

            // if previous search was autosearch and current one isn't, it should be readded before autosearches
            if(s.token != "auto" && i->token == "auto")
            {
                searchQueue.erase(i);
                break;
            }

            return false;
        }
    }

    if(s.token == "auto") {
        // Insert last (automatic search)
        searchQueue.push_back(s);
    } else {
        bool added = false;
        if(searchQueue.empty()) {
            searchQueue.push_front(s);
            added = true;
        } else {
            // Insert before the automatic searches (manual search)
            for(deque<SearchCore>::iterator i = searchQueue.begin(); i != searchQueue.end(); i++) {
                if(i->token == "auto") {
                    searchQueue.insert(i, s);
                    added = true;
                    break;
                }
            }
        }
        if (!added) {
            searchQueue.push_back(s);
        }
    }
    return true;
}

bool SearchQueue::pop(SearchCore& s)
{
    dcassert(interval);

    uint64_t now = GET_TICK();
    if(now <= lastSearchTime + interval)
        return false;

    {
        Lock l(cs);
        if(!searchQueue.empty()){
            s = searchQueue.front();
            searchQueue.pop_front();
            lastSearchTime = now;
            return true;
        }
    }

    return false;
}

uint64_t SearchQueue::getSearchTime(void* aOwner){
    Lock l(cs);

    if(aOwner == 0) return 0xFFFFFFFF;

    uint64_t x = max(lastSearchTime, GET_TICK() - interval);

    for(deque<SearchCore>::iterator i = searchQueue.begin(); i != searchQueue.end(); i++){
        x += interval;

        if(i->owners.count(aOwner))
            return x;
        else if(i->owners.empty())
            break;
    }

    return 0;
}

bool SearchQueue::cancelSearch(void* aOwner){
    dcassert(aOwner);

    Lock l(cs);
    for(deque<SearchCore>::iterator i = searchQueue.begin(); i != searchQueue.end(); i++){
        if(i->owners.count(aOwner)){
            i->owners.erase(aOwner);
            if(i->owners.empty())
                searchQueue.erase(i);
            return true;
        }
    }
    return false;
}

}
