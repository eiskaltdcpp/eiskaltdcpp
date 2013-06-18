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

#pragma once

#include "debug.h"
#include "noexcept.h"

#ifndef DO_NOT_USE_MUTEX

#if defined (_WIN32)
#include <boost/thread/recursive_mutex.hpp>

#ifdef FIX_FOR_OLD_BOOST
template <typename Mutex>
class lock_guard
{
private:
    Mutex& m;

public:
    explicit lock_guard(Mutex& m_) : m(m_)
    {
        m.lock();
    }
    lock_guard(Mutex& m_, boost::adopt_lock_t) : m(m_)
    {
        m.lock();
    }
    ~lock_guard()
    {
        m.unlock();
    }
};
#else // FIX_FOR_OLD_BOOST
#include <boost/thread/lock_guard.hpp>
#endif // FIX_FOR_OLD_BOOST

#else
#include <mutex>
#endif

namespace dcpp {

#if defined (_WIN32)
typedef boost::recursive_mutex CriticalSection;
typedef boost::detail::spinlock FastCriticalSection;
typedef boost::unique_lock<boost::recursive_mutex> Lock;
typedef boost::lock_guard<boost::detail::spinlock> FastLock;
#else
typedef std::recursive_mutex CriticalSection;
typedef std::mutex FastCriticalSection;
typedef std::unique_lock<std::recursive_mutex> Lock;
typedef std::lock_guard<std::mutex> FastLock;
#endif

} // namespace dcpp

#else // DO_NOT_USE_MUTEX

#include <boost/signals2/mutex.hpp>

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
#else // _WIN32
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
#endif // _WIN32
    CriticalSection(const CriticalSection&);
    CriticalSection& operator=(const CriticalSection&);
};

// A fast, non-recursive and unfair implementation of the Critical Section.
// It is meant to be used in situations where the risk for lock conflict is very low,
// i.e. locks that are held for a very short time. The lock is _not_ recursive, i e if
// the same thread will try to grab the lock it'll hang in a never-ending loop. The lock
// is not fair, i e the first to try to lock a locked lock is not guaranteed to be the
// first to get it when it's freed...

class FastCriticalSection {
public:
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
private:
    typedef boost::signals2::mutex mutex_t;
    mutex_t mtx;
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

}

#endif // DO_NOT_USE_MUTEX

