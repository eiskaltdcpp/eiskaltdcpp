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

#include "stdinc.h"

#include "TimerManager.h"

namespace dcpp {

static auto g_start = std::chrono::high_resolution_clock::now();
static volatile bool g_running = false;

TimerManager::TimerManager() {
    // This mutex will be unlocked only upon shutdown
    mtx.lock();
    g_running = true;
}

TimerManager::~TimerManager() {
    dcassert(listeners.empty());
}

void TimerManager::shutdown() {
    mtx.unlock();
    join();
}

int TimerManager::run() {
    setThreadName("TimerManager");
    auto now = std::chrono::high_resolution_clock::now();
    auto nextSecond = now + std::chrono::seconds(1);
    auto nextMin = now + std::chrono::minutes(1);
    
    while(!mtx.try_lock_until(nextSecond)) {
        now = std::chrono::high_resolution_clock::now();
        nextSecond += std::chrono::seconds(1);
        if (nextSecond <= now)
        {
            nextSecond = now + std::chrono::seconds(1);
        }
        const auto t = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_start).count();
        fire(TimerManagerListener::Second(), t);
        if (nextMin <= now)
        {
            nextMin += std::chrono::minutes(1);
            fire(TimerManagerListener::Minute(), t);
        }
    }
    mtx.unlock();
    g_running = false;

    dcdebug("TimerManager done\n");
    return 0;
}

uint64_t TimerManager::getTick() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - g_start).count();
}

} // namespace dcpp
