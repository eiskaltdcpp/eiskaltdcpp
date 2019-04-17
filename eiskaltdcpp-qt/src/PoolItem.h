/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <memory>
#include <assert.h>

template <class T>
class PoolItem{
public:
    PoolItem(){
    }

    virtual ~PoolItem(){
    }

    static void* operator new(size_t s) {
        assert(sizeof(T) == s);
        (void)s;

        return reinterpret_cast<void*>(pool.allocate(1));
    }

    static void* operator new(size_t, void* m) {
        return m;
    }

    static void operator delete(void*, void*) { }

    static void operator delete(void* m, size_t s) {
        assert(sizeof(T) == s);
        (void)s;

        pool.deallocate(reinterpret_cast<T*>(m), 1);
    }

private:
    static std::allocator<T> pool;
};

template <class T>
std::allocator<T> PoolItem<T>::pool = std::allocator<T>();
