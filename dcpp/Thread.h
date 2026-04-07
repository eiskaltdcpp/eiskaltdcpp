/*
 * Copyright (C) 2001-2019 Jacek Sieka, arnetheduck on gmail point com
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

#include <chrono>
#include <cstdint>
#include <thread>
#include <version>

#ifndef _WIN32
#include <pthread.h>
#endif

#include "Exception.h"

namespace dcpp {

STANDARD_EXCEPTION(ThreadException);

/**
 * Base class for objects that run work on a background thread.
 *
 * On POSIX, threads are created via pthread_create with a reduced
 * 1 MiB stack (the default 8 MiB causes excessive virtual-memory
 * consumption on busy NMDC hubs where each peer connection spawns
 * its own BufferedSocket thread).
 */
class Thread {
public:
    enum Priority { IDLE = 1, LOW = 1, NORMAL = 0, HIGH = -1 };

    Thread() = default;
    virtual ~Thread() { join(); }

    // Non-copyable
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    void start();

    void join() {
#ifndef _WIN32
        if (threadActive_) {
            pthread_join(thread_, nullptr);
            threadActive_ = false;
        }
#else
        if (thread_.joinable()) {
            thread_.join();
        }
#endif
    }

    void detach() {
#ifndef _WIN32
        if (threadActive_) {
            pthread_detach(thread_);
            threadActive_ = false;
        }
#else
        if (thread_.joinable()) {
            thread_.detach();
        }
#endif
    }

    [[nodiscard]] bool joinable() const {
#ifndef _WIN32
        return threadActive_;
#else
        return thread_.joinable();
#endif
    }

    void setThreadPriority([[maybe_unused]] Priority p) {
        // Platform-specific priority setting can be added if needed.
    }

    static void sleep(uint32_t millis) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }

    static void yield() { std::this_thread::yield(); }

protected:
    virtual int run() = 0;

#ifndef _WIN32
    pthread_t thread_{};
    bool threadActive_ = false;
#else
#if defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L
    std::jthread thread_;
#else
    std::thread thread_;
#endif
#endif
};

} // namespace dcpp
