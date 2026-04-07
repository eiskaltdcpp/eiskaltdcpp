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

#include "stdinc.h"
#include "Thread.h"

#include "format.h"

#include <cstdio>
#include <exception>

namespace dcpp {

/// Stack size for background threads (POSIX only).
/// The default (often 8 MiB) causes ~500 MiB virtual-memory usage
/// on busy NMDC hubs where each peer connection spawns a
/// BufferedSocket thread.  1 MiB is more than enough for socket I/O.
static constexpr size_t THREAD_STACK_SIZE = 1024 * 1024;  // 1 MiB

void Thread::start() {
    join();  // join any previous thread first

#ifndef _WIN32
    // Use pthread_create directly so we can set a smaller stack size.
    struct Ctx {
        Thread* self;
    };
    auto* ctx = new Ctx{this};
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
    int err = pthread_create(&thread_, &attr, [](void* arg) -> void* {
        auto* c = static_cast<Ctx*>(arg);
        Thread* self = c->self;
        delete c;
        try {
            self->run();
        } catch(const std::exception& e) {
            fprintf(stderr, "[Thread] uncaught std::exception in run(): %s\n", e.what());
        } catch(...) {
            fprintf(stderr, "[Thread] uncaught unknown exception in run()\n");
        }
        return nullptr;
    }, ctx);
    pthread_attr_destroy(&attr);
    if (err) {
        delete ctx;
        throw ThreadException("pthread_create failed");
    }
    threadActive_ = true;
#else
#if defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L
    thread_ = std::jthread([this] {
#else
    thread_ = std::thread([this] {
#endif
        try {
            run();
        } catch(const std::exception& e) {
            fprintf(stderr, "[Thread] uncaught std::exception in run(): %s\n", e.what());
        } catch(...) {
            fprintf(stderr, "[Thread] uncaught unknown exception in run()\n");
        }
    });
#endif
}

} // namespace dcpp
