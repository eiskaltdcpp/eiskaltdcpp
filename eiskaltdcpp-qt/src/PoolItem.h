#ifndef POOLITEM_H

#include <boost/pool/pool_alloc.hpp>

template <class T>
class PoolItem{
public:
    PoolItem(){
    }

    virtual ~PoolItem(){
    }

    static void* operator new(size_t s) {
        return reinterpret_cast<void*>(pool.allocate());
    }

    static void* operator new(size_t, void* m) {
        return m;
    }

    static void operator delete(void*, void*) { }

    static void operator delete(void* m, size_t) {
        pool.deallocate(reinterpret_cast<T*>(m));
    }

private:
    static boost::fast_pool_allocator<T> pool;
};

template <class T>
boost::fast_pool_allocator<T> PoolItem<T>::pool = boost::fast_pool_allocator<T>();
#else
template <class T>
class PoolItem;
#endif
