/*
 * Copyright (C) 2009-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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

#include "DCContext.h"
#include "Socket.h"
#include "TimerManager.h"
#include "SettingsManager.h"
#include "DCPlusPlus.h"

namespace dcpp
{
/**
 * Manager for throttling traffic flow.
 * Inspired by Token Bucket algorithm: https://en.wikipedia.org/wiki/Token_bucket
 */
class ThrottleManager :
        private TimerManagerListener, public ContextAware
{
public:

    /*
     * Throttles traffic and reads a packet from the network
     */
    int read(Socket* sock, void* buffer, size_t len);

    /*
     * Throttles traffic and writes a packet to the network
     * Handle this a little bit differently than downloads due to OpenSSL stupidity
     */
    int write(Socket* sock, void* buffer, size_t& len);

    void shutdown();

    SettingsManager::IntSetting getCurSetting(SettingsManager::IntSetting setting);

    int getUpLimit();
    int getDownLimit();

    void setSetting(SettingsManager::IntSetting setting, int value);

private:
    // stack up throttled read & write threads
    CriticalSection stateCS;
    CriticalSection waitCS[2];
    long activeWaiter;

    // shutdown synchronization — the timer callback thread owns the
    // waitCS locks, so shutdown() must cooperatively ask it to release
    // them rather than calling unlock() from a non-owning thread (which
    // is undefined behavior with std::recursive_mutex).
    CriticalSection shutdownCS;
    long n_lock, halt;

    // download limiter
    CriticalSection downCS;
    int64_t         downTokens;

    // upload limiter
    CriticalSection upCS;
    int64_t         upTokens;

public:
    explicit ThrottleManager(DCContext& ctx) : ContextAware(ctx), activeWaiter(-1), n_lock(0), halt(0), downTokens(0), upTokens(0)
    {
        this->ctx().getTimerManager()->addListener(this);
    }

    virtual ~ThrottleManager();

private:

    bool getCurThrottling();
    void waitToken();

    // TimerManagerListener
    void on(TimerManagerListener::Second, uint64_t /* aTick */);
};

}   // namespace dcpp
