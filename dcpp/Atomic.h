/*
* Copyright (C) 2010 Gennady Proskurin (https://launchpad.net/~gpr)
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

#if !defined(DCPP_ATOMIC_H)
#define DCPP_ATOMIC_H
#include "CriticalSection.h"
#include <boost/interprocess/detail/atomic.hpp>
#include <boost/cstdint.hpp>

namespace dcpp {

// Ordering arguments:

// memory_ordering_weak
// Suitable only for thread-safe accounting of some statistics.
// Value can not be used as "flag" (you cannot do any multi-thread action, based
// on this value) since it does not garantees necessary memory barriers.
class memory_ordering_weak {};

// memory_ordering_strong
// Suitable for any multi-thread purpose
class memory_ordering_strong {};

template <typename DataType, class Ordering = memory_ordering_strong>
class Atomic;


// uint32_t
template <>
class Atomic<boost::uint32_t, memory_ordering_weak> {
        typedef boost::uint32_t value_type;
public:
        Atomic(value_type val) { assign(val); }
        Atomic(const Atomic& other) { assign(static_cast<value_type>(other)); }

        // operator=
        // return void to be safe
        void operator=(value_type val) { assign(val); }
        void operator=(const Atomic& other) {
                return operator=(static_cast<value_type>(other));
        }

        // type cast
        operator value_type() const {
                return boost::interprocess::detail::atomic_read32(&m_value);
        }

        // increment
        void inc() { boost::interprocess::detail::atomic_inc32(&m_value); }

        // decrement
        void dec() { boost::interprocess::detail::atomic_dec32(&m_value); }

private:
        mutable value_type m_value;
        void assign(value_type val) { boost::interprocess::detail::atomic_write32(&m_value, val); }
};

// int32_t
// just forward all operations to underlying Atomic<uint32_t, ...> variable
template <>
class Atomic<boost::int32_t, memory_ordering_weak> {
        typedef boost::int32_t value_type;
public:
        Atomic(value_type val) : m_value(val) {}
        Atomic(const Atomic& other) : m_value(other) {}

        void operator=(value_type val)          { m_value=val; }
        void operator=(const Atomic& other)     { m_value=other; }
        operator value_type() const             { return static_cast<value_type>(m_value); }

        void inc() { m_value.inc(); }
        void dec() { m_value.dec(); }
private:
        Atomic<boost::uint32_t,memory_ordering_weak> m_value;
};

// memory_ordering_strong
template <typename DataType>
class Atomic<DataType, memory_ordering_strong> {
        typedef DataType value_type;
public:
        Atomic(value_type new_value) : m_value(new_value) {}
        Atomic(const Atomic& other) : m_value(static_cast<value_type>(other)) {}

        void operator=(value_type new_value) {
                FastLock Lock(cs);
                m_value = new_value;
        }
        void operator=(const Atomic& other) {
                FastLock Lock(cs);
                m_value = other;
        }
        operator value_type() const {
                FastLock Lock(cs); // shared (read-only) lock would be sufficient here
                return m_value;
        }

        void inc() {
                FastLock Lock(cs);
                ++m_value;
        }
        void dec() {
                FastLock Lock(cs);
                --m_value;
        }

        // assign new value, return old value
        value_type exchange(value_type new_val) {
                FastLock Lock(cs);
                value_type old_val = m_value;
                m_value = new_val;
                return old_val;
        }
private:
        value_type m_value;
        mutable FastCriticalSection cs;
};

} // namespace dcpp
#endif // !defined(DCPP_ATOMIC_H)
