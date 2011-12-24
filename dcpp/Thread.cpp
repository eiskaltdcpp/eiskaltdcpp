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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"
#include "Thread.h"

#if defined(_DEBUG) && defined(__linux__)
#include "sys/prctl.h"
#endif

#include "format.h"

namespace dcpp {

#ifdef _WIN32
void Thread::start() {
    join();
    if( (threadHandle = CreateThread(NULL, 0, &starter, this, 0, &threadId)) == NULL) {
        throw ThreadException(_("Unable to create thread"));
    }
}

#else
void Thread::start() {
    join();
    if(pthread_create(&threadHandle, NULL, &starter, this) != 0) {
        throw ThreadException(_("Unable to create thread"));
    }
}
#endif

void Thread::setThreadName(const char* const threadName) const {
#ifdef _DEBUG

#if defined(__HAIKU__)
    // TODO, see http://haiku-os.org/legacy-docs/bebook/TheKernelKit_ThreadsAndTeams.html#rename_thread
#elif defined(APPLE) && (DARWIN_MAJOR_VERSION >= 10)
    // pthread_setname_np is supported starting Mac OS X 10.6

    // Mac OS X allegedly truncates thread names to 63 chars
    // pthread_setname_np(threadName);
#elif defined(_WIN32)
    // TODO, see http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
#elif defined(__linux__)
    /* I'm using the old prctl way here because the only new feature of pthread_setname_np
     * is that it can be called from outside the thread (not necessary and less portable)
     * and because it would require at least Linux 2.6.33 and glibc 2.12.
     *
     * Linux truncates thread names to TASK_COMM_LEN chars (default: 16)
     */
    prctl(PR_SET_NAME, threadName);
#elif defined(BSD)
    // TODO, see http://www.unix.com/man-page/All/3/PTHREAD_SET_NAME_NP/
#endif
    
#endif // _DEBUG
} // setThreadName()

} // namespace dcpp
