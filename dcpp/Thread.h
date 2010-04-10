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

#ifndef DCPLUSPLUS_DCPP_THREAD_H
#define DCPLUSPLUS_DCPP_THREAD_H

#ifndef _WIN32
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#endif

#include "Exception.h"

namespace dcpp {

STANDARD_EXCEPTION(ThreadException);

class Thread : private boost::noncopyable
{
public:
#ifdef _WIN32
	enum Priority {
		IDLE = THREAD_PRIORITY_IDLE,
		LOW = THREAD_PRIORITY_BELOW_NORMAL,
		NORMAL = THREAD_PRIORITY_NORMAL,
		HIGH = THREAD_PRIORITY_ABOVE_NORMAL
	};

	Thread() throw() : threadHandle(INVALID_HANDLE_VALUE), threadId(0){ }
	virtual ~Thread() {
		if(threadHandle != INVALID_HANDLE_VALUE)
			CloseHandle(threadHandle);
	}

	void start() throw(ThreadException);
	void join() throw(ThreadException) {
		if(threadHandle == INVALID_HANDLE_VALUE) {
			return;
		}

		WaitForSingleObject(threadHandle, INFINITE);
		CloseHandle(threadHandle);
		threadHandle = INVALID_HANDLE_VALUE;
	}

	void setThreadPriority(Priority p) throw() { ::SetThreadPriority(threadHandle, p); }

	static void sleep(uint32_t millis) { ::Sleep(millis); }
	static void yield() { ::Sleep(1); }

#ifdef __MINGW32__
	static long safeInc(volatile long& v) { return InterlockedIncrement((long*)&v); }
	static long safeDec(volatile long& v) { return InterlockedDecrement((long*)&v); }
	static long safeExchange(volatile long& target, long value) { return InterlockedExchange((long*)&target, value); }

#else
	static long safeInc(volatile long& v) { return InterlockedIncrement(&v); }
	static long safeDec(volatile long& v) { return InterlockedDecrement(&v); }
	static long safeExchange(volatile long& target, long value) { return InterlockedExchange(&target, value); }
#endif

#else

	enum Priority {
		IDLE = 1,
		LOW = 1,
		NORMAL = 0,
		HIGH = -1
	};
	Thread() throw() : threadHandle(0) { }
	virtual ~Thread() {
		if(threadHandle != 0) {
			pthread_detach(threadHandle);
		}
	}
	void start() throw(ThreadException);
	void join() throw() {
		if (threadHandle) {
			pthread_join(threadHandle, 0);
			threadHandle = 0;
		}
	}

	void setThreadPriority(Priority p) { setpriority(PRIO_PROCESS, 0, p); }
	static void sleep(uint32_t millis) { ::usleep(millis*1000); }
	static void yield() { ::sched_yield(); }
	static long safeInc(volatile long& v) {
		pthread_mutex_lock(&mtx);
		long ret = ++v;
		pthread_mutex_unlock(&mtx);
		return ret;
	}
	static long safeDec(volatile long& v) {
		pthread_mutex_lock(&mtx);
		long ret = --v;
		pthread_mutex_unlock(&mtx);
		return ret;
	}
	static long safeExchange(volatile long& target, long value) {
		pthread_mutex_lock(&mtx);
		long ret = target;
		target = value;
		pthread_mutex_unlock(&mtx);
		return ret;
	}
        static bool safeCmp(volatile long & target, long value) {
            pthread_mutex_lock(&mtx);
            bool ret = (target == value);
            pthread_mutex_unlock(&mtx);
            return ret;
        }

#endif

protected:
	virtual int run() = 0;

#ifdef _WIN32
	HANDLE threadHandle;
	DWORD threadId;
	static DWORD WINAPI starter(void* p) {
		Thread* t = (Thread*)p;
		t->run();
		return 0;
	}
#else
	static pthread_mutex_t mtx;
	pthread_t threadHandle;
	static void* starter(void* p) {
		Thread* t = (Thread*)p;
		t->run();
		return NULL;
	}
#endif
};

} // namespace dcpp

#endif // !defined(THREAD_H)
