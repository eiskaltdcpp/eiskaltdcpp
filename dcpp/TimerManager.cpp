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
#include "DCPlusPlus.h"

#include "TimerManager.h"

#include <boost/date_time/posix_time/ptime.hpp>

namespace dcpp {

using namespace boost::posix_time;

TimerManager::TimerManager() {
        // This mutex will be unlocked only upon shutdown
        boostmtx.lock();
}

TimerManager::~TimerManager() throw() {
        dcassert(listeners.size() == 0);
}

void TimerManager::shutdown() {
        boostmtx.unlock();
        join();
}

int TimerManager::run() {
    int nextMin = 0;

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

        dcdebug("TimerManager done\n");
    return 0;
}

uint64_t TimerManager::getTick() {
        static ptime start = microsec_clock::universal_time();
        return (microsec_clock::universal_time() - start).total_milliseconds();
}

} // namespace dcpp
