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

#if defined (_WIN32) || defined (__HAIKU__)
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
#else
#include <boost/thread/lock_guard.hpp>
#endif

#else
#include <mutex>
#endif

namespace dcpp {

#if defined (_WIN32) || defined (__HAIKU__)
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
