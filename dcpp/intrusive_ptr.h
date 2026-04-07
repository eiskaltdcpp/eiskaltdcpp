/*
 * Copyright (C) 2020 Boris Pek <tehnick-8@yandex.ru>
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

#include <cstddef>

namespace dcpp
{

template<typename T>
class intrusive_ptr
{
    typedef intrusive_ptr value_type;

public:
    constexpr intrusive_ptr() : ptr_(nullptr) { }

    intrusive_ptr(T* val, bool add_ref = true): ptr_(val) {
        if(ptr_ && add_ref)
            intrusive_ptr_add_ref(ptr_);
    }

    intrusive_ptr(const intrusive_ptr& other): ptr_(other.ptr_) {
        if(ptr_)
            intrusive_ptr_add_ref(ptr_);
    }

    ~intrusive_ptr() {
        if(ptr_)
            intrusive_ptr_release(ptr_);
    }

    intrusive_ptr(intrusive_ptr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    intrusive_ptr& operator=(intrusive_ptr&& other) noexcept {
        value_type(static_cast<intrusive_ptr&&>(other)).swap(*this);
        return *this;
    }

    intrusive_ptr& operator=(const intrusive_ptr& other) {
        value_type(other).swap(*this);
        return *this;
    }

    intrusive_ptr& operator=(T* val) {
        value_type(val).swap(*this);
        return *this;
    }

    void reset() {
        value_type().swap(*this);
    }

    void reset(T* val) {
        value_type(val).swap(*this);
    }

    T* get() const {
        return ptr_;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    explicit operator bool() const {
        return (ptr_ != 0);
    }

    void swap(intrusive_ptr& other) noexcept {
        T* tmp = ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = tmp;
    }

private:
    T* ptr_;
};

template<typename T, typename U>
bool operator==(const intrusive_ptr<T>& t, const intrusive_ptr<U>& u) {
    return (t.get() == u.get());
}

template<typename T, typename U>
bool operator==(const intrusive_ptr<T>& t, U* u) {
    return (t.get() == u);
}

template<typename T, typename U>
bool operator==(T* t, const intrusive_ptr<U>& u) {
    return (t == u.get());
}

template<typename T>
bool operator==(const intrusive_ptr<T>& p, std::nullptr_t) {
    return (p.get() == 0);
}

template<typename T>
bool operator==(std::nullptr_t, const intrusive_ptr<T>& p) {
    return (p.get() == 0);
}

template<typename T, typename U>
bool operator!=(const intrusive_ptr<T>& t, const intrusive_ptr<U>& u) {
    return (t.get() != u.get());
}

template<typename T, typename U>
bool operator!=(const intrusive_ptr<T>& t, U* u) {
    return (t.get() != u);
}

template<typename T, typename U>
bool operator!=(T* t, const intrusive_ptr<U>& u) {
    return (t != u.get());
}

template<typename T>
bool operator!=(const intrusive_ptr<T>& p, std::nullptr_t) {
    return (p.get() != 0);
}

template<typename T>
bool operator!=(std::nullptr_t, const intrusive_ptr<T>& p) {
    return (p.get() != 0);
}

template<typename T>
bool operator<(const intrusive_ptr<T>& t, const intrusive_ptr<T>& u) {
    return std::less<T*>()(t.get(), u.get());
}

template<typename T>
std::size_t hash_value(const intrusive_ptr<T>& p) {
    return std::hash<T*>()(p.get());
}

} // namespace dcpp

