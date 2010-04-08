/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(FAST_ALLOC_H)
#define FAST_ALLOC_H

#include "CriticalSection.h"

namespace dcpp {

#ifndef _DEBUG
struct FastAllocBase {
	static FastCriticalSection cs;
};

/**
 * Fast new/delete replacements for constant sized objects, that also give nice
 * reference locality...
 */
template<class T>
struct FastAlloc : public FastAllocBase {
	// Custom new & delete that (hopefully) use the node allocator
	static void* operator new(size_t s) {
		if(s != sizeof(T))
			return ::operator new(s);
		return allocate();
	}

	// Avoid hiding placement new that's needed by the stl containers...
	static void* operator new(size_t, void* m) {
		return m;
	}
	// ...and the warning about missing placement delete...
	static void operator delete(void*, void*) {
		// ? We didn't allocate so...
	}

	static void operator delete(void* m, size_t s) {
		if (s != sizeof(T)) {
			::operator delete(m);
		} else if(m != NULL) {
			deallocate((uint8_t*)m);
		}
	}
protected:
	~FastAlloc() { }

private:

	static void* allocate() {
		FastLock l(cs);
		if(freeList == NULL) {
			grow();
		}
		void* tmp = freeList;
		freeList = *((void**)freeList);
		return tmp;
	}

	static void deallocate(void* p) {
		FastLock l(cs);
		*(void**)p = freeList;
		freeList = p;
	}

	static void* freeList;

	static void grow() {
		dcassert(sizeof(T) >= sizeof(void*));
		// We want to grow by approximately 128kb at a time...
		size_t items = ((128*1024 + sizeof(T) - 1)/sizeof(T));
		freeList = new uint8_t[sizeof(T)*items];
		uint8_t* tmp = (uint8_t*)freeList;
		for(size_t i = 0; i < items - 1; i++) {
			*(void**)tmp = tmp + sizeof(T);
			tmp += sizeof(T);
		}
		*(void**)tmp = NULL;
	}
};
template<class T> void* FastAlloc<T>::freeList = NULL;
#else
template<class T> struct FastAlloc { };
#endif

} // namespace dcpp

#endif // !defined(FAST_ALLOC_H)
