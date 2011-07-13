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

#include "stdinc.h"

#include "TimerManager.h"

#ifndef TIMER_OLD_BOOST
#include <boost/date_time/posix_time/ptime.hpp>
#endif
namespace dcpp {

#ifdef TIMER_OLD_BOOST
timeval TimerManager::tv;
#else
using namespace boost::posix_time;
#endif

TimerManager::TimerManager() {
#ifdef TIMER_OLD_BOOST
    gettimeofday(&tv, NULL);
#else
        // This mutex will be unlocked only upon shutdown
        boostmtx.lock();
#endif
}

TimerManager::~TimerManager() {
        dcassert(listeners.size() == 0);
}

void TimerManager::shutdown() {
#ifdef TIMER_OLD_BOOST
    s.signal();
#else
    boostmtx.unlock();
#endif
        join();
}

int TimerManager::run() {
    int nextMin = 0;
#ifdef TIMER_OLD_BOOST
    uint64_t x = getTick();
    uint64_t nextTick = x + 1000;

    while(!s.wait(nextTick > x ? nextTick - x : 0)) {
        uint64_t z = getTick();
        nextTick = z + 1000;
        fire(TimerManagerListener::Second(), z);
        if(nextMin++ >= 60) {
            fire(TimerManagerListener::Minute(), z);
             nextMin = 0;
         }
        x = getTick();
    }
#else
    ptime now = microsec_clock::universal_time();
    ptime nextSecond = now + seconds(1);

    while(!boostmtx.timed_lock(nextSecond)) {
        uint64_t t = getTick();
        now = microsec_clock::universal_time();
        nextSecond += seconds(1);
        if(nextSecond < now) {
                nextSecond = now;
        }

        fire(TimerManagerListener::Second(), t);
        if(nextMin++ >= 60) {
            fire(TimerManagerListener::Minute(), t);
            nextMin = 0;
        }
    }
#endif

    dcdebug("TimerManager done\n");
    return 0;
}

uint64_t TimerManager::getTick() {
#ifdef TIMER_OLD_BOOST
        timeval tv2;
        gettimeofday(&tv2, NULL);
        return static_cast<uint64_t>(((tv2.tv_sec - tv.tv_sec) * 1000 ) + ( (tv2.tv_usec - tv.tv_usec) / 1000));
#else
        static ptime start = microsec_clock::universal_time();
        return (microsec_clock::universal_time() - start).total_milliseconds();
#endif
}

} // namespace dcpp
