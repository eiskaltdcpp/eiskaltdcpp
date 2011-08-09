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
 * \file tcp-server.cpp
 * \brief Simple JSON-RPC TCP server.
 * \author Sebastien Vincent
 */

#include <cstdio>
#include <cstdlib>
#include <csignal>

#include "../src/jsonrpc.h"

#include "test-rpc.h"

/**
 * \var g_run
 * \brief Running state of the program.
 */
static volatile bool g_run = false;

/**
 * \brief Signal management.
 * \param code signal code
 */
static void signal_handler(int code)
{
  switch(code)
  {
    case SIGINT:
    case SIGTERM:
      g_run = false;
      break;
    default:
      break;
  }
}

/**
 * \brief Entry point of the program.
 * \param argc number of argument
 * \param argv array of arguments
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char** argv)
{
  TestRpc a;
  Json::Rpc::TcpServer server(std::string("127.0.0.1"), 8086);

  /* to avoid compilation warnings */
  argc = argc;
  argv = argv;

  if(!networking::init())
  {
    std::cerr << "Networking initialization failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  if(signal(SIGTERM, signal_handler) == SIG_ERR)
  {
    std::cout << "Error signal SIGTERM will not be handled" << std::endl;
  }

  if(signal(SIGINT, signal_handler) == SIG_ERR)
  {
    std::cout << "Error signal SIGINT will not be handled" << std::endl;
  }

  server.AddMethod(new Json::Rpc::RpcMethod<TestRpc>(a, &TestRpc::Print, std::string("print")));
  server.AddMethod(new Json::Rpc::RpcMethod<TestRpc>(a, &TestRpc::Notify, std::string("notify")));
  
  /* server.SetEncapsulatedFormat(Json::Rpc::NETSTRING); */

  if(!server.Bind())
  {
    std::cout << "Bind failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  if(!server.Listen())
  {
    std::cout << "Listen failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  g_run = true;

  std::cout << "Start JSON-RPC TCP server" << std::endl;

  while(g_run)
  {
    server.WaitMessage(1000);
  }

  std::cout << "Stop JSON-RPC TCP server" << std::endl;
  server.Close();
  networking::cleanup();

  return EXIT_SUCCESS;
}

