#if !defined(DCPP_ATOMIC_H)
#define DCPP_ATOMIC_H

#include <boost/interprocess/detail/atomic.hpp>
#include <boost/cstdint.hpp>


/*
 * Atomic
 *
 * Suitable only for thread-safe accounting of some statistics.
 * Not suitable for any synchronization between threads,
 * since does not garantees necessary memory barriers.
 *
 */

template <typename T>
class Atomic;

// uint32_t
template <>
class Atomic<boost::uint32_t> {
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
// just forward all operations to underlying uint32_t variable
template <>
class Atomic<boost::int32_t> {
	typedef boost::int32_t value_type;
public:
	Atomic(value_type val) : m_value(val) {}
	Atomic(const Atomic& other) : m_value(other) {}

	void operator=(value_type val)		{ m_value=val; }
	void operator=(const Atomic& other)	{ m_value=other; }
	operator value_type() const		{ return static_cast<value_type>(m_value); }

	void inc() { m_value.inc(); }
	void dec() { m_value.dec(); }
private:
	Atomic<boost::uint32_t> m_value;
};

#endif // !defined(DCPP_ATOMIC_H)
