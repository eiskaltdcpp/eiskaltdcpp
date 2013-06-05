/*
 *  JsonRpc-Cpp - JSON-RPC implementation.
 *  Copyright (C) 2008-2011 Sebastien Vincent <sebastien.vincent@cppextrem.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file system.cpp
 * \brief System utils.
 * \author Sebastien Vincent
 */

#include <time.h>

#include "system.h"

namespace system_util
{
  void msleep(unsigned long ms)
  {
#ifdef _WIN32
    Sleep(ms);
#else
    /* Unix */
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&req, NULL);
#endif
  }

  ThreadArg::~ThreadArg()
  {
  }

#ifndef WIN32
  
  /* POSIX specific part for thread and mutex */

  Thread::Thread(ThreadArg* arg)
  {
    m_arg = arg;
  }

  Thread::~Thread()
  {
    delete m_arg;
  }

  bool Thread::Start(bool detach)
  {
    pthread_attr_t attr;
    int ret = -1;

    /* must have valid object argument */
    if(m_arg == NULL)
    {
      return false;
    }

    /* set the detach state value */
    if(pthread_attr_init(&attr) != 0)
    {
      return false;
    }

    if(pthread_attr_setdetachstate(&attr,
          detach ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE) != 0)
    {
      pthread_attr_destroy(&attr);
      return false;
    }

    /* create thread */
    ret = pthread_create(&m_id, &attr, &Thread::Call, this);
    pthread_attr_destroy(&attr);
    return ret == 0;
  }

  bool Thread::Stop()
  {
    return pthread_cancel(m_id) == 0;
  }
  
  bool Thread::Join(void** ret)
  {
    return pthread_join(m_id, ret) == 0;
  }

  void* Thread::Call(void* arg)
  {
    Thread* thread = static_cast<Thread*>(arg);

    /* call our specific object method */
    return thread->m_arg->Call();
  }
  
  Mutex::Mutex()
  {
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutex_init(&m_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
  }
  
  Mutex::~Mutex()
  {
    pthread_mutex_destroy(&m_mutex);
  }

  bool Mutex::Lock()
  {
    return !pthread_mutex_lock(&m_mutex);
  }

  bool Mutex::Unlock()
  {
    return !pthread_mutex_unlock(&m_mutex);
  }

#else

  /* Windows specific part for thread and mutex */
  
  Thread::Thread(ThreadArg* arg)
  {
    m_arg = arg;
  }

  Thread::~Thread()
  {
    delete m_arg;
  }

  bool Thread::Start(bool detach)
  {
    detach = detach; /* unused parameter */

    m_id = CreateThread(NULL,          /* default security attributes */
                        0,             /* use default stack size */ 
                        &Thread::Call, /* thread function name */
                        this,          /* argument to thread function */
                        0,             /* use default creation flags */
                        NULL);         /* returns the thread identifier */

    return m_id != NULL;
  }

  bool Thread::Stop()
  {
    return TerminateThread(m_id, (DWORD)-1);
  }

  bool Thread::Join(void** ret)
  {
    DWORD val = 0;
    WaitForSingleObject(m_id, INFINITE);
    GetExitCodeThread(m_id, &val);
    CloseHandle(m_id);
    m_id = NULL;
    *ret = (void*)val;
    return true;
  }

  DWORD WINAPI Thread::Call(LPVOID arg)
  {
    Thread* thread = static_cast<Thread*>(arg);

    /* call our specific object method */
#ifdef _WIN64
    return (DWORD64)thread->m_arg->Call();
#else
    return (DWORD)thread->m_arg->Call();
#endif
  }

  Mutex::Mutex()
  {
    m_mutex = CreateMutex(NULL,  /* no security attribute */ 
                          0,     /* not initial owner (i.e. no first lock) */
                          NULL); /* no name */
  }

  Mutex::~Mutex()
  {
    /* free mutex */
    if(m_mutex)
    {
      CloseHandle(m_mutex);
    }
  }

  bool Mutex::Lock()
  {
    if(!m_mutex)
    {
      return false;
    }

    return (WaitForSingleObject(m_mutex, INFINITE) == WAIT_OBJECT_0);
  }

  bool Mutex::Unlock()
  {
    if(!m_mutex)
    {
      return false;
    }

    return ReleaseMutex(m_mutex); 
  }
#endif
} /* namespace system */

