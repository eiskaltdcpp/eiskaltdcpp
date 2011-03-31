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

#if !defined(SEMAPHORE_H)
#define SEMAPHORE_H

#ifndef _WIN32
#include <errno.h>
#include <semaphore.h>
#include <sys/time.h>
#endif

namespace dcpp {

class Semaphore
{
#ifdef _WIN32
public:
    Semaphore() throw() {
        h = CreateSemaphore(NULL, 0, MAXLONG, NULL);
    }

    void signal() throw() {
        ReleaseSemaphore(h, 1, NULL);
    }

    bool wait() throw() { return WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0; }
    bool wait(uint32_t millis) throw() { return WaitForSingleObject(h, millis) == WAIT_OBJECT_0; }

    ~Semaphore() throw() {
        CloseHandle(h);
    }

private:
    HANDLE h;
#else
public:
    Semaphore() throw() {
        sem_init(&semaphore, 0, 0);
    }

    ~Semaphore() throw() {
        sem_destroy(&semaphore);
    }

    void signal() throw() {
        sem_post(&semaphore);
    }

    bool wait() throw() {
        int retval = 0;
        do {
            retval = sem_wait(&semaphore);
        } while (retval != 0);

        return true;
    }
    bool wait(uint32_t millis) throw() {
        timeval timev;
        timespec t;
        gettimeofday(&timev, NULL);
        millis+=timev.tv_usec/1000;
        t.tv_sec = timev.tv_sec + (millis/1000);
        t.tv_nsec = (millis%1000)*1000*1000;
        int ret;
        do {
            ret = sem_timedwait(&semaphore, &t);
        } while (ret != 0 && errno == EINTR);

        if (ret != 0) {
            return false;
        }

        return true;
    }

private:
    sem_t semaphore;;
#endif
    Semaphore(const Semaphore&);
    Semaphore& operator=(const Semaphore&);

};

} // namespace dcpp

#endif // !defined(SEMAPHORE_H)
