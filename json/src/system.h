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
 * \file system.h
 * \brief System utils.
 * \author Sebastien Vincent
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef _WIN32

#include <windows.h>

#else

#include <pthread.h>

#endif

/**
 * \namespace System
 * \brief System related class (thread, ...).
 */
namespace System
{

  /**
   * \brief Sleep for x milliseconds
   * \param ms millisecond to sleep
   */
  void msleep(unsigned long ms);

  /**
   * \class ThreadArg
   * \brief Abstract class to represent thread argument.
   * \see ThreadArgImpl
   * \see Thread
   */
  class ThreadArg 
  {
    public:
      /**
       * \brief Destructor.
       */
      virtual ~ThreadArg();

      /**
       * \brief Call the method.
       * \note Have to be implemented by subclasses
       */
      virtual void* Call() = 0;
  };

  /**
   * \class ThreadArgImpl
   * \brief Template class that represent thread argument.
   *
   * This class is used to provide callback function within 
   * an object. The method which will be called during thread
   * execution must be of the form <code>void* MyMethod(void* arg)</code>. 
   * Inside this method you are free to called any method of the object.
   *
   * \warning As class keep pointer of object reference, you should take 
   * care at the lifetime of object you pass in ThreadArgImpl constructor,
   * else it could lead to crash your program.\n See Thread class documentation
   * for an example of how to use ThreadArgImpl class.
   * \see Thread
   */
  template<class T> class ThreadArgImpl : public ThreadArg
  {
    public:
      /**
       * \typedef Method
       * \brief T method signature.
       */
      typedef void* (T::*Method)(void*);

      /**
       * \brief Constructor.
       * \param obj object
       * \param method class method
       * \param arg argument to method
       */
      ThreadArgImpl(T& obj, Method method, void* arg)
      {
        m_obj = &obj;
        m_method = method;
        m_arg = arg;
      }

      /**
       * \brief Call the method.
       */
      virtual void* Call()
      {
        return (m_obj->*m_method)(m_arg);
      }

    private:
      /**
       * \brief Object pointer.
       */
      T* m_obj;

      /**
       * \brief Method of T class.
       */
      Method m_method;

      /**
       * \brief Argument of method.
       */
      void* m_arg;
  };

  /**
   * \class Thread
   * \brief Thread implementation.
   *
   * Preferred use of this class is to construct ThreadArgImpl inside
   * another class and pass <code>*this</code> as obj parameter:\n
   * \n
   * \code
   * class MyClass
   * {
   *    public:
   *      void MyMethod()
   *      {
   *        ThreadArg* arg = new ThreadArgImpl<MyClass>(*this, &MyClass::MethodForThread, NULL);
   *        Thread th(arg);
   *        th.Start();
   *      }
   *
   *      void* MethodForThread(void * arg)
   *      {
   *        // do stuff 
   *      }
   * };
   * \endcode
   *
   */
  class Thread
  {
    public:
      /**
       * \brief Constructor.
       * \param arg thread argument (MUST be dynamically allocated using new)
       * \note System::Thread object takes care of freeing method memory.\n
       * The way of calling constructor is:
       * <code>
       * Thread thread(new ThreadArgImpl<MyClass>(instanceOfMyClass, &MyClass::Method));
       * </code>
       * \warning You should take care of the object (instanceOfMyClass) lifetime pass 
       * into ThreadArgImpl constructor, else it could lead to a crash because ThreadArgImpl
       * keep pointer of the reference.
       * \warning The "arg" parameter MUST be dynamically allocated (using new).
       * \see ThreadArgImpl
       */
      Thread(ThreadArg* arg);

      /**
       * \brief Destructor.
       */
      virtual ~Thread();

      /**
       * \brief Start thread.
       * \param detach if set to true, the thread will be in detach state so 
       * you do not have to call join on this type of thread.
       * \return true if success, false otherwise
       * \warning Do NOT <code>Join</code> a detached thread.
       */
      bool Start(bool detach);

      /**
       * \brief Stop thread.
       * \return true if success, false otherwise
       * \warning Calling this method could lead callback object to an 
       * incoherent state. You should call it really in desperate situations when
       * you really want to stop thread and do not care about the rest.
       * \warning With POSIX thread implementation, calling Stop (one or more times)
       * will leak 28 bytes of memory.
       */
      bool Stop();

      /**
       * \brief Join thread.
       * \param ret pointer to return code of the joined thread
       * \return true if success, false otherwise
       * \warning Do NOT <code>Join</code> a detached thread.
       */
      bool Join(void** ret = NULL);

    private:
      /**
       * \brief Entry point of thread before calling specific 
       * callback.
       * \param arg thread argument
       * \return result of ThreadArg callback
       */
#ifdef _WIN32
      static DWORD WINAPI Call(LPVOID arg);
#else
      static void* Call(void* arg);
#endif
      /**
       * \brief Thread identifier.
       */
#ifdef _WIN32 /* Win32 thread */
      HANDLE m_id;
#else /* POSIX thread */
      pthread_t m_id;
#endif

      /**
       * \brief Thread argument.
       */
      ThreadArg* m_arg;
  };

  /**
   * \class Mutex
   * \brief Mutex implementation.
   */
  class Mutex
  {
    public:
      /**
       * \brief Constructor.
       */
      Mutex();

      /**
       * \brief Destructor.
       */
      ~Mutex();

      /**
       * \brief Lock the mutex.
       * \return true if mutex is locked, false if error
       */
      bool Lock();
      
      /**
       * \brief Unlock the mutex.
       * \return true if mutex is unlocked, false if error
       */
      bool Unlock();

    private:
      /**
       * \brief The mutex.
       */
#ifdef _WIN32
      HANDLE m_mutex;
#else
      pthread_mutex_t m_mutex;
#endif
  };

} /* namespace System */

#endif /* SYSTEM_H */

