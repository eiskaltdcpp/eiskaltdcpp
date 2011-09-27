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

#ifndef DCPLUSPLUS_DCPP_CRITICAL_SECTION_H
#define DCPLUSPLUS_DCPP_CRITICAL_SECTION_H

#include "debug.h"
#include "noexcept.h"

#ifdef FIX_FOR_OLD_BOOST
    #include "Thread.h"
#else
    // header-only implementation of mutex
    #include <boost/signals2/mutex.hpp>
#endif

namespace dcpp {

class CriticalSection
{
#ifdef _WIN32
public:
    void lock() noexcept {
        EnterCriticalSection(&cs);
        dcdrun(counter++);
    }
    void unlock() noexcept {
        dcassert(--counter >= 0);
        LeaveCriticalSection(&cs);
    }
    CriticalSection() noexcept {
        dcdrun(counter = 0;);
        InitializeCriticalSection(&cs);
    }
    ~CriticalSection() noexcept {
        dcassert(counter==0);
        DeleteCriticalSection(&cs);
    }
private:
    dcdrun(long counter);
    CRITICAL_SECTION cs;
#else
public:
    CriticalSection() noexcept {
        pthread_mutexattr_init(&ma);
        pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mtx, &ma);
    }
    ~CriticalSection() noexcept {
        pthread_mutex_destroy(&mtx);
        pthread_mutexattr_destroy(&ma);
    }
    void lock() noexcept { pthread_mutex_lock(&mtx); }
    void unlock() noexcept { pthread_mutex_unlock(&mtx); }
    pthread_mutex_t& getMutex() { return mtx; }
private:
    pthread_mutex_t mtx;
    pthread_mutexattr_t ma;
#endif
    CriticalSection(const CriticalSection&);
    CriticalSection& operator=(const CriticalSection&);
};

/**
 * A fast, non-recursive and unfair implementation of the Critical Section.
 * It is meant to be used in situations where the risk for lock conflict is very low,
 * i e locks that are held for a very short time. The lock is _not_ recursive, i e if
 * the same thread will try to grab the lock it'll hang in a never-ending loop. The lock
 * is not fair, i e the first to try to lock a locked lock is not guaranteed to be the
 * first to get it when it's freed...
 */
class FastCriticalSection {
public:
#ifdef FIX_FOR_OLD_BOOST
    // We have to use a pthread (nonrecursive) mutex, didn't find any test_and_set on linux...
    FastCriticalSection() {
        static pthread_mutex_t fastmtx = PTHREAD_MUTEX_INITIALIZER;
        mtx = fastmtx;
    }
    ~FastCriticalSection() { pthread_mutex_destroy(&mtx); }
    void lock() { pthread_mutex_lock(&mtx); }
    void unlock() { pthread_mutex_unlock(&mtx); }
private:
    pthread_mutex_t mtx;
#else
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
private:
    typedef boost::signals2::mutex mutex_t;
    mutex_t mtx;
#endif
};

template<class T>
class LockBase {
public:
    LockBase(T& aCs) noexcept : cs(aCs) { cs.lock(); }
    ~LockBase() noexcept { cs.unlock(); }
private:
    LockBase& operator=(const LockBase&);
    T& cs;
};
typedef LockBase<CriticalSection> Lock;
typedef LockBase<FastCriticalSection> FastLock;

} // namespace dcpp

#endif // DCPLUSPLUS_DCPP_CRITICAL_SECTION_H
