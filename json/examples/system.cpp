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
 * \brief Example of how to use System classes.
 * \author Sebastien Vincent
 */

#include <cstdlib>

#include <iostream>

#include "../src/system.h"

using namespace System;

/**
 * \class MyClass
 * \brief Example class.
 */
class MyClass
{
  public:
    /**
     * \brief Method to create thread.
     */
    void MyMethod()
    {
      void* ret = NULL;
      ThreadArg* arg = new ThreadArgImpl<MyClass>(*this, &MyClass::MethodForThread, NULL);
      Thread th(arg);

      th.Start(false);
      th.Join(&ret);

      std::cout << ret << std::endl;
    }

    /**
     * \brief Method executed in thread.
     * \param arg argument
     * \return return 0xABCD
     */
    void* MethodForThread(void* arg)
    {
      size_t i = 0;
      
      arg = arg; /* not used */

      for(i = 0 ; i < 5 ; i++)
      {
        std::cout << "Print" << std::endl;
      }

      return (void*)0xABCD;
    }
};

/**
 * \brief Entry point of the program.
 * \param argc number of argument
 * \param argv array of arguments
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char** argv)
{
  MyClass myObj;

  argc = argc; /* not used */
  argv = argv; /* not used */

  myObj.MyMethod();

  return EXIT_SUCCESS;
}

